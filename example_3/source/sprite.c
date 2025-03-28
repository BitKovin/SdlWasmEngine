
inline const Sprite*
get_sprite(SpriteId id)
{
    return &sprites[id];
}


const Sprite*
get_entity_sprite(Entity *e)
{
    return &sprites[e->sprite_id];
}



void
update_sprite(Entity* e)
{
    //printf("her1\n, sprite_id: %d", e->sprite_id);
    const Sprite* spr = get_entity_sprite(e);
    //get_entity_sprite();
    //const Sprite* spr = &sprites[e->sprite_id];
    //printf("her2! count: %d, %d\n", spr->frame_count, spr->image_id);
    if (spr->frame_count <= 1)
        return;
    e->frame_time += gi->elapsed_seconds;
    //printf("elsapsed: %f\n", gi->elapsed_seconds);
    r32 max_frame_time = (1.0f/(r32)spr->fps);
    //r32 max_frame_time = 0.042;
    //printf("max_frame_time: %f , frame_time: %f\n",max_frame_time, e->frame_time);
    if(e->frame_time > max_frame_time)
    {
        e->frame_time += -max_frame_time;
        ++e->frame;
        if(e->frame_time >= max_frame_time)
            e->frame_time = 0;
        //printf("swap\n");
        if(e->frame >= spr->frame_count)
            e->frame = 0;
    }
}



void
update_sprites()
{
    entities_apply(&update_sprite);
}

