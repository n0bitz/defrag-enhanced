dfe/spawnpoint
{
    cull none
    deformvertexes move 0 0 0.5 sin 2 1 0 0.35
    {
        map $whiteimage
        blendfunc blend
        rgbgen entity
        alphagen entity
    }
    {
        map textures/sfx/hologirl.jpg
        tcmod scale 5 25
        tcmod scroll 0 0.5
        blendfunc gl_src_alpha gl_one
        rgbgen entity
        alphagen entity
    }
    {
        map textures/sfx/hologirl.jpg
        tcmod transform 0 1 1 0 0 0
        tcmod scale 5 25
        tcmod scroll 0 0.25
        blendfunc gl_src_alpha gl_one
        rgbgen entity
        alphagen entity
    }
}

// Using deformvertexes instead of polygonoffset on the OB shaders to combat
// z fighting with the surface because polygonoffset effectively pushes the
// surface toward the viewer, which can cause us to see things through walls in
// certain cases.

dfe/obHighlight
{
    deformvertexes move 0 0 0.1 square 0 1 0 0
    {
        map $whiteimage
        blendfunc gl_src_alpha gl_one
        rgbgen vertex
        alphagen vertex
    }
}

dfe/obGo
{
    sort additive
    deformvertexes move 0 0 0.1 square 0 1 0 0
    {
        map textures/dfe/g.tga
        blendfunc gl_zero gl_one
        alphafunc ge128
        depthwrite
    }
    {
        map textures/dfe/g.tga
        blendfunc gl_src_alpha gl_one_minus_src_alpha
        depthfunc equal
        alphagen vertex
    }
}

dfe/obJump
{
    sort additive
    deformvertexes move 0 0 0.1 square 0 1 0 0
    {
        map textures/dfe/j.tga
        blendfunc gl_zero gl_one
        alphafunc ge128
        depthwrite
    }
    {
        map textures/dfe/j.tga
        blendfunc gl_src_alpha gl_one_minus_src_alpha
        depthfunc equal
        alphagen vertex
    }
}
