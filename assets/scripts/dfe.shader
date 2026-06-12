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
