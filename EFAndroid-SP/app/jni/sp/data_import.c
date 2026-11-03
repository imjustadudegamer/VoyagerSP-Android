/*
===========================================================================
Voyager Elite Force SP — desktop retail-data importer (engine side).

Android parity authority: EFAndroid-SP/app/src/main/java/com/voyager/ef/
ImportActivity.java.  On launch the app requires pak0.pk3 .. pak3.pk3 plus a
genuine retail efgamex86.dll, verifies them, copies them into its own
baseEF/ and only then starts the engine.  This module runs the same flow
inside the Linux binary, before SDL/Vulkan initialisation.

Design notes:
  * The data root is ${VOYAGERSP_DATA_ROOT}, else
    ${XDG_DATA_HOME:-$HOME/.local/share}/voyagersp.  The engine uses it for
    BOTH fs_basepath and fs_homepath (see sys_main.c / sys_unix.c), so the
    launcher never has to pass +set.
  * A complete retail folder is installed as a REAL <root>/baseEF directory
    holding one symlink per retail file (pak0..3.pk3 + efgamex86.dll): the
    engine reads the paks through those links, while efspconfig.cfg, saves/,
    qconsole.log, demos and screenshots are written into the user's data
    root and never into the retail install.  The retail folder is called
    "BaseEF" and the engine BASEGAME is "baseEF": the case bridge happens
    here, inside the engine, and never renames anything on disk.  The old
    folder-symlink form (<root>/baseEF -> <retail>/BaseEF) is only a
    fallback for filesystems that cannot create symlinks, and existing
    installations that still use it are migrated automatically on launch.
  * Individual files are stream-copied into <root>/baseEF/ (write to .tmp,
    then rename, so a half copy never looks like valid data).
  * Nothing is downloaded and no retail content is shipped.
  * Host pickers are invoked with fork/exec (zenity, kdialog): the binary
    gains no GTK/Qt/Python link dependency.
===========================================================================
*/

#include "data_import.h"

#ifdef __ANDROID__

/* Android has its own ImportActivity; the desktop importer is inert here. */
int DataImport_PreInit(int *argc, char ***argv, dataImportResult_t *out)
{
	(void)argc;
	(void)argv;
	if (out) {
		out->dataRoot[0] = '\0';
		out->fullscreen = -1;
	}
	return -1;
}

#else /* !__ANDROID__ */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define DI_NUM_PAKS   4
#define DI_NUM_FILES  5
#define DI_MAX_CAND   64
#define DI_COPY_CHUNK (1024 * 1024)

static const char *const di_paks[DI_NUM_PAKS] = {
	"pak0.pk3", "pak1.pk3", "pak2.pk3", "pak3.pk3"
};
static const char *const di_dll_name = "efgamex86.dll";

static const char *const di_required[DI_NUM_FILES] = {
	"pak0.pk3", "pak1.pk3", "pak2.pk3", "pak3.pk3", "efgamex86.dll"
};

/* --------------------------------------------------------------------- */
/* small helpers                                                          */
/* --------------------------------------------------------------------- */

static void di_copy(char *dst, size_t dstsz, const char *src)
{
	if (!dstsz)
		return;
	if (!src)
		src = "";
	snprintf(dst, dstsz, "%s", src);
}

static int di_join(char *out, size_t outsz, const char *dir, const char *name)
{
	int n;
	if (!dir || !*dir)
		n = snprintf(out, outsz, "%s", name);
	else if (dir[strlen(dir) - 1] == '/')
		n = snprintf(out, outsz, "%s%s", dir, name);
	else
		n = snprintf(out, outsz, "%s/%s", dir, name);
	return (n > 0 && (size_t)n < outsz) ? 0 : -1;
}

static const char *di_basename(const char *path)
{
	const char *s = strrchr(path, '/');
	return s ? s + 1 : path;
}

/* path = base + suffix + pid (used for temporary/backup names). */
static int di_pid_path(char *out, size_t outsz, const char *base, const char *suffix)
{
	size_t bl = strlen(base);
	size_t sl = strlen(suffix);

	if (bl + sl + 16 >= outsz)
		return -1;
	memcpy(out, base, bl);
	memcpy(out + bl, suffix, sl);
	snprintf(out + bl + sl, outsz - bl - sl, "%d", (int)getpid());
	return 0;
}

static int di_is_dir(const char *path)
{
	struct stat st;
	return (path && stat(path, &st) == 0 && S_ISDIR(st.st_mode)) ? 1 : 0;
}

static int di_is_file(const char *path)
{
	struct stat st;
	return (path && stat(path, &st) == 0 && S_ISREG(st.st_mode)) ? 1 : 0;
}

static long long di_file_size(const char *path)
{
	struct stat st;
	if (!path || stat(path, &st) != 0 || !S_ISREG(st.st_mode))
		return -1;
	return (long long)st.st_size;
}

static int di_mkdir_p(const char *path)
{
	char tmp[DI_MAXPATH];
	size_t len, i;

	di_copy(tmp, sizeof(tmp), path);
	len = strlen(tmp);
	while (len > 1 && tmp[len - 1] == '/')
		tmp[--len] = '\0';

	for (i = 1; i <= len; i++) {
		if (tmp[i] == '/' || tmp[i] == '\0') {
			char saved = tmp[i];
			tmp[i] = '\0';
			if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
				tmp[i] = saved;
				return -1;
			}
			tmp[i] = saved;
		}
	}
	return di_is_dir(path) ? 0 : -1;
}

/* Make `path` absolute and normalise a trailing slash.  `~` is expanded:
   the interactive prompt is a plain typed path, so users will type it. */
static int di_normalize(char *path, size_t pathsz)
{
	char tmp[DI_MAXPATH];
	char cwd[DI_MAXPATH];

	if (!path || !*path)
		return -1;

	if (path[0] == '~' && (path[1] == '/' || path[1] == '\0')) {
		const char *home = getenv("HOME");
		size_t hl, pl;
		if (!home || !*home)
			return -1;
		hl = strlen(home);
		pl = strlen(path + 1);
		if (hl + pl >= sizeof(tmp))
			return -1;
		memcpy(tmp, home, hl);
		memcpy(tmp + hl, path + 1, pl + 1);
		di_copy(path, pathsz, tmp);
	}

	if (path[0] != '/') {
		size_t cl, pl;
		if (!getcwd(cwd, sizeof(cwd)))
			return -1;
		cl = strlen(cwd);
		pl = strlen(path);
		if (cl + 1 + pl >= sizeof(tmp))
			return -1;
		memcpy(tmp, cwd, cl);
		tmp[cl] = '/';
		memcpy(tmp + cl + 1, path, pl + 1);
		di_copy(path, pathsz, tmp);
	}

	{
		size_t len = strlen(path);
		while (len > 1 && path[len - 1] == '/')
			path[--len] = '\0';
	}
	return 0;
}

