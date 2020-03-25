/**
 * @file mmc.h
 * @author zhao.wei (hw)
 * @brief 对外提供mmc设备操作实现的支撑函数
 * @version 0.1
 * @date 2019-12-18
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#define MMC_DEBUG_EN

#ifdef MMC_DEBUG_EN
#define MMC_DEBUG(format, ...) printf("File: "__FILE__              \
                                      ", Line: %05d: " format "/n", \
                                      __LINE__, ##__VA_ARGS__)

#else
#define MMC_DEBUG(format, ...)
#endif

u32 mmc_open(u8 *partition_name);
u32 mmc_close(s32 fd);
u32 mmc_read(s32 fd, u32 offset, u32 len, u8 *buf);
u32 mmc_write(s32 fd, u32 offset, u32 len, u8 *buf);