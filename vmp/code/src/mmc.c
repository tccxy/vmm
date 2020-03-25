/**
 * @file mmc.c
 * @author zhao.wei (hw)
 * @brief mmc设备操作实现的支撑函数
 * @version 0.1
 * @date 2019-12-18
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "pub.h"

/**
 * @brief 打开mmc设备获取文件描述符
 * 
 * @param partition_name 分区描述 eg:/dev/mmcblk0p3
 * @return u32 描述符
 */
u32 mmc_open(u8 *partition_name)
{
    s32 fd = -1;

    if (NULL == partition_name)
    {
        return ERR_MMC_NULL_POINTER;
    }
    fd = open(partition_name, O_RDWR);
    if (fd < 0)
    {
        MMC_DEBUG("mmc open %s fail", partition_name);
        return ERR_MMC_OPEN_FILE;
    }

    return fd;
}

/**
 * @brief 关闭mmc设备
 * 
 * @param fd 描述符
 * @return u32 
 */
u32 mmc_close(s32 fd)
{
    if (fd < 0)
    {
        MMC_DEBUG("mmc open %x fail", fd);
        return ERR_MMC_OPEN_FILE;
    }
    close(fd);
    return SUCCESS;
}

/**
 * @brief 从mmc设备读取数据
 * 
 * @param fd 描述符
 * @param offset 偏移地址
 * @param len 长度
 * @param buf 缓存数据区
 * @return u32 实际读取的长度
 */
u32 mmc_read(s32 fd, u32 offset, u32 len, u8 *buf)
{

    s32 result = -1;
    s32 true_len = 0;


    /*移动文件指针,指向要操作的起始地址*/
    result = lseek(fd, offset, SEEK_SET);
    if (result < 0)
    {
        MMC_DEBUG("mmc lseek(%x) fail", offset);
        mmc_close(fd);
        return ERR_MMC_OFFSET;
    }

    /*读数据*/
    true_len = read(fd, buf, len);
    if (true_len < 0)
    {
        FLASH_DEBUG("mmc read length error!\n");
        mmc_close(fd);
        return ERR_MMC_READ;
    }

    return true_len;
}

/**
 * @brief mmc设备写入函数
 * 
 * @param fd 描述符
 * @param offset 偏移地址
 * @param len 长度
 * @param buf 写入数据的缓存区
 * @return u32 实际写入的长度
 */
u32 mmc_write(s32 fd, u32 offset, u32 len, u8 *buf)
{
    s32 result = -1;
    s32 true_len = 0;

    /*移动文件指针,指向要操作的起始地址*/
    result = lseek(fd, offset, SEEK_SET);
    if (result < 0)
    {
        MMC_DEBUG("mmc lseek(%x) fail", offset);
        mmc_close(fd);
        return ERR_MMC_OFFSET;
    }

    /*读数据*/
    true_len = write(fd, buf, len);
    if (true_len < 0)
    {
        FLASH_DEBUG("mmc write length error!\n");
        mmc_close(fd);
        return ERR_MMC_WRITE;
    }

    return true_len;
}