/* date = February 21st 2024 4:19 pm */

#ifndef SPRITE_H
#define SPRITE_H

typedef struct
{
    u32 image_id;
    i32 offset_x,offset_y;
    i32 width, height;
    u32 frame_count;
    u32 fps;
}Sprite;

typedef enum
{
    TITLE_BG_SPR,
    HOG_IDLE_SPR,
    HOG_RUN_SPR,
    SODA_SPR,
    SODA_SPLOD_SPR,
    TOTAL_SPRITES,
}SpriteId;

const Sprite sprites[TOTAL_SPRITES] = 
{
    [TITLE_BG_SPR] = 
    {
        .image_id = TITLE_BG,
        .offset_x = 0,
        .offset_y = 0,
        .width = 800,
        .height = 800,
        .frame_count = 0,
        .fps = 0,
    },
    
    [HOG_IDLE_SPR] = 
    {
        .image_id = HOG_ANI,
        .offset_x = 0,
        .offset_y = 0,
        .width = 400,
        .height = 400,
        .frame_count = 20,
        .fps = 24,
    },
    
    [HOG_RUN_SPR] = 
    {
        .image_id = HOG_ANI,
        .offset_x = 0,
        .offset_y = 800,
        .width = 400,
        .height = 400,
        .frame_count = 20,
        .fps = 20,
    },
    
    [SODA_SPR] = 
    {
        .image_id = HOG_ANI,
        .offset_x = 0,
        .offset_y = 1700,
        .width = 128,
        .height = 128,
        .frame_count = 20,
        .fps = 20,
    },
    
    [SODA_SPLOD_SPR] = 
    {
        .image_id = HOG_ANI,
        .offset_x = 0,
        .offset_y = 1850,
        .width = 128,
        .height = 128,
        .frame_count = 20,
        .fps = 20,
    },
    
};

inline const Sprite*
get_sprite(SpriteId);




void update_sprites();

#endif //SPRITE_H
