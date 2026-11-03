#!/usr/bin/env python3
"""Compile every GLES shader permutation with glslangValidator.

There is no GL compiler in the tree at runtime, and the renderer builds its
programs with glCompileShader() from these sources, so this is the static gate:
each `#version 320 es` source is compiled as ESSL 3.20 for exactly the
permutations the Vulkan renderer bakes (see renderervk/shaders/compile.bat,
plus the tr_* stage combinations). Run from this directory.
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
os.chdir(HERE)

VERT = "gen_vert.glsl"
FRAG = "gen_frag.glsl"

COLOR_VARIANTS = [[], ["USE_CLX_IDENT"], ["USE_FIXED_COLOR"], ["USE_CL1"], ["USE_CL2"]]
TEX_VARIANTS = [[], ["USE_TX1"], ["USE_TX2"]]
FRAG_COLOR = [[], ["USE_CLX_IDENT"], ["USE_FIXED_COLOR"], ["USE_ENT_COLOR"]]

cases = []

# gen_vert: texture count x colour source x fog x env
for color in COLOR_VARIANTS:
    for tex in TEX_VARIANTS:
        for fog in ([], ["USE_FOG"]):
            for env in ([], ["USE_ENV"]):
                cases.append((VERT, color + tex + fog + env))
# gen_frag: atest x depth fragment x texture count x colour source x fog
for color in FRAG_COLOR:
    for tex in TEX_VARIANTS:
        for fog in ([], ["USE_FOG"]):
            for atest in ([], ["USE_ATEST"]):
                # Vulkan bakes USE_DF only together with USE_ATEST
                # (compile.bat: -DUSE_CLX_IDENT -DUSE_ATEST -DUSE_DF), so the
                # depth-fragment variant is only meaningful there -- the
                # depth_fragment uniform is declared inside USE_ATEST.
                for df in (([], ["USE_DF"]) if atest else ([],)):
                    cases.append((FRAG, color + tex + fog + atest + df))
# light / fog / colour
for fog in ([], ["USE_FOG"]):
    cases.append(("light_vert.glsl", fog))
    cases.append(("light_frag.glsl", fog))
cases.append(("fog.frag", []))
cases.append(("color.vert", []))
cases.append(("color.frag", []))
# post-process passes (no permutations)
for f in ("gamma.vert", "gamma.frag", "bloom.frag", "blend.frag", "blur.frag"):
    cases.append((f, []))

fail = 0
for src, defs in cases:
    cmd = ["glslangValidator", "-l", "-S", "vert" if src.endswith(".vert") or "vert" in src else "frag", src]
    cmd += ["-D" + d for d in defs]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        fail += 1
        print("FAIL", src, defs)
        print(r.stdout[-1500:])

print("%d permutations, %d failures" % (len(cases), fail))
sys.exit(1 if fail else 0)
