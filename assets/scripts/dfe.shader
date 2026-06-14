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

dfe/gob
{
    deformvertexes move 0 0 0.1 square 0 1 0 0
    {
        map textures/dfe/g.jpg
        blendfunc gl_src_alpha gl_one
        rgbgen const ( 0 0.5 0.75 )
        alphagen vertex
    }
}

dfe/job
{
    deformvertexes move 0 0 0.1 square 0 1 0 0
    {
        map textures/dfe/j.jpg
        blendfunc gl_src_alpha gl_one
        rgbgen const ( 0.5 0.5 0.1 )
        alphagen vertex
    }
}
