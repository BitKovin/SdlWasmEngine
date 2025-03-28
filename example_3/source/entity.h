/* date = February 22nd 2024 0:45 am */

#ifndef ENTITY_H
#define ENTITY_H



#define LIST_OF_ENTITY_TYPES \
X(PLAYER_ENT) \
X(DECOR_ENT) \
X(COLLECTABLE_ENT) \


#define X(name) name,
typedef enum
{
    LIST_OF_ENTITY_TYPES
        TOTAL_ENTITY_TYPES
}EntityType;
#undef X




typedef struct
{
    EntityType type;
    V2 pos;
    SpriteId sprite_id;
    r32 frame_time;
    u32 frame;
    b32 invisible;
}Entity;


#define X(name) void name##_sim(Entity* );
LIST_OF_ENTITY_TYPES
#undef X


#define X(name) [name] = &name##_sim,
void (* entity_sims[TOTAL_ENTITY_TYPES])(Entity*) = 
{
    LIST_OF_ENTITY_TYPES
};
#undef X



void entities_apply(void (*f) (Entity*));



void set_entity_sprite(Entity*, SpriteId );

Entity* add_entity(Entity* e);




#endif //ENTITY_H
