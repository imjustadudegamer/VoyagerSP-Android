# GLES 3.2 renderer — audit and port plan

Status: **implementation in progress, Android only.** The module now compiles
the *same* EF1 front end as the Vulkan module (`renderervk/tr_*.c`, unmodified)
and supplies the drawing backend (`gl3_backend.c`) plus the ported EF2 GL layer.
It still reports `renderer_backend_ready() == 0`: nothing has ever been
executed, so the menu must not offer it and `GetRefAPI` returns NULL, which makes
the client fall back to Vulkan (see "Load/fallback contract" below). Linux does
not build this module at all — the desktop stays Vulkan-only.

Milestone state (see the end of this file for the detailed list): M1 (context /
frame loop / present) and the M2/M3 drawing path are written; bloom, MSAA,
r_smp, flares and full readback remain.

This document is the delta that has to be understood before writing backend
code. It was written before the port, deliberately.

## 1. `refexport_t` — engine ↔ renderer ABI

The renderer module is *this* engine's renderer, so it must fill **this
engine's** `refexport_t` (`renderercommon/tr_public.h`, `REF_API_VERSION` 9 with
`USE_FLEXIBLE_DISPLAY`, 8 without). EF2's renderer fills `REF_API_VERSION` 14,
a different struct. The two cannot be shared.

| EF1 (this tree, v9) | EF2 (v14) |
|---|---|
| Shutdown, BeginRegistration, RegisterModel/Skin/Shader/ShaderNoMip | same names, plus ForceRegisterModel, RefreshShaderNoMip |
| **RegisterShader3D** (EF1 3D-model UI/HUD shaders) | ReplaceShader, GetRegistrationSequence |
| LoadWorld/SetWorldVisData/EndRegistration | same |
| ClearScene/AddRefEntityToScene/AddPolyToScene | same + **AddRefSpriteToScene**, ClearFlares, SetDynamicLights, SetWindDirection/Intensity, SetTimeScale |
| AddLightToScene(org,intensity,r,g,b), AddAdditiveLightToScene | **AddLightToScene(..., int type)**, AddLensflareToScene, LensflareNameToNum |
| LightForPoint (EF1 polls player lighting) | absent |
| RenderScene, SetColor, DrawStretchPic, DrawStretchRaw, UploadCinematic | same (+more 2D helpers) |
| BeginFrame/EndFrame | same |
| MarkFragments/LerpTag/ModelBounds, A3D_RenderGeometry, RegisterFont, RemapShader, GetEntityToken, inPVS, TakeVideoFrame | same, plus **TIKI_*** (10 entry points), hwtex texture pipeline, view-screen API |
| **RequestSaveThumb/GetSaveThumb** (this port's savegame thumbnail) | absent |
| ResizeWindow (flexible display; never installed by renderervk today) | absent/different |

Only EF2's *GL layer* is reusable; the API it exports is not.

## 2. `refimport_t` — renderer → engine

| EF1 (this tree) | EF2 |
|---|---|
| `Cvar_CheckRange(cvar, const char *min, const char *max, int type)` — Quake3e signature, selected globally by `USE_VULKAN` in tr_public.h; any renderer module must use this variant | `Cvar_CheckRange(cvar, float, float, qboolean)` |
| `VKimp_Init/Shutdown`, `VK_GetInstanceProcAddr`, `VK_CreateSurface` — **Vulkan-specific**, implemented by the client's `sdl_glimp.c` and handed to the renderer through `ri` | no such hooks; its renderer owns the SDL/GLES context |
| `GLimp_SpawnRenderThread/RendererSleep/FrontEndSleep/WakeRenderer/ShutdownRenderThread` (r_smp) | different render-thread API |
| `CL_IsMinimized`, `CL_SetScaling`, `CL_SaveJPG*`, `Sys_SetClipboardBitmap`, `Com_RealTime`, `Cvar_SetGroup/CheckGroup/ResetGroup` stubs, `FS_FileIsInPAK`, `IN_Init/Shutdown/Restart`, `CM_ClusterPVS` | tiki/skeletor/hwtex calls, main-thread asset upload hooks |

Consequences for a GLES module here:

* It must not call `ri.VKimp_*`; those exist only for the Vulkan backend. The
  GLES renderer has to create its own SDL window and GLES 3.2 context, exactly
  as EF2's `gl_gles3_platform.c` does (`SDL_WINDOW_OPENGL`,
  `SDL_GL_CONTEXT_PROFILE_ES`, 3.2, `SDL_GL_MakeCurrent`).
