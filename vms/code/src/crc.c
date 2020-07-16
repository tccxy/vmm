/**
 * @file crc.c
 * @author zhao.wei (hw)
 * @brief crc校验实现的支持函数,(查表法)
 * @version 0.1
 * @date 2019-12-24
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "pub.h"

static u32 crc32_table[256];

/**
 * @brief 计算crc校验值
 * 
 * @param data 数据
 * @param len 长度
 * @param crc 上一段crc值
 * @return u32 
 */
u32 count_crc32(const u8 *pdata, u32 len, u32 crc)
{
    u8 index;
    u32 i = 0;

    for (i = 0; i < len; i++, pdata++)
    {
        index = *pdata ^ (crc >> 24);
        crc = (crc << 8) ^ crc32_table[index];
    }
    return crc;
}

/**
 * @brief 初始化表
 * 
 */
void init_crc32_table(u32 poly)
{
    u32 i = 0;
    u32 bits = 0;
    u32 crc = 0;

    for (i = 0; i < 256; i++)
    {
        crc = i << 24;

        for (bits = 0; bits < 8; bits++)
        {
            if (crc & 0x80000000)
            {
                crc = (crc << 1) ^ poly;
            }
            else
            {
                crc <<= 1;
            }
        }

        crc32_table[i] = crc;
    }

}