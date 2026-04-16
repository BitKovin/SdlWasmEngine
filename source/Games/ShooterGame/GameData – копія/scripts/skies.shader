textures/skies/skybox1_cube
{
    surfaceparm noimpact
    surfaceparm nolightmap
    surfaceparm sky
    q3map_lightimage env/skybox1_cube_up.png
    q3map_sun        0.82 0.92 1 0 -45 80
    //q3map_surfacelight 20
    q3map_skylight 15 4

    skyparms env/skybox1_cube - -
    {
        map env/skybox1_cube_up.png
        blendfunc GL_ONE GL_ONE
        tcMod scroll 0.05 0.06
        tcMod scale 3 2
    }
}

textures/skies/skybox2_overcast_cube
{
    surfaceparm noimpact
    surfaceparm nolightmap
    surfaceparm sky

    q3map_lightimage env/skybox2_overcast_cube_up.png

    // --- SKY LIGHT CONTROL ---
    // Weak ambient skylight (brightness 20, iterations 3)
    q3map_skylight 20 3

    // Optional: dim sun to match overcast feel
    q3map_sun 0.6 0.6 0.7 20 -15 70

    // --- SKYBOX DRAW ---
    skyparms env/skybox2_overcast_cube - -
    {
        map env/skybox2_overcast_cube_up.png
        blendfunc GL_ONE GL_ONE
        tcMod scroll 0.05 0.06
        tcMod scale 3 2
    }
}

textures/skies/skybox2_overcast_night_cube
{
    surfaceparm noimpact
    surfaceparm nolightmap
    surfaceparm sky

    q3map_lightimage env/skybox2_overcast_cube_up.png

    // --- SKY LIGHT CONTROL ---
    // Weak ambient skylight (brightness 20, iterations 3)
    q3map_skylight 0 3

    // Optional: dim sun to match overcast feel
    q3map_sun 0.6 0.6 0.7 0 -15 70

    // --- SKYBOX DRAW ---
    skyparms env/skybox2_overcast_night_cube - -
    {
        map env/skybox2_overcast_night_cube.png
        blendfunc GL_ONE GL_ONE
        tcMod scroll 0.05 0.06
        tcMod scale 3 2
    }
}
