/* date = August 3rd 2023 10:01 pm */

#ifndef M_MATH_H
#define M_MATH_H



//todo: does this just get me the macros?
//#define _USE_MATH_DEFINES
#include <math.h>


typedef union V2
{
    struct
    {
        real32 x, y;
    };
    
    real32 e[2];
}V2;


typedef union V3
{
    struct
    {
        real32 x, y, z;
    };
    
    real32 e[3];
}V3;

inline V2
v2_add (V2 a, V2 b)
{
    V2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    
    return result;
}


inline V2
v2_sub (V2 a, V2 b)
{
    V2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    
    return result;
}


inline V2
v2_mul (real32 a, V2 b)
{
    V2 result;
    result.x = a * b.x;
    result.y = a * b.y;
    
    return result;
}

inline f32
v2_mag(V2 a)
{
    return sqrt((a.x * a.x) + (a.y * a.y));
}

inline V2
v2_normalize(V2 a)
{
    f32 mag = v2_mag(a);
    f32 scale = 1.0f / mag;
    
    return v2_mul(scale, a);
}


inline V2
v2_perp(V2 a)
{
    V2 result = {a.y, -a.x};
    return result;
}

inline f32
v2_dot(V2 a, V2 b)
{
    return (a.x * b.x) + (a.y * b.y);
}



inline f32
v2_theta(V2 a, V2 b)
{
    f32 dot = v2_dot(a, b);
    f32 mag = v2_mag(a) * v2_mag(b);
    return acos(dot/mag);
    
}


inline f32
v2_theta_cc(V2 a)
{
    f32 result = 0.0f;
    if(a.y < 0)
    {
        result = v2_theta(a, (V2){-1.0f, 0.0f});
        result += M_PI;
    }
    else
    {
        result = v2_theta(a, (V2){1.0f, 0.0f});
    }
    return result;
}

inline V2
theta_to_vec(f32 theta)
{
    V2 result = {cos(theta), sin(theta)};
    return result;
}


inline int32
abs_i32(int32 a) 
{
    if( a < 0)
    {
        return (a * -1);
    }
    else
    {
        return a;
    }
}


inline real32
abs_r32(real32 a) 
{
    
    if( a < 0)
    {
        return (a * -1);
    }
    else
    {
        return a;
    }
}



inline bool32
opposite_signs_r32(real32 a, real32 b)
{
    return (a < 0)? (b >= 0): (b < 0);
}


i32 clamp_i32(i32 min, i32 val, i32 max)
{
    if(val < min) return min;
    if(val > max) return max;
    return val;
}

i32 cap_i32(i32 val, i32 max)
{
    if(val > max)
        return max;
    return val;
}

f32 clamp_f32(f32 min, f32 val, f32 max)
{
    if(val < min) return min;
    if(val > max) return max;
    return val;
}

f32 cap_f32(f32 val, f32 max)
{
    if(val > max)
        return max;
    return val;
}



inline int32 min_i32(int32 a, int32 b) {return a < b ? a : b;}
inline int32 min_u32(uint32 a, uint32 b) {return a < b ? a : b;}
inline real32 min_r32(real32 a, real32 b) {return a < b ? a : b;}

inline int32 max_i32(int32 a, int32 b) {return a > b ? a : b;}
inline int32 max_u32(uint32 a, uint32 b) {return a > b ? a : b;}
inline real32 max_r32(real32 a, real32 b) {return a > b ? a : b;}

inline f32 square_f32(f32 num){return num * num;};

inline r32 squared_dist(V2 pos1, V2 pos2){return (square_f32(pos2.x - pos1.x) + square_f32(pos2.y - pos1.y)) ;};
inline r32 dist(V2 pos1, V2 pos2){return sqrt(squared_dist(pos1, pos2));};

inline f32 lerp(f32 a, f32 b, f32 f){return (a * (1.0f - f)) + (b*f);};
inline f32 capped_lerp(f32 a, f32 b, f32 f)
{
    if(f >= 1.0)
        return b;
    return (a * (1.0f - f)) + (b*f);
};

inline f32 constant_change(f32 a, f32 b, f32 time, f32 time_for_change)
{
    if(time > time_for_change)
        return b;
    else
        return lerp(a,b,time/time_for_change);
}


inline f32 constant_change_measured(f32 *val, f32 a, f32 b, f32 time, f32 time_for_change)
{
    if(time > time_for_change)
    {
        *val = b;
        return time - time_for_change;
    }
    else
    {
        *val = lerp(a,b,time/time_for_change);
        return 0;
    }
}



inline V2 v2_between_2_pts(V2 pt1, V2 pt2){return (V2){pt2.x - pt1.x, pt2.y - pt2.y};};
inline V2 unit_v2_between_2_pts(V2 pt1, V2 pt2)
{
    V2 vec = v2_between_2_pts(pt1, pt2);
    f32 mag = v2_mag(vec);
    return (V2){vec.x / mag, vec.y / mag};
}


//matrix

typedef union Mat4 
{
    struct
    {
        r32 a, b, c, d, e, f, g, h, i, j, k ,l, m, n, o, p;
    };
    
    r32 elements[16];
    
}Mat4;


typedef union Mat3
{
    struct
    {
        r32 a, b, c, d, e, f, g, h, i;
    };
    
    r32 elements[9];
    
}Mat3;


typedef union Mat2
{
    struct
    {
        r32 a, b, c, d;
    };
    
    r32 elements[4];
    
}Mat2;


const Mat4 mat4_identity = 
{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};


const Mat3 mat3_identity = 
{
    1, 0, 0,
    0, 1, 0,
    0, 0, 1,
};


Mat4 mat4_mat4_mul(Mat4* m0, Mat4* m1)
{
    Mat4 result = {0};
    i32 index = 0;
    for(i32 col = 0; col < 4; col++)
    {
        for(i32 row = 0; row < 4; row++)
        {
            r32 sum = 0.0f;
            for(i32 i = 0; i < 4; i++) 
            {
                sum += m0->elements[i + col * 4] * m1->elements[row + i * 4];
            }
            result.elements[index] = sum;
            index++;
        }
    }
    
    return result;
}



Mat4 mat4_ortho(r32 x_min, r32 x_max, r32 y_min, r32 y_max, r32 z_far, r32 z_near)
{
    Mat4 result = 
    {
        .a = 2.0f / (x_max - x_min),
        .f = 2.0f / (y_max - y_min),
        .k = - 2.0f / (z_far - z_near),
        .m = - (x_max + x_min) / (x_max - x_min),
        .n = - (y_max + y_min) / (y_max - y_min),
        .o = - (z_far + z_near) / (z_far - z_near),
        .p = 1.0f
    };
    return result;
}

Mat4 mat4_scale(Mat4* m, V3* v)
{
    Mat4 s = mat4_identity;
    s.elements[0] = m->elements[0];
    s.elements[5] = m->elements[1];
    s.elements[10] = m->elements[2];
    return mat4_mat4_mul(&s, m);
}

Mat4 mat4_translate(Mat4* m, V3* v)
{
    Mat4 t = mat4_identity;
    t.elements[12] = v->x;
    t.elements[13] = v->y;
    t.elements[14] = v->z;
    return mat4_mat4_mul(&t, m);
}





#endif //M_MATH_H
