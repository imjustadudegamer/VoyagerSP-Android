/*
===========================================================================
This file is part of the port of Star Trek: Voyager -- Elite Force II, whose
engine descends from id Tech 3 through OpenMoHAA.

It is free software; you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

It is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License, COPYING.txt at the repository
root, for more details.
===========================================================================
*/
// SPDX-License-Identifier: GPL-2.0-or-later
// Modified 2026-10-04 for this port; the changes are listed in PROVENANCE.tsv.
#include "qgl_gles3.h"
#include "../qcommon/q_shared.h"   // PRODUCT_VERSION/ARCH_STRING for the build log
#include "qgl_gles3_tokens.h" 

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static void (*gles3_logger)(const char *line) = NULL;

void QGL_GLES3_SetLogger(void (*fn)(const char *line))
{
	gles3_logger = fn;
}

static void GLES3_Log(const char *fmt, ...) GLES3_PRINTF_FUNC(1, 2);

static void GLES3_Log(const char *fmt, ...)
{
	char buf[1024];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (gles3_logger) {
		gles3_logger(buf);
	} else {
		fputs(buf, stderr);
	}
}

static QGL_GLES3_GetProcAddress_t gles3_getproc = NULL;

void QGL_GLES3_SetGetProcAddress(QGL_GLES3_GetProcAddress_t fn)
{
	gles3_getproc = fn;
}

/* The table is core entry points only.  An optional extension the renderer gates
 * at run time is resolved through this, so one resolver and its eglGetProcAddress
 * fallback stay the only path to a driver symbol. */
QGL_GLES3_GetProcAddress_t QGL_GLES3_GetGetProcAddress(void)
{
	return gles3_getproc;
}

int qglMajorVersion = 0, qglMinorVersion = 0;
int qglesMajorVersion = 0, qglesMinorVersion = 0;

static int gles3_core_missing = 0;
static int gles3_staged_traps = 0;
static int gles3_cap_drops = 0;

#define GLES3_MAX_MISSING 512
static char gles3_core_missing_names[GLES3_MAX_MISSING][40];
static int gles3_core_missing_stored = 0;
static char gles3_staged_names[GLES3_MAX_MISSING][40];
static int gles3_staged_stored = 0;

static int GLES3_SeenBefore(char (*tab)[40], int n, const char *row)
{
	int i;

	for (i = 0; i < n; i++) {
		if (strcmp(tab[i], row) == 0) {
			return 1;
		}
	}
	return 0;
}

static void GLES3_RecordMissing(const char *name, int core)
{
	char row[40];

	snprintf(row, sizeof(row), "gl%s", name);

	if (core) {
		gles3_core_missing++;
		if (gles3_core_missing_stored < GLES3_MAX_MISSING) {
			snprintf(gles3_core_missing_names[gles3_core_missing_stored],
					 sizeof(gles3_core_missing_names[0]), "%s", row);
			gles3_core_missing_stored++;
		}
		if (GLES3_SeenBefore(gles3_core_missing_names, gles3_core_missing_stored - 1, row)) {
			return;
		}
		GLES3_Log("[GLES3:glmissing] name=%s\n", row);
		return;
	}

	gles3_staged_traps++;
	if (gles3_staged_stored < GLES3_MAX_MISSING) {
		snprintf(gles3_staged_names[gles3_staged_stored],
				 sizeof(gles3_staged_names[0]), "%s", row);
		gles3_staged_stored++;
	}
	if (GLES3_SeenBefore(gles3_staged_names, gles3_staged_stored - 1, row)) {
		return;
	}
	GLES3_Log("[GLES3:glfixed] name=%s\n", row);
}

