/*
===========================================================================
Voyager Elite Force SP — desktop retail-data importer.

The Android app (ImportActivity.java) is the behavioural authority: before
the engine starts it makes sure the retail game files exist, asks the user
for them if they do not, verifies them and copies them into its own
baseEF/ directory. This module implements the same policy inside the
desktop binary, before any video / Vulkan initialisation.

The required retail files (never shipped, never downloaded):
    pak0.pk3 .. pak3.pk3   +   efgamex86.dll

efgamex86.dll is only a proof-of-ownership token. It is never loaded or
executed by this port.

Exit codes used by the import path (returned from main()):
    0  valid data / import completed (with --import-only)
    2  cancelled by the user
    3  invalid selection
    4  no picker available (manual instructions were printed)
    -1 from DataImport_PreInit() means "continue starting the engine".
===========================================================================
*/
#ifndef VOYAGER_DATA_IMPORT_H
#define VOYAGER_DATA_IMPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DI_MAXPATH 4096

typedef struct dataImportResult_s {
	/* Resolved data root. The engine uses this for BOTH fs_basepath and
	   fs_homepath (see sys_main.c / sys_unix.c). */
	char dataRoot[DI_MAXPATH];
	/* -1 = leave the engine default, 0 = force windowed, 1 = force fullscreen.
	   Set by the --windowed / --fullscreen launcher flags. */
	int fullscreen;
} dataImportResult_t;

/*
 * Parse the desktop-only flags, resolve the data root and validate/import
 * the retail data.  Runs before SDL/Vulkan is touched.
 *
 * Returns -1 when the engine should continue starting, otherwise an exit
 * code (0/2/3/4) that main() returns directly.
 */
int DataImport_PreInit(int *argc, char ***argv, dataImportResult_t *out);

#ifdef __cplusplus
}
#endif

#endif /* VOYAGER_DATA_IMPORT_H */
