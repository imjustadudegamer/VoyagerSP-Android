# renderer_gles3 — launch test guide

This is for the first run **when the user decides to boot the game**. Nothing
here has been executed by the developer; the renderer is built, statically
verified (ELF, shader validation) and instrumented so that a single run says
exactly where it stops.

## 0. Status you are testing

`renderer_backend_ready()` returns 0, so the client keeps using Vulkan. The
GLES backend is opt-in:

* **without a rebuild**: put `seta cl_renderer "gles3"` and
  `seta gles3_forceReady "1"` in `efspconfig.cfg` (or pass
  `+set cl_renderer gles3 +set gles3_forceReady 1`), then start the game;
* the forced case loads `librenderer_gles3.so`, runs `GetRefAPI`, and either
  initializes (log below) or returns NULL and the client falls back to Vulkan
  automatically (the "Load/fallback" path in `AUDIT.md §6`).

`gles3_forceReady` is read only by the game client, never by the module probe,
so the video menu keeps offering Vulkan only until readiness is turned on in
code (`gl3_backend.c`, `renderer_backend_ready`).

## 1. What to run

1. Start the game with the two cvars above.
2. Watch the log (`qconsole.log` in the game home dir, or `adb logcat -s
   VoyagerSP` on Android; `developer 1` shows the console).
3. At the main menu, open the console and run **`gles3_selftest`**.
4. Then enter a map.

## 2. What the log must print, stage by stage

```
----- Initializing Renderer ----
GLES3: vendor   : ...
GLES3: renderer : ...
GLES3: version  : OpenGL ES 3.2 ... (ES 3,2)
GLES3: config   : 1920x1080 depth=24 stencil=8 multisample=0 (samples=1) fbo=1
GLES3: extensions: ...
GLES3: backend initialized (1920x1080, gamma pass on, programs=N)
Renderer modules available: vulkan
```
(`programs=0` at this point is normal: programs compile on first bind.)

`gles3_selftest` must print:
* `objects: programs=N textures=M fbo=.. screenmap_fbo=.. vbo=.. ibo=.. ubo=.. vao=..`
* `UBO block=0 data_size=128 expected=128 (delta=0)` — **delta must be 0**; a
  non-zero delta means the GLSL block and `vkUniform_t` disagree and colours,
  fog or lighting will be wrong;
* `attrib in_position=0`, `in_color0=1`, `in_tex_coord0=2`,
  `in_normal=5 or -1` — other values mean the attribute contract in
  `gl3_backend.c` (the tess-flags table) needs adjusting;
* `main FBO status=complete`, `screenMap FBO status=complete`;
* no `GLES3: <GL error> at ...` lines (the sweep prints the first 16 with
  `file:line`, then suppresses).

## 3. Likely failure signatures

| Log / symptom | Meaning | Where to look |
|---|---|---|
| `renderer_gles3: ... not marked ready; refusing to start` then the game runs on Vulkan | `gles3_forceReady` was not set (or not archived into the config actually read). The fallback worked; nothing is broken. | the two cvars, section 0 |
| `GLES3: shader compile failed (vertex/fragment) defines=...: <log>` | A program permutation did not compile on this driver. The message names every `#define` used for that permutation. | `gl3_backend.c` `GL3_ClassifyDef` / `GL3_GetProgram`; the GLSL is `shaders/*.glsl` |
| `GLES3: program link failed: ...` | Stage locations disagree (varyings) even though each stage compiled. | `convert_glsl.py` locations |
| `GLES3 selftest: UBO ... (delta != 0)` | Driver padded the block differently; every draw would read shifted colours/fog. | `vkUniform_t` vs `gen_vert/gen_frag` block |
| `GLES3: GL_INVALID_OPERATION at ...` / `INVALID_VALUE at ...` | First offending call site is printed. Most likely a buffer not sized before `glBufferSubData`, a texture unit, or a draw with no program. | the printed `file:line` |
| `main FBO status=INCOMPLETE` | Depth/stencil renderbuffer format unsupported. | `GL3_CreateTarget` |
| `GLES3: context lost` / `GL_CONTEXT_LOST` | Driver reset the EGL context; the client will recreate the renderer. | `vk_device_lost()` path |
| Game runs but the screen is black, selftest clean | Most likely a draw-path issue rather than init: check whether the log shows `programs=N` growing, then the tess-flag/attribute table. | `GL3_BindTessellation`, `GL3_DefaultMVP` |

## 4. One-step fallback to Vulkan

Console: `setrenderer vulkan` (writes the config immediately), then restart the
game. Or edit `efspconfig.cfg` and replace `cl_renderer "gles3"` with
`cl_renderer "vulkan"` and remove `gles3_forceReady`. No rebuild, no reinstall;
`cl_rendererModules` will list only `vulkan` again.

## 5. What is not implemented (expect missing visuals, not crashes)

* bloom is ignored (`r_bloom` prints one warning), MSAA is ignored
  (`r_ext_multisample` is logged but not honoured);
* flares do nothing (`r_flares` has no effect);
* `r_smp` is forced to 0 with a printed reason (single GL context);
* `r_dlightMode` keeps the classic multipass dlight path;
* static world geometry is re-submitted per draw (no world VBO);
* no warm program cache across runs.

`renderer_gles3/AUDIT.md` §5 has the full milestone table and remaining work.