static void di_trim(char *s)
{
	char *p = s;
	size_t len;

	while (*p && isspace((unsigned char)*p))
		p++;
	if (p != s)
		memmove(s, p, strlen(p) + 1);

	len = strlen(s);
	while (len > 0 && isspace((unsigned char)s[len - 1]))
		s[--len] = '\0';

	/* strip a pair of surrounding quotes */
	if (len >= 2 && ((s[0] == '"' && s[len - 1] == '"') ||
	                 (s[0] == '\'' && s[len - 1] == '\''))) {
		memmove(s, s + 1, len - 2);
		s[len - 2] = '\0';
	}
}

static void di_append_name(char *list, size_t listsz, const char *name)
{
	if (list[0])
		strncat(list, ", ", listsz - strlen(list) - 1);
	strncat(list, name, listsz - strlen(list) - 1);
}

/* pak present and non-trivial: real pk3 files are at least hundreds of KB,
   an empty/truncated placeholder must not pass. */
static int di_pak_ok(const char *path)
{
	return di_file_size(path) >= 1024;
}

/* efgamex86.dll: a plausible DOS/PE binary, exactly like the Android check
   (>= 4096 bytes and the "MZ" header).  It is never loaded. */
static int di_dll_ok(const char *path)
{
	FILE *f;
	int m, z;
	long long sz = di_file_size(path);

	if (sz < 4096)
		return 0;
	f = fopen(path, "rb");
	if (!f)
		return 0;
	m = fgetc(f);
	z = fgetc(f);
	fclose(f);
	return (m == 'M' && z == 'Z') ? 1 : 0;
}

static int di_is_required_name(const char *name)
{
	int i;
	for (i = 0; i < DI_NUM_FILES; i++)
		if (!strcmp(name, di_required[i]))
			return 1;
	return 0;
}

/* --------------------------------------------------------------------- */
/* data-root layout                                                       */
/* --------------------------------------------------------------------- */

/* The folder that holds the paks: <dir>/BaseEF, <dir>/baseEF, or <dir>. */
static int di_retail_base(const char *dir, char *out, size_t outsz)
{
	char p[DI_MAXPATH];

	if (di_join(p, sizeof(p), dir, "BaseEF") == 0 && di_is_dir(p)) {
		di_copy(out, outsz, p);
		return 1;
	}
	if (di_join(p, sizeof(p), dir, "baseEF") == 0 && di_is_dir(p)) {
		di_copy(out, outsz, p);
		return 1;
	}
	if (di_is_file(dir) || !di_is_dir(dir))
		return 0;
	if (di_join(p, sizeof(p), dir, "pak0.pk3") == 0 && di_is_file(p)) {
		di_copy(out, outsz, dir);
		return 1;
	}
	return 0;
}

static int di_parent_dir(const char *path, char *out, size_t outsz)
{
	char tmp[DI_MAXPATH];
	char *slash;
	size_t len;

	di_copy(tmp, sizeof(tmp), path);
	len = strlen(tmp);
	while (len > 1 && tmp[len - 1] == '/')
		tmp[--len] = '\0';
	slash = strrchr(tmp, '/');
	if (!slash || slash == tmp) {
		di_copy(out, outsz, "/");
		return 0;
	}
	*slash = '\0';
	di_copy(out, outsz, tmp);
	return 0;
}

static int di_find_dll(const char *dir, const char *base, char *out, size_t outsz)
{
	static const char *const names[] = { "", "BaseEF", "baseEF" };
	char p[DI_MAXPATH];
	int i;

	/* retail keeps efgamex86.dll in the install root, next to BaseEF/ */
	if (di_join(p, sizeof(p), dir, di_dll_name) == 0 && di_is_file(p)) {
		di_copy(out, outsz, p);
		return 1;
	}
	if (base && di_join(p, sizeof(p), base, di_dll_name) == 0 && di_is_file(p)) {
		di_copy(out, outsz, p);
		return 1;
	}
	/* the user may have picked the BaseEF folder itself; in a retail install
	   the proof DLL sits one level up, next to that folder */
	if (base && !strcmp(base, dir)) {
		char parent[DI_MAXPATH];
		char q[DI_MAXPATH];
		di_parent_dir(base, parent, sizeof(parent));
		if (di_join(q, sizeof(q), parent, di_dll_name) == 0 && di_is_file(q)) {
			di_copy(out, outsz, q);
			return 1;
		}
	}
	for (i = 0; i < 3; i++) {
		if (!names[i][0])
			continue;
		if (di_join(p, sizeof(p), dir, names[i]) == 0) {
			char q[DI_MAXPATH];
			if (di_join(q, sizeof(q), p, di_dll_name) == 0 && di_is_file(q)) {
				di_copy(out, outsz, q);
				return 1;
			}
		}
	}
	return 0;
}

/* Is `dir` a complete retail folder (install root or BaseEF itself)? */
static int di_dir_complete(const char *dir, char *missing, size_t missz)
{
	char base[DI_MAXPATH];
	char dll[DI_MAXPATH];
	char p[DI_MAXPATH];
	int i, ok = 1;

	missing[0] = '\0';
	if (!di_retail_base(dir, base, sizeof(base))) {
		di_copy(missing, missz, "BaseEF/, pak0.pk3 .. pak3.pk3, efgamex86.dll");
		return 0;
	}
	for (i = 0; i < DI_NUM_PAKS; i++) {
		if (di_join(p, sizeof(p), base, di_paks[i]) != 0 || !di_pak_ok(p)) {
			di_append_name(missing, missz, di_paks[i]);
			ok = 0;
		}
	}
	if (!di_find_dll(dir, base, dll, sizeof(dll))) {
		di_append_name(missing, missz, di_dll_name);
		ok = 0;
	} else if (!di_dll_ok(dll)) {
		di_append_name(missing, missz, di_dll_name);
		ok = 0;
	}
	return ok;
}

/* Validate the engine-side layout <root>/baseEF/...  The proof DLL may also
   sit at <root>/efgamex86.dll: that is where the folder-symlink fallback
   keeps it (retail stores efgamex86.dll one level above BaseEF). */
static int di_data_valid(const char *root, char *missing, size_t missz)
{
	char base[DI_MAXPATH];
	char p[DI_MAXPATH];
	int i, ok = 1;

	missing[0] = '\0';
	if (di_join(p, sizeof(p), root, "baseEF") == 0 && di_is_dir(p))
		di_copy(base, sizeof(base), p);
	else if (di_join(p, sizeof(p), root, "BaseEF") == 0 && di_is_dir(p))
		di_copy(base, sizeof(base), p);
	else {
		di_copy(missing, missz, "baseEF/ (pak0.pk3 .. pak3.pk3, efgamex86.dll)");
		return 0;
	}

	for (i = 0; i < DI_NUM_PAKS; i++) {
		if (di_join(p, sizeof(p), base, di_paks[i]) != 0 || !di_pak_ok(p)) {
			di_append_name(missing, missz, di_paks[i]);
			ok = 0;
		}
	}
	if (di_join(p, sizeof(p), base, di_dll_name) == 0 && di_dll_ok(p)) {
		/* primary contract path */
	} else if (di_join(p, sizeof(p), root, di_dll_name) == 0 && di_dll_ok(p)) {
		/* linked beside baseEF for the retail install-root layout */
	} else {
		di_append_name(missing, missz, di_dll_name);
		ok = 0;
	}
	return ok;
}

