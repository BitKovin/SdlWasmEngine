/* date = February 16th 2024 10:08 pm */

#ifndef HOG_H
#define HOG_H

#define NUM_ENTITIES 300

typedef struct
{
    //ImageId bg;
    
    u32 font_handle;
    
    Entity entities[NUM_ENTITIES];
    u32 num_entities;
    r32 total_time;
    b32 soda_achieved;
}GameState;


GameState gs;
GameInput* gi;

void hog_init();
void hog_update(GameInput* game_input);

#endif //HOG_H
