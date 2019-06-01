#ifndef Z_COMMON_H
#define Z_COMMON_H

#define local static

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float	real32;

typedef u8 bool;
enum {false, true};

void *_malloc(size_t size);
void _free(void *ptr);

#define DT 1.0/60.0

#endif