/* Bring a manually prepared <root>/BaseEF (retail case) into the literal
   name the engine asks for, without renaming anything on disk. */
static void di_ensure_case_alias(const char *root)
{
	char lower[DI_MAXPATH];
	char upper[DI_MAXPATH];
	struct stat st;
	char lowerL[DI_MAXPATH];

	if (di_join(lower, sizeof(lower), root, "baseEF") != 0)
		return;
	if (lstat(lower, &st) == 0)
		return;                         /* already there (dir or symlink) */
	if (di_join(upper, sizeof(upper), root, "BaseEF") != 0 || !di_is_dir(upper))
		return;
	di_copy(lowerL, sizeof(lowerL), upper);
	if (symlink(lowerL, lower) == 0)
		printf("  linked %s -> %s (retail BaseEF / engine baseEF case bridge)\n",
		       lower, upper);
}

/* --------------------------------------------------------------------- */
/* filesystem mutation                                                    */
/* --------------------------------------------------------------------- */

static int di_symlink_force(const char *target, const char *linkpath)
{
	struct stat st;

	if (lstat(linkpath, &st) == 0) {
		if (S_ISLNK(st.st_mode)) {
			char cur[DI_MAXPATH];
			ssize_t n = readlink(linkpath, cur, sizeof(cur) - 1);
			if (n > 0) {
				cur[n] = '\0';
				if (!strcmp(cur, target))
					return 0;       /* already correct */
			}
			if (unlink(linkpath) != 0)
				return -1;
		} else {
			/* never delete a real file/dir here */
			return -1;
		}
	}
	return symlink(target, linkpath) == 0 ? 0 : -1;
}

static int di_copy_file(const char *src, const char *dst, const char *label,
                        long long total)
{
	char tmp[DI_MAXPATH + 8];
	FILE *in = NULL, *out = NULL;
	char *buf;
	long long done = 0, last_report = 0;
	int ok = 0;

	snprintf(tmp, sizeof(tmp), "%s.tmp", dst);

	buf = (char *)malloc(DI_COPY_CHUNK);
	if (!buf) {
		fprintf(stderr, "import: out of memory\n");
		return -1;
	}
	in = fopen(src, "rb");
	if (!in) {
		fprintf(stderr, "import: cannot open %s: %s\n", src, strerror(errno));
		free(buf);
		return -1;
	}
	out = fopen(tmp, "wb");
	if (!out) {
		fprintf(stderr, "import: cannot create %s: %s\n", tmp, strerror(errno));
		fclose(in);
		free(buf);
		return -1;
	}

	printf("  copying %s", label);
	fflush(stdout);

	for (;;) {
		size_t n = fread(buf, 1, DI_COPY_CHUNK, in);
		if (n > 0) {
			if (fwrite(buf, 1, n, out) != n) {
				fprintf(stderr, "\nimport: write failed on %s: %s\n",
				        tmp, strerror(errno));
				goto done;
			}
			done += (long long)n;
			if (done - last_report >= 32LL * 1024 * 1024) {
				last_report = done;
				if (total > 0)
					printf("\r  copying %s: %lld / %lld MB", label,
					       done / (1024 * 1024), total / (1024 * 1024));
				else
					printf("\r  copying %s: %lld MB", label,
					       done / (1024 * 1024));
				fflush(stdout);
			}
		}
		if (n < DI_COPY_CHUNK) {
			if (ferror(in)) {
				fprintf(stderr, "\nimport: read failed on %s\n", src);
				goto done;
			}
			break;
		}
	}
	if (fflush(out) != 0 || ferror(out)) {
		fprintf(stderr, "\nimport: write failed on %s: %s\n", tmp, strerror(errno));
		goto done;
	}
	ok = 1;

done:
	if (in)
		fclose(in);
	if (out)
		fclose(out);
	if (!ok) {
		unlink(tmp);
	} else {
		if (rename(tmp, dst) != 0) {
			fprintf(stderr, "\nimport: cannot rename %s -> %s: %s\n",
			        tmp, dst, strerror(errno));
			unlink(tmp);
			ok = 0;
		} else {
			printf("\r  copied %s (%lld MB)\n", label, done / (1024 * 1024));
		}
	}
	free(buf);
	return ok ? 0 : -1;
}

/* Turn <root>/baseEF into a real directory ready to receive copies.  If it
   is currently a symlink to the retail install, remove the link only (the
   retail folder itself is untouched). */
static int di_prepare_baseef(const char *root)
{
	char base[DI_MAXPATH];
	struct stat st;

	if (di_mkdir_p(root) != 0) {
		fprintf(stderr, "import: cannot create data root %s: %s\n",
		        root, strerror(errno));
		return -1;
	}
	if (di_join(base, sizeof(base), root, "baseEF") != 0)
		return -1;

	if (lstat(base, &st) == 0) {
		if (S_ISLNK(st.st_mode)) {
			printf("  replacing the %s symlink with a real directory for the copies\n", base);
			if (unlink(base) != 0) {
				fprintf(stderr, "import: cannot remove %s: %s\n", base, strerror(errno));
				return -1;
			}
		} else if (!S_ISDIR(st.st_mode)) {
			fprintf(stderr, "import: %s exists and is not a directory\n", base);
			return -1;
		}
	}
	if (mkdir(base, 0755) != 0 && errno != EEXIST) {
		fprintf(stderr, "import: cannot create %s: %s\n", base, strerror(errno));
		return -1;
	}
	return 0;
}

/* Copy the given individual files into <root>/baseEF/. */
static int di_copy_into_root(const char *root, char *const *files, int nfiles)
{
	char base[DI_MAXPATH];
	char dst[DI_MAXPATH];
	int i;

	if (di_prepare_baseef(root) != 0)
		return -1;
	if (di_join(base, sizeof(base), root, "baseEF") != 0)
		return -1;

	for (i = 0; i < nfiles; i++) {
		const char *name = di_basename(files[i]);
		long long sz;

		if (!di_is_required_name(name)) {
			printf("  skipping %s (not one of pak0.pk3 .. pak3.pk3, %s)\n",
			       files[i], di_dll_name);
			continue;
		}
		if (di_join(dst, sizeof(dst), base, name) != 0)
			return -1;
		if (!strcmp(name, di_dll_name)) {
			if (!di_dll_ok(files[i])) {
				fprintf(stderr,
				        "import: %s is not a plausible retail efgamex86.dll "
				        "(need >= 4096 bytes and an MZ header)\n", files[i]);
				return -1;
			}
		} else if (!di_pak_ok(files[i])) {
			fprintf(stderr, "import: %s is empty or too small to be a retail pak\n",
			        files[i]);
			return -1;
		}
		sz = di_file_size(files[i]);
		if (di_copy_file(files[i], dst, name, sz) != 0)
			return -1;
	}
	return 0;
}

