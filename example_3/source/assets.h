/* date = February 16th 2024 9:38 pm */

#ifndef ASSETS_H
#define ASSETS_H

#define START_GLYPH ' '
#define END_GLYPH '~'

#define TOTAL_GLYPHS (END_GLYPH - START_GLYPH + 1)



#define FONT_ATLAS_WIDTH 1024
#define FONT_ATLAS_HIEGHT 1024
#define FONT_SIZE 100

#define BITMAP_BYTES_PER_PIXEL 4


typedef struct
{
    i32 minx, maxx, miny, maxy, advance;
    u32 x, y;
    u32 width, height;
}GlyphInfo;

typedef struct
{
    i32  ascent, descent, line_gap, line_advance;
    r32 scale;
    char* filename;
    stbtt_bakedchar* char_data;
    r32* kern_data;
    i32 width;
    i32 height;
    b32 loaded;
    u32 gpu_info;
}FontAsset;


typedef struct
{
    char* filename;
    b32 loaded;
    u32 gpu_info;
    i32 width;
    i32 height;
}ImageAsset;


typedef enum
{
    IMG_ASSET,
    FONT_ASSET,
    SND_ASSET,
    TOTAL_ASSETS
}AssetType;


typedef enum
{
    SPRITE_IMAGE,
    FONT_IMAGE,
    TOTAL_IMAGE_TYPES
}ImageType;

#define FONT_ASSETS \
X(FONT_0, "assets/Kingthings_Calligraphica_2.ttf") \
//X(FONT_0, "assets/AppleGaramond.ttf") \
//X(FONT_0, "assets/Kingthings_Calligraphica_2.ttf") \
//X(FONT_0, "assets/OpenSans-Regular.ttf") \
//X(FONT_0, "assets/Kingthings_Calligraphica_2.ttf") \
//X(FONT_0, "assets/Kingthings_Calligraphica_2.ttf") \


#define IMAGE_ASSETS \
X(TITLE_BG, "assets/title_bg_crop.png") \
X(LEVEL_1_BG, "assets/level_1_bg_crop.png")\
X(HOG_ANI, "assets/hog_sheet.png")\


//X(SPRITE_IMG, "assets/sprites.png") 




#define X(name, path) name,
typedef enum
{
    IMAGE_ASSETS
        TOTAL_IMAGE_ASSETS,
}ImageId;

typedef enum
{
    FONT_ASSETS
        TOTAL_FONT_ASSETS,
}FontId;

#undef X


#define X(name, path) [name] = {.filename = path,},
ImageAsset image_assets [TOTAL_IMAGE_ASSETS] = 
{
    IMAGE_ASSETS
};

FontAsset font_assets [TOTAL_FONT_ASSETS] = 
{
    FONT_ASSETS
};
#undef X

ImageAsset* get_image_asset(ImageId id)
{
    return &image_assets[id];
}


FontAsset* get_font_asset(FontId id)
{
    return &font_assets[id];
}

void
set_image_dim(ImageType type, u32 asset_id, i32* width, i32* height)
{
    switch(type)
    {
        case SPRITE_IMAGE:
        {
            *width = image_assets[asset_id].width;
            *height = image_assets[asset_id].height;
        }break;
        case FONT_IMAGE:
        {
            *width = font_assets[asset_id].width;
            *height = font_assets[asset_id].height;
        }break;
    }
}


#endif //ASSETS_H
