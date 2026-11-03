#!/usr/bin/env python3
"""Regenerate the GLES 3.2 shader sources from renderervk's templates.

The Vulkan renderer's shaders are GLSL 4.50 with Vulkan-only constructs
(push constants, descriptor-set bindings, specialization constants). The GLES
backend renders the same tr_* geometry with the same math, so the shaders are
derived mechanically rather than rewritten:

  * #version 450 -> #version 320 es (+ precision qualifiers)
  * layout(push_constant) Transform block -> plain `uniform mat4 mvp`
  * layout(set=N, binding=M) -> layout(binding=M)  (a Vulkan descriptor set per
    texture becomes a GL texture unit; the block keeps a std140 layout)
  * layout(constant_id=N) const T name = default -> `uniform T name`
    (per-pipeline constants become per-draw uniforms; the backend must upload
    every one of them -- their old defaults are named in the comment)
  * Vulkan's `out gl_PerVertex {}` block is implicit in GLSL ES 3.20
  * Vulkan's layout(depth_less) redeclaration of gl_FragDepth is desktop-only;
    ES 3.20 has gl_FragDepth as a built-in output

Usage: python3 convert_glsl.py   (run from this directory)
"""
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.normpath(os.path.join(HERE, "..", "..", "renderervk", "shaders"))
FILES = ["gen_vert.tmpl", "gen_frag.tmpl", "light_vert.tmpl", "light_frag.tmpl",
         "fog.frag", "color.vert", "color.frag",
         # post-process (gamma composite; bloom/blur kept for parity work)
         "gamma.vert", "gamma.frag", "bloom.frag", "blend.frag", "blur.frag"]

HEADER = """#version 320 es

// Converted from renderervk/shaders/%(name)s for GLES 3.2 by
// renderergles3/shaders/convert_glsl.py. Do not edit by hand: edit the source
// template and re-run the script.
//
// GLES conversions applied: #version 320 es + precision qualifiers; Vulkan
// push-constant block -> `uniform mat4 mvp`; descriptor sets -> GL texture
// units (`layout(binding = N)`); specialization constants -> uniforms (the
// backend uploads all of them per draw, defaults listed inline below);
// gl_PerVertex and layout(depth_less) are GLSL ES built-ins.
//
precision highp float;
precision highp int;
"""


def convert(name, text):
    def spec(m):
        cid, typ, nm, val = m.group(1), m.group(2), m.group(3), m.group(4)
        return "uniform %s %s; // was constant_id=%s default %s" % (typ, nm, cid, val)

    t = re.sub(r'layout\s*\(\s*constant_id\s*=\s*(\d+)\s*\)\s*const\s+(\w+)\s+(\w+)\s*=\s*([^;]+);', spec, text)
    t = re.sub(r'#version 450[^\n]*\n', HEADER % {"name": name}, t, count=1)
    t = re.sub(r'layout\(push_constant\)\s*uniform\s+Transform\s*\{\s*mat4\s+mvp;\s*\};',
               'uniform mat4 mvp;', t)
    t = re.sub(r'layout\(set\s*=\s*\d+\s*,\s*binding\s*=\s*(\d+)\)\s*uniform\s+UBO',
               r'layout(std140, binding = \1) uniform UBO', t)
    # Texture units: in Vulkan every sampler lives in its own descriptor set at
    # binding 0, so the set number carried no information; in GL all samplers
    # must have distinct units. The unit is the sampler's role: texture0..2 ->
    # units 0..2, fog_texture -> unit 3.
    def sampler_unit(m):
        name = m.group(3)
        if name.startswith("texture") and name[7:].isdigit():
            unit = int(name[7:])
        elif name == "fog_texture":
            unit = 3
        else:
            unit = 0
        return "layout(binding = %d) uniform sampler2D %s" % (unit, name)

    t = re.sub(r'layout\(set\s*=\s*\d+\s*,\s*binding\s*=\s*(\d+)\)\s*uniform\s+(sampler\w+)\s+(\w+)',
               sampler_unit, t)
    t = re.sub(r'out gl_PerVertex\s*\{[^}]*\};', '', t)
    t = re.sub(r'layout\s*\(depth_less\)\s*out\s+float\s+gl_FragDepth;',
               '// gl_FragDepth is a built-in output in GLSL ES 3.20', t)

    # GLSL ES 3.20 has no implicit int->float conversion (desktop GLSL does),
    # so the few integer literals the templates use in float expressions need a
    # decimal point. Both are documented here rather than in the templates,
    # which the bake for Vulkan depends on byte-for-byte.
    t = t.replace("in_normal.xy * 2 * d", "in_normal.xy * 2.0 * d")
    t = t.replace("diffuse * dot(N, nV) <= 0 ", "diffuse * dot(N, nV) <= 0.0 ")
    # identity_color/identity_alpha were specialization constants there; now
    # they are uniforms, so this local cannot be `const` in ES.
    t = t.replace("const vec4 frag_color = vec4(identity_color",
                  "vec4 frag_color = vec4(identity_color")

    # GLSL ES 3.20 has no C-style `{ }` initializers: they must be constructor
    # calls. Covers the constant colour/luma vectors in the post-process
    # shaders.
    t = re.sub(r'const (vec[234])\s+(\w+)\s*=\s*\{([^}]*)\};',
               lambda m: "const %s %s = %s(%s);" % (m.group(1), m.group(2), m.group(1), m.group(3)),
               t)
    # ... and the ordered-dither matrix, which becomes an array constructor.
    def float_array(m):
        body = m.group(3)
        # ES has no implicit int -> float conversion, and the matrix literals are
        # written as integers.
        body = re.sub(r'(?<![\w.])(\d+)(?![\d.])', r'\1.0', body)
        return "const float %s[%s] = float[%s](%s);" % (m.group(1), m.group(2), m.group(2), body)

    t = re.sub(r'const float (\w+)\[(\w+(?:\s*\*\s*\w+)?)\]\s*=\s*\{([^}]*)\};',
               float_array, t)
    # a `const` local cannot be initialised from a non-constant expression in ES
    t = t.replace("const float v = dot(", "float v = dot(")
    # no implicit ivec3 -> vec3 arithmetic in ES; the constructor converts
    t = t.replace("ivec3 depth = ivec3(", "vec3 depth = vec3(")
    # greyscale was a specialization float; ES will not compare float to int
    t = t.replace("greyscale == 1 )", "greyscale == 1.0 )")
    t = t.replace("greyscale != 0 )", "greyscale != 0.0 )")
    # Vulkan's gl_VertexIndex is gl_VertexID in GL.
    t = t.replace("gl_VertexIndex", "gl_VertexID")

    # GLSL has no C-style floating-point suffix; glslang tolerates "1.0f" but
    # mobile GLES compilers (Mali in particular) reject it, which is how the
    # gamma program failed to build on the test device. Strip it.
    t = re.sub(r'(\d(?:\.\d*)?(?:[eE][-+]?\d+)?)[fF](?![\w.])', r'\1', t)
    return t.rstrip() + "\n"


def main():
    for f in FILES:
        out = f.replace(".tmpl", ".glsl")
        text = convert(f, open(os.path.join(SRC, f)).read())
        with open(os.path.join(HERE, out), "w") as fh:
            fh.write(text)
        print("wrote", out)


if __name__ == "__main__":
    main()
