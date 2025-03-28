/* date = February 16th 2024 10:21 pm */

#ifndef RENDER_H
#define RENDER_H


#define MAX_RENDER_COMMANDS 3000
#define NUM_INDICES 6
#define QUAD_POINTS 4

#define NUM_DRAW_FLOATS 5
#define SINGLE_DRAW_SIZE NUM_DRAW_FLOATS * QUAD_POINTS * sizeof(f32)



const char* vs_standard =
//"#version 330 core\n"

//"layout (location = 0) in vec2 verts;\n"
//"layout (location = 1) in vec3 tex;\n"
"precision highp float;\n"
"attribute vec2 verts;\n"
"attribute vec3 tex;\n"

"uniform mat4 view;\n"
"uniform mat4 model;\n"
"uniform mat4 proj;\n"
"uniform vec2 image_offset;\n"
"uniform vec2 image_scale;\n"
//"out vec3 pass_tex_coord;\n"
"varying vec3 pass_tex_coord;\n"
"void main()\n"
"{\n"
"pass_tex_coord = tex;\n"
"gl_Position =  proj * model * vec4(verts.xy, 0.0, 1.0);\n"
"}";


const char* fs_standard =
//"#version 330 core\n"
//"in vec3 pass_tex_coord;\n"
"precision mediump float;\n"
"varying vec3 pass_tex_coord;\n"
//"out vec4 color;\n"
"uniform sampler2D img;\n"
"uniform vec3 sprite_color;\n"
"void main()\n"
"{\n"
"gl_FragColor = texture2D(img, pass_tex_coord.xy);\n"
"gl_FragColor.a += -pass_tex_coord.z;"
"}";



typedef enum
{
    STANDARD_SHADER,
    
    TOTAL_SHADERS,
}ShaderId;


typedef struct
{
    u32 tex_sheet;
    ImageType img_type;
    u32 asset_id;
    u32 shader_id;
    r32 x_min, y_min, x_max, y_max;
    r32 t_x0, t_y0, t_x1, t_y1;
    r32 depth;
    r32 rotation;
    u32 color;
    f32 alpha;
    
}RenderCommand;


typedef enum
{
    FLOOR_TILE,
    LOWER_TILE,
    MID_TILE,
    ABOVE_TILE,
    GUI_LOW,
    GUI_MED,
    GUI_TOP,
}DepthLayer;


typedef struct
{
    u32 standard_model;
    u32 standard_proj;
    
}ShaderUniforms;

typedef struct
{
    SpriteId id;
    u32 frame;
}SpriteInfo;

typedef struct
{
    union
    {
        u32 color;
        u32 char_index;
        SpriteInfo spr_info;
    };
}SpecialRenderInfo;


typedef struct
{
    
    RenderCommand draws[MAX_RENDER_COMMANDS];
    f32 gpu_buffer [NUM_DRAW_FLOATS * QUAD_POINTS * MAX_RENDER_COMMANDS];
    u32 shaders [TOTAL_SHADERS];
    u32 render_count;
    ShaderUniforms shader_uniforms;
    u32 VAO, VBO, EBO;
    
}RenderData;
global_variable RenderData rd;


#endif //RENDER_H