#define GLE(ret, name, ...) \
	static ret APIENTRY GLES3_trap_##name(__VA_ARGS__) \
	{ \
		GLES3_RecordMissing(#name, 0); \
		return (ret)0; \
	}
QGL_1_1_PROCS;
QGL_1_1_FIXED_FUNCTION_PROCS;
QGL_DESKTOP_1_1_PROCS;
QGL_DESKTOP_1_1_FIXED_FUNCTION_PROCS;
QGL_ES_1_1_PROCS;
QGL_ES_1_1_FIXED_FUNCTION_PROCS;
QGL_1_3_PROCS;
QGL_1_5_PROCS;
QGL_2_0_PROCS;
QGL_3_0_PROCS;
QGL_ARB_occlusion_query_PROCS;
QGL_ARB_framebuffer_object_PROCS;
QGL_ARB_vertex_array_object_PROCS;
QGL_EXT_direct_state_access_PROCS;
#undef GLE

#define GLE(ret, name, ...) \
	static ret APIENTRY GLES3_trap_##name(__VA_ARGS__) \
	{ \
		GLES3_RecordMissing(#name, 1); \
		return (ret)0; \
	}
QGL_GLES3_CORE_PROCS;
#undef GLE

/* Core in a LATER ES than the 3.0 set: trapped as staged, so a 3.0 context still comes up. */
#define GLE(ret, name, ...) \
	static ret APIENTRY GLES3_trap_##name(__VA_ARGS__) \
	{ \
		GLES3_RecordMissing(#name, 0); \
		return (ret)0; \
	}
QGL_GLES3_32_PROCS;
#undef GLE

/* An extension entry point the send path gates on: absent on a driver without
 * GL_EXT_buffer_storage, and the arm stands down rather than drawing from a map
 * the draw is not allowed to read. */
#define GLE(ret, name, ...) \
	static ret APIENTRY GLES3_trap_##name(__VA_ARGS__) \
	{ \
		GLES3_RecordMissing(#name, 0); \
		return (ret)0; \
	}
QGL_EXT_buffer_storage_PROCS;
#undef GLE

static void APIENTRY GLES3_trap_ActiveTextureARB(GLenum texture)
{
	(void)texture;
	GLES3_RecordMissing("ActiveTextureARB", 0);
}
static void APIENTRY GLES3_trap_ClientActiveTextureARB(GLenum texture)
{
	(void)texture;
	GLES3_RecordMissing("ClientActiveTextureARB", 0);
}
static void APIENTRY GLES3_trap_MultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t)
{
	(void)target; (void)s; (void)t;
	GLES3_RecordMissing("MultiTexCoord2fARB", 0);
}
static void APIENTRY GLES3_trap_LockArraysEXT(GLint first, GLsizei count)
{
	(void)first; (void)count;
	GLES3_RecordMissing("LockArraysEXT", 0);
}
static void APIENTRY GLES3_trap_UnlockArraysEXT(void)
{
	GLES3_RecordMissing("UnlockArraysEXT", 0);
}

#define GLE(ret, name, ...) ret (APIENTRYP qgl##name)(__VA_ARGS__) = NULL;
QGL_1_1_PROCS;
QGL_1_1_FIXED_FUNCTION_PROCS;
QGL_DESKTOP_1_1_PROCS;
QGL_DESKTOP_1_1_FIXED_FUNCTION_PROCS;
QGL_ES_1_1_PROCS;
QGL_ES_1_1_FIXED_FUNCTION_PROCS;
QGL_1_3_PROCS;
QGL_1_5_PROCS;
QGL_2_0_PROCS;
QGL_3_0_PROCS;
QGL_ARB_occlusion_query_PROCS;
QGL_ARB_framebuffer_object_PROCS;
QGL_ARB_vertex_array_object_PROCS;
QGL_EXT_direct_state_access_PROCS;
QGL_GLES3_CORE_PROCS;
QGL_GLES3_32_PROCS;
QGL_EXT_buffer_storage_PROCS;
#undef GLE

void (APIENTRYP qglActiveTextureARB)(GLenum texture) = NULL;
void (APIENTRYP qglClientActiveTextureARB)(GLenum texture) = NULL;
void (APIENTRYP qglMultiTexCoord2fARB)(GLenum target, GLfloat s, GLfloat t) = NULL;
void (APIENTRYP qglLockArraysEXT)(GLint first, GLsizei count) = NULL;
void (APIENTRYP qglUnlockArraysEXT)(void) = NULL;

#define GLE(ret, name, ...) + 1
static const int gles3_proc_total = 0
QGL_1_1_PROCS
QGL_1_1_FIXED_FUNCTION_PROCS
QGL_DESKTOP_1_1_PROCS
QGL_DESKTOP_1_1_FIXED_FUNCTION_PROCS
QGL_ES_1_1_PROCS
QGL_ES_1_1_FIXED_FUNCTION_PROCS
QGL_1_3_PROCS
QGL_1_5_PROCS
QGL_2_0_PROCS
QGL_3_0_PROCS
QGL_ARB_occlusion_query_PROCS
QGL_ARB_framebuffer_object_PROCS
QGL_ARB_vertex_array_object_PROCS
QGL_EXT_direct_state_access_PROCS
;
#undef GLE

#define GLE(ret, name, ...) + 1
static const int gles3_proc_core = 0
QGL_GLES3_CORE_PROCS
;
#undef GLE

#define GLE(ret, name, ...) + 1
static const int gles3_proc_core32 = 0
QGL_GLES3_32_PROCS
;
#undef GLE

int QGL_GLES3_TotalProcs(void) { return gles3_proc_total + 5; }
int QGL_GLES3_CoreProcs(void) { return gles3_proc_core; }
int QGL_GLES3_Core32Procs(void) { return gles3_proc_core32; }
int QGL_GLES3_CoreMissing(void) { return gles3_core_missing; }
int QGL_GLES3_StagedTraps(void) { return gles3_staged_traps; }
int QGL_GLES3_StagedCapDrops(void) { return gles3_cap_drops; }

const char *QGL_GLES3_CoreMissingList(void)
{
	static char list[GLES3_MAX_MISSING * 44];
	size_t used = 0;
	int i;

	list[0] = '\0';
	for (i = 0; i < gles3_core_missing_stored; i++) {
		used += (size_t)snprintf(list + used, sizeof(list) - used, "%s%s",
								 i ? "," : "", gles3_core_missing_names[i]);
	}
	return list;
}

static void GLES3_BindOne(const char *name, void **slot, void *trap, int core)
{
	void *p = gles3_getproc ? gles3_getproc(name) : NULL;

	if (p) {
		*slot = p;
		return;
	}
	*slot = trap;
	GLES3_RecordMissing(name + 2, core); 
}

static const char *GLES3_CapName(GLenum cap)
{
	switch (cap) {
	case GL_TEXTURE_2D:            return "GL_TEXTURE_2D";
	case GL_TEXTURE_1D:            return "GL_TEXTURE_1D";
	case GL_TEXTURE_3D:            return "GL_TEXTURE_3D";
	case GL_TEXTURE_CUBE_MAP:      return "GL_TEXTURE_CUBE_MAP";
	case GL_ALPHA_TEST:            return "GL_ALPHA_TEST";
	case GL_FOG:                   return "GL_FOG";
	case GL_LIGHTING:              return "GL_LIGHTING";
	case GL_LIGHT0:                return "GL_LIGHT0";
	case GL_LIGHT1:                return "GL_LIGHT1";
	case GL_LIGHT2:                return "GL_LIGHT2";
	case GL_LIGHT3:                return "GL_LIGHT3";
	case GL_LIGHT4:                return "GL_LIGHT4";
	case GL_LIGHT5:                return "GL_LIGHT5";
	case GL_LIGHT6:                return "GL_LIGHT6";
	case GL_LIGHT7:                return "GL_LIGHT7";
	case GL_CLIP_PLANE0:           return "GL_CLIP_PLANE0";
	case GL_CLIP_PLANE1:           return "GL_CLIP_PLANE1";
	case GL_CLIP_PLANE2:           return "GL_CLIP_PLANE2";
	case GL_CLIP_PLANE3:           return "GL_CLIP_PLANE3";
	case GL_CLIP_PLANE4:           return "GL_CLIP_PLANE4";
	case GL_CLIP_PLANE5:           return "GL_CLIP_PLANE5";
	case GL_LINE_STIPPLE:          return "GL_LINE_STIPPLE";
	case GL_TEXTURE_GEN_S:         return "GL_TEXTURE_GEN_S";
	case GL_TEXTURE_GEN_T:         return "GL_TEXTURE_GEN_T";
	case GL_TEXTURE_GEN_R:         return "GL_TEXTURE_GEN_R";
	case GL_NORMALIZE:             return "GL_NORMALIZE";
	case GL_RESCALE_NORMAL:        return "GL_RESCALE_NORMAL";
	case GL_COLOR_MATERIAL:        return "GL_COLOR_MATERIAL";
	case GL_POLYGON_STIPPLE:       return "GL_POLYGON_STIPPLE";
	case GL_INDEX_LOGIC_OP:        return "GL_INDEX_LOGIC_OP";
	case GL_AUTO_NORMAL:           return "GL_AUTO_NORMAL";
	default:                       return NULL;
	}
}

static void (APIENTRYP gles3_real_Enable)(GLenum cap) = NULL;
static void (APIENTRYP gles3_real_Disable)(GLenum cap) = NULL;
static void (APIENTRYP gles3_real_GetIntegerv)(GLenum pname, GLint *params) = NULL;

static void APIENTRY GLES3_Filter_Enable(GLenum cap)
{
	const char *nm = GLES3_CapName(cap);

	if (nm) {
		gles3_cap_drops++;
		GLES3_Log("[GLES3:glcap] name=glEnable cap=%s action=drop\n", nm);
		return;
	}
	if (gles3_real_Enable) {
		gles3_real_Enable(cap);
	}
}

static void APIENTRY GLES3_Filter_Disable(GLenum cap)
{
	const char *nm = GLES3_CapName(cap);

	if (nm) {
		gles3_cap_drops++;
		GLES3_Log("[GLES3:glcap] name=glDisable cap=%s action=drop\n", nm);
		return;
	}
	if (gles3_real_Disable) {
		gles3_real_Disable(cap);
	}
}

static void APIENTRY GLES3_Filter_GetIntegerv(GLenum pname, GLint *params)
{
	if (pname == GLES3_GL_MAX_TEXTURE_UNITS_ARB) {
		if (gles3_real_GetIntegerv) {
			gles3_real_GetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, params);
		}
		return;
	}
	if (gles3_real_GetIntegerv) {
		gles3_real_GetIntegerv(pname, params);
	}
}

static void APIENTRY GLES3_ClearDepth(GLclampd depth)
{
	if (qglClearDepthf) {
		qglClearDepthf((GLclampf)depth);
	}
}

static void APIENTRY GLES3_DepthRange(GLclampd near_val, GLclampd far_val)
{
	if (qglDepthRangef) {
		qglDepthRangef((GLclampf)near_val, (GLclampf)far_val);
	}
}

static void APIENTRY GLES3_Noop_DrawBuffer(GLenum mode)
{
	(void)mode; 
}

static void APIENTRY GLES3_Noop_PolygonMode(GLenum face, GLenum mode)
{
	(void)face; (void)mode; 
}

void QGL_GLES3_QueryVersion(void)
{
	const char *version;

	if (!qglGetString) {
		return;
	}
	version = (const char *)qglGetString(GL_VERSION);
	if (!version) {
		return;
	}
	if (strncmp(version, "OpenGL ES", 9) == 0) {
		if (sscanf(version, "OpenGL ES %d.%d", &qglesMajorVersion, &qglesMinorVersion) != 2) {
			qglesMajorVersion = qglesMinorVersion = 0;
		}
		qglMajorVersion = qglesMajorVersion;
		qglMinorVersion = qglesMinorVersion;
	} else {
		if (sscanf(version, "%d.%d", &qglMajorVersion, &qglMinorVersion) != 2) {
			qglMajorVersion = qglMinorVersion = 0;
		}
	}
}

int QGL_GLES3_HasExtension(const char *ext)
{
	GLint num = 0;
	GLint i;

	if (!ext || !qglGetString) {
		return 0;
	}
	if (qglGetStringi && qglGetIntegerv) {
		qglGetIntegerv(GL_NUM_EXTENSIONS, &num);
		for (i = 0; i < num; i++) {
			const char *e = (const char *)qglGetStringi(GL_EXTENSIONS, (GLuint)i);
			if (e && strcmp(e, ext) == 0) {
				return 1;
			}
		}
		return 0;
	}
	{
		const char *list = (const char *)qglGetString(GL_EXTENSIONS);
		return (list && strstr(list, ext)) ? 1 : 0;
	}
}

void QGL_GLES3_LogDeviceInfo(void)
{
	const char *version = qglGetString ? (const char *)qglGetString(GL_VERSION) : NULL;
	const char *vendor = qglGetString ? (const char *)qglGetString(GL_VENDOR) : NULL;
	const char *renderer = qglGetString ? (const char *)qglGetString(GL_RENDERER) : NULL;
	GLint numExt = 0;
	GLint maxTex = 0;
	GLint maxUnits = 0;

	if (qglGetStringi && qglGetIntegerv) {
		qglGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
	}
	if (qglGetIntegerv) {
		qglGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTex);
		qglGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits);
	}

	GLES3_Log("[GLES3:glinfo] gles=%d.%d vendor=\"%s\" renderer=\"%s\" version=\"%s\"\n",
			 qglesMajorVersion, qglesMinorVersion,
			 vendor ? vendor : "(null)", renderer ? renderer : "(null)",
			 version ? version : "(null)");
	GLES3_Log("[GLES3:glinfo] max_texture_size=%d max_texture_units=%d extensions=%d\n",
			 maxTex, maxUnits, numExt);
	// EF1 port: this engine has PRODUCT_VERSION/ARCH_STRING, not EF2's
	// q_version.h triple + git stamp.
	GLES3_Log("[GLES3:build] version=\"%s\" arch=\"%s\"\n",
			 Q3_VERSION, ARCH_STRING);
}