/* --------------------------------------------------------------------- */
/* <root>/baseEF layout                                                    */
/*                                                                         */
/* Default: <root>/baseEF is a REAL directory containing one symlink per   */
/* retail file (pak0..3.pk3 + efgamex86.dll).  The engine reads the paks   */
/* through those links but writes efspconfig.cfg, saves/, qconsole.log,    */
/* demos and screenshots into the user's data root — never into the        */
/* (possibly read-only) retail install.  The old folder-symlink form is    */
/* only an explicit fallback for filesystems that cannot create symlinks.  */
/* --------------------------------------------------------------------- */

static void di_tmp_cleanup(const char *tmpdir)
{
	char p[DI_MAXPATH];
	int i;

	for (i = 0; i < DI_NUM_FILES; i++) {
		if (di_join(p, sizeof(p), tmpdir, di_required[i]) == 0)
			unlink(p);
	}
	rmdir(tmpdir);
}

/* Point `link` at `target`, keeping an already-correct link or a real file
   that is already in place (idempotent re-import). */
static int di_link_file(const char *target, const char *link)
{
	struct stat st;

	if (lstat(link, &st) == 0) {
		if (S_ISLNK(st.st_mode)) {
			char cur[DI_MAXPATH];
			ssize_t n = readlink(link, cur, sizeof(cur) - 1);
			if (n > 0) {
				cur[n] = '\0';
				if (!strcmp(cur, target))
					return 0;       /* already correct */
			}
			if (unlink(link) != 0)
				return -1;
		} else {
			/* a real file is already there (e.g. an earlier copy) */
			return 0;
		}
	}
	return symlink(target, link) == 0 ? 0 : -1;
}

static int di_build_baseef_links(const char *root, const char *base, const char *dll)
{
	char finalpath[DI_MAXPATH];
	char dir[DI_MAXPATH];
	char target[DI_MAXPATH];
	char link[DI_MAXPATH];
	struct stat st;
	int use_tmp = 0;
	int i;

	if (di_join(finalpath, sizeof(finalpath), root, "baseEF") != 0)
		return -1;

	if (lstat(finalpath, &st) == 0) {
		if (S_ISLNK(st.st_mode))
			use_tmp = 1;            /* replace the old folder symlink */
		else if (!S_ISDIR(st.st_mode))
			return -1;
	} else {
		use_tmp = 1;                    /* fresh install */
	}

	if (use_tmp) {
		/* build beside the final name, then rename atomically */
		if (di_pid_path(dir, sizeof(dir), root, "/.baseEF.tmp.") != 0)
			return -1;
		di_tmp_cleanup(dir);
		if (mkdir(dir, 0755) != 0) {
			fprintf(stderr, "import: cannot create %s: %s\n", dir, strerror(errno));
			return -1;
		}
	} else {
		di_copy(dir, sizeof(dir), finalpath);
	}

	printf("  %s: real directory + per-file symlinks (no copy; saves stay here)\n",
	       finalpath);

	for (i = 0; i < DI_NUM_PAKS; i++) {
		if (di_join(target, sizeof(target), base, di_paks[i]) != 0 ||
		    di_join(link, sizeof(link), dir, di_paks[i]) != 0 ||
		    di_link_file(target, link) != 0) {
			fprintf(stderr, "import: cannot link %s into %s: %s\n",
			        di_paks[i], dir, strerror(errno));
			if (use_tmp)
				di_tmp_cleanup(dir);
			return -1;
		}
	}
	if (di_join(link, sizeof(link), dir, di_dll_name) != 0 ||
	    di_link_file(dll, link) != 0) {
		fprintf(stderr, "import: cannot link %s into %s: %s\n",
		        di_dll_name, dir, strerror(errno));
		if (use_tmp)
			di_tmp_cleanup(dir);
		return -1;
	}

	if (use_tmp) {
		char backup[DI_MAXPATH];
		int had_link = 0;

		/*
		 * rename() of a directory onto an existing symlink fails with
		 * ENOTDIR, so move the old folder symlink aside first and put it
		 * back if the install somehow fails.
		 */
		if (lstat(finalpath, &st) == 0 && S_ISLNK(st.st_mode)) {
			if (di_pid_path(backup, sizeof(backup), finalpath, ".old.") != 0) {
				di_tmp_cleanup(dir);
				return -1;
			}
			unlink(backup);
			if (rename(finalpath, backup) != 0) {
				fprintf(stderr, "import: cannot move %s aside: %s\n",
				        finalpath, strerror(errno));
				di_tmp_cleanup(dir);
				return -1;
			}
			had_link = 1;
		}
		if (rename(dir, finalpath) != 0) {
			fprintf(stderr, "import: cannot install %s: %s\n",
			        finalpath, strerror(errno));
			if (had_link)
				rename(backup, finalpath);
			di_tmp_cleanup(dir);
			return -1;
		}
		if (had_link)
			unlink(backup);
	}
	return 0;
}

/* Complete retail folder: install it as <root>/baseEF with one symlink per
   file (no 600 MB copy).  If <root>/baseEF is already a real directory the
   links are refreshed in place (never destructive). */
