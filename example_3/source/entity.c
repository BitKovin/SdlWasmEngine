




Entity* add_entity(Entity* e)
{
    if(gs.num_entities >= NUM_ENTITIES)
        return NULL;
    Entity* t = &gs.entities[gs.num_entities];
    *t = *e;
    ++gs.num_entities;
    return t;
}


void
entities_apply(void (*f) (Entity* e))
{
    for(u32 i = 0; i <= gs.num_entities - 1; i++)
    {
        f(&gs.entities[i]);
    }
}



void set_entity_sprite(Entity* e, SpriteId id)
{
    if(e->sprite_id == id)
        return;
    e->sprite_id = id;
    e->frame = 0;
}


void update_entity(Entity* e)
{
    entity_sims[e->type](e);
}


void update_entities()
{
    entities_apply(&update_entity);
}