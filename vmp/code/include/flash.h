/**
 * @file flash.h
 * @author zhao.wei (hw)
 * @brief 对外提供操作flash实现的支撑函数
 * @version 0.1
 * @date 2019-12-17
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef __FLASH_H__
#define __FLASH_H__

#define FLASH_DEBUG_EN

#ifdef FLASH_DEBUG_EN
#define FLASH_DEBUG(format, ...) printf("File: "__FILE__              \
                                        ", Line: %05d: " format "/n", \
                                        __LINE__, ##__VA_ARGS__)
#else
#define FLASH_DEBUG(format, ...)
#endif

/**
 * @brief mtd 设备
 * 
 */
struct mtd_partition
{
    u8 *name;       /**identifier string */
    u32 size;       /**partition size */
    u32 offset;     /**offset within the master MTD space */
    u32 mask_flags; /**master MTD flags to mask out for this partition */
};

/**
 * @brief flash 设备
 * 
 */
struct drv_flash_mtd
{
    u8 mtd_dev_name[16]; /**MTD设备名*/
    u32 mtd_use_flag;    /**MTD设备使用标志*/
    u32 mtd_offset;      /**MTD设备地址偏移*/
    u32 mtd_size;        /**MTD大小*/
    u32 erase_size;      /**分页大小*/
    u8 mtd_name[64];     /**MTD名称*/
};

#define NOR_FLASH 0  //NOR FALSH
#define NAND_FLASH 1 //NAND FALSH

#define SYSTEM_MTD_MAX 24

#define _DRV_NOR_SECTOR_SIZE 0x20000
#define _DRV_NAND_SECTOR_SIZE 0x20000

u32 flash_init(void);
u32 flash_read(u32 flash_id, u32 offset, u32 len, u8 *buf, u32 *read_len);
u32 flash_write(u32 flash_id, u32 offset, u32 len, u8 *buf, u32 *write_len);
u32 flash_erase(u32 flash_id, u32 offset, u32 len);

#endif
