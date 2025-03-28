
void DECOR_ENT_sim(Entity *e)
{
    //none lol
}



void COLLECTABLE_ENT_sim(Entity *e)
{
    if(gs.soda_achieved)
    {
        set_entity_sprite(e, SODA_SPLOD_SPR);
        if(e->frame >= 12)
            e->invisible = true;
    }
}



void PLAYER_ENT_sim(Entity *e)
{
    GameControllerInput* in = &gi->controller;
    if(in->move_right.ended_down)
    {
        e->pos.x += gi->elapsed_seconds * 80.0f;
        set_entity_sprite(e, HOG_RUN_SPR);
    }
    else
    {
        set_entity_sprite(e, HOG_IDLE_SPR);
    }
    
    if(e->pos.x > 600 && !gs.soda_achieved)
    {
        Mix_PlayChannel(-1, yummy_snd,0);
        gs.soda_achieved = true;
    }
    
}