* Everything else goes through `ri` — the module links no engine symbol, and
  `dlopen` stays `RTLD_LOCAL`, so the module's own `Com_Printf`/`Cvar_*`/`Q_*`
  copies (q_shared.c/q_math.c are compiled into every module) cannot interpose.
* The engine's `USE_VULKAN` define stays on for the client, which is why the
  Q3e `Cvar_CheckRange` signature is the one in the import table; GL-side code
  must call `ri.Cvar_CheckRange(cv, "min", "max", CV_INTEGER|CV_FLOAT)` and not
  the old float form.

## 3. Renderer-visible data

| Area | EF1 (this engine) | EF2 |
|---|---|---|
| World | BSP 46 (`qfiles.h: BSP_VERSION 46`), dlight → classic VQ3 textured-additive | BSP-derived + tiki worlds? EF2 uses its own lumps (no BSP 46) |
| Models | MD3 v15 (`MD3_IDENT`), MDR v2 (`MDR_IDENT`), IQM (`iqm.h`); per-surface shader names resolved at registration (`tr_model.c:544,813`), skins (`R_RegisterSkin`, EF facial viseme extension in `tr_image.c`) | tiki/skeletor `.tik/.skb/.ska` skeletal models with bone weights, morph controllers |
| Shader stages | `shaderStage_t` with up to 2 bundles, `tcGen` (incl. EF X/Y environment formula), `rgbGen/alphaGen`, texMods, `fogParms`, EF-only `depthfunc disable`, `RegisterShader3D` (un-mipped) | same stage model *plus* hwtex (ASTC/ETC2 transcode), program-cache permutations, lensflares |
| Entities | `refEntity_t` EF union (`data.sprite.radius/vertRGBA`, line/bezier/cylinder/electricity), RT_ORIENTEDSPRITE…RT_ELECTRICITY, `RF_FORCE_ENT_ALPHA`, `RF_FULLBRIGHT` | EF2 entity model with tiki handles and sprite APIs |
| Sky/portal/fog | EF1 sky (`r_origfastsky`), portals, fog collapse | same concepts, different code |

**Therefore:** the tr_* layer to adapt is *this port's own*
`renderervk/tr_*.c` (already EF1-correct, including the eight EF extensions and
the entity surface types), not EF2's tr_*. EF2's tr_* are bound to
tiki/skeletor/hwtex/ghost/swipe and bring the entire EF2 data model with them —
explicitly out of scope.

## 4. Implementation architecture (what replaced the plan below)

The plan of forking `renderervk/tr_*.c` was dropped: the module compiles those
files **unchanged** and implements their backend interface (`vk_*`,
`Vk_Pipeline_Def`, the `vk` state struct) in GLES 3.2 terms. That keeps one copy
of the EF1 front-end logic for both renderers. The only edits to files shared
with the Vulkan build are guarded by `USE_GLES3_BACKEND` and compile to the
original code otherwise:

* `renderervk/tr_init.c` — create the window/context through the module's own
  `GL3_PlatformInit` instead of the client's Vulkan `ri.VKimp_*`, skip the
  client-side window teardown, and refuse `GetRefAPI` while
  `renderer_backend_ready()` is 0.
* `renderervk/tr_local.h` — under `USE_GLES3_BACKEND`, include the real GLES
  headers and keep only the renderer-private tokens of the `glCompat`
  substitution enum (the standard GL names come from the GL header).
* `renderercommon/tr_common.h` — under `USE_GLES3_BACKEND`, include the GLES
  qgl table instead of SDL's desktop GL header.

