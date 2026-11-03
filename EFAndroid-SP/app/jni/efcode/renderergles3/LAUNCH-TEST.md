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

## 5. What to expect from the optional paths

* bloom: honoured (`r_bloom`, `r_bloom_threshold`, `_threshold_mode`,
  `_intensity`, `_modulate`). The log prints nothing extra; a failed bloom
  program build prints one warning and disables bloom.
* MSAA: honoured (`r_ext_multisample`), except on Mali/PowerVR where it is
  disabled with a printed line (the documented device-loss caution). The init
  log prints the chosen `samples`.
* flares: honoured (`r_flares`) through occlusion queries; cost is one 1px draw
  and one query per flare per frame with a one-frame latency.
* `r_smp` is forced to 0 with a printed reason (single GL context).
* `r_dlightMode` 0 (classic multipass) and 1/2 (per-pixel) are both supported;
  the per-pixel path uses the lighting pipeline defs created at init.
* static world geometry is re-submitted per draw (no world VBO; deliberate).
* the program binary cache lives at `<fs_homepath>/gl3_programs.bin` and is
  revalidated against a vendor/renderer/version/uniform-size stamp each run;
  any mismatch recompiles from source.

`renderer_gles3/AUDIT.md` §5 has the full milestone table and remaining work.


## 6. After the 1.2.1 colour fix — what should change, what to send if not

The 1.2.1 module fixes three root causes found from the 1.2.0 device log
(Pixel 8 / Mali-G715). No per-texture swizzles were added; the byte order was
verified against each producer instead.

What the log must now show, and what changed:

* `GLES3: backend initialized (2400x1080, gamma pass on, programs=N)` — the
  gamma program now builds. 1.2.0 said "no gamma pass": Mali rejected
  `gamma.vert` because the converted GLSL carried C-style `1.0f` literals
  (glslang tolerated them, GLSL ES does not). The converter now strips the
  suffix; the same fixed vertex shader also feeds the bloom/blur/blend passes.
  The final image is therefore the gamma/bloom composite of the resolved main
  target, not the raw blit.
* No `GL_INVALID_VALUE`/`GL_INVALID_OPERATION` lines at present. The uniform
  buffer object was only `glGenBuffers`'d, never allocated, so every draw's
  `glBufferSubData(0, 128)` exceeded a zero-sized store (INVALID_VALUE) and the
  fog/env/entity-colour uniforms never reached the shaders. That is why the 2D
  UI (whose shaders do not declare the block) looked right while the 3D scene
  did not. It is now allocated once as a dynamic uniform buffer.
* Wrong colours on multi-texture / fixed-colour / identity stages were also
  caused by the program's default uniforms being written without making the
  program current (INVALID_OPERATION, and every value left at zero:
  `tex_mode`, `color_mode`, `identity_color/alpha`). The defaults are now set
  with the program bound, and `tex_mode`/`color_mode` are re-applied per draw.

Byte order (proven, not assumed): every producer feeds RGBA — the TGA/BMP
decoders write RGBA, the JPEG decoder explicitly expands RGB to RGBA, lightmap
tiles are RGBA 4-byte pixels, the cinematic decoder (`sp/bink_ff.c`) converts to
`AV_PIX_FMT_RGBA`, and `ResampleTexture`/the mip builder operate on 4-byte
pixels. The Vulkan backend uploads these same buffers as
`VK_FORMAT_R8G8B8A8_UNORM` (not BGRA). `GL_RGBA`/`GL_RGBA8` is therefore the
correct upload for every class, and the UI art already rendered correctly with
it — a global R/B swap would have broken the UI.

If something is still wrong after flashing 1.2.1, send the log from the first
~10 seconds plus the output of `gles3_selftest`, specifically:
1. the `backend initialized (...)` line (must say `gamma pass on`);
2. every `GLES3:` warning line (shader compile/link failures name the
   permutation defines; GL errors now print the call group and `file:line`:
   `bind index`, `draw: state`, `draw: uniform upload`, `present`);
3. the selftest `UBO block=... data_size=... expected=128 (delta=...)` line
   (delta must be 0) and the `attrib in_*` lines (0/1/2/5);
4. whether the wrong colour is in the whole 3D view, only fogged areas, only
   certain surfaces, or only the FMV — that distinguishes the remaining fog,
   env, multitexture or cinematic paths.

The intro FMV is a cinematic quad through the same 2D path and the same RGBA
upload as the UI; with the uniform-default and UBO fixes in, if it is still
magenta the next step is to add a one-shot log of the scratch image's
`uploadWidth/uploadHeight` versus the frame's `cols/rows` (the only remaining
way the upload rectangle can disagree with the texture).
