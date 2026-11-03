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
#ifndef GLES3_QGL_GLES3_TOKENS_H
#define GLES3_QGL_GLES3_TOKENS_H

// EF1 port: this token/alias block is only used by the GLES 3.2 module.
#if 1


#ifndef GL_CLAMP
#define GL_CLAMP GL_CLAMP_TO_EDGE
#endif

#ifndef GL_TEXTURE0_ARB
#define GL_TEXTURE0_ARB GL_TEXTURE0
#endif
#ifndef GL_TEXTURE1_ARB
#define GL_TEXTURE1_ARB GL_TEXTURE1
#endif
#ifndef GL_TEXTURE2_ARB
#define GL_TEXTURE2_ARB GL_TEXTURE2
#endif
#ifndef GL_TEXTURE3_ARB
#define GL_TEXTURE3_ARB GL_TEXTURE3
#endif
#ifndef GL_ACTIVE_TEXTURE_ARB
#define GL_ACTIVE_TEXTURE_ARB GL_ACTIVE_TEXTURE
#endif


#ifndef GL_TEXTURE_1D
#define GL_TEXTURE_1D 0x0DE0
#endif
#ifndef GL_TEXTURE_3D
#define GL_TEXTURE_3D 0x806F
#endif
#ifndef GL_TEXTURE_CUBE_MAP
#define GL_TEXTURE_CUBE_MAP 0x8513
#endif
#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST 0x0BC0
#endif
#ifndef GL_FOG
#define GL_FOG 0x0B60
#endif
#ifndef GL_FOG_START
#define GL_FOG_START 0x0B63
#endif
#ifndef GL_FOG_END
#define GL_FOG_END 0x0B64
#endif
#ifndef GL_FOG_MODE
#define GL_FOG_MODE 0x0B65
#endif
#ifndef GL_FOG_COLOR
#define GL_FOG_COLOR 0x0B66
#endif
#ifndef GL_LIGHTING
#define GL_LIGHTING 0x0B50
#endif
#ifndef GL_LIGHT0
#define GL_LIGHT0 0x4000
#endif
#ifndef GL_LIGHT1
#define GL_LIGHT1 0x4001
#endif
#ifndef GL_LIGHT2
#define GL_LIGHT2 0x4002
#endif
#ifndef GL_LIGHT3
#define GL_LIGHT3 0x4003
#endif
#ifndef GL_LIGHT4
#define GL_LIGHT4 0x4004
#endif
#ifndef GL_LIGHT5
#define GL_LIGHT5 0x4005
#endif
#ifndef GL_LIGHT6
#define GL_LIGHT6 0x4006
#endif
#ifndef GL_LIGHT7
#define GL_LIGHT7 0x4007
#endif
#ifndef GL_CLIP_PLANE0
#define GL_CLIP_PLANE0 0x3000
#endif
#ifndef GL_CLIP_PLANE1
#define GL_CLIP_PLANE1 0x3001
#endif
#ifndef GL_CLIP_PLANE2
#define GL_CLIP_PLANE2 0x3002
#endif
#ifndef GL_CLIP_PLANE3
#define GL_CLIP_PLANE3 0x3003
#endif
#ifndef GL_CLIP_PLANE4
#define GL_CLIP_PLANE4 0x3004
#endif
#ifndef GL_CLIP_PLANE5
#define GL_CLIP_PLANE5 0x3005
#endif
#ifndef GL_LINE_STIPPLE
#define GL_LINE_STIPPLE 0x0B24
#endif
#ifndef GL_TEXTURE_GEN_S
#define GL_TEXTURE_GEN_S 0x0C60
#endif
#ifndef GL_TEXTURE_GEN_T
#define GL_TEXTURE_GEN_T 0x0C61
#endif
#ifndef GL_TEXTURE_GEN_R
#define GL_TEXTURE_GEN_R 0x0C62
#endif
#ifndef GL_NORMALIZE
#define GL_NORMALIZE 0x0BA1
#endif
#ifndef GL_RESCALE_NORMAL
#define GL_RESCALE_NORMAL 0x803A
#endif
#ifndef GL_COLOR_MATERIAL
#define GL_COLOR_MATERIAL 0x0B57
#endif
#ifndef GL_POLYGON_STIPPLE
#define GL_POLYGON_STIPPLE 0x0B42
#endif
#ifndef GL_INDEX_LOGIC_OP
#define GL_INDEX_LOGIC_OP 0x0BF1
#endif
#ifndef GL_AUTO_NORMAL
#define GL_AUTO_NORMAL 0x0D80
#endif
#ifndef GL_MODELVIEW
#define GL_MODELVIEW 0x1700
#endif
#ifndef GL_PROJECTION
#define GL_PROJECTION 0x1701
#endif
#ifndef GL_TEXTURE
#define GL_TEXTURE 0x1702
#endif
#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif
#ifndef GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION 0x8E4C
#endif
#ifndef GL_POLYGON
#define GL_POLYGON 0x0009
#endif
#ifndef GL_VERTEX_ARRAY
#define GL_VERTEX_ARRAY 0x8074
#endif
#ifndef GL_COLOR_ARRAY
#define GL_COLOR_ARRAY 0x8076
#endif
#ifndef GL_TEXTURE_COORD_ARRAY
#define GL_TEXTURE_COORD_ARRAY 0x8078
#endif
#ifndef GL_NORMAL_ARRAY
#define GL_NORMAL_ARRAY 0x8075
#endif
#ifndef GL_TEXTURE_ENV
#define GL_TEXTURE_ENV 0x2300
#endif
#ifndef GL_TEXTURE_ENV_MODE
#define GL_TEXTURE_ENV_MODE 0x2200
#endif
#ifndef GL_MODULATE
#define GL_MODULATE 0x2100
#endif
#ifndef GL_DECAL
#define GL_DECAL 0x2101
#endif
#ifndef GL_ADD
#define GL_ADD 0x0104
#endif
#ifndef GL_COMBINE
#define GL_COMBINE 0x8570
#endif
#ifndef GL_COMBINE_RGB
#define GL_COMBINE_RGB 0x8571
#endif
#ifndef GL_COMBINE_ALPHA
#define GL_COMBINE_ALPHA 0x8572
#endif
#ifndef GL_SOURCE0_RGB
#define GL_SOURCE0_RGB 0x8580
#endif
#ifndef GL_SOURCE1_RGB
#define GL_SOURCE1_RGB 0x8581
#endif
#ifndef GL_SOURCE2_RGB
#define GL_SOURCE2_RGB 0x8582
#endif
#ifndef GL_SOURCE3_RGB
#define GL_SOURCE3_RGB 0x8583
#endif
#ifndef GL_SOURCE0_ALPHA
#define GL_SOURCE0_ALPHA 0x8588
#endif
#ifndef GL_SOURCE1_ALPHA
#define GL_SOURCE1_ALPHA 0x8589
#endif
#ifndef GL_SOURCE2_ALPHA
#define GL_SOURCE2_ALPHA 0x858A
#endif
#ifndef GL_SOURCE3_ALPHA
#define GL_SOURCE3_ALPHA 0x858B
#endif
#ifndef GL_OPERAND0_RGB
#define GL_OPERAND0_RGB 0x8590
#endif
#ifndef GL_OPERAND1_RGB
#define GL_OPERAND1_RGB 0x8591
#endif
#ifndef GL_OPERAND2_RGB
#define GL_OPERAND2_RGB 0x8592
#endif
#ifndef GL_OPERAND3_RGB
#define GL_OPERAND3_RGB 0x8593
#endif
#ifndef GL_OPERAND0_ALPHA
#define GL_OPERAND0_ALPHA 0x8598
#endif
#ifndef GL_OPERAND1_ALPHA
#define GL_OPERAND1_ALPHA 0x8599
#endif
#ifndef GL_OPERAND2_ALPHA
#define GL_OPERAND2_ALPHA 0x859A
#endif
#ifndef GL_OPERAND3_ALPHA
#define GL_OPERAND3_ALPHA 0x859B
#endif
#ifndef GL_INTERPOLATE
#define GL_INTERPOLATE 0x8575
#endif
#ifndef GL_PREVIOUS
#define GL_PREVIOUS 0x8578
#endif
#ifndef GL_PRIMARY_COLOR
#define GL_PRIMARY_COLOR 0x8577
#endif
#ifndef GL_COMBINE4_NV
#define GL_COMBINE4_NV 0x8503
#endif
#ifndef GL_SMOOTH
#define GL_SMOOTH 0x1D01
#endif
#ifndef GL_FLAT
#define GL_FLAT 0x1D00
#endif
#ifndef GL_FILL
#define GL_FILL 0x1B02
#endif
#ifndef GL_LINE
#define GL_LINE 0x1B01
#endif
#ifndef GL_POINT
#define GL_POINT 0x1B00
#endif
#ifndef GL_STENCIL_INDEX
#define GL_STENCIL_INDEX 0x1901
#endif
#ifndef GL_BACK_LEFT
#define GL_BACK_LEFT 0x0402
#endif
#ifndef GL_BACK_RIGHT
#define GL_BACK_RIGHT 0x0403
#endif
#ifndef GL_FRONT_LEFT
#define GL_FRONT_LEFT 0x0400
#endif
#ifndef GL_FRONT_RIGHT
#define GL_FRONT_RIGHT 0x0401
#endif
#ifndef GL_BGR
#define GL_BGR 0x80E0
#endif
#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif
#ifndef GL_BGR_EXT
#define GL_BGR_EXT 0x80E0
#endif
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif
#ifndef GL_STACK_OVERFLOW
#define GL_STACK_OVERFLOW 0x0503
#endif
#ifndef GL_STACK_UNDERFLOW
#define GL_STACK_UNDERFLOW 0x0504
#endif
#ifndef GL_RGB4
#define GL_RGB4 0x804F
#endif
#ifndef GL_RGB5
#define GL_RGB5 0x8050
#endif
#ifndef GL_RGB10
#define GL_RGB10 0x8052
#endif
#ifndef GL_RGB12
#define GL_RGB12 0x8053
#endif
#ifndef GL_RGBA2
#define GL_RGBA2 0x8055
#endif

#if defined( GL_EXT_texture_compression_s3tc )
#ifndef GL_COMPRESSED_RGB_S3TC_DXT1
#define GL_COMPRESSED_RGB_S3TC_DXT1 GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1
#define GL_COMPRESSED_RGBA_S3TC_DXT1 GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3
#define GL_COMPRESSED_RGBA_S3TC_DXT3 GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5
#define GL_COMPRESSED_RGBA_S3TC_DXT5 GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#endif
#ifndef GL_RGB4_S3TC
#define GL_RGB4_S3TC GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#endif
#ifndef GL_RGBA4_S3TC
#define GL_RGBA4_S3TC GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#endif
#endif 

#endif 

#endif 
