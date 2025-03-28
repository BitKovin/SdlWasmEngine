/* date = March 4th 2022 10:55 pm */

#ifndef PRODEFS_H
#define PRODEFS_H



#include <stdint.h>
#include <stddef.h>



#if DEBUG
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif


#define array_count(array) (sizeof(array) / sizeof((array)[0]))
#define STRINGIFY(x) #x

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;


typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef int32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;


typedef size_t memory_index;

typedef float real32;
typedef double real64;

typedef float r32;
typedef double r64;

typedef float f32;
typedef double f64;

typedef uintptr_t umm;
typedef intptr_t smm;


typedef unsigned char u8;

#define MAX_U32 UINT32_MAX
#define MIN_I16 INT16_MIN
#define MAX_I16 INT16_MAX

#define true 1
#define false 0

//#define M_PI 3.14159265358979323846264338327950288
#define M_2PI 2*M_PI


#define internal static
#define local_persist static
#define global_variable static


#define B_00    0x0
#define B_01    0x1
#define B_10    0x2
#define B_11    0x3


#define B_0000    0x0
#define B_0001    0x1
#define B_0010    0x2
#define B_0011    0x3
#define B_0100    0x4
#define B_0101    0x5
#define B_0110    0x6
#define B_0111    0x7
#define B_1000    0x8
#define B_1001    0x9
#define B_1010    0xa
#define B_1011    0xb
#define B_1100    0xc
#define B_1101    0xd
#define B_1110    0xe
#define B_1111    0xf

#endif //PRODEFS_H