static int di_install_folder(const char *root, const char *dir)
{
	char base[DI_MAXPATH];
	char dll[DI_MAXPATH];
	char linkpath[DI_MAXPATH];
	char p[DI_MAXPATH];
	int rc;

	if (!di_dir_complete(dir, p, sizeof(p))) {
		fprintf(stderr, "import: %s is not a complete retail folder (missing: %s)\n",
		        dir, p);
		return -1;
	}
	if (!di_retail_base(dir, base, sizeof(base)))
		return -1;
	if (!di_find_dll(dir, base, dll, sizeof(dll)))
		return -1;
	if (di_mkdir_p(root) != 0) {
		fprintf(stderr, "import: cannot create data root %s: %s\n",
		        root, strerror(errno));
		return -1;
	}
	if (di_join(linkpath, sizeof(linkpath), root, "baseEF") != 0)
		return -1;

	rc = di_build_baseef_links(root, base, dll);
	if (rc != 0) {
		/*
		 * Explicit fallback only: the filesystem refused per-file
		 * symlinks.  Link the whole folder (old layout) so the game
		 * still runs, and say clearly that saves will land in retail.
		 */
		struct stat st;
		printf("  WARNING: per-file symlinks failed; falling back to a folder symlink\n"
		       "           (the engine will write saves/config into %s)\n", base);
		if (lstat(linkpath, &st) == 0 && !S_ISLNK(st.st_mode)) {
			fprintf(stderr, "import: cannot fall back: %s is a real directory\n",
			        linkpath);
			return -1;
		}
		if (di_symlink_force(base, linkpath) != 0) {
			fprintf(stderr, "import: cannot create symlink %s: %s\n",
			        linkpath, strerror(errno));
			return -1;
		}
		/* the proof DLL lives outside the retail BaseEF: keep it beside
		   baseEF, which di_data_valid() also accepts */
		if (di_join(p, sizeof(p), linkpath, di_dll_name) != 0 || !di_dll_ok(p)) {
			char dlllink[DI_MAXPATH];
			if (di_join(dlllink, sizeof(dlllink), root, di_dll_name) != 0)
				return -1;
			if (!di_dll_ok(dlllink) && di_symlink_force(dll, dlllink) != 0) {
				fprintf(stderr, "import: cannot link %s: %s\n",
				        dlllink, strerror(errno));
				return -1;
			}
		}
	}

	if (!di_data_valid(root, p, sizeof(p))) {
		fprintf(stderr, "import: data root still incomplete after import (missing: %s)\n", p);
		return -1;
	}
	return 0;
}

/* Migrate a pre-existing folder-symlink <root>/baseEF (the old layout, which
   made the engine write into the retail install) to the real-directory +
   per-file-link layout.  Best effort: on failure the old symlink is left
   untouched and a warning explains why. */
static void di_upgrade_folder_symlink(const char *root)
{
	char linkpath[DI_MAXPATH];
	char target[DI_MAXPATH];
	char base[DI_MAXPATH];
	char dll[DI_MAXPATH];
	char p[DI_MAXPATH];
	struct stat st;

	if (di_join(linkpath, sizeof(linkpath), root, "baseEF") != 0)
		return;
	if (lstat(linkpath, &st) != 0 || !S_ISLNK(st.st_mode))
		return;
	if (!realpath(linkpath, target))
		return;                         /* dangling link: nothing to import */
	if (!di_is_dir(target))
		return;
	if (!di_retail_base(target, base, sizeof(base)))
		return;

	if (!di_find_dll(target, base, dll, sizeof(dll))) {
		/* the old layout linked the proof DLL beside baseEF */
		if (di_join(p, sizeof(p), root, di_dll_name) != 0 || !di_is_file(p))
			return;
		di_copy(dll, sizeof(dll), p);
	}

	printf("  upgrading %s from a folder symlink to a real directory\n", linkpath);
	printf("  (per-file symlinks; saves/config now stay in %s)\n", root);
	if (di_build_baseef_links(root, base, dll) != 0)
		fprintf(stderr,
		        "import: could not upgrade %s; keeping the old folder symlink\n",
		        linkpath);
}

/* --------------------------------------------------------------------- */
/* retail candidate discovery                                             */
/* --------------------------------------------------------------------- */

static void di_add_candidate(char list[][DI_MAXPATH], int *n, int max,
                             const char *path)
{
	char norm[DI_MAXPATH];
	char missing[256];
	int i;

	if (*n >= max || !path || !*path)
		return;
	di_copy(norm, sizeof(norm), path);
	if (di_normalize(norm, sizeof(norm)) != 0)
		return;
	if (!di_dir_complete(norm, missing, sizeof(missing)))
		return;
	for (i = 0; i < *n; i++)
		if (!strcmp(list[i], norm))
			return;
	di_copy(list[*n], DI_MAXPATH, norm);
	(*n)++;
}

static void di_add_glob(char list[][DI_MAXPATH], int *n, int max,
                        const char *pattern)
{
	glob_t g;
	size_t i;

	if (*n >= max)
		return;
	memset(&g, 0, sizeof(g));
	if (glob(pattern, 0, NULL, &g) != 0) {
		globfree(&g);
		return;
	}
	for (i = 0; i < g.gl_pathc && *n < max; i++)
		if (di_is_dir(g.gl_pathv[i]))
			di_add_candidate(list, n, max, g.gl_pathv[i]);
	globfree(&g);
}

/* Steam keeps extra library roots in steamapps/libraryfolders.vdf:
     "path"   "/home/user/.local/share/Steam"                          */
static void di_add_steam_libraries(char list[][DI_MAXPATH], int *n, int max,
                                   const char *vdf)
{
	FILE *f;
	char line[2048];

	if (*n >= max)
		return;
	f = fopen(vdf, "r");
	if (!f)
		return;
	while (fgets(line, sizeof(line), f)) {
		char *p = strstr(line, "\"path\"");
		char *q;
		char pattern[DI_MAXPATH];

		if (!p)
			continue;
		p += strlen("\"path\"");
		while (*p && isspace((unsigned char)*p))
			p++;
		if (*p != '"')
			continue;
		p++;
		q = strchr(p, '"');
		if (!q)
			continue;
		*q = '\0';
		if (!*p)
			continue;
		snprintf(pattern, sizeof(pattern), "%s/steamapps/common/*", p);
		di_add_glob(list, n, max, pattern);
	}
	fclose(f);
}

static void di_scan_candidates(char list[][DI_MAXPATH], int *n, int max)
{
	const char *home = getenv("HOME");
	char p[DI_MAXPATH];

	*n = 0;
	if (!home || !*home)
		return;

	/* the locations named in the task, then GOG / Games / Steam */
	snprintf(p, sizeof(p), "%s/Star Trek Elite Force", home);
	di_add_candidate(list, n, max, p);
	snprintf(p, sizeof(p), "%s/Games/Star Trek Elite Force", home);
	di_add_candidate(list, n, max, p);
	snprintf(p, sizeof(p), "%s/.local/share/Star Trek Elite Force", home);
	di_add_candidate(list, n, max, p);

	snprintf(p, sizeof(p), "%s/GOG Games/*", home);
	di_add_glob(list, n, max, p);
	snprintf(p, sizeof(p), "%s/Games/*", home);
	di_add_glob(list, n, max, p);

	snprintf(p, sizeof(p), "%s/.steam/steam/steamapps/common/*", home);
	di_add_glob(list, n, max, p);
	snprintf(p, sizeof(p), "%s/.local/share/Steam/steamapps/common/*", home);
	di_add_glob(list, n, max, p);
	snprintf(p, sizeof(p), "%s/.var/app/com.valvesoftware.Steam/.local/share/Steam/steamapps/common/*", home);
	di_add_glob(list, n, max, p);

	snprintf(p, sizeof(p), "%s/.steam/steam/steamapps/libraryfolders.vdf", home);
	di_add_steam_libraries(list, n, max, p);
	snprintf(p, sizeof(p), "%s/.local/share/Steam/steamapps/libraryfolders.vdf", home);
	di_add_steam_libraries(list, n, max, p);
}

