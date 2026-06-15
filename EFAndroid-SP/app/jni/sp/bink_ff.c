// bink_ff.c — FFmpeg Bink (.bik) backend for the engine cinematic system.
// Reads .bik straight out of the pak (pk3) via a custom AVIO bound to the engine FS,
// decodes BIKi video -> RGBA (scaled to a power-of-two for DrawStretchRaw) and Bink
// audio -> S16 into S_RawSamples. cl_cin.c branches here on the .bik extension.
#include "bink_ff.h"
#ifdef BINK_HOST_TEST
#include "bink_host_shim.h"               // host test: minimal q_shared-equivalent types
#else
#include "../efcode/qcommon/q_shared.h"   // qboolean/byte/fileHandle_t/Q_stricmp/Q_strncpyz
#endif

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"

#ifdef BINK_HOST_TEST
#include <stdio.h>
#define BLOG(...) do { printf( "[bink] " __VA_ARGS__ ); printf( "\n" ); } while (0)
#else
#include <android/log.h>
#define BLOG(...) __android_log_print( ANDROID_LOG_INFO, "bink", __VA_ARGS__ )
#endif

// engine services (resolved at link time in the main .so)
extern int  FS_FOpenFileRead( const char *qpath, fileHandle_t *file, qboolean uniqueFILE );
extern int  FS_Read( void *buffer, int len, fileHandle_t f );
extern int  FS_Seek( fileHandle_t f, long offset, int origin );
extern void FS_FCloseFile( fileHandle_t f );
extern void S_RawSamples( int stream, int samples, int rate, int width, int channels,
                          const byte *data, float volume, int entityNum );
extern void S_Update( void );        // snd_main.c — refresh the DMA consume clock
extern int  s_soundtime;             // snd_dma.c — DMA consume clock (sample pairs)
extern int  s_rawend[];              // snd_dma.c — raw stream write head; [0] = our stream

// Hold the raw-audio ring this far ahead of the DMA clock (in sample pairs) so a
// brief scheduling stall never starves the mixer (which would otherwise snap
// s_rawend back to s_soundtime and play silence — the "rapid cut in/out" bug).
// Must clear one 15fps burst gap (~66ms) + frame jitter, yet stay small enough
// that the video lead it implies keeps A/V (lip) sync within tolerance, and well
// under MAX_RAW_SAMPLES (16384). ~6000 pairs ≈ 125ms @48k / 136ms @44.1k.
#define BINK_RAW_CUSHION 6000

#define MAX_BINK_HANDLES 16
#define AVIO_BUFSZ       (32*1024)

typedef struct {
    qboolean         inUse;
    char             name[256];
    fileHandle_t     fh;
    long             fileLen;
    long             pos;            // tracked stream position for the AVIO seek cb

    AVFormatContext *fmt;
    AVIOContext     *avio;
    unsigned char   *avioBuf;
    AVCodecContext  *vdec, *adec;
    int              vstream, astream;
    AVFrame         *vframe, *aframe;
    AVPacket        *pkt;
    struct SwsContext *sws;

    double           fps;
    int              srcW, srcH;     // native bink frame size
    int              dstW, dstH;     // power-of-two RGBA buffer size (== drawX/drawY)
    byte            *rgba;           // dstW*dstH*4 output buffer
    long             framesDecoded;  // # video frames produced
    int              eof;
    int              audioPrimed;    // s_rawend aligned to the DMA clock once at start
} binkHandle_t;

static binkHandle_t g_bink[MAX_BINK_HANDLES];

