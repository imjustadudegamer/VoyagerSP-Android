// bink_ff.h — FFmpeg-backed Bink (.bik) decoder for the engine cinematic system.
// EF1 ships its movies as BIKi (Bink 1) in video/*.bik; the reused lilium cl_cin.c
// only decodes RoQ, so cl_cin.c branches to these when the filename ends in .bik.
// Neutral types only (int / unsigned char) so cl_cin.c can include this without the
// nested-relative-include problem; bink_ff.c pulls in the engine + FFmpeg headers.
#ifndef BINK_FF_H
#define BINK_FF_H
#ifdef __cplusplus
extern "C" {
#endif

int  Bink_IsBinkFile( const char *name );                 // 1 if name ends in .bik
int  Bink_Open( int handle, const char *name, int *outW, int *outH );  // 1 on success; fills pow2 RGBA size
int  Bink_Update( int handle, int startMs, int nowMs, int silent, unsigned char **outBuf ); // 1 playing, 0 EOF
void Bink_Restart( int handle );                          // seek back to first frame (looping)
void Bink_Stop( int handle );                             // close + free

#ifdef __cplusplus
}
#endif
#endif
