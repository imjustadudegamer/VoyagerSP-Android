/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/*****************************************************************************
 * name:		cl_cin.c
 *
 * desc:		video and cinematic playback
 *
 * $Archive: /MissionPack/code/client/cl_cin.c $
 *
 * cl_glconfig.hwtype trtypes 3dfx/ragepro need 256x256
 *
 *****************************************************************************/

#include "client.h"
#include "snd_local.h"
#include "../../sp/bink_ff.h"   // EF1 SP: Bink (.bik) movie support via FFmpeg
#include "../../sp/sp_ui.h"     // EF1 SP native UI (libefui.so) drive hooks

#define MAXSIZE				8
#define MINSIZE				4

#define DEFAULT_CIN_WIDTH	512
#define DEFAULT_CIN_HEIGHT	512

#define ROQ_QUAD			0x1000
#define ROQ_QUAD_INFO		0x1001
#define ROQ_CODEBOOK		0x1002
#define ROQ_QUAD_VQ			0x1011
#define ROQ_QUAD_JPEG		0x1012
#define ROQ_QUAD_HANG		0x1013
#define ROQ_PACKET			0x1030
#define ZA_SOUND_MONO		0x1020
#define ZA_SOUND_STEREO		0x1021

#define MAX_VIDEO_HANDLES	16


static void RoQ_init( void );

/******************************************************************************
*
* Class:		trFMV
*
* Description:	RoQ/RnR manipulation routines
*				not entirely complete for first run
*
******************************************************************************/

static	long				ROQ_YY_tab[256];
static	long				ROQ_UB_tab[256];
static	long				ROQ_UG_tab[256];
static	long				ROQ_VG_tab[256];
static	long				ROQ_VR_tab[256];
static	unsigned short		vq2[256*16*4];
static	unsigned short		vq4[256*64*4];
static	unsigned short		vq8[256*256*4];


typedef struct {
	byte				linbuf[DEFAULT_CIN_WIDTH*DEFAULT_CIN_HEIGHT*4*2];
	byte				file[65536];
	short				sqrTable[256];

	int		mcomp[256];
	byte				*qStatus[2][32768];

	long				oldXOff, oldYOff, oldysize, oldxsize;

	int					currentHandle;
} cinematics_t;

typedef struct {
	char				fileName[MAX_OSPATH];
	int					CIN_WIDTH, CIN_HEIGHT;
	int					xpos, ypos, width, height;
	qboolean			looping, holdAtEnd, dirty, alterGameState, silent, shader;
	fileHandle_t		iFile;
	e_status			status;
	int					startTime;
	int					lastTime;
	long				tfps;
	long				RoQPlayed;
	long				ROQSize;
	unsigned int		RoQFrameSize;
	long				onQuad;
	long				numQuads;
	long				samplesPerLine;
	unsigned int		roq_id;
	long				screenDelta;

	void ( *VQ0)(byte *status, void *qdata );
	void ( *VQ1)(byte *status, void *qdata );
	void ( *VQNormal)(byte *status, void *qdata );
	void ( *VQBuffer)(byte *status, void *qdata );

	long				samplesPerPixel;				// defaults to 2
	byte*				gray;
	unsigned int		xsize, ysize, maxsize, minsize;

	qboolean			half, smootheddouble, inMemory;
	long				normalBuffer0;
	long				roq_flags;
	long				roqF0;
	long				roqF1;
	long				t[2];
	long				roqFPS;
	int					playonwalls;
	byte*				buf;
	long				drawX, drawY;
	int					module;
	qboolean			isBink;			// EF1 SP: this handle is a Bink (.bik) movie
	// EF1 SP 1:1: retail in-game-cinematic flag, latched from the CIN_PlayCinematic
	// in-game parameter. When set, this .bik is an in-level
	// OVERLAY: clc.state stays CA_ACTIVE, cl_paused=1, the sim freezes, the video draws over
	// the paused world, and skip/EOF unpause+resume the mission (NEVER disconnect / nextmap).
	qboolean			inGameCinematic;
	// EF1 SP 1:1 video fade. The retail engine latches cl_VidFadeUp/Down at play time and
	// ramps the DrawStretchRaw alpha 0->1 / 1->0 over 1000ms. The retail re.DrawStretchRaw
	// took an alpha; the port's does not, so the fade is reproduced as a black overlay in
	// CIN_DrawCinematic.
	qboolean			vidFadeUp;		// cl_VidFadeUp latched at play
	qboolean			vidFadeDown;	// cl_VidFadeDown latched at play
	int					fadeDownStart;	// CL_ScaledMilliseconds() when the last frame was reached
} cin_cache;

static cinematics_t		cin;
static cin_cache		cinTable[MAX_VIDEO_HANDLES];
static int				currentHandle = -1;
static int				CL_handle = -1;

extern int				s_soundtime;		// sample PAIRS


void CIN_CloseAllVideos(void) {
	int		i;

	for ( i = 0 ; i < MAX_VIDEO_HANDLES ; i++ ) {
		if (cinTable[i].fileName[0] != 0 ) {
			CIN_StopCinematic(i);
		}
	}
}


static int CIN_HandleForVideo(void) {
	int		i;

	for ( i = 0 ; i < MAX_VIDEO_HANDLES ; i++ ) {
		if ( cinTable[i].fileName[0] == 0 ) {
			return i;
		}
	}
	Com_Error( ERR_DROP, "CIN_HandleForVideo: none free" );
	return -1;
}


extern int CL_ScaledMilliseconds(void);

//-----------------------------------------------------------------------------
// RllSetupTable
//
// Allocates and initializes the square table.
//
// Parameters:	None
//
// Returns:		Nothing
//-----------------------------------------------------------------------------
static void RllSetupTable( void )
{
	int z;

	for (z=0;z<128;z++) {
		cin.sqrTable[z] = (short)(z*z);
		cin.sqrTable[z+128] = (short)(-cin.sqrTable[z]);
	}
}



//-----------------------------------------------------------------------------
// RllDecodeMonoToMono
//
// Decode mono source data into a mono buffer.
//
// Parameters:	from -> buffer holding encoded data
//				to ->	buffer to hold decoded data
//				size =	number of bytes of input (= # of shorts of output)
//				signedOutput = 0 for unsigned output, non-zero for signed output
//				flag = flags from asset header
//
// Returns:		Number of samples placed in output buffer
//-----------------------------------------------------------------------------
long RllDecodeMonoToMono(unsigned char *from,short *to,unsigned int size,char signedOutput ,unsigned short flag)
{
	unsigned int z;
	int prev;
	
	if (signedOutput)	
		prev =  flag - 0x8000;
	else 
		prev = flag;

	for (z=0;z<size;z++) {
		prev = to[z] = (short)(prev + cin.sqrTable[from[z]]); 
	}
	return size;	//*sizeof(short));
}


//-----------------------------------------------------------------------------
// RllDecodeMonoToStereo
//
// Decode mono source data into a stereo buffer. Output is 4 times the number
// of bytes in the input.
//
// Parameters:	from -> buffer holding encoded data
//				to ->	buffer to hold decoded data
//				size =	number of bytes of input (= 1/4 # of bytes of output)
//				signedOutput = 0 for unsigned output, non-zero for signed output
//				flag = flags from asset header
//
// Returns:		Number of samples placed in output buffer
//-----------------------------------------------------------------------------
long RllDecodeMonoToStereo(unsigned char *from,short *to,unsigned int size,char signedOutput,unsigned short flag)
{
	unsigned int z;
	int prev;
	
	if (signedOutput)	
		prev =  flag - 0x8000;
	else 
		prev = flag;

	for (z = 0; z < size; z++) {
		prev = (short)(prev + cin.sqrTable[from[z]]);
		to[z*2+0] = to[z*2+1] = (short)(prev);
	}
	
	return size;	// * 2 * sizeof(short));
}


//-----------------------------------------------------------------------------
// RllDecodeStereoToStereo
//
// Decode stereo source data into a stereo buffer.
//
// Parameters:	from -> buffer holding encoded data
//				to ->	buffer to hold decoded data
//				size =	number of bytes of input (= 1/2 # of bytes of output)
//				signedOutput = 0 for unsigned output, non-zero for signed output
//				flag = flags from asset header
//
// Returns:		Number of samples placed in output buffer
//-----------------------------------------------------------------------------
long RllDecodeStereoToStereo(unsigned char *from,short *to,unsigned int size,char signedOutput, unsigned short flag)
{
	unsigned int z;
	unsigned char *zz = from;
	int	prevL, prevR;

	if (signedOutput) {
		prevL = (flag & 0xff00) - 0x8000;
		prevR = ((flag & 0x00ff) << 8) - 0x8000;
	} else {
		prevL = flag & 0xff00;
		prevR = (flag & 0x00ff) << 8;
	}

	for (z=0;z<size;z+=2) {
                prevL = (short)(prevL + cin.sqrTable[*zz++]); 
                prevR = (short)(prevR + cin.sqrTable[*zz++]);
                to[z+0] = (short)(prevL);
                to[z+1] = (short)(prevR);
	}
	
	return (size>>1);	//*sizeof(short));
}