// ---- custom AVIO backed by the engine FS (so .bik inside a pk3 just works) ----
static int bink_avio_read( void *opaque, uint8_t *buf, int size ) {
    binkHandle_t *b = (binkHandle_t *)opaque;
    int r = FS_Read( buf, size, b->fh );
    if ( r <= 0 ) return AVERROR_EOF;
    b->pos += r;
    return r;
}
static int64_t bink_avio_seek( void *opaque, int64_t off, int whence ) {
    binkHandle_t *b = (binkHandle_t *)opaque;
    whence &= ~AVSEEK_FORCE;
    if ( whence == AVSEEK_SIZE ) return (int64_t)b->fileLen;
    long np;
    if      ( whence == SEEK_SET ) np = (long)off;
    else if ( whence == SEEK_CUR ) np = b->pos + (long)off;
    else                            np = b->fileLen + (long)off;   // SEEK_END
    FS_Seek( b->fh, np, FS_SEEK_SET );
    b->pos = np;
    return (int64_t)np;
}

static int next_pow2( int v ) { int p = 1; while ( p < v ) p <<= 1; return p; }

int Bink_IsBinkFile( const char *name ) {
    size_t n = name ? strlen( name ) : 0;
    return ( n > 4 && !Q_stricmp( name + n - 4, ".bik" ) ) ? 1 : 0;
}

static void bink_free( binkHandle_t *b ) {
    if ( b->sws )    { sws_freeContext( b->sws ); b->sws = NULL; }
    if ( b->vframe ) av_frame_free( &b->vframe );
    if ( b->aframe ) av_frame_free( &b->aframe );
    if ( b->pkt )    av_packet_free( &b->pkt );
    if ( b->vdec )   avcodec_free_context( &b->vdec );
    if ( b->adec )   avcodec_free_context( &b->adec );
    if ( b->fmt )    avformat_close_input( &b->fmt );   // also frees avio->buffer? no
    if ( b->avio )   { av_freep( &b->avio->buffer ); avio_context_free( &b->avio ); }
    if ( b->rgba )   { free( b->rgba ); b->rgba = NULL; }
    if ( b->fh )     { FS_FCloseFile( b->fh ); b->fh = 0; }
    memset( b, 0, sizeof( *b ) );
}

static qboolean bink_setup( binkHandle_t *b, const char *name ) {
    b->fileLen = FS_FOpenFileRead( name, &b->fh, qtrue );
    if ( b->fileLen <= 0 || !b->fh ) { BLOG( "open failed: %s", name ); return qfalse; }
    b->pos = 0;

    b->avioBuf = (unsigned char *)av_malloc( AVIO_BUFSZ );
    b->avio = avio_alloc_context( b->avioBuf, AVIO_BUFSZ, 0, b, bink_avio_read, NULL, bink_avio_seek );
    b->fmt = avformat_alloc_context();
    b->fmt->pb = b->avio;
    b->fmt->flags |= AVFMT_FLAG_CUSTOM_IO;

    if ( avformat_open_input( &b->fmt, NULL, NULL, NULL ) < 0 ) { BLOG( "avformat_open_input failed: %s", name ); return qfalse; }
    if ( avformat_find_stream_info( b->fmt, NULL ) < 0 )        { BLOG( "find_stream_info failed: %s", name ); return qfalse; }

    b->vstream = b->astream = -1;
    for ( unsigned i = 0; i < b->fmt->nb_streams; i++ ) {
        enum AVMediaType t = b->fmt->streams[i]->codecpar->codec_type;
        if ( t == AVMEDIA_TYPE_VIDEO && b->vstream < 0 ) b->vstream = i;
        else if ( t == AVMEDIA_TYPE_AUDIO && b->astream < 0 ) b->astream = i;
    }
    if ( b->vstream < 0 ) { BLOG( "no video stream: %s", name ); return qfalse; }

    // video decoder
    AVStream *vs = b->fmt->streams[b->vstream];
    const AVCodec *vc = avcodec_find_decoder( vs->codecpar->codec_id );
    if ( !vc ) { BLOG( "no video decoder" ); return qfalse; }
    b->vdec = avcodec_alloc_context3( vc );
    avcodec_parameters_to_context( b->vdec, vs->codecpar );
    if ( avcodec_open2( b->vdec, vc, NULL ) < 0 ) { BLOG( "video open2 failed" ); return qfalse; }

    b->srcW = b->vdec->width;
    b->srcH = b->vdec->height;
    AVRational fr = av_guess_frame_rate( b->fmt, vs, NULL );
    b->fps = ( fr.num && fr.den ) ? av_q2d( fr ) : 15.0;

    // audio decoder (optional)
    if ( b->astream >= 0 ) {
        AVStream *as = b->fmt->streams[b->astream];
        const AVCodec *ac = avcodec_find_decoder( as->codecpar->codec_id );
        if ( ac ) {
            b->adec = avcodec_alloc_context3( ac );
            avcodec_parameters_to_context( b->adec, as->codecpar );
            if ( avcodec_open2( b->adec, ac, NULL ) < 0 ) { avcodec_free_context( &b->adec ); b->adec = NULL; }
        }
    }

    // power-of-two RGBA target for DrawStretchRaw
    b->dstW = next_pow2( b->srcW ); if ( b->dstW > 1024 ) b->dstW = 1024;
    b->dstH = next_pow2( b->srcH ); if ( b->dstH > 1024 ) b->dstH = 1024;
    b->rgba = (byte *)calloc( 1, (size_t)b->dstW * b->dstH * 4 );

    b->sws = sws_getContext( b->srcW, b->srcH, b->vdec->pix_fmt,
                             b->dstW, b->dstH, AV_PIX_FMT_RGBA,
                             SWS_BILINEAR, NULL, NULL, NULL );
    b->vframe = av_frame_alloc();
    b->aframe = av_frame_alloc();
    b->pkt    = av_packet_alloc();
    if ( !b->rgba || !b->sws || !b->vframe || !b->pkt ) { BLOG( "alloc failed" ); return qfalse; }

    BLOG( "open %s  %dx%d -> %dx%d  %.2ffps  audio=%d", name, b->srcW, b->srcH, b->dstW, b->dstH, b->fps, b->astream >= 0 );
    return qtrue;
}