/* --------------------------------------------------------------------- */
/* host pickers — fork/exec only, no toolkit linkage                      */
/* --------------------------------------------------------------------- */

static int di_have_in_path(const char *prog)
{
	const char *path = getenv("PATH");
	char buf[DI_MAXPATH];
	char *copy, *tok, *save = NULL;
	int found = 0;

	if (!path || !*path)
		path = "/usr/local/bin:/usr/bin:/bin";
	copy = strdup(path);
	if (!copy)
		return 0;
	for (tok = strtok_r(copy, ":", &save); tok; tok = strtok_r(NULL, ":", &save)) {
		if (di_join(buf, sizeof(buf), tok, prog) != 0)
			continue;
		if (access(buf, X_OK) == 0) {
			found = 1;
			break;
		}
	}
	free(copy);
	return found;
}

/* A GUI picker needs a display; on a headless box zenity/kdialog just fail.
   Skipping them there sends the user to the typed-path prompt instead of a
   bogus "cancelled" result. */
static int di_gui_possible(void)
{
	const char *d = getenv("DISPLAY");
	const char *w = getenv("WAYLAND_DISPLAY");
	return ((d && *d) || (w && *w)) ? 1 : 0;
}

/* Run argv, capture stdout.  Returns 1 = selection, 0 = cancelled,
   -1 = tool could not be run / failed. */
static int di_run_capture(char *const argv[], char *out, size_t outsz)
{
	int fds[2];
	pid_t pid;
	int status = 0;
	size_t used = 0;
	ssize_t n;
	char buf[4096];

	out[0] = '\0';
	if (pipe(fds) != 0)
		return -1;

	pid = fork();
	if (pid < 0) {
		close(fds[0]);
		close(fds[1]);
		return -1;
	}
	if (pid == 0) {
		int devnull;
		close(fds[0]);
		if (fds[1] != STDOUT_FILENO) {
			if (dup2(fds[1], STDOUT_FILENO) < 0)
				_exit(127);
			close(fds[1]);
		}
		/* pickers must never inherit a console description */
		devnull = open("/dev/null", O_RDONLY);
		if (devnull >= 0) {
			if (devnull != STDIN_FILENO) {
				dup2(devnull, STDIN_FILENO);
				close(devnull);
			}
		}
		execvp(argv[0], argv);
		_exit(127);
	}

	close(fds[1]);
	while ((n = read(fds[0], buf, sizeof(buf))) > 0) {
		if (used + (size_t)n < outsz) {
			memcpy(out + used, buf, (size_t)n);
			used += (size_t)n;
		}
	}
	out[used < outsz ? used : outsz - 1] = '\0';
	close(fds[0]);

	while (waitpid(pid, &status, 0) < 0 && errno == EINTR)
		;

	if (!WIFEXITED(status))
		return -1;
	if (WEXITSTATUS(status) == 0) {
		di_trim(out);
		return out[0] ? 1 : 0;
	}
	if (WEXITSTATUS(status) == 1)  /* both zenity and kdialog cancel with 1 */
		return 0;
	return -1;
}

/* folder = 1 -> pick a directory; folder = 0 -> pick multiple files
   (newline separated output). */
static int di_pick_one(const char *tool, int folder, const char *start,
                       char *out, size_t outsz)
{
	char filename_arg[DI_MAXPATH + 32];

	snprintf(filename_arg, sizeof(filename_arg), "--filename=%.*s",
	         DI_MAXPATH - 1, (start && *start) ? start : "/");

	if (!strcmp(tool, "zenity")) {
		if (folder) {
			char *argv[] = {
				(char *)"zenity", (char *)"--file-selection",
				(char *)"--directory",
				(char *)"--title=Star Trek: Elite Force — select the retail game folder (Cancel = pick individual files)",
				filename_arg, NULL
			};
			return di_run_capture(argv, out, outsz);
		} else {
			char *argv[] = {
				(char *)"zenity", (char *)"--file-selection",
				(char *)"--multiple", (char *)"--separator=\n",
				(char *)"--title=Select pak0.pk3 .. pak3.pk3 and efgamex86.dll",
				filename_arg, NULL
			};
			return di_run_capture(argv, out, outsz);
		}
	}

	/* kdialog */
	if (folder) {
		char *argv[] = {
			(char *)"kdialog",
			(char *)"--title", (char *)"Star Trek: Elite Force — select the retail game folder (Cancel = pick individual files)",
			(char *)"--getexistingdirectory", (char *)start, NULL
		};
		return di_run_capture(argv, out, outsz);
	} else {
		char *argv[] = {
			(char *)"kdialog",
			(char *)"--title", (char *)"Select pak0.pk3 .. pak3.pk3 and efgamex86.dll",
			(char *)"--getopenfilename", (char *)start,
			(char *)"*.pk3 *.dll",
			(char *)"--multiple", (char *)"--separate-output", NULL
		};
		return di_run_capture(argv, out, outsz);
	}
}

static int di_pick(int folder, const char *start, char *out, size_t outsz)
{
	static const char *const tools[] = { "zenity", "kdialog" };
	int i;

	if (!di_gui_possible())
		return -1;                      /* headless: use the terminal prompt */

	for (i = 0; i < 2; i++) {
		int r;
		if (!di_have_in_path(tools[i]))
			continue;
		r = di_pick_one(tools[i], folder, start, out, outsz);
		if (r >= 0)
			return r;               /* selected or cancelled */
	}
	return -1;                          /* no working picker */
}

/* --------------------------------------------------------------------- */
/* user-facing text                                                       */
/* --------------------------------------------------------------------- */

static void di_print_manual(const char *root, const char *missing)
{
	printf("\n"
	       "Star Trek: Voyager - Elite Force needs the data from your retail copy:\n"
	       "\n"
	       "    pak0.pk3  pak1.pk3  pak2.pk3  pak3.pk3   (in the game's BaseEF folder)\n"
	       "    efgamex86.dll                            (in the game's install folder)\n"
	       "\n"
	       "Put them here:\n"
	       "    %s/baseEF/\n"
	       "\n"
	       "or point the engine at an existing retail install:\n"
	       "    voyagersp --import-from \"/path/to/Star Trek Elite Force\"\n"
	       "\n"
	       "The DLL is only proof of ownership; this port never loads or runs it.\n"
	       "Nothing is downloaded and no retail data is shipped with the port.\n",
	       root);
	if (missing && *missing)
		printf("\nStill missing: %s\n", missing);
	printf("\n");
	fflush(stdout);
}

/* --------------------------------------------------------------------- */
/* interactive terminal fallback                                          */
/* --------------------------------------------------------------------- */

