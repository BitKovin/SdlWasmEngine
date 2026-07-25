textures/skies/skybox1_cube
{
    surfaceparm noimpact
    surfaceparm nolightmap
    surfaceparm sky
    q3map_lightimage env/skybox1_cube_up.jpg
    q3map_sun        0.82 0.92 1 0 -45 80
    //q3map_surfacelight 20
    q3map_skylight 15 4

    skyparms env/skybox1_cube - -
    {
        map env/skybox1_cube_up.jpg
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

    q3map_lightimage env/skybox2_overcast_cube_up.jpg

    // --- SKY LIGHT CONTROL ---
    // Weak ambient skylight (brightness 20, iterations 3)
    q3map_skylight 80 3

    // Optional: dim sun to match overcast feel
    q3map_sun 0.6 0.6 0.7 20 -15 70

    // --- SKYBOX DRAW ---
    skyparms env/skybox2_overcast_cube - -
    {
        map env/skybox2_overcast_cube_up.jpg
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

    q3map_lightimage env/skybox2_overcast_cube_up.jpg

    // --- SKY LIGHT CONTROL ---
    // Weak ambient skylight (brightness 20, iterations 3)
    q3map_skylight 190 3

    // Optional: dim sun to match overcast feel
    q3map_sun 0.6 0.6 0.7 0 -15 70

    // --- SKYBOX DRAW ---
    skyparms env/skybox2_overcast_night_cube - -
    {
        map env/skybox2_overcast_night_cube.jpg
        blendfunc GL_ONE GL_ONE
        tcMod scroll 0.05 0.06
        tcMod scale 3 2
    }
}

textures/skies/skybox_red_cube
{
    surfaceparm noimpact
    surfaceparm nolightmap
    surfaceparm sky

    q3map_lightimage env/skybox_red_cube_up.jpg

    // --- SKY LIGHT CONTROL ---
    // Weak ambient skylight (brightness 20, iterations 3)
    q3map_skylight 120 5

    // Optional: dim sun to match overcast feel
    q3map_sun 0.7 0.2 0.2 0 -15 70

    // --- SKYBOX DRAW ---
    skyparms env/skybox_red_cube - -
    {
        map env/skybox_red_cube.jpg
        blendfunc GL_ONE GL_ONE
        tcMod scroll 0.05 0.06
        tcMod scale 3 2
    }
}

textures/skies/skybox_overcast_cube
{
    surfaceparm noimpact
    surfaceparm nolightmap
    surfaceparm sky

    q3map_lightimage env/skybox_overcast_cube_up.jpg

    // --- SKY LIGHT CONTROL ---
    // Weak ambient skylight (brightness 20, iterations 3)
    q3map_skylight 110 3

    // Optional: dim sun to match overcast feel
    q3map_sun 0.4 0.4 0.4 0 -15 70

    // --- SKYBOX DRAW ---
    skyparms env/skybox_overcast_cube - -
    {
        map env/skybox_overcast_cube.jpg
        blendfunc GL_ONE GL_ONE
        tcMod scroll 0.05 0.06
        tcMod scale 3 2
    }
}

textures/skies/skybox_cloudy_cube
{
    surfaceparm noimpact
    surfaceparm nolightmap
    surfaceparm sky

    q3map_lightimage env/skybox_cloudy_cube_up.jpg

    // --- SKY LIGHT CONTROL ---
    // Weak ambient skylight (brightness 20, iterations 3)
    q3map_skylight 180 3

    // Optional: dim sun to match overcast feel
    q3map_sun 1.0 1.0 1.0 0 -15 70

    // --- SKYBOX DRAW ---
    skyparms env/skybox_cloudy_cube - -
    {
        map env/skybox_cloudy_cube.jpg
        blendfunc GL_ONE GL_ONE
        tcMod scroll 0.05 0.06
        tcMod scale 3 2
    }
}