int Bink_Open( int handle, const char *name, int *outW, int *outH ) {
    if ( handle < 0 || handle >= MAX_BINK_HANDLES ) return 0;
    binkHandle_t *b = &g_bink[handle];
    if ( b->inUse ) bink_free( b );
    memset( b, 0, sizeof( *b ) );
    Q_strncpyz( b->name, name, sizeof( b->name ) );
    if ( !bink_setup( b, name ) ) { bink_free( b ); return 0; }
    b->inUse = qtrue;
    if ( outW ) *outW = b->dstW;
    if ( outH ) *outH = b->dstH;
    return 1;
}

// drain one audio frame -> S16 interleaved -> S_RawSamples
static void bink_emit_audio( binkHandle_t *b ) {
    static int16_t s16[8 * 4096];
    AVFrame *f = b->aframe;
    int ch = b->adec->ch_layout.nb_channels;
    int ns = f->nb_samples;
    if ( ch < 1 ) ch = 1;
    if ( ns < 1 || ns * ch > (int)( sizeof( s16 ) / sizeof( s16[0] ) ) ) return;

    enum AVSampleFormat fmt = b->adec->sample_fmt;
    if ( av_sample_fmt_is_planar( fmt ) ) {
        for ( int i = 0; i < ns; i++ )
            for ( int c = 0; c < ch; c++ ) {
                float v = ( (const float *)f->data[c] )[i];
                int s = (int)( v * 32767.0f );
                s16[i * ch + c] = (int16_t)( s > 32767 ? 32767 : s < -32768 ? -32768 : s );
            }
    } else {
        const float *in = (const float *)f->data[0];
        for ( int i = 0; i < ns * ch; i++ ) {
            int s = (int)( in[i] * 32767.0f );
            s16[i] = (int16_t)( s > 32767 ? 32767 : s < -32768 ? -32768 : s );
        }
    }
    S_RawSamples( 0, ns, b->adec->sample_rate, 2, ch, (const byte *)s16, 1.0f, -1 );
}