static int di_terminal_import(const char *root, char list[][DI_MAXPATH], int ncand)
{
	char line[DI_MAXPATH];
	int tries = 0;

	for (;;) {
		char missing[512];
		int i;

		printf("Retail folders found on this machine:\n");
		if (!ncand)
			printf("    (none)\n");
		for (i = 0; i < ncand; i++)
			printf("    [%d] %s\n", i + 1, list[i]);
		printf("\nEnter a number, the path to the retail folder, the path to one\n"
		       "game file (pak0.pk3 .. pak3.pk3, efgamex86.dll), or 'q' to cancel:\n> ");
		fflush(stdout);

		if (!fgets(line, sizeof(line), stdin))
			return 2;               /* EOF == cancelled */
		di_trim(line);
		if (!*line)
			continue;
		if (!strcmp(line, "q") || !strcmp(line, "quit") || !strcmp(line, "Q"))
			return 2;

		if (isdigit((unsigned char)line[0])) {
			char *end = NULL;
			long idx = strtol(line, &end, 10);
			if (end && !*end && idx >= 1 && idx <= ncand) {
				di_copy(line, sizeof(line), list[idx - 1]);
			}
		}

		if (di_normalize(line, sizeof(line)) != 0) {
			printf("That path could not be resolved.\n\n");
			tries++;
			if (tries >= 12)
				return 3;
			continue;
		}

		if (di_is_dir(line)) {
			if (di_install_folder(root, line) == 0)
				return 0;
			if (!di_dir_complete(line, missing, sizeof(missing)))
				printf("\nThat folder is not a complete retail install (missing: %s).\n"
				       "Pick the folder that contains BaseEF/, or select the files.\n\n",
				       missing);
			else
				printf("\nCould not link the game data from that folder "
				       "(check permissions on %s).\n\n", root);
			tries++;
		} else if (di_is_file(line)) {
			char *files[1];
			const char *name = di_basename(line);

			if (!di_is_required_name(name)) {
				printf("%s is not one of the required game files.\n\n", line);
				tries++;
			} else {
				files[0] = line;
				if (di_copy_into_root(root, files, 1) != 0) {
					printf("Could not import %s.\n\n", line);
					tries++;
				} else if (di_data_valid(root, missing, sizeof(missing))) {
					return 0;
				} else {
					printf("Imported %s. Still missing: %s\n\n", name, missing);
				}
			}
		} else {
			printf("No such file or folder: %s\n\n", line);
			tries++;
		}

		if (tries >= 12) {
			printf("Too many invalid selections.\n");
			return 3;
		}
	}
}

/* --------------------------------------------------------------------- */
/* scripted / GUI / terminal entry points                                 */
/* --------------------------------------------------------------------- */

static int di_import_pick_files(const char *root)
{
	char listing[DI_MAXPATH * 4];
	const char *home = getenv("HOME");
	char *paths[16];
	int npaths = 0;
	int r, i;
	char missing[512];

	if (!home || !*home)
		home = "/";
	r = di_pick(0, home, listing, sizeof(listing));
	if (r == 0)
		return 2;
	if (r < 0)
		return 4;

	{
		char *p = listing;
		while (*p && npaths < 16) {
			char *nl = strchr(p, '\n');
			if (nl)
				*nl = '\0';
			di_trim(p);
			if (*p)
				paths[npaths++] = p;
			if (!nl)
				break;
			p = nl + 1;
		}
	}
	if (!npaths) {
		printf("No files were selected.\n");
		return 2;
	}

	for (i = 0; i < npaths; i++) {
		if (!di_is_required_name(di_basename(paths[i]))) {
			fprintf(stderr,
			        "import: %s is not one of pak0.pk3 .. pak3.pk3, %s\n",
			        paths[i], di_dll_name);
			return 3;
		}
	}
	if (di_copy_into_root(root, paths, npaths) != 0)
		return 3;
	if (!di_data_valid(root, missing, sizeof(missing))) {
		fprintf(stderr, "import: still missing: %s\n", missing);
		return 3;
	}
	return 0;
}

static int di_interactive_import(const char *root, char list[][DI_MAXPATH], int ncand)
{
	char selection[DI_MAXPATH];
	const char *start;
	char missing[512];
	int r;

	start = (ncand > 0) ? list[0] : (getenv("HOME") ? getenv("HOME") : "/");

	di_print_manual(root, di_data_valid(root, missing, sizeof(missing)) ? "" : missing);

	r = di_pick(1, start, selection, sizeof(selection));
	if (r < 0) {
		/* no GUI picker: interactive terminal prompt (folder or file paths) */
		if (!isatty(STDIN_FILENO)) {
			printf("No graphical file picker (zenity/kdialog) and no terminal to\n"
			       "prompt on. Follow the instructions above, then run again.\n");
			return 4;
		}
		printf("No graphical file picker found; using the terminal prompt.\n\n");
		return di_terminal_import(root, list, ncand);
	}

	if (r == 1) {
		if (di_is_dir(selection)) {
			if (di_install_folder(root, selection) == 0)
				return 0;
			printf("\nThat folder is not a complete retail install.\n"
			       "Select the individual game files instead.\n\n");
		} else {
			printf("\nThat is not a folder; select the individual game files instead.\n\n");
		}
	} else {
		printf("\nFolder selection cancelled; select the individual game files instead.\n\n");
	}

	/* Second chance: pick the individual files (copied into <root>/baseEF/). */
	r = di_import_pick_files(root);
	if (r == 4 && isatty(STDIN_FILENO))
		return di_terminal_import(root, list, ncand);
	return r;
}

/* --------------------------------------------------------------------- */
/* flag parsing                                                           */
/* --------------------------------------------------------------------- */

typedef struct {
	int force_import;
	int skip_import;
	int import_only;
	int show_help;
	int fullscreen;
	char data_dir[DI_MAXPATH];
	char import_from[DI_MAXPATH];
} di_options_t;