`gl3_backend.c` implements: the pipeline-def registry (same `vk.pipelines[]`),
lazy GL program cache per `(shader_type, fog_stage)` with the same permutation
defines and specialization values the Vulkan backend bakes, state-bits →
blend/depth/cull/polygon-offset/stencil mapping, viewport/scissor rectangles,
reversed-depth conventions (clear 0, `GL_GEQUAL`), the std140 uniform block fed
by the front end's `VK_PushUniform`, per-draw attribute streaming and index
upload, GL textures (images and lightmaps, including sub-uploads and cinematic
frames), offscreen colour+depth targets for the main and screenMap/mirror
passes, the gamma composite on present, `glReadPixels` capture, and no-op/stub
implementations for the parts listed under "missing".

## 4b. The GL layer ported in the earlier step

* `qgl_gles3.{c,h}`, `qgl_gles3_procs.h`, `qgl_gles3_tokens.h` — EF2's GLES 3.2
  entry-point loader (name → `void (*)(void)` table, `glGetString` sanity, EGL/
  GLES diagnostics), with EF2's `q_version.h` dependency replaced by this
  engine's version macros and the include path switched to the local
  `renderercommon/qgl.h` compatibility header.
* `gl_gles3_platform.{c,h}` — EF2's SDL + EGL/GLES 3.2 context layer, reduced to
  what an EF1 renderer needs: create the window (`SDL_WINDOW_OPENGL`), request
  an ES 3.2 core context, load the entry points, fill this engine's
  `glconfig_t` (vidWidth/vidHeight/windowAspect/deviceSupportsGamma=false,
  colorBits 32, depthBits 24, stencilBits 8), `SDL_GL_SwapWindow`, shutdown.
  EF2's hwtex/firstlight/texture-transcode initialisation is deliberately not
  imported (see §3).
* `gl3_shaders.h` — the GLSL ES 3.20 versions of the shader permutations the
  existing tr_* logic needs (`gen_vert`/`gen_frag` with tx0/tx1/tx2, env, fog,
  entity colour, atest; `light`; `fog`; `color`; `dot`), converted from
  `renderervk/shaders/*.tmpl` by hand: `#version 320 es`, `precision highp
  float`, the `push_constant` mat4 becomes a `uniform mat4`, `layout(set=N,
  binding=M)` becomes `layout(binding=M)` on samplers, and Vulkan
  specialization constants become `#define`d permutations. Every source in the
  file compiles with `glslangValidator -S vert|frag` as ESSL 3.20 (see the
  header comment for the exact command).
* `gl3_renderer.c` — the module entry point. Exports `GetRefAPI` (returns NULL
  with an explicit message until the backend exists) and
  `renderer_backend_ready()` (returns 0). It also carries the module's private
  `Com_Printf`/`Com_Error` shims over `ri` (q_shared.c calls them; a module
  cannot link the engine's qcommon — same shims renderervk/tr_init.c has for
  the Vulkan module).

## 5. Milestone status (Android)

| Milestone | State |
|---|---|
| M1 context / frame loop / present / shutdown | written: SDL ES 3.2 context, offscreen main target, glClear, gamma composite + swap, `r_mode`/`r_fullscreen` re-sync on vid_restart, clean shutdown. Unverified — nothing has been run. |
| M2 2D/UI | written: the same program/state/attribute path the world uses; 2D transform with the GL Y flip, blending, scissor, fonts and console go through it. Unverified. |
| M3 world | written: pipeline-def → program/state, streaming attributes + indices, std140 UBO from `VK_PushUniform`, lightmaps and sub-uploads, shader stages (`tcGen`/`rgbGen`/`tcMod` are front-end, so they arrive as the same defs), alpha test, env mapping, fog (fog-collapse and fog-only), `RT_*` surfaces (front-end), MD3/MDR/IQM (front-end), screenMap/mirror target. Unverified. |
| M4 post-process | complete: offscreen colour+depth, multisampled main target + resolve honouring `r_ext_multisample` (Mali/PowerVR gated, as the Android port documents), bloom extract/blur/blend chain with the Vulkan constants (threshold/mode/modulate/intensity), gamma composite after bloom, screenshots and save-thumbnail via a general-size readback (full target read + nearest resample to the requested rectangle), cinematic/Bink upload through `vk_upload_image_data(update=qtrue)`. |
| M5 the rest | complete: `r_smp` single-threaded by explicit decision (forced off with a printed reason), flares via `GL_ANY_SAMPLES_PASSED` occlusion queries (one 1px draw + query per flare per frame, one-frame latency like Vulkan), context-loss recovery through `GL_CONTEXT_LOST` + the client's existing teardown/recreate loop, program binary cache (`glGetProgramBinary`/`glProgramBinary`, disk file under `fs_homepath`, stamp-validated, atomic write, source fallback) with a prewarm sweep, and per-pixel lighting reachable (`r_dlightMode` no longer forced; fog/dlight/lighting pipeline defs created). |

