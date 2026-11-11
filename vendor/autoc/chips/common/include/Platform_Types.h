#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#define CPU_TYPE_8  8u
#define CPU_TYPE_16 16u
#define CPU_TYPE_32 32u
#define CPU_TYPE_64 64u

#define MSB_FIRST 0u
#define LSB_FIRST 1u

#define HIGH_BYTE_FIRST 0u
#define LOW_BYTE_FIRST  1u

#define CPU_TYPE       (CPU_TYPE_32)
#define CPU_BIT_ORDER  (LSB_FIRST)
#define CPU_BYTE_ORDER (LOW_BYTE_FIRST)

#ifndef TRUE
#define TRUE (1u)
#endif
#ifndef FALSE
#define FALSE (0u)
#endif

typedef unsigned char boolean;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned long long uint64;
typedef signed char sint8;
typedef signed short sint16;
typedef signed long sint32;
typedef signed long long sint64;
typedef unsigned long uint8_least;
typedef unsigned long uint16_least;
typedef unsigned long uint32_least;
typedef signed long sint8_least;
typedef signed long sint16_least;
typedef signed long sint32_least;
typedef float float32;
typedef double float64;

#endif