static int di_parse_args(int *argcP, char ***argvP, di_options_t *opt)
{
	char **argv = *argvP;
	int argc = *argcP;
	int r, w = 1;

	memset(opt, 0, sizeof(*opt));
	opt->fullscreen = -1;

	for (r = 1; r < argc; r++) {
		const char *a = argv[r];

		if (!strcmp(a, "--data-dir")) {
			if (r + 1 >= argc) {
				fprintf(stderr, "voyagersp: --data-dir needs a path\n");
				return -1;
			}
			di_copy(opt->data_dir, sizeof(opt->data_dir), argv[++r]);
			continue;
		}
		if (!strncmp(a, "--data-dir=", 11)) {
			di_copy(opt->data_dir, sizeof(opt->data_dir), a + 11);
			continue;
		}
		if (!strcmp(a, "--import")) {
			opt->force_import = 1;
			continue;
		}
		if (!strcmp(a, "--no-import")) {
			opt->skip_import = 1;
			continue;
		}
		if (!strcmp(a, "--import-only")) {
			opt->import_only = 1;
			continue;
		}
		if (!strcmp(a, "--import-from")) {
			if (r + 1 >= argc) {
				fprintf(stderr, "voyagersp: --import-from needs a directory\n");
				return -1;
			}
			di_copy(opt->import_from, sizeof(opt->import_from), argv[++r]);
			continue;
		}
		if (!strncmp(a, "--import-from=", 14)) {
			di_copy(opt->import_from, sizeof(opt->import_from), a + 14);
			continue;
		}
		if (!strcmp(a, "--fullscreen")) {
			opt->fullscreen = 1;
			continue;
		}
		if (!strcmp(a, "--windowed")) {
			opt->fullscreen = 0;
			continue;
		}
		if (!strcmp(a, "--help") || !strcmp(a, "-h")) {
			opt->show_help = 1;
			continue;
		}

		/* everything else belongs to the engine */
		argv[w++] = argv[r];
	}
	argv[w] = NULL;
	*argcP = w;
	*argvP = argv;
	return 0;
}

static void di_print_usage(const char *argv0)
{
	printf(
		"Voyager Elite Force SP — desktop launcher\n"
		"\n"
		"Usage: %s [options] [engine args...]\n"
		"\n"
		"Game data:\n"
		"  --data-dir <path>     data root (default $VOYAGERSP_DATA_ROOT, else\n"
		"                        ${XDG_DATA_HOME:-$HOME/.local/share}/voyagersp)\n"
		"  --import              force the retail-data import/picker\n"
		"  --no-import           never ask; error out if the data is missing\n"
		"  --import-from <dir>   non-interactive import from a retail folder\n"
		"  --import-only         import and exit 0 without starting the engine\n"
		"\n"
		"Video:\n"
		"  --windowed            start windowed (default; r_fullscreen 0)\n"
		"  --fullscreen          start fullscreen (r_fullscreen 1)\n"
		"\n"
		"Exit codes of the import path: 0 ok, 2 cancelled, 3 invalid selection,\n"
		"4 no picker available.\n",
		argv0);
}

static int di_resolve_root(const char *explicit_dir, char *out, size_t outsz)
{
	const char *env;
	const char *xdg;
	const char *home;

	if (explicit_dir && *explicit_dir) {
		di_copy(out, outsz, explicit_dir);
		return di_normalize(out, outsz);
	}
	env = getenv("VOYAGERSP_DATA_ROOT");
	if (env && *env) {
		di_copy(out, outsz, env);
		return di_normalize(out, outsz);
	}
	xdg = getenv("XDG_DATA_HOME");
	home = getenv("HOME");
	if (xdg && *xdg)
		snprintf(out, outsz, "%s/voyagersp", xdg);
	else if (home && *home)
		snprintf(out, outsz, "%s/.local/share/voyagersp", home);
	else {
		fprintf(stderr,
		        "voyagersp: cannot determine the data root: set VOYAGERSP_DATA_ROOT "
		        "or HOME/XDG_DATA_HOME, or pass --data-dir.\n");
		return -1;
	}
	return di_normalize(out, outsz);
}

/* --------------------------------------------------------------------- */
/* entry point                                                            */
/* --------------------------------------------------------------------- */

int DataImport_PreInit(int *argc, char ***argv, dataImportResult_t *out)
{
	di_options_t opt;
	char root[DI_MAXPATH];
	char importFrom[DI_MAXPATH];
	char missing[512];
	char list[DI_MAX_CAND][DI_MAXPATH];
	int ncand = 0;
	int rc;

	if (!out)
		return 3;
	memset(out, 0, sizeof(*out));
	out->fullscreen = -1;

	if (di_parse_args(argc, argv, &opt) != 0)
		return 3;

	if (opt.show_help) {
		di_print_usage((*argv)[0]);
		return 0;
	}

	if (di_resolve_root(opt.data_dir, root, sizeof(root)) != 0) {
		fprintf(stderr,
		        "voyagersp: no usable data root. Pass --data-dir <path> or set "
		        "VOYAGERSP_DATA_ROOT.\n");
		return 3;
	}
	di_copy(out->dataRoot, sizeof(out->dataRoot), root);
	out->fullscreen = opt.fullscreen;

	/* Bring a hand-prepared retail "BaseEF" into the engine's "baseEF". */
	di_ensure_case_alias(root);

	/* Convert a pre-existing folder-symlink baseEF (old layout that made the
	   engine write into retail) to a real dir + per-file links. */
	di_upgrade_folder_symlink(root);

	{
		int valid = di_data_valid(root, missing, sizeof(missing));

		if (valid && !opt.force_import && !opt.import_from[0]) {
			printf("data root: %s (retail data OK)\n", root);
			fflush(stdout);
			return opt.import_only ? 0 : -1;
		}

		if (opt.skip_import && !opt.import_from[0]) {
			if (valid)
				return -1;
			fprintf(stderr,
			        "voyagersp: --no-import was given but the retail data is missing "
			        "from %s/baseEF (missing: %s).\n",
			        root, missing);
			di_print_manual(root, missing);
			return 3;
		}

		if (opt.import_from[0]) {
			di_copy(importFrom, sizeof(importFrom), opt.import_from);
			if (di_normalize(importFrom, sizeof(importFrom)) != 0) {
				fprintf(stderr, "voyagersp: bad --import-from path: %s\n",
				        opt.import_from);
				return 3;
			}
			printf("importing retail data from %s into %s\n", importFrom, root);
			fflush(stdout);
			if (di_is_dir(importFrom)) {
				/* dir may be the install root (contains BaseEF/), the
				   BaseEF folder itself, or a flat folder holding the
				   five files — di_retail_base() handles all three. */
				if (di_install_folder(root, importFrom) != 0)
					return 3;
			} else if (di_is_file(importFrom)) {
				char *files[1];
				files[0] = importFrom;
				if (di_copy_into_root(root, files, 1) != 0)
					return 3;
			} else {
				fprintf(stderr, "voyagersp: --import-from path does not exist: %s\n",
				        importFrom);
				return 3;
			}
			if (!di_data_valid(root, missing, sizeof(missing))) {
				fprintf(stderr, "voyagersp: import incomplete (missing: %s)\n", missing);
				return 3;
			}
			printf("import complete: %s/baseEF\n", root);
			fflush(stdout);
			return opt.import_only ? 0 : -1;
		}

		/* Missing data: discover candidates, then ask. */
		di_scan_candidates(list, &ncand, DI_MAX_CAND);
		rc = di_interactive_import(root, list, ncand);
		if (rc == 0) {
			printf("\nImport complete: %s/baseEF\n", root);
			fflush(stdout);
			return opt.import_only ? 0 : -1;
		}
		return rc;
	}
}

#endif /* !__ANDROID__ */