//-----------------------------------------------------------------------------
// RllDecodeStereoToMono
//
// Decode stereo source data into a mono buffer.
//
// Parameters:	from -> buffer holding encoded data
//				to ->	buffer to hold decoded data
//				size =	number of bytes of input (= # of bytes of output)
//				signedOutput = 0 for unsigned output, non-zero for signed output
//				flag = flags from asset header
//
// Returns:		Number of samples placed in output buffer
//-----------------------------------------------------------------------------
long RllDecodeStereoToMono(unsigned char *from,short *to,unsigned int size,char signedOutput, unsigned short flag)
{
	unsigned int z;
	int prevL,prevR;
	
	if (signedOutput) {
		prevL = (flag & 0xff00) - 0x8000;
		prevR = ((flag & 0x00ff) << 8) -0x8000;
	} else {
		prevL = flag & 0xff00;
		prevR = (flag & 0x00ff) << 8;
	}

	for (z=0;z<size;z+=1) {
		prevL= prevL + cin.sqrTable[from[z*2]];
		prevR = prevR + cin.sqrTable[from[z*2+1]];
		to[z] = (short)((prevL + prevR)/2);
	}

	return size;
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void move8_32( byte *src, byte *dst, int spl )
{
	int i;

	for(i = 0; i < 8; ++i)
	{
		memcpy(dst, src, 32);
		src += spl;
		dst += spl;
	}
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void move4_32( byte *src, byte *dst, int spl  )
{
	int i;

	for(i = 0; i < 4; ++i)
	{
		memcpy(dst, src, 16);
		src += spl;
		dst += spl;
	}
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void blit8_32( byte *src, byte *dst, int spl  )
{
	int i;

	for(i = 0; i < 8; ++i)
	{
		memcpy(dst, src, 32);
		src += 32;
		dst += spl;
	}
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/
static void blit4_32( byte *src, byte *dst, int spl  )
{
	int i;

	for(i = 0; i < 4; ++i)
	{
		memmove(dst, src, 16);
		src += 16;
		dst += spl;
	}
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void blit2_32( byte *src, byte *dst, int spl  )
{
	memcpy(dst, src, 8);
	memcpy(dst+spl, src+8, 8);
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void blitVQQuad32fs( byte **status, unsigned char *data )
{
unsigned short	newd, celdata, code;
unsigned int	index, i;
int		spl;

	newd	= 0;
	celdata = 0;
	index	= 0;
	
        spl = cinTable[currentHandle].samplesPerLine;
        
	do {
		if (!newd) { 
			newd = 7;
			celdata = data[0] + data[1]*256;
			data += 2;
		} else {
			newd--;
		}

		code = (unsigned short)(celdata&0xc000); 
		celdata <<= 2;
		
		switch (code) {
			case	0x8000:													// vq code
				blit8_32( (byte *)&vq8[(*data)*128], status[index], spl );
				data++;
				index += 5;
				break;
			case	0xc000:													// drop
				index++;													// skip 8x8
				for(i=0;i<4;i++) {
					if (!newd) { 
						newd = 7;
						celdata = data[0] + data[1]*256;
						data += 2;
					} else {
						newd--;
					}
						
					code = (unsigned short)(celdata&0xc000); celdata <<= 2; 

					switch (code) {											// code in top two bits of code
						case	0x8000:										// 4x4 vq code
							blit4_32( (byte *)&vq4[(*data)*32], status[index], spl );
							data++;
							break;
						case	0xc000:										// 2x2 vq code
							blit2_32( (byte *)&vq2[(*data)*8], status[index], spl );
							data++;
							blit2_32( (byte *)&vq2[(*data)*8], status[index]+8, spl );
							data++;
							blit2_32( (byte *)&vq2[(*data)*8], status[index]+spl*2, spl );
							data++;
							blit2_32( (byte *)&vq2[(*data)*8], status[index]+spl*2+8, spl );
							data++;
							break;
						case	0x4000:										// motion compensation
							move4_32( status[index] + cin.mcomp[(*data)], status[index], spl );
							data++;
							break;
					}
					index++;
				}
				break;
			case	0x4000:													// motion compensation
				move8_32( status[index] + cin.mcomp[(*data)], status[index], spl );
				data++;
				index += 5;
				break;
			case	0x0000:
				index += 5;
				break;
		}
	} while ( status[index] != NULL );
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void ROQ_GenYUVTables( void )
{
	float t_ub,t_vr,t_ug,t_vg;
	long i;

	t_ub = (1.77200f/2.0f) * (float)(1<<6) + 0.5f;
	t_vr = (1.40200f/2.0f) * (float)(1<<6) + 0.5f;
	t_ug = (0.34414f/2.0f) * (float)(1<<6) + 0.5f;
	t_vg = (0.71414f/2.0f) * (float)(1<<6) + 0.5f;
	for(i=0;i<256;i++) {
		float x = (float)(2 * i - 255);
	
		ROQ_UB_tab[i] = (long)( ( t_ub * x) + (1<<5));
		ROQ_VR_tab[i] = (long)( ( t_vr * x) + (1<<5));
		ROQ_UG_tab[i] = (long)( (-t_ug * x)		 );
		ROQ_VG_tab[i] = (long)( (-t_vg * x) + (1<<5));
		ROQ_YY_tab[i] = (long)( (i << 6) | (i >> 2) );
	}
}

#define VQ2TO4(a,b,c,d) { \
    	*c++ = a[0];	\
	*d++ = a[0];	\
	*d++ = a[0];	\
	*c++ = a[1];	\
	*d++ = a[1];	\
	*d++ = a[1];	\
	*c++ = b[0];	\
	*d++ = b[0];	\
	*d++ = b[0];	\
	*c++ = b[1];	\
	*d++ = b[1];	\
	*d++ = b[1];	\
	*d++ = a[0];	\
	*d++ = a[0];	\
	*d++ = a[1];	\
	*d++ = a[1];	\
	*d++ = b[0];	\
	*d++ = b[0];	\
	*d++ = b[1];	\
	*d++ = b[1];	\
	a += 2; b += 2; }
 
#define VQ2TO2(a,b,c,d) { \
	*c++ = *a;	\
	*d++ = *a;	\
	*d++ = *a;	\
	*c++ = *b;	\
	*d++ = *b;	\
	*d++ = *b;	\
	*d++ = *a;	\
	*d++ = *a;	\
	*d++ = *b;	\
	*d++ = *b;	\
	a++; b++; }

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static unsigned short yuv_to_rgb( long y, long u, long v )
{ 
	long r,g,b,YY = (long)(ROQ_YY_tab[(y)]);

	r = (YY + ROQ_VR_tab[v]) >> 9;
	g = (YY + ROQ_UG_tab[u] + ROQ_VG_tab[v]) >> 8;
	b = (YY + ROQ_UB_tab[u]) >> 9;
	
	if (r<0) r = 0;
	if (g<0) g = 0;
	if (b<0) b = 0;
	if (r > 31) r = 31;
	if (g > 63) g = 63;
	if (b > 31) b = 31;

	return (unsigned short)((r<<11)+(g<<5)+(b));
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/
static unsigned int yuv_to_rgb24( long y, long u, long v )
{ 
	long r,g,b,YY = (long)(ROQ_YY_tab[(y)]);

	r = (YY + ROQ_VR_tab[v]) >> 6;
	g = (YY + ROQ_UG_tab[u] + ROQ_VG_tab[v]) >> 6;
	b = (YY + ROQ_UB_tab[u]) >> 6;
	
	if (r<0) r = 0;
	if (g<0) g = 0;
	if (b<0) b = 0;
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	
	return LittleLong ((unsigned long)((r)|(g<<8)|(b<<16))|(255UL<<24));
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void decodeCodeBook( byte *input, unsigned short roq_flags )
{
	long	i, j, two, four;
	unsigned short	*aptr, *bptr, *cptr, *dptr;
	long	y0,y1,y2,y3,cr,cb;
	byte	*bbptr, *baptr, *bcptr, *bdptr;
	union {
		unsigned int *i;
		unsigned short *s;
	} iaptr, ibptr, icptr, idptr;

	if (!roq_flags) {
		two = four = 256;
	} else {
		two  = roq_flags>>8;
		if (!two) two = 256;
		four = roq_flags&0xff;
	}

	four *= 2;

	bptr = (unsigned short *)vq2;

	if (!cinTable[currentHandle].half) {
		if (!cinTable[currentHandle].smootheddouble) {
//
// normal height
//
			if (cinTable[currentHandle].samplesPerPixel==2) {
				for(i=0;i<two;i++) {
					y0 = (long)*input++;
					y1 = (long)*input++;
					y2 = (long)*input++;
					y3 = (long)*input++;
					cr = (long)*input++;
					cb = (long)*input++;
					*bptr++ = yuv_to_rgb( y0, cr, cb );
					*bptr++ = yuv_to_rgb( y1, cr, cb );
					*bptr++ = yuv_to_rgb( y2, cr, cb );
					*bptr++ = yuv_to_rgb( y3, cr, cb );
				}

				cptr = (unsigned short *)vq4;
				dptr = (unsigned short *)vq8;
		
				for(i=0;i<four;i++) {
					aptr = (unsigned short *)vq2 + (*input++)*4;
					bptr = (unsigned short *)vq2 + (*input++)*4;
					for(j=0;j<2;j++)
						VQ2TO4(aptr,bptr,cptr,dptr);
				}
			} else if (cinTable[currentHandle].samplesPerPixel==4) {
				ibptr.s = bptr;
				for(i=0;i<two;i++) {
					y0 = (long)*input++;
					y1 = (long)*input++;
					y2 = (long)*input++;
					y3 = (long)*input++;
					cr = (long)*input++;
					cb = (long)*input++;
					*ibptr.i++ = yuv_to_rgb24( y0, cr, cb );
					*ibptr.i++ = yuv_to_rgb24( y1, cr, cb );
					*ibptr.i++ = yuv_to_rgb24( y2, cr, cb );
					*ibptr.i++ = yuv_to_rgb24( y3, cr, cb );
				}

				icptr.s = vq4;
				idptr.s = vq8;
	
				for(i=0;i<four;i++) {
					iaptr.s = vq2;
					iaptr.i += (*input++)*4;
					ibptr.s = vq2;
					ibptr.i += (*input++)*4;
					for(j=0;j<2;j++) 
						VQ2TO4(iaptr.i, ibptr.i, icptr.i, idptr.i);
				}
			} else if (cinTable[currentHandle].samplesPerPixel==1) {
				bbptr = (byte *)bptr;
				for(i=0;i<two;i++) {
					*bbptr++ = cinTable[currentHandle].gray[*input++];
					*bbptr++ = cinTable[currentHandle].gray[*input++];
					*bbptr++ = cinTable[currentHandle].gray[*input++];
					*bbptr++ = cinTable[currentHandle].gray[*input]; input +=3;
				}

				bcptr = (byte *)vq4;
				bdptr = (byte *)vq8;
	
				for(i=0;i<four;i++) {
					baptr = (byte *)vq2 + (*input++)*4;
					bbptr = (byte *)vq2 + (*input++)*4;
					for(j=0;j<2;j++) 
						VQ2TO4(baptr,bbptr,bcptr,bdptr);
				}
			}
		} else {
//
// double height, smoothed
//
			if (cinTable[currentHandle].samplesPerPixel==2) {
				for(i=0;i<two;i++) {
					y0 = (long)*input++;
					y1 = (long)*input++;
					y2 = (long)*input++;
					y3 = (long)*input++;
					cr = (long)*input++;
					cb = (long)*input++;
					*bptr++ = yuv_to_rgb( y0, cr, cb );
					*bptr++ = yuv_to_rgb( y1, cr, cb );
					*bptr++ = yuv_to_rgb( ((y0*3)+y2)/4, cr, cb );
					*bptr++ = yuv_to_rgb( ((y1*3)+y3)/4, cr, cb );
					*bptr++ = yuv_to_rgb( (y0+(y2*3))/4, cr, cb );
					*bptr++ = yuv_to_rgb( (y1+(y3*3))/4, cr, cb );
					*bptr++ = yuv_to_rgb( y2, cr, cb );
					*bptr++ = yuv_to_rgb( y3, cr, cb );
				}

				cptr = (unsigned short *)vq4;
				dptr = (unsigned short *)vq8;
		
				for(i=0;i<four;i++) {
					aptr = (unsigned short *)vq2 + (*input++)*8;
					bptr = (unsigned short *)vq2 + (*input++)*8;
					for(j=0;j<2;j++) {
						VQ2TO4(aptr,bptr,cptr,dptr);
						VQ2TO4(aptr,bptr,cptr,dptr);
					}
				}
			} else if (cinTable[currentHandle].samplesPerPixel==4) {
				ibptr.s = bptr;
				for(i=0;i<two;i++) {
					y0 = (long)*input++;
					y1 = (long)*input++;
					y2 = (long)*input++;
					y3 = (long)*input++;
					cr = (long)*input++;
					cb = (long)*input++;
					*ibptr.i++ = yuv_to_rgb24( y0, cr, cb );
					*ibptr.i++ = yuv_to_rgb24( y1, cr, cb );
					*ibptr.i++ = yuv_to_rgb24( ((y0*3)+y2)/4, cr, cb );
					*ibptr.i++ = yuv_to_rgb24( ((y1*3)+y3)/4, cr, cb );
					*ibptr.i++ = yuv_to_rgb24( (y0+(y2*3))/4, cr, cb );
					*ibptr.i++ = yuv_to_rgb24( (y1+(y3*3))/4, cr, cb );
					*ibptr.i++ = yuv_to_rgb24( y2, cr, cb );
					*ibptr.i++ = yuv_to_rgb24( y3, cr, cb );
				}

				icptr.s = vq4;
				idptr.s = vq8;
	
				for(i=0;i<four;i++) {
					iaptr.s = vq2;
					iaptr.i += (*input++)*8;
					ibptr.s = vq2;
					ibptr.i += (*input++)*8;
					for(j=0;j<2;j++) {
						VQ2TO4(iaptr.i, ibptr.i, icptr.i, idptr.i);
						VQ2TO4(iaptr.i, ibptr.i, icptr.i, idptr.i);
					}
				}
			} else if (cinTable[currentHandle].samplesPerPixel==1) {
				bbptr = (byte *)bptr;
				for(i=0;i<two;i++) {
					y0 = (long)*input++;
					y1 = (long)*input++;
					y2 = (long)*input++;
					y3 = (long)*input; input+= 3;
					*bbptr++ = cinTable[currentHandle].gray[y0];
					*bbptr++ = cinTable[currentHandle].gray[y1];
					*bbptr++ = cinTable[currentHandle].gray[((y0*3)+y2)/4];
					*bbptr++ = cinTable[currentHandle].gray[((y1*3)+y3)/4];
					*bbptr++ = cinTable[currentHandle].gray[(y0+(y2*3))/4];
					*bbptr++ = cinTable[currentHandle].gray[(y1+(y3*3))/4];						
					*bbptr++ = cinTable[currentHandle].gray[y2];
					*bbptr++ = cinTable[currentHandle].gray[y3];
				}

				bcptr = (byte *)vq4;
				bdptr = (byte *)vq8;
	
				for(i=0;i<four;i++) {
					baptr = (byte *)vq2 + (*input++)*8;
					bbptr = (byte *)vq2 + (*input++)*8;
					for(j=0;j<2;j++) {
						VQ2TO4(baptr,bbptr,bcptr,bdptr);
						VQ2TO4(baptr,bbptr,bcptr,bdptr);
					}
				}
			}			
		}
	} else {
//
// 1/4 screen
//
		if (cinTable[currentHandle].samplesPerPixel==2) {
			for(i=0;i<two;i++) {
				y0 = (long)*input; input+=2;
				y2 = (long)*input; input+=2;
				cr = (long)*input++;
				cb = (long)*input++;
				*bptr++ = yuv_to_rgb( y0, cr, cb );
				*bptr++ = yuv_to_rgb( y2, cr, cb );
			}

			cptr = (unsigned short *)vq4;
			dptr = (unsigned short *)vq8;
	
			for(i=0;i<four;i++) {
				aptr = (unsigned short *)vq2 + (*input++)*2;
				bptr = (unsigned short *)vq2 + (*input++)*2;
				for(j=0;j<2;j++) { 
					VQ2TO2(aptr,bptr,cptr,dptr);
				}
			}
		} else if (cinTable[currentHandle].samplesPerPixel == 1) {
			bbptr = (byte *)bptr;
				
			for(i=0;i<two;i++) {
				*bbptr++ = cinTable[currentHandle].gray[*input]; input+=2;
				*bbptr++ = cinTable[currentHandle].gray[*input]; input+=4;
			}

			bcptr = (byte *)vq4;
			bdptr = (byte *)vq8;
	
			for(i=0;i<four;i++) {
				baptr = (byte *)vq2 + (*input++)*2;
				bbptr = (byte *)vq2 + (*input++)*2;
				for(j=0;j<2;j++) { 
					VQ2TO2(baptr,bbptr,bcptr,bdptr);
				}
			}			
		} else if (cinTable[currentHandle].samplesPerPixel == 4) {
			ibptr.s = bptr;
			for(i=0;i<two;i++) {
				y0 = (long)*input; input+=2;
				y2 = (long)*input; input+=2;
				cr = (long)*input++;
				cb = (long)*input++;
				*ibptr.i++ = yuv_to_rgb24( y0, cr, cb );
				*ibptr.i++ = yuv_to_rgb24( y2, cr, cb );
			}

			icptr.s = vq4;
			idptr.s = vq8;
	
			for(i=0;i<four;i++) {
				iaptr.s = vq2;
				iaptr.i += (*input++)*2;
				ibptr.s = vq2 + (*input++)*2;
				ibptr.i += (*input++)*2;
				for(j=0;j<2;j++) { 
					VQ2TO2(iaptr.i,ibptr.i,icptr.i,idptr.i);
				}
			}
		}
	}
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void recurseQuad( long startX, long startY, long quadSize, long xOff, long yOff )
{
	byte *scroff;
	long bigx, bigy, lowx, lowy, useY;
	long offset;

	offset = cinTable[currentHandle].screenDelta;
	
	lowx = lowy = 0;
	bigx = cinTable[currentHandle].xsize;
	bigy = cinTable[currentHandle].ysize;

	if (bigx > cinTable[currentHandle].CIN_WIDTH) bigx = cinTable[currentHandle].CIN_WIDTH;
	if (bigy > cinTable[currentHandle].CIN_HEIGHT) bigy = cinTable[currentHandle].CIN_HEIGHT;

	if ( (startX >= lowx) && (startX+quadSize) <= (bigx) && (startY+quadSize) <= (bigy) && (startY >= lowy) && quadSize <= MAXSIZE) {
		useY = startY;
		scroff = cin.linbuf + (useY+((cinTable[currentHandle].CIN_HEIGHT-bigy)>>1)+yOff)*(cinTable[currentHandle].samplesPerLine) + (((startX+xOff))*cinTable[currentHandle].samplesPerPixel);

		cin.qStatus[0][cinTable[currentHandle].onQuad  ] = scroff;
		cin.qStatus[1][cinTable[currentHandle].onQuad++] = scroff+offset;
	}

	if ( quadSize != MINSIZE ) {
		quadSize >>= 1;
		recurseQuad( startX,		  startY		  , quadSize, xOff, yOff );
		recurseQuad( startX+quadSize, startY		  , quadSize, xOff, yOff );
		recurseQuad( startX,		  startY+quadSize , quadSize, xOff, yOff );
		recurseQuad( startX+quadSize, startY+quadSize , quadSize, xOff, yOff );
	}
}


/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void setupQuad( long xOff, long yOff )
{
	long numQuadCels, i,x,y;
	byte *temp;

	if (xOff == cin.oldXOff && yOff == cin.oldYOff && cinTable[currentHandle].ysize == cin.oldysize && cinTable[currentHandle].xsize == cin.oldxsize) {
		return;
	}

	cin.oldXOff = xOff;
	cin.oldYOff = yOff;
	cin.oldysize = cinTable[currentHandle].ysize;
	cin.oldxsize = cinTable[currentHandle].xsize;

	numQuadCels  = (cinTable[currentHandle].xsize*cinTable[currentHandle].ysize) / (16);
	numQuadCels += numQuadCels/4;
	numQuadCels += 64;							  // for overflow

	cinTable[currentHandle].onQuad = 0;

	for(y=0;y<(long)cinTable[currentHandle].ysize;y+=16) 
		for(x=0;x<(long)cinTable[currentHandle].xsize;x+=16) 
			recurseQuad( x, y, 16, xOff, yOff );

	temp = NULL;

	for(i=(numQuadCels-64);i<numQuadCels;i++) {
		cin.qStatus[0][i] = temp;			  // eoq
		cin.qStatus[1][i] = temp;			  // eoq
	}
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void readQuadInfo( byte *qData )
{
	if (currentHandle < 0) return;

	cinTable[currentHandle].xsize    = qData[0]+qData[1]*256;
	cinTable[currentHandle].ysize    = qData[2]+qData[3]*256;
	cinTable[currentHandle].maxsize  = qData[4]+qData[5]*256;
	cinTable[currentHandle].minsize  = qData[6]+qData[7]*256;
	
	cinTable[currentHandle].CIN_HEIGHT = cinTable[currentHandle].ysize;
	cinTable[currentHandle].CIN_WIDTH  = cinTable[currentHandle].xsize;

	cinTable[currentHandle].samplesPerLine = cinTable[currentHandle].CIN_WIDTH*cinTable[currentHandle].samplesPerPixel;
	cinTable[currentHandle].screenDelta = cinTable[currentHandle].CIN_HEIGHT*cinTable[currentHandle].samplesPerLine;

	cinTable[currentHandle].half = qfalse;
	cinTable[currentHandle].smootheddouble = qfalse;
	
	cinTable[currentHandle].VQ0 = cinTable[currentHandle].VQNormal;
	cinTable[currentHandle].VQ1 = cinTable[currentHandle].VQBuffer;

	cinTable[currentHandle].t[0] = cinTable[currentHandle].screenDelta;
	cinTable[currentHandle].t[1] = -cinTable[currentHandle].screenDelta;

        cinTable[currentHandle].drawX = cinTable[currentHandle].CIN_WIDTH;
        cinTable[currentHandle].drawY = cinTable[currentHandle].CIN_HEIGHT;
        
	// rage pro is very slow at 512 wide textures, voodoo can't do it at all
	if ( cls.glconfig.hardwareType == GLHW_RAGEPRO || cls.glconfig.maxTextureSize <= 256) {
                if (cinTable[currentHandle].drawX>256) {
                        cinTable[currentHandle].drawX = 256;
                }
                if (cinTable[currentHandle].drawY>256) {
                        cinTable[currentHandle].drawY = 256;
                }
		if (cinTable[currentHandle].CIN_WIDTH != 256 || cinTable[currentHandle].CIN_HEIGHT != 256) {
			Com_Printf("HACK: approxmimating cinematic for Rage Pro or Voodoo\n");
		}
	}
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void RoQPrepMcomp( long xoff, long yoff ) 
{
	long i, j, x, y, temp, temp2;

	i=cinTable[currentHandle].samplesPerLine; j=cinTable[currentHandle].samplesPerPixel;
	if ( cinTable[currentHandle].xsize == (cinTable[currentHandle].ysize*4) && !cinTable[currentHandle].half ) { j = j+j; i = i+i; }
	
	for(y=0;y<16;y++) {
		temp2 = (y+yoff-8)*i;
		for(x=0;x<16;x++) {
			temp = (x+xoff-8)*j;
			cin.mcomp[(x*16)+y] = cinTable[currentHandle].normalBuffer0-(temp2+temp);
		}
	}
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void initRoQ( void ) 
{
	if (currentHandle < 0) return;

	cinTable[currentHandle].VQNormal = (void (*)(byte *, void *))blitVQQuad32fs;
	cinTable[currentHandle].VQBuffer = (void (*)(byte *, void *))blitVQQuad32fs;
	cinTable[currentHandle].samplesPerPixel = 4;
	ROQ_GenYUVTables();
	RllSetupTable();
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/
/*
static byte* RoQFetchInterlaced( byte *source ) {
	int x, *src, *dst;

	if (currentHandle < 0) return NULL;

	src = (int *)source;
	dst = (int *)cinTable[currentHandle].buf2;

	for(x=0;x<256*256;x++) {
		*dst = *src;
		dst++; src += 2;
	}
	return cinTable[currentHandle].buf2;
}
*/
static void RoQReset( void ) {
	
	if (currentHandle < 0) return;

	FS_FCloseFile( cinTable[currentHandle].iFile );
	FS_FOpenFileRead (cinTable[currentHandle].fileName, &cinTable[currentHandle].iFile, qtrue);
	// let the background thread start reading ahead
	FS_Read (cin.file, 16, cinTable[currentHandle].iFile);
	RoQ_init();
	cinTable[currentHandle].status = FMV_LOOPED;
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void RoQInterrupt(void)
{
	byte				*framedata;
        short		sbuf[32768];
        int		ssize;
        
	if (currentHandle < 0) return;

	FS_Read( cin.file, cinTable[currentHandle].RoQFrameSize+8, cinTable[currentHandle].iFile );
	if ( cinTable[currentHandle].RoQPlayed >= cinTable[currentHandle].ROQSize ) { 
		if (cinTable[currentHandle].holdAtEnd==qfalse) {
			if (cinTable[currentHandle].looping) {
				RoQReset();
			} else {
				cinTable[currentHandle].status = FMV_EOF;
			}
		} else {
			cinTable[currentHandle].status = FMV_IDLE;
		}
		return; 
	}

	framedata = cin.file;
//
// new frame is ready
//
redump:
	switch(cinTable[currentHandle].roq_id) 
	{
		case	ROQ_QUAD_VQ:
			if ((cinTable[currentHandle].numQuads&1)) {
				cinTable[currentHandle].normalBuffer0 = cinTable[currentHandle].t[1];
				RoQPrepMcomp( cinTable[currentHandle].roqF0, cinTable[currentHandle].roqF1 );
				cinTable[currentHandle].VQ1( (byte *)cin.qStatus[1], framedata);
				cinTable[currentHandle].buf = 	cin.linbuf + cinTable[currentHandle].screenDelta;
			} else {
				cinTable[currentHandle].normalBuffer0 = cinTable[currentHandle].t[0];
				RoQPrepMcomp( cinTable[currentHandle].roqF0, cinTable[currentHandle].roqF1 );
				cinTable[currentHandle].VQ0( (byte *)cin.qStatus[0], framedata );
				cinTable[currentHandle].buf = 	cin.linbuf;
			}
			if (cinTable[currentHandle].numQuads == 0) {		// first frame
				Com_Memcpy(cin.linbuf+cinTable[currentHandle].screenDelta, cin.linbuf, cinTable[currentHandle].samplesPerLine*cinTable[currentHandle].ysize);
			}
			cinTable[currentHandle].numQuads++;
			cinTable[currentHandle].dirty = qtrue;
			break;
		case	ROQ_CODEBOOK:
			decodeCodeBook( framedata, (unsigned short)cinTable[currentHandle].roq_flags );
			break;
		case	ZA_SOUND_MONO:
			if (!cinTable[currentHandle].silent) {
				ssize = RllDecodeMonoToStereo( framedata, sbuf, cinTable[currentHandle].RoQFrameSize, 0, (unsigned short)cinTable[currentHandle].roq_flags);
                                S_RawSamples(0, ssize, 22050, 2, 1, (byte *)sbuf, 1.0f, -1);
			}
			break;
		case	ZA_SOUND_STEREO:
			if (!cinTable[currentHandle].silent) {
				if (cinTable[currentHandle].numQuads == -1) {
					S_Update();
					s_rawend[0] = s_soundtime;
				}
				ssize = RllDecodeStereoToStereo( framedata, sbuf, cinTable[currentHandle].RoQFrameSize, 0, (unsigned short)cinTable[currentHandle].roq_flags);
                                S_RawSamples(0, ssize, 22050, 2, 2, (byte *)sbuf, 1.0f, -1);
			}
			break;
		case	ROQ_QUAD_INFO:
			if (cinTable[currentHandle].numQuads == -1) {
				readQuadInfo( framedata );
				setupQuad( 0, 0 );
				cinTable[currentHandle].startTime = cinTable[currentHandle].lastTime = CL_ScaledMilliseconds();
			}
			if (cinTable[currentHandle].numQuads != 1) cinTable[currentHandle].numQuads = 0;
			break;
		case	ROQ_PACKET:
			cinTable[currentHandle].inMemory = cinTable[currentHandle].roq_flags;
			cinTable[currentHandle].RoQFrameSize = 0;           // for header
			break;
		case	ROQ_QUAD_HANG:
			cinTable[currentHandle].RoQFrameSize = 0;
			break;
		case	ROQ_QUAD_JPEG:
			break;
		default:
			cinTable[currentHandle].status = FMV_EOF;
			break;
	}	
//
// read in next frame data
//
	if ( cinTable[currentHandle].RoQPlayed >= cinTable[currentHandle].ROQSize ) { 
		if (cinTable[currentHandle].holdAtEnd==qfalse) {
			if (cinTable[currentHandle].looping) {
				RoQReset();
			} else {
				cinTable[currentHandle].status = FMV_EOF;
			}
		} else {
			cinTable[currentHandle].status = FMV_IDLE;
		}
		return; 
	}
	
	framedata		 += cinTable[currentHandle].RoQFrameSize;
	cinTable[currentHandle].roq_id		 = framedata[0] + framedata[1]*256;
	cinTable[currentHandle].RoQFrameSize = framedata[2] + framedata[3]*256 + framedata[4]*65536;
	cinTable[currentHandle].roq_flags	 = framedata[6] + framedata[7]*256;
	cinTable[currentHandle].roqF0		 = (signed char)framedata[7];
	cinTable[currentHandle].roqF1		 = (signed char)framedata[6];

	if (cinTable[currentHandle].RoQFrameSize>65536||cinTable[currentHandle].roq_id==0x1084) {
		Com_DPrintf("roq_size>65536||roq_id==0x1084\n");
		cinTable[currentHandle].status = FMV_EOF;
		if (cinTable[currentHandle].looping) {
			RoQReset();
		}
		return;
	}
	if (cinTable[currentHandle].inMemory && (cinTable[currentHandle].status != FMV_EOF)) { cinTable[currentHandle].inMemory--; framedata += 8; goto redump; }
//
// one more frame hits the dust
//
//	assert(cinTable[currentHandle].RoQFrameSize <= 65536);
//	r = FS_Read( cin.file, cinTable[currentHandle].RoQFrameSize+8, cinTable[currentHandle].iFile );
	cinTable[currentHandle].RoQPlayed	+= cinTable[currentHandle].RoQFrameSize+8;
}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void RoQ_init( void )
{
	cinTable[currentHandle].startTime = cinTable[currentHandle].lastTime = CL_ScaledMilliseconds();

	cinTable[currentHandle].RoQPlayed = 24;

/*	get frame rate */	
	cinTable[currentHandle].roqFPS	 = cin.file[ 6] + cin.file[ 7]*256;
	
	if (!cinTable[currentHandle].roqFPS) cinTable[currentHandle].roqFPS = 30;

	cinTable[currentHandle].numQuads = -1;

	cinTable[currentHandle].roq_id		= cin.file[ 8] + cin.file[ 9]*256;
	cinTable[currentHandle].RoQFrameSize	= cin.file[10] + cin.file[11]*256 + cin.file[12]*65536;
	cinTable[currentHandle].roq_flags	= cin.file[14] + cin.file[15]*256;

	if (cinTable[currentHandle].RoQFrameSize > 65536 || !cinTable[currentHandle].RoQFrameSize) { 
		return;
	}

}

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static void RoQShutdown( void ) {
	const char *s;

	if (!cinTable[currentHandle].buf) {
		return;
	}

	if ( cinTable[currentHandle].status == FMV_IDLE ) {
		return;
	}
	Com_DPrintf("finished cinematic\n");
	cinTable[currentHandle].status = FMV_IDLE;

	if (cinTable[currentHandle].iFile) {
		FS_FCloseFile( cinTable[currentHandle].iFile );
		cinTable[currentHandle].iFile = 0;
	}

	if (cinTable[currentHandle].alterGameState) {
		clc.state = CA_DISCONNECTED;
		// we can't just do a vstr nextmap, because
		// if we are aborting the intro cinematic with
		// a devmap command, nextmap would be valid by
		// the time it was referenced
		s = Cvar_VariableString( "nextmap" );
		if ( s[0] ) {
			Cbuf_ExecuteText( EXEC_APPEND, va("%s\n", s) );
			Cvar_Set( "nextmap", "" );
		}
		CL_handle = -1;
	}
	cinTable[currentHandle].fileName[0] = 0;
	currentHandle = -1;
}

/*
==================
CIN_StopCinematic
==================
*/
e_status CIN_StopCinematic(int handle) {
	
	if (handle < 0 || handle>= MAX_VIDEO_HANDLES || cinTable[handle].status == FMV_EOF) return FMV_EOF;
	currentHandle = handle;

	if ( cinTable[handle].isBink ) {
		// EF1 SP 1:1 retail SCR_StopCinematic: close the movie, then restore game state by
		// kind. A fullscreen cinematic (alterGameState, CA_CINEMATIC) drops to CA_DISCONNECTED;
		// the in-game overlay clears cl_paused so the paused mission resumes and stays
		// CA_ACTIVE. Neither chains nextmap here — that lives in the EOF path (CIN_RunCinematic).
		Bink_Stop( handle );
		cinTable[handle].isBink = qfalse;
		cinTable[handle].buf = NULL;
		cinTable[handle].status = FMV_EOF;
		if ( cinTable[handle].alterGameState && clc.state == CA_CINEMATIC ) {
			clc.state = CA_DISCONNECTED;
		}
		if ( cinTable[handle].inGameCinematic ) {
			Cvar_Set( "cl_paused", "0" );
			cinTable[handle].inGameCinematic = qfalse;
		}
		return FMV_EOF;
	}

	Com_DPrintf("trFMV::stop(), closing %s\n", cinTable[currentHandle].fileName);

	if (!cinTable[currentHandle].buf) {
		return FMV_EOF;
	}

	if (cinTable[currentHandle].alterGameState) {
		if ( clc.state != CA_CINEMATIC ) {
			return cinTable[currentHandle].status;
		}
	}
	cinTable[currentHandle].status = FMV_EOF;
	RoQShutdown();

	return FMV_EOF;
}

/*
==================
CIN_RunCinematic

Fetch and decompress the pending frame
==================
*/


e_status CIN_RunCinematic (int handle)
{
	int	start = 0;
	int     thisTime = 0;

	if (handle < 0 || handle>= MAX_VIDEO_HANDLES || cinTable[handle].status == FMV_EOF) return FMV_EOF;

	if ( cinTable[handle].isBink ) {
		byte *out = NULL;
		int now = CL_ScaledMilliseconds();
		int playing = Bink_Update( handle, cinTable[handle].startTime, now, cinTable[handle].silent, &out );
		if ( out ) { cinTable[handle].buf = out; cinTable[handle].dirty = qtrue; }
		if ( !playing ) {
			if ( cinTable[handle].looping ) {
				Bink_Restart( handle );
				cinTable[handle].startTime = now;
				cinTable[handle].status = FMV_PLAY;
			} else if ( cinTable[handle].inGameCinematic ) {
				// EF1 SP 1:1: in the retail engine an in-game overlay that reaches its last
				// frame is stopped via SCR_StopCinematic(0), which clears cl_paused and
				// resumes the (paused) mission. It NEVER chains nextmap or disconnects; clc.state
				// stays CA_ACTIVE. If a fade-down is active (cl_VidFadeDown), hold the last frame
				// until the 1s fade completes first, then stop.
				if ( cinTable[handle].vidFadeDown ) {
					if ( !cinTable[handle].fadeDownStart ) cinTable[handle].fadeDownStart = now;
					if ( now - cinTable[handle].fadeDownStart < 1000 ) {
						cinTable[handle].status = FMV_IDLE;	// still fading the held last frame
						return FMV_IDLE;
					}
				}
				SCR_StopCinematic();	// Bink stop: clears flag, cl_paused 0, CL_handle=-1, stays CA_ACTIVE
				return FMV_EOF;
			} else if ( cinTable[handle].holdAtEnd ) {
				// hold last frame (inGameCinematic CIN_hold): the script resumes the map.
				// EF1 SP 1:1: the retail engine starts the 1s fade-down the moment the
				// final frame is reached; latch the time here.
				if ( cinTable[handle].vidFadeDown && !cinTable[handle].fadeDownStart ) {
					cinTable[handle].fadeDownStart = now;
				}
				cinTable[handle].status = FMV_IDLE;
			} else {
				// EF1 SP 1:1 with RoQShutdown(): a fullscreen (alterGameState) .bik that
				// ends naturally must drop CA_CINEMATIC and chain nextmap, else the screen
				// freezes in CA_CINEMATIC forever.
				cinTable[handle].status = FMV_EOF;
				if ( cinTable[handle].alterGameState ) {
					const char *s;
					clc.state = CA_DISCONNECTED;
					s = Cvar_VariableString( "nextmap" );
					if ( s[0] ) {
						Cbuf_ExecuteText( EXEC_APPEND, va( "%s\n", s ) );
						Cvar_Set( "nextmap", "" );
					}
					CL_handle = -1;
				}
				cinTable[handle].fileName[0] = 0;
				return FMV_EOF;
			}
		}
		return cinTable[handle].status;
	}

	if (cin.currentHandle != handle) {
		currentHandle = handle;
		cin.currentHandle = currentHandle;
		cinTable[currentHandle].status = FMV_EOF;
		RoQReset();
	}

	if (cinTable[handle].playonwalls < -1)
	{
		return cinTable[handle].status;
	}

	currentHandle = handle;

	if (cinTable[currentHandle].alterGameState) {
		if ( clc.state != CA_CINEMATIC ) {
			return cinTable[currentHandle].status;
		}
	}

	if (cinTable[currentHandle].status == FMV_IDLE) {
		return cinTable[currentHandle].status;
	}

	thisTime = CL_ScaledMilliseconds();
	if (cinTable[currentHandle].shader && (abs(thisTime - cinTable[currentHandle].lastTime))>100) {
		cinTable[currentHandle].startTime += thisTime - cinTable[currentHandle].lastTime;
	}
	cinTable[currentHandle].tfps = (((CL_ScaledMilliseconds() - cinTable[currentHandle].startTime)*3)/100);

	start = cinTable[currentHandle].startTime;
	while(  (cinTable[currentHandle].tfps != cinTable[currentHandle].numQuads)
		&& (cinTable[currentHandle].status == FMV_PLAY) ) 
	{
		RoQInterrupt();
		if (start != cinTable[currentHandle].startTime) {
			cinTable[currentHandle].tfps = (((CL_ScaledMilliseconds() - cinTable[currentHandle].startTime)*3)/100);
			start = cinTable[currentHandle].startTime;
		}
	}

	cinTable[currentHandle].lastTime = thisTime;

	if (cinTable[currentHandle].status == FMV_LOOPED) {
		cinTable[currentHandle].status = FMV_PLAY;
	}

	if (cinTable[currentHandle].status == FMV_EOF) {
	  if (cinTable[currentHandle].looping) {
		RoQReset();
	  } else {
		RoQShutdown();
		return FMV_EOF;
	  }
	}

	return cinTable[currentHandle].status;
}

/*
==================
CIN_PlayCinematic
==================
*/
int CIN_PlayCinematic( const char *arg, int x, int y, int w, int h, int systemBits, int module ) {
	unsigned short RoQID;
	char	name[MAX_OSPATH];
	int		i;

	if (strstr(arg, "/") == NULL && strstr(arg, "\\") == NULL) {
		Com_sprintf (name, sizeof(name), "video/%s", arg);
	} else {
		Com_sprintf (name, sizeof(name), "%s", arg);
	}

	// EF1 SP 1:1: the retail engine resolves cinematics as
	// "video/%s.bik" from a BARE name — every trigger ('cinematic eflogo' at boot, the
	// New-Game intro, st_NN mission cutscenes) passes no extension. Append .bik so those
	// hit the Bink path; explicit .bik/.roq names are left untouched.
	{
		const char *dot   = strrchr( name, '.' );
		const char *slash = strrchr( name, '/' );
		if ( !dot || ( slash && dot < slash ) ) {
			Q_strcat( name, sizeof(name), ".bik" );
		}
	}

	// EF1 SP: both the fullscreen (CIN_system) and in-game-overlay (CIN_inGame) story FMVs
	// always play fresh; only cgame TV-wall videos reuse an existing handle by name.
	if (!(systemBits & (CIN_system | CIN_inGame))) {
		for ( i = 0 ; i < MAX_VIDEO_HANDLES ; i++ ) {
			if (!strcmp(cinTable[i].fileName, name) ) {
				return i;
			}
		}
	}

	Com_DPrintf("CIN_PlayCinematic( %s )\n", arg);

	Com_Memset(&cin, 0, sizeof(cinematics_t) );
	currentHandle = CIN_HandleForVideo();

	cin.currentHandle = currentHandle;

	strcpy(cinTable[currentHandle].fileName, name);

	cinTable[currentHandle].ROQSize = 0;
	cinTable[currentHandle].ROQSize = FS_FOpenFileRead (cinTable[currentHandle].fileName, &cinTable[currentHandle].iFile, qtrue);

	if (cinTable[currentHandle].ROQSize<=0) {
		Com_DPrintf("play(%s), ROQSize<=0\n", arg);
		cinTable[currentHandle].fileName[0] = 0;
		return -1;
	}

	CIN_SetExtents(currentHandle, x, y, w, h, module);
	CIN_SetLooping(currentHandle, (systemBits & CIN_loop)!=0);

	cinTable[currentHandle].CIN_HEIGHT = DEFAULT_CIN_HEIGHT;
	cinTable[currentHandle].CIN_WIDTH  =  DEFAULT_CIN_WIDTH;
	// EF1 SP: clear stale fade state from a previously-recycled handle (the Bink path
	// re-latches cl_VidFadeUp/Down below; the RoQ path leaves the fade disabled).
	cinTable[currentHandle].vidFadeUp = cinTable[currentHandle].vidFadeDown = qfalse;
	cinTable[currentHandle].fadeDownStart = 0;
	cinTable[currentHandle].inGameCinematic = qfalse;
	cinTable[currentHandle].holdAtEnd = (systemBits & CIN_hold) != 0;
	cinTable[currentHandle].alterGameState = (systemBits & CIN_system) != 0;
	// EF1 SP 1:1: retail in-game-overlay flag.
	cinTable[currentHandle].inGameCinematic = (systemBits & CIN_inGame) != 0;
	cinTable[currentHandle].playonwalls = 1;
	cinTable[currentHandle].silent = (systemBits & CIN_silent) != 0;
	cinTable[currentHandle].shader = (systemBits & CIN_shader) != 0;

	if (cinTable[currentHandle].alterGameState) {
		// close the menu
		if ( SPUI_IsActive() ) {
			SPUI_SetActiveMenu( NULL );
		} else if ( uivm ) {
			VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_NONE );
#ifdef USE_FLEXIBLE_DISPLAY
			cls.syncUICursor = qtrue;
#endif
		}
	} else if (cinTable[currentHandle].inGameCinematic) {
		// EF1 SP 1:1: the retail engine pauses the sim for the in-game overlay
		// (Cvar_Set("cl_paused","1")). The SP bridge freezes the sim while cl_paused is set
		// and re-draws the frozen world; the video then overlays on top (SCR_DrawScreenField,
		// CA_ACTIVE case). playonwalls stays 1 so this draws full-screen via SCR_DrawCinematic
		// (it is NOT a cgame TV-wall video). Unpaused on EOF/skip by CIN_StopCinematic.
		Cvar_Set( "cl_paused", "1" );
	} else {
		cinTable[currentHandle].playonwalls = cl_inGameVideo->integer;
	}

	// EF1 SP: Bink (.bik) movies are decoded by FFmpeg, not the RoQ path.
	if ( Bink_IsBinkFile( cinTable[currentHandle].fileName ) ) {
		int bw = 0, bh = 0;
		FS_FCloseFile( cinTable[currentHandle].iFile );   // bink opens its own FS handle
		cinTable[currentHandle].iFile = 0;
		if ( !Bink_Open( currentHandle, cinTable[currentHandle].fileName, &bw, &bh ) ) {
			Com_Printf( "CIN_PlayCinematic: Bink_Open failed for %s\n", cinTable[currentHandle].fileName );
			cinTable[currentHandle].fileName[0] = 0;
			return -1;
		}
		cinTable[currentHandle].isBink     = qtrue;
		cinTable[currentHandle].CIN_WIDTH  = cinTable[currentHandle].drawX = bw;
		cinTable[currentHandle].CIN_HEIGHT = cinTable[currentHandle].drawY = bh;
		cinTable[currentHandle].buf        = NULL;        // filled on first decoded frame
		cinTable[currentHandle].dirty      = qfalse;
		cinTable[currentHandle].startTime  = cinTable[currentHandle].lastTime = CL_ScaledMilliseconds();
		cinTable[currentHandle].status     = FMV_PLAY;
		// EF1 SP 1:1: latch the video-fade cvars at play time exactly like the retail
		// engine. The ICARUS SET_VIDEO_FADE_IN/OUT ops (efgame Q3_Interface.cpp) cvar_set
		// these right before triggering the FMV.
		cinTable[currentHandle].vidFadeUp     = Cvar_VariableValue( "cl_VidFadeUp" )   != 0.0f;
		cinTable[currentHandle].vidFadeDown   = Cvar_VariableValue( "cl_VidFadeDown" ) != 0.0f;
		cinTable[currentHandle].fadeDownStart = 0;
		if ( cinTable[currentHandle].alterGameState ) {
			clc.state = CA_CINEMATIC;
		}
		Con_Close();
		if ( !cinTable[currentHandle].silent ) {
			s_rawend[0] = s_soundtime;
		}
		Com_DPrintf( "trFMV::play(), playing Bink %s\n", arg );
		return currentHandle;
	}

	initRoQ();
					
	FS_Read (cin.file, 16, cinTable[currentHandle].iFile);

	RoQID = (unsigned short)(cin.file[0]) + (unsigned short)(cin.file[1])*256;
	if (RoQID == 0x1084)
	{
		RoQ_init();
//		FS_Read (cin.file, cinTable[currentHandle].RoQFrameSize+8, cinTable[currentHandle].iFile);

		cinTable[currentHandle].status = FMV_PLAY;
		Com_DPrintf("trFMV::play(), playing %s\n", arg);

		if (cinTable[currentHandle].alterGameState) {
			clc.state = CA_CINEMATIC;
		}
		
		Con_Close();

		if (!cinTable[currentHandle].silent) {
			s_rawend[0] = s_soundtime;
		}

		return currentHandle;
	}
	Com_DPrintf("trFMV::play(), invalid RoQ ID\n");

	RoQShutdown();
	return -1;
}

void CIN_SetExtents (int handle, int x, int y, int w, int h, int module) {
	if (handle < 0 || handle>= MAX_VIDEO_HANDLES || cinTable[handle].status == FMV_EOF) return;
	cinTable[handle].xpos = x;
	cinTable[handle].ypos = y;
	cinTable[handle].width = w;
	cinTable[handle].height = h;
	cinTable[handle].module = module;
	cinTable[handle].dirty = qtrue;
}

void CIN_SetLooping(int handle, qboolean loop) {
	if (handle < 0 || handle>= MAX_VIDEO_HANDLES || cinTable[handle].status == FMV_EOF) return;
	cinTable[handle].looping = loop;
}

/*
==================
CIN_ResampleCinematic

Resample cinematic to 256x256 and store in buf2
==================
*/
void CIN_ResampleCinematic(int handle, int *buf2) {
	int ix, iy, *buf3, xm, ym, ll;
	byte	*buf;

	buf = cinTable[handle].buf;

	xm = cinTable[handle].CIN_WIDTH/256;
	ym = cinTable[handle].CIN_HEIGHT/256;
	ll = 8;
	if (cinTable[handle].CIN_WIDTH==512) {
		ll = 9;
	}

	buf3 = (int*)buf;
	if (xm==2 && ym==2) {
		byte *bc2, *bc3;
		int	ic, iiy;

		bc2 = (byte *)buf2;
		bc3 = (byte *)buf3;
		for (iy = 0; iy<256; iy++) {
			iiy = iy<<12;
			for (ix = 0; ix<2048; ix+=8) {
				for(ic = ix;ic<(ix+4);ic++) {
					*bc2=(bc3[iiy+ic]+bc3[iiy+4+ic]+bc3[iiy+2048+ic]+bc3[iiy+2048+4+ic])>>2;
					bc2++;
				}
			}
		}
	} else if (xm==2 && ym==1) {
		byte *bc2, *bc3;
		int	ic, iiy;

		bc2 = (byte *)buf2;
		bc3 = (byte *)buf3;
		for (iy = 0; iy<256; iy++) {
			iiy = iy<<11;
			for (ix = 0; ix<2048; ix+=8) {
				for(ic = ix;ic<(ix+4);ic++) {
					*bc2=(bc3[iiy+ic]+bc3[iiy+4+ic])>>1;
					bc2++;
				}
			}
		}
	} else {
		for (iy = 0; iy<256; iy++) {
			for (ix = 0; ix<256; ix++) {
					buf2[(iy<<8)+ix] = buf3[((iy*ym)<<ll) + (ix*xm)];
			}
		}
	}
}

/*
==================
CIN_DrawFadeOverlay

EF1 SP 1:1 video fade. The retail engine scaled the DrawStretchRaw vertex alpha 0->1
(fade-up, cl_VidFadeUp) over the first 1000ms and 1->0 (fade-down, cl_VidFadeDown) over
the last 1000ms, using a 0.001/ms ramp and clamping to 1.0. The port renderer's
re.DrawStretchRaw has no alpha parameter, so the identical visual is produced by laying a
black quad of alpha (1-fade) over the cinematic rect (640x480 virtual space, same coords
as the video).
==================
*/
static void CIN_DrawFadeOverlay( int handle ) {
	float	fade = 1.0f;	// 1 = fully visible, 0 = black
	int		now, elapsed;
	float	black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if ( !cinTable[handle].vidFadeUp && !cinTable[handle].vidFadeDown ) {
		return;
	}

	now = CL_ScaledMilliseconds();

	if ( cinTable[handle].vidFadeDown && cinTable[handle].fadeDownStart ) {
		// fading out over the held last frame
		elapsed = now - cinTable[handle].fadeDownStart;
		if ( elapsed >= 1000 ) {
			fade = 0.0f;
		} else if ( elapsed > 0 ) {
			fade = 1.0f - (float)elapsed * 0.001f;
		}
	} else if ( cinTable[handle].vidFadeUp ) {
		// fading in from the start
		elapsed = now - cinTable[handle].startTime;
		if ( elapsed >= 1000 ) {
			return;		// fully faded in, no overlay needed
		} else if ( elapsed > 0 ) {
			fade = (float)elapsed * 0.001f;
		} else {
			fade = 0.0f;
		}
	} else {
		return;
	}

	black[3] = 1.0f - fade;
	if ( black[3] <= 0.0f ) {
		return;
	}
	SCR_FillRect( (float)cinTable[handle].xpos, (float)cinTable[handle].ypos,
				  (float)cinTable[handle].width, (float)cinTable[handle].height, black );
}

/*
==================
CIN_DrawCinematic
==================
*/
void CIN_DrawCinematic (int handle) {
	float	x, y, w, h;
	byte	*buf;

	if (handle < 0 || handle>= MAX_VIDEO_HANDLES || cinTable[handle].status == FMV_EOF) return;

	if (!cinTable[handle].buf) {
		return;
	}

	x = cinTable[handle].xpos;
	y = cinTable[handle].ypos;
	w = cinTable[handle].width;
	h = cinTable[handle].height;
	buf = cinTable[handle].buf;

#ifdef USE_FLEXIBLE_DISPLAY
	if ( cl_flexibleDisplay->integer ) {
		switch ( cinTable[handle].module ) {
			case CIN_UI:
				CL_AdjustFromUI( &x, &y, &w, &h );
				break;
			case CIN_CGAME:
				CL_AdjustFromCGame( &x, &y, &w, &h );
				break;
			case CIN_CLIENT:
			default:
				if ( cl_viewmode->integer <= 3 ) {
					SCR_SetScreenPlacement( SCR_VERT_CENTER | SCR_HOR_CENTER );
				} else {
					SCR_SetScreenPlacement( SCR_VERT_STRETCH | SCR_HOR_STRETCH );
				}
				SCR_AdjustFrom640( &x, &y, &w, &h );
				break;
		}
	} else
#endif
	{
		SCR_SetScreenPlacement( SCR_VERT_STRETCH | SCR_HOR_STRETCH );
		SCR_AdjustFrom640( &x, &y, &w, &h );
	}

	if (cinTable[handle].dirty && (cinTable[handle].CIN_WIDTH != cinTable[handle].drawX || cinTable[handle].CIN_HEIGHT != cinTable[handle].drawY)) {
		int *buf2;

		buf2 = Hunk_AllocateTempMemory( 256*256*4 );

		CIN_ResampleCinematic(handle, buf2);

		re.DrawStretchRaw( x, y, w, h, 256, 256, (byte *)buf2, handle, qtrue);
		cinTable[handle].dirty = qfalse;
		Hunk_FreeTempMemory(buf2);
		CIN_DrawFadeOverlay( handle );
		return;
	}

	re.DrawStretchRaw( x, y, w, h, cinTable[handle].drawX, cinTable[handle].drawY, buf, handle, cinTable[handle].dirty);
	cinTable[handle].dirty = qfalse;
	CIN_DrawFadeOverlay( handle );
}

// EF1 SP 1:1: the retail "inGameCinematic" path (CIN_PlayCinematic(name, 1)) plays the .bik
// as an IN-LEVEL OVERLAY, NOT the fullscreen CA_CINEMATIC path. The ICARUS SET_VIDEO_PLAY op
// (efgame Q3_Interface.cpp) is FIRE-AND-FORGET — it sends this command and the script
// continues — so the engine pauses the sim (cl_paused=1), overlays the video on the frozen
// world, and unpauses+resumes the mission on EOF/skip.
// (The old port routed this through "cinematic %s 1" = the fullscreen CA_CINEMATIC path, which
// froze SP_DrawFrame -> sim/script stopped -> HANG, and a tap hit the CA_CINEMATIC skip path
// -> CL_Disconnect_f -> main menu. CIN_inGame selects the retail overlay path instead.)
void CL_PlayInGameCinematic_f(void) {
	if ( Cmd_Argc() < 2 ) {
		Com_Printf( "inGameCinematic <videoname>\n" );
		return;
	}

	// a story FMV always plays fresh; close any cinematic still up first
	if ( CL_handle >= 0 ) {
		SCR_StopCinematic();
	}

	S_StopAllSounds();

	CL_handle = CIN_PlayCinematic( Cmd_Argv( 1 ), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, CIN_inGame, CIN_CLIENT );
	if ( CL_handle >= 0 ) {
		do {
			SCR_RunCinematic();
		} while ( cinTable[currentHandle].buf == NULL && cinTable[currentHandle].status == FMV_PLAY );	// wait for first frame
	}
}

void CL_PlayCinematic_f(void) {
	char	*arg, *s;
	int bits = CIN_system;

	Com_DPrintf("CL_PlayCinematic_f\n");
	if (clc.state == CA_CINEMATIC) {
		SCR_StopCinematic();
	}

	arg = Cmd_Argv( 1 );
	s = Cmd_Argv(2);

	if ((s && s[0] == '1') || Q_stricmp(arg,"demoend.roq")==0 || Q_stricmp(arg,"end.roq")==0) {
		bits |= CIN_hold;
	}
	if (s && s[0] == '2') {
		bits |= CIN_loop;
	}

	S_StopAllSounds ();

	CL_handle = CIN_PlayCinematic( arg, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, bits, CIN_CLIENT );
	if (CL_handle >= 0) {
		do {
			SCR_RunCinematic();
		} while (cinTable[currentHandle].buf == NULL && cinTable[currentHandle].status == FMV_PLAY);		// wait for first frame (load codebook and sound)
	}
}


void SCR_DrawCinematic (void) {
	if (CL_handle >= 0 && CL_handle < MAX_VIDEO_HANDLES) {
		CIN_DrawCinematic(CL_handle);
	}
}

void SCR_RunCinematic (void)
{
	if (CL_handle >= 0 && CL_handle < MAX_VIDEO_HANDLES) {
		CIN_RunCinematic(CL_handle);
	}
}

void SCR_StopCinematic(void) {
	if (CL_handle >= 0 && CL_handle < MAX_VIDEO_HANDLES) {
		CIN_StopCinematic(CL_handle);
		S_StopAllSounds ();
		CL_handle = -1;
	}
}

// EF1 SP 1:1: retail "is in-game cinematic" query.
// True while an inGameCinematic overlay is on screen: SCR_DrawScreenField overlays the FMV in
// the CA_ACTIVE frame, the SP bridge keeps the sim frozen (cl_paused), and a tap/key skips it
// (the 1.2s-guarded SCR_StopCinematic) instead of routing to the CA_CINEMATIC disconnect path.
qboolean CL_InGameCinematicActive( void ) {
	return ( CL_handle >= 0 && CL_handle < MAX_VIDEO_HANDLES
	         && cinTable[CL_handle].inGameCinematic
	         && cinTable[CL_handle].status != FMV_EOF );
}

// EF1 SP 1:1: retail in-cinematic skip key (the key handler calls SCR_StopCinematic).
// A key/tap during an in-game overlay skips it, but only after the 1.2s guard (1200 ms since
// play time) so an accidental tap at the very start cannot instantly cut the FMV.
// Returns qtrue when the key belongs to the cinematic (caller consumes it) — including inside the
// guard window, exactly like retail (which returns after the stop call regardless), so taps
// during the FMV never leak through to gameplay. On a real skip the overlay is stopped, which
// clears cl_paused and resumes the mission (CIN_StopCinematic) — it never disconnects.
qboolean CL_SkipInGameCinematic( void ) {
	if ( !CL_InGameCinematicActive() ) {
		return qfalse;
	}
	if ( CL_ScaledMilliseconds() - cinTable[CL_handle].startTime >= 1200 ) {
		SCR_StopCinematic();
	}
	return qtrue;
}

void CIN_UploadCinematic(int handle) {
	if (handle >= 0 && handle < MAX_VIDEO_HANDLES) {
		if (!cinTable[handle].buf) {
			return;
		}
		if (cinTable[handle].playonwalls <= 0 && cinTable[handle].dirty) {
			if (cinTable[handle].playonwalls == 0) {
				cinTable[handle].playonwalls = -1;
			} else {
				if (cinTable[handle].playonwalls == -1) {
					cinTable[handle].playonwalls = -2;
				} else {
					cinTable[handle].dirty = qfalse;
				}
			}
		}

		// Resample the video if needed
		if (cinTable[handle].dirty && (cinTable[handle].CIN_WIDTH != cinTable[handle].drawX || cinTable[handle].CIN_HEIGHT != cinTable[handle].drawY))  {
			int *buf2;

			buf2 = Hunk_AllocateTempMemory( 256*256*4 );

			CIN_ResampleCinematic(handle, buf2);

			re.UploadCinematic( cinTable[handle].CIN_WIDTH, cinTable[handle].CIN_HEIGHT, 256, 256, (byte *)buf2, handle, qtrue);
			cinTable[handle].dirty = qfalse;
			Hunk_FreeTempMemory(buf2);
		} else {
			// Upload video at normal resolution
			re.UploadCinematic( cinTable[handle].CIN_WIDTH, cinTable[handle].CIN_HEIGHT, cinTable[handle].drawX, cinTable[handle].drawY,
					cinTable[handle].buf, handle, cinTable[handle].dirty);
			cinTable[handle].dirty = qfalse;
		}

		if (cl_inGameVideo->integer == 0 && cinTable[handle].playonwalls == 1) {
			cinTable[handle].playonwalls--;
		}
		else if (cl_inGameVideo->integer != 0 && cinTable[handle].playonwalls != 1) {
			cinTable[handle].playonwalls = 1;
		}
	}
}

