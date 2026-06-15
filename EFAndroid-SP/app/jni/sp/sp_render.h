// sp_render.h — neutral interface between the SP cgame dispatcher (SP headers) and the
// HM renderer glue (HM headers). Plain types only, so both worlds can include it.
#ifndef SP_RENDER_H
#define SP_RENDER_H
#ifdef __cplusplus
extern "C" {
#endif

// HM-side renderer wrappers (defined in sp_integration.c, call the HM `re` refexport).
// refEntity/refdef are passed as raw bytes + size; the HM side memcpys the shared prefix
// and remaps reType (SP enum -> HM ELITEFORCE enum).
void  SPR_ClearScene(void);
void  SPR_AddRefEntity(const void* spRefEntity, int spSize);
void  SPR_AddPoly(int hShader, int numVerts, const void* verts);
void  SPR_AddLight(const float* org, float intensity, float r, float g, float b);
void  SPR_RenderScene(const void* spRefdef, int spSize);
void  SPR_SetColor(const float* rgba);
void  SPR_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, int hShader);
int   SPR_RegisterModel(const char* name);
int   SPR_RegisterSkin(const char* name);
int   SPR_RegisterShader(const char* name);
int   SPR_RegisterShaderNoMip(const char* name);
void  SPR_LoadWorld(const char* name);
void  SPR_ModelBounds(int model, float* mins, float* maxs);
int   SPR_LerpTag(void* tag, int model, int startFrame, int endFrame, float frac, const char* tagName);
void  SPR_GetLighting(const float* origin, float* ambient, float* directed, float* lightDir);  // AI stealth + model lighting
int   SPR_MarkFragments(int numPoints, const void* points, const float* projection, int maxPoints, float* pointBuffer, int maxFragments, void* fragmentBuffer);  // decals
void  SPR_DrawStretchRaw(int x, int y, int w, int h, int cols, int rows, const unsigned char* data);  // UI savegame thumbnails
void  SPR_RequestSaveThumb(void);                 // EFSP: arm a 256x256 framebuffer capture for the save 'SHOT' chunk
int   SPR_GetSaveThumb(unsigned char* dst);       // EFSP: copy latest 256x256x4 RGBA thumbnail; 0 if none yet
void  SPR_ScissorPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, int hShader);  // UI clip (falls back to stretch)
void  SPR_GetVidSize(int* w, int* h);  // engine's real surface size
void  SPR_GetLatestCmd(int* angles, signed char* fwd, signed char* side, signed char* up, int* buttons);
int   SPR_S_RegisterSound(const char* name);
void  SPR_S_StartSound(const float* origin, int entnum, int entchannel, int sfx);
void  SPR_S_StartLocalSound(int sfx, int channel);
void  SPR_S_ClearLoopingSounds(void);
void  SPR_S_AddLoopingSound(int entnum, const float* origin, const float* velocity, int sfx);
void  SPR_S_UpdateEntityPosition(int entnum, const float* origin);
void  SPR_S_Respatialize(int entnum, const float* origin, float (*axis)[3], int inwater);
void  SPR_S_StartBackgroundTrack(const char* intro, const char* loop);
int   SPR_S_GetSampleLength(int sfx);   // sound playing time ms (subtitle timing)

// SP-side (defined in sp_bridge.cpp) — driven by the HM render hook each frame.
int      SP_IsActive(void);          // 1 once spmap has loaded a map
void     SP_DrawFrame(int serverTime, int stereo);  // run sim + SP cgame draw (calls SPR_*)

#ifdef __cplusplus
}
#endif
#endif
