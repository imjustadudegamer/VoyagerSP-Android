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
#ifndef GLES3_QGL_GLES3_H
#define GLES3_QGL_GLES3_H

#include "qgl.h"
#include "qgl_gles3_procs.h"
#include "qgl_gles3_tokens.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GLES3_PRINTF_FUNC
#	if defined(__GNUC__)
#		define GLES3_PRINTF_FUNC(f, v) __attribute__((format(printf, f, v)))
#	else
#		define GLES3_PRINTF_FUNC(f, v)
#	endif
#endif

#define GLE(ret, name, ...) typedef ret APIENTRY name##proc(__VA_ARGS__);
QGL_GLES3_CORE_PROCS;
QGL_GLES3_32_PROCS;
QGL_EXT_buffer_storage_PROCS;
#undef GLE

#ifdef __cplusplus
#define GLE(ret, name, ...) extern "C" name##proc *qgl##name;
#else
#define GLE(ret, name, ...) extern name##proc *qgl##name;
#endif
QGL_GLES3_CORE_PROCS;
QGL_GLES3_32_PROCS;
#undef GLE

#ifdef __cplusplus
#define GLE(ret, name, ...) extern "C" name##proc *qgl##name;
#else
#define GLE(ret, name, ...) extern name##proc *qgl##name;
#endif
/* GL_EXT_buffer_storage. Not core in ES 3.0/3.2, and the one optional entry point the send path
   gates on: a persistently mapped store is the only way to write a batch as a memcpy and still
   draw from the same buffer, because a non-persistent map forbids the draw that reads it. */
QGL_EXT_buffer_storage_PROCS;
#undef GLE

#ifdef __cplusplus
#define GLE(ret, name, ...) extern "C" name##proc *qgl##name;
#else
#define GLE(ret, name, ...) extern name##proc *qgl##name;
#endif
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


void QGL_GLES3_SetLogger(void (*fn)(const char *line));

typedef void *(*QGL_GLES3_GetProcAddress_t)(const char *name);
void QGL_GLES3_SetGetProcAddress(QGL_GLES3_GetProcAddress_t fn);
QGL_GLES3_GetProcAddress_t QGL_GLES3_GetGetProcAddress(void);

int QGL_GLES3_LoadProcs(int *staged);

int QGL_GLES3_TotalProcs(void);
int QGL_GLES3_CoreProcs(void);
int QGL_GLES3_Core32Procs(void);
int QGL_GLES3_CoreMissing(void);
int QGL_GLES3_StagedTraps(void);

void QGL_GLES3_QueryVersion(void);

const char *QGL_GLES3_CoreMissingList(void);

void QGL_GLES3_LogDeviceInfo(void);

int QGL_GLES3_HasExtension(const char *ext);

int QGL_GLES3_StagedCapDrops(void);

#define GLES3_GL_MAX_TEXTURE_UNITS_ARB 0x84E2

#ifdef __cplusplus
}
#endif

#endif 