Known gaps that a first launch will hit if the module is selected:
bloom is ignored with a warning; MSAA is ignored; `r_smp` must stay 0;
flares do nothing; the `glReadPixels` capture path assumes the render target
matches `gls.captureWidth/Height` (true with the shipped `r_renderScale 0`,
`r_ext_supersample 0`).

## 5b. What is missing (the original plan, kept for the remaining work)

1. **Backend layer** (`gl3_backend.c`): implement the drawing interface the
   tr_* files call, in GL terms —
   `gl3_find_pipeline_ext`/`gl3_get_pipeline_def` (keep the Vulkan renderer's
   `Vk_Pipeline_Def` keying; it is plain data), `gl3_bind_pipeline` (def →
   program + blend/depth/cull/polygon-offset GL state), `gl3_bind_geometry`/
   `gl3_bind_index`/`gl3_draw_geometry` (VAO/VBO ring from `tess`),
   `gl3_update_mvp`/`gl3_update_descriptor` (std140 UBO + sampler bindings),
   `gl3_create_image`/`gl3_upload_image_data`/`gl3_image` (GL textures,
   replacing the `VkImage` handles).
2. **tr_* adaptation**: copy `renderervk/tr_*.c` into this directory and rename
   the backend prefix (`vk_` → `gl3_`, `vk.` → `gl3.`, `Vk_Pipeline_Def` →
   `gl3_pipeline_def_t`), keeping the EF1 logic byte-for-byte otherwise. The
   compiler then enumerates the remaining interface (35 entry points, 22
   `gl3.` fields at last count).
3. **Render targets**: FBO + optional MSAA resolve, gamma pass (the GLES
   equivalent of `vk_bloom`/`vk_create_post_process_pipeline`), which is also
   where `r_fbo`/screenshots/cinematic upload attach. Not implemented; GLES can
   render straight to the default framebuffer first, losing bloom and the gamma
   post-pass until this lands.
4. **Render thread** (`gles3_renderthread.c` in EF2): r_smp support. The
   single-threaded path is sufficient to start.
5. **Flares**: `vk_flares.c` uses a storage-buffer depth test; GLES needs a
   different visibility test (or the same fragment-shader-interlock technique
   via `GL_EXT_shader_framebuffer_fetch`/SSBO on ES 3.1+). Not ported.
6. **Video menu value**: text is already assigned in code ("GLES 3.2"); the row
   appears automatically once `renderer_backend_ready()` returns 1.

## 6. Load/fallback contract (why an incomplete module cannot brick the game)

* `renderer_backend_ready()` is exported by every renderer module (Vulkan: 1).
  The client's module probe (`CL_PublishRendererModules`) only lists a backend
  whose `renderer_backend_ready()` returns 1, so the UI never offers a
  half-built module.
* If a user nevertheless sets `cl_renderer gles3` (console), the client loads
  the module, calls `GetRefAPI`, gets NULL, unloads it, force-resets
  `cl_renderer` to its compiled default (`vulkan`), retries the load, and only
  fails fatally if that also fails — i.e. the same reset-then-fatal shape as a
  missing module. A half-ported backend therefore degrades to Vulkan instead of
  crashing.
