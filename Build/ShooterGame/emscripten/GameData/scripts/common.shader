textures/common/nolightmap
{
	surfaceparm nolightmap
}

textures/common/trigger
{
    qer_editorimage textures/lq_utility/trigger.png

    // make this surface invisible to the standard light build
    //surfaceparm nodraw
    surfaceparm nolightmap
    q3map_nolightmap

    // disable the ambient-occlusion (-dirty) pass
    q3map_noDirty

    // still allow the trigger to be solid/functional if needed
    surfaceparm nonsolid
}

textures/common/clip
{
    qer_editorimage textures/lq_utility/clip.png
	qer_trans 0.40
	surfaceparm 	nolightmap
	surfaceparm nomarks
	//surfaceparm nodraw
	surfaceparm nonsolid
        //surfaceparm 	nolightmap //proto_addition 11/08/99
	surfaceparm playerclip
	surfaceparm noimpact
}

textures/delvenPack/dlv_door1a
{	
	surfaceparm noclip
}