int QGL_GLES3_LoadProcs(int *staged)
{
	gles3_core_missing = 0;
	gles3_core_missing_stored = 0;
	gles3_staged_traps = 0;
	gles3_staged_stored = 0;
	gles3_cap_drops = 0;

#define GLE(ret, name, ...) GLES3_BindOne("gl" #name, (void **)&qgl##name, (void *)GLES3_trap_##name, 1);
	QGL_GLES3_CORE_PROCS;
#undef GLE

/* a later-ES core name is resolved through the same resolver and trapped the same
 * way, but its absence is not a missing core entry point */
#define GLE(ret, name, ...) GLES3_BindOne("gl" #name, (void **)&qgl##name, (void *)GLES3_trap_##name, 0);
	QGL_GLES3_32_PROCS;
#undef GLE

#define GLE(ret, name, ...) GLES3_BindOne("gl" #name, (void **)&qgl##name, (void *)GLES3_trap_##name, 0);
	QGL_1_1_PROCS;
	QGL_1_1_FIXED_FUNCTION_PROCS;
	QGL_DESKTOP_1_1_PROCS;
	QGL_DESKTOP_1_1_FIXED_FUNCTION_PROCS;
	QGL_ES_1_1_PROCS;
	QGL_ES_1_1_FIXED_FUNCTION_PROCS;
	QGL_1_3_PROCS;
	QGL_1_5_PROCS;
	QGL_2_0_PROCS;
	QGL_3_0_PROCS;
	QGL_ARB_occlusion_query_PROCS;
	QGL_ARB_framebuffer_object_PROCS;
	QGL_ARB_vertex_array_object_PROCS;
	QGL_EXT_direct_state_access_PROCS;
	QGL_EXT_buffer_storage_PROCS;
#undef GLE

	GLES3_BindOne("glActiveTextureARB", (void **)&qglActiveTextureARB, (void *)GLES3_trap_ActiveTextureARB, 0);
	GLES3_BindOne("glClientActiveTextureARB", (void **)&qglClientActiveTextureARB, (void *)GLES3_trap_ClientActiveTextureARB, 0);
	GLES3_BindOne("glMultiTexCoord2fARB", (void **)&qglMultiTexCoord2fARB, (void *)GLES3_trap_MultiTexCoord2fARB, 0);
	GLES3_BindOne("glLockArraysEXT", (void **)&qglLockArraysEXT, (void *)GLES3_trap_LockArraysEXT, 0);
	GLES3_BindOne("glUnlockArraysEXT", (void **)&qglUnlockArraysEXT, (void *)GLES3_trap_UnlockArraysEXT, 0);

	if (qglActiveTexture) {
		qglActiveTextureARB = (void (APIENTRYP)(GLenum))qglActiveTexture;
	}

	gles3_real_Enable = qglEnable;
	gles3_real_Disable = qglDisable;
	gles3_real_GetIntegerv = qglGetIntegerv;
	if (gles3_real_Enable) { qglEnable = GLES3_Filter_Enable; }
	if (gles3_real_Disable) { qglDisable = GLES3_Filter_Disable; }
	if (gles3_real_GetIntegerv) { qglGetIntegerv = GLES3_Filter_GetIntegerv; }
	if (qglClearDepthf) { qglClearDepth = GLES3_ClearDepth; }
	if (qglDepthRangef) { qglDepthRange = GLES3_DepthRange; }
	qglDrawBuffer = GLES3_Noop_DrawBuffer;
	qglPolygonMode = GLES3_Noop_PolygonMode;

	QGL_GLES3_QueryVersion();

	if (staged) {
		*staged = gles3_staged_traps;
	}
	return gles3_core_missing;
}