int Bink_Update( int handle, int startMs, int nowMs, int silent, unsigned char **outBuf ) {
    if ( outBuf ) *outBuf = NULL;
    if ( handle < 0 || handle >= MAX_BINK_HANDLES ) return 0;
    binkHandle_t *b = &g_bink[handle];
    if ( !b->inUse ) return 0;
    if ( b->eof ) return 0;

    long target = (long)( (double)( nowMs - startMs ) * b->fps / 1000.0 );
    if ( target < 0 ) target = 0;
    int produced = 0, guard = 0;

    // Prime the raw-audio clock once so s_rawend starts aligned to the live DMA
    // position (mirror of the RoQ path in cl_cin.c). Without this the first burst
    // snaps s_rawend forward and the look-ahead cushion never builds.
    if ( b->adec && !silent && !b->audioPrimed ) {
        S_Update();                  // refresh s_soundtime
        s_rawend[0] = s_soundtime;
        b->audioPrimed = 1;
    }

    // Single packet pump: keep pulling while EITHER video is behind its time target
    // OR the raw-audio ring is below the cushion. Decoupling audio from the video
    // target keeps the ring filled on frames where video is already at/ahead of
    // target (the common case: 15fps video vs a 30-90fps engine) — that starvation
    // was the rapid cut-in/out underrun stutter. guard bumped 256->512 since one
    // call may now also pre-roll the initial audio cushion.
    while ( !b->eof && guard++ < 512 ) {
        int videoBehind = ( b->framesDecoded <= target );
        int audioLow    = ( b->adec && !silent ) &&
                          ( ( s_rawend[0] - s_soundtime ) < BINK_RAW_CUSHION );
        if ( !videoBehind && !audioLow ) break;

        int rr = av_read_frame( b->fmt, b->pkt );
        if ( rr < 0 ) { b->eof = 1; break; }

        if ( b->pkt->stream_index == b->vstream ) {
            if ( avcodec_send_packet( b->vdec, b->pkt ) == 0 ) {
                while ( avcodec_receive_frame( b->vdec, b->vframe ) == 0 ) {
                    uint8_t *dst[4] = { b->rgba, NULL, NULL, NULL };
                    int      lin[4] = { b->dstW * 4, 0, 0, 0 };
                    sws_scale( b->sws, (const uint8_t * const *)b->vframe->data,
                               b->vframe->linesize, 0, b->srcH, dst, lin );
                    b->framesDecoded++;
                    produced = 1;
                }
            }
        } else if ( b->adec && b->pkt->stream_index == b->astream && !silent ) {
            if ( avcodec_send_packet( b->adec, b->pkt ) == 0 ) {
                while ( avcodec_receive_frame( b->adec, b->aframe ) == 0 )
                    bink_emit_audio( b );
            }
        }
        av_packet_unref( b->pkt );
    }

    if ( produced && outBuf ) *outBuf = b->rgba;
    return b->eof ? 0 : 1;
}

void Bink_Restart( int handle ) {
    if ( handle < 0 || handle >= MAX_BINK_HANDLES ) return;
    binkHandle_t *b = &g_bink[handle];
    if ( !b->inUse ) return;
    av_seek_frame( b->fmt, b->vstream, 0, AVSEEK_FLAG_BACKWARD );
    if ( b->vdec ) avcodec_flush_buffers( b->vdec );
    if ( b->adec ) avcodec_flush_buffers( b->adec );
    b->framesDecoded = 0;
    b->eof = 0;
    b->audioPrimed = 0;   // re-prime the raw-audio clock on the next update
}

void Bink_Stop( int handle ) {
    if ( handle < 0 || handle >= MAX_BINK_HANDLES ) return;
    binkHandle_t *b = &g_bink[handle];
    if ( b->inUse ) bink_free( b );
}
