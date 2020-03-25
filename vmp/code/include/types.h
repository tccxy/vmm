/**
 * @file types.h
 * @author zhao.wei (hw)
 * @brief 对所有文件提供数据类型的重定向
 * @version 0.1
 * @date 2019-12-18
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef __TYPES_H__
#define __TYPES_H__


//机器位数
#define _CPU_WORD_SIZE 64

#if (_CPU_WORD_SIZE == 16)
typedef unsigned short int  u6;
typedef signed   short int  s16;

typedef unsigned long       u32;
typedef signed   long       s32;

#endif


#if (_CPU_WORD_SIZE == 32)
typedef unsigned short int  u16;
typedef signed short int    s16;

typedef unsigned int        u32;
typedef signed   int        s32;

typedef unsigned long long  u64;
typedef signed long long    s64;

#endif


#if (_CPU_WORD_SIZE == 64)
typedef unsigned short int  u16;
typedef signed short int    s16;

typedef unsigned int        u32;
typedef signed   int        s32;

typedef unsigned long       u64;
typedef signed   long       s64;

#endif
typedef unsigned char       u8;
typedef signed char         s8;

#endif
