/**
 * @file flash.c
 * @author zhao.wei (hw)
 * @brief 操作flash实现的支撑函数
 * @version 0.1
 * @date 2019-12-17
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include "pub.h"

/**存放flash设备信息 */
struct drv_flash_mtd sg_mtd_info[SYSTEM_MTD_MAX];

static u32 sg_nor_total_size = 0;      /**nor flash容量*/
static u32 sg_nand_total_size = 0;     /**nand flash容量*/
static u32 sg_nor_mtd_base_index = 0;  /**nor flash 类型MTD设备起始标号*/
static u32 sg_nand_mtd_base_index = 0; /**nand flash 类型MTD设备起始标号*/
static u32 sg_nor_mtd_end_index = 0;   /**nor flash 类型MTD设备起始标号*/
static u32 sg_nand_mtd_end_index = 0;  /**nand flash 类型MTD设备起始标号*/

/**
 * @brief 根据flashId 和偏移量查找操作的mtd设备,以及该设备的偏移量
 * 
 * @param flash_id 
 * @param offset 
 * @param mtd_index 
 * @param mtd_offset 
 * @return u32 
 */
static u32 get_mtd_info(u32 flash_id, u32 offset, u8 *mtd_index, u32 *mtd_offset)
{
    u32 loop = 0;
    if (NOR_FLASH == flash_id)
    {
        for (loop = sg_nor_mtd_base_index; loop <= sg_nor_mtd_end_index; loop++)
        {
            if (offset < (sg_mtd_info[loop].mtd_offset + sg_mtd_info[loop].mtd_size))
            {
                *mtd_index = (u8)loop;
                *mtd_offset = offset - sg_mtd_info[loop].mtd_offset;
                return SUCCESS;
            }
        }
    }
    else if (NAND_FLASH == flash_id)
    {
        for (loop = sg_nand_mtd_base_index; loop <= sg_nand_mtd_end_index; loop++)
        {
            if (offset < (sg_mtd_info[loop].mtd_offset + sg_mtd_info[loop].mtd_size))
            {
                *mtd_index = (u8)loop;
                *mtd_offset = offset - sg_mtd_info[loop].mtd_offset;
                return SUCCESS;
            }
        }
    }
    else
    {
        return ERR_FLASH_TYPE;
    }
    return ERR_FLASH_OFFSET;
}

/**
 * @brief flash初始化
 * 
 * @return u32 
 */
u32 flash_init(void)
{
    FILE *file = NULL;   //文件句柄
    u8 buf[128] = {0}; //临时缓存
    u8 loop = 0;
    u32 ret = 0;
    u32 nand_flag = 0; //nand flash 起始标号是否找到标志
    u32 nor_flag = 0;

    FLASH_DEBUG("flash init start!\n");

    for (loop = 0; loop < SYSTEM_MTD_MAX; loop++)
    {
        sprintf(sg_mtd_info[loop].mtd_dev_name, "/dev/mtd%u", loop);
    }

    /*只读方式打开文件*/
    file = fopen("/proc/mtd", "r");
    if (NULL == file)
    {
        return ERR_FLASH_OPEN_FILE;
    }

    /*跳过/proc/mtd的第一行*/
    if (NULL == fgets(buf, sizeof(buf), file))
    {
        return ERR_FLASH_READ_FILE;
    }

    sg_nor_mtd_base_index = 0;
    sg_nand_mtd_base_index = 0;

    for (loop = sg_nor_mtd_base_index; loop < (sizeof(sg_mtd_info) / sizeof(struct drv_flash_mtd)); loop++)
    {
        ret = fscanf(file, "%s %x %x %s", buf, (&(sg_mtd_info[loop].mtd_size)),
                     (&(sg_mtd_info[loop].erase_size)), sg_mtd_info[loop].mtd_dev_name);
        if (ret != 4) /*到文件末尾返回*/
        {
            break;
        }

        if (NULL != strstr(sg_mtd_info[loop].mtd_dev_name, "nand")) /*计算flash总容量*/
        {
            sg_nand_mtd_end_index = loop;
            sg_nand_total_size += sg_mtd_info[loop].mtd_size;

            sg_mtd_info[loop].mtd_use_flag = TRUE;

            if (0 == nand_flag)
            {
                sg_nand_mtd_base_index = loop;
                nand_flag = 1;
            }
            else
            {
                sg_mtd_info[loop].mtd_offset = sg_mtd_info[loop - 1].mtd_size + sg_mtd_info[loop - 1].mtd_offset;
            }
        }
        else if (1) //(NULL != strstr(sg_mtd_info[loop].mtd_dev_name, "nor"))//后续更改
        {
            sg_nor_mtd_end_index = loop;
            sg_nor_total_size += sg_mtd_info[loop].mtd_size;

            sg_mtd_info[loop].mtd_use_flag = TRUE;

            if (0 == nor_flag)
            {
                sg_nor_mtd_base_index = loop;
                nor_flag = 1;
            }
            else
            {
                sg_mtd_info[loop].mtd_offset = sg_mtd_info[loop - 1].mtd_size + sg_mtd_info[loop - 1].mtd_offset;
            }
        }
        else
        {
            if (0 == nand_flag)
            {
                nand_flag = 1;
            }
            else
            {
                sg_mtd_info[loop].mtd_offset = sg_mtd_info[loop - 1].mtd_size + sg_mtd_info[loop - 1].mtd_offset;
            }

            sg_mtd_info[loop].mtd_use_flag = FALSE;
        }

        FLASH_DEBUG("mtddevname=%s,size=%x,offset=%x erase=%x,loop=%d,ret=%x\n", sg_mtd_info[loop].mtd_dev_name, sg_mtd_info[loop].mtd_size, sg_mtd_info[loop].mtd_offset, sg_mtd_info[loop].erase_size, loop, ret);
    }

    FLASH_DEBUG("Nor flash size  = %x,Nand flash size = %x\n", sg_nor_total_size, sg_nand_total_size);
    FLASH_DEBUG("Nand Base Index = %x,Nand End  Index = %x\n", sg_nand_mtd_base_index, sg_nand_mtd_end_index);
    FLASH_DEBUG("Nor  Base Index = %x,Nor  End  Index = %x\n", sg_nor_mtd_base_index, sg_nor_mtd_end_index);

    fclose(file); /*关闭文件*/
    return SUCCESS;
}

/**
 * @brief flash读函数
 * 
 * @param flash_id 
 * @param offset 
 * @param len 
 * @param buf 
 * @param read_len 
 * @return u32 
 */
u32 flash_read(u32 flash_id, u32 offset, u32 len, u8 *buf, u32 *read_len)
{
    u8 mtd_index = 0;
    u32 ret = 0;
    u32 read_offset = 0;
    s32 fd = -1;
    s32 result = -1;
    s32 true_len = -1;

    /*入参检查*/
    if ((NOR_FLASH != flash_id) && (NAND_FLASH != flash_id))
    {
        FLASH_DEBUG("flash_read Flash Type Error!\n");
        return ERR_FLASH_TYPE;
    }

    if ((NULL == buf) || (NULL == read_len))
    {
        return ERR_FLASH_NULL_POINTER;
    }

    /*获取MTD设备文件信息*/
    ret = get_mtd_info(flash_id, offset, &mtd_index, &read_offset);
    if (SUCCESS != ret)
    {
        FLASH_DEBUG("flash_read get_mtd_info Fail!\n");
        return ret;
    }

    FLASH_DEBUG("flash_read(%d, 0x%x, %d) MtdIndex:%d MtdOffset:%x Use:%d MtdSize:%x\n", flash_id, offset, len, mtd_index, read_offset, sg_mtd_info[mtd_index].mtd_use_flag, sg_mtd_info[mtd_index].mtd_size);

    if (FALSE == sg_mtd_info[mtd_index].mtd_use_flag)
    {
        FLASH_DEBUG("flash_read mtd_use_flag is False!\n");
        return ERR_FLASH_UNUSED;
    }

    if (read_offset + len > sg_mtd_info[mtd_index].mtd_size)
    {
        FLASH_DEBUG("flash_read Offser error!\n");
        return ERR_FLASH_OFFSET;
    }

    /*打开文件*/
    fd = open(sg_mtd_info[mtd_index].mtd_dev_name, O_RDWR);
    if (fd < 0)
    {
        FLASH_DEBUG("flash_read Open(mtd%d:%s) Fail!\n", mtd_index, sg_mtd_info[mtd_index].mtd_dev_name);
        return ERR_FLASH_MTDOPEN;
    }

    /*移动文件指针,指向要操作的起始地址*/
    result = lseek(fd, read_offset, SEEK_SET);
    if (result < 0)
    {
        FLASH_DEBUG("flash_read lseek(%x) fail\n", read_offset);
        close(fd);
        return ERR_FLASH_OFFSET;
    }

    /*读数据*/
    true_len = read(fd, buf, len);
    if (true_len < 0)
    {
        FLASH_DEBUG("flash_read read length error!\n");
        close(fd);
        return ERR_FLASH_READ;
    }

    close(fd);

    (*read_len) = (u32)true_len;

    return SUCCESS;
}

/**
 * @brief 获取flash擦长度
 * 
 * @param flash_id 
 * @param offset 
 * @return u32 
 */
u32 flash_erase_size(u32 flash_id, u32 offset)
{
    u8 mtd_index = 0;
    u32 ret = 0;
    u32 erase_offset = 0;

    /*入参检查*/
    if ((NOR_FLASH != flash_id) && (NAND_FLASH != flash_id))
    {
        return ERR_FLASH_ERASE;
    }

    /*获取MTD设备文件信息*/
    ret = get_mtd_info(flash_id, offset, &mtd_index, &erase_offset);
    if (SUCCESS != ret)
    {
        return ERR_FLASH_ERASE;
    }
    return sg_mtd_info[mtd_index].erase_size;
}

/**
 * @brief flash擦除函数
 * 
 * @param flash_id 
 * @param offset 
 * @param len 
 * @return u32 
 */
u32 flash_erase(u32 flash_id, u32 offset, u32 len)
{
    u8 mtd_index = 0;
    u32 ret = 0;
    u32 erase_offset = 0;
    s32 fd = -1;
    s32 result = -1;

    erase_info_t *erase_info;
    u32 sector_size = 0;
    /*入参检查*/
    if ((NOR_FLASH != flash_id) && (NAND_FLASH != flash_id))
    {
        FLASH_DEBUG("flash_erase Flash Type Error!\n");
        return ERR_FLASH_TYPE;
    }

    /*获取MTD设备文件信息*/
    ret = get_mtd_info(flash_id, offset, &mtd_index, &erase_offset);
    if (SUCCESS != ret)
    {
        FLASH_DEBUG("flash_erase get_mtd_info Fail(%x)!\n", ret);
        return ret;
    }

    FLASH_DEBUG("flash_erase mtd%d offset:%08x size:%x!\n", mtd_index, erase_offset, sg_mtd_info[mtd_index].mtd_size);

    if (erase_offset + len > sg_mtd_info[mtd_index].mtd_size)
    {
        FLASH_DEBUG("flash_erase mtd%d offset:%08x len:%x Error!\n", mtd_index, erase_offset, len);
        return ERR_FLASH_OFFSET;
    }

    if (FALSE == sg_mtd_info[mtd_index].mtd_use_flag)
    {
        FLASH_DEBUG("flash_erase MtdUseFlag Error!\n");
        return ERR_FLASH_UNUSED;
    }

    /*打开文件*/
    fd = open(sg_mtd_info[mtd_index].mtd_dev_name, O_RDWR);
    if (fd < 0)
    {
        FLASH_DEBUG("flash_erase open(mtd%d:%s) Fail(%x)!\n", mtd_index, sg_mtd_info[mtd_index].mtd_dev_name, fd);
        FLASH_DEBUG("flash_erase error: %s(errno: %d)", strerror(errno), errno);
        return ERR_FLASH_MTDOPEN;
    }

    /*申请内存*/
    erase_info = (erase_info_t *)malloc(sizeof(erase_info_t));
    if (NULL == erase_info)
    {
        close(fd);

        FLASH_DEBUG("flash_erase malloc Fail!\n");

        return ERR_FLASH_ERASEMALLOC;
    }

    switch (flash_id)
    {
    case 0:
    {
        sector_size = _DRV_NOR_SECTOR_SIZE;
        break;
    }
    case 1:
    {
        sector_size = _DRV_NAND_SECTOR_SIZE;
        break;
    }
    default:
    {
        sector_size = _DRV_NOR_SECTOR_SIZE;
    }
    }

    erase_info->start = (u32)((erase_offset / sector_size) * sector_size);
    if ((((erase_offset + len) / sector_size) * sector_size) < (erase_offset + len))
    {
        erase_info->length = (u32)(((((erase_offset + len) / sector_size) - (erase_offset / sector_size)) + 1) * sector_size);
    }
    else
    {
        erase_info->length = (u32)((((erase_offset + len) / sector_size) - (erase_offset / sector_size)) * sector_size);
    }

    /*擦除FLASH*/
    result = ioctl(fd, MEMERASE, erase_info); //MEMERASE define in kernel, we use direct number, that is 0x80084d02
    if (result < 0)
    {
        close(fd);
        free(erase_info);
        FLASH_DEBUG("flash_erase erase Fail!\n");
        return ERR_FLASH_ERASE;
    }

    /*关闭文件,释放内存*/
    close(fd);
    free(erase_info);
    return SUCCESS;
}

/**
 * @brief flash写函数
 * 
 * @param flash_id 
 * @param offset 
 * @param len 
 * @param buf 
 * @param write_len 
 * @return u32 
 */
u32 flash_write(u32 flash_id, u32 offset, u32 len, u8 *buf, u32 *write_len)
{
    u8 mtd_index = 0;
    u16 block_num = 0;
    u16 times = 0;
    u32 ret = 0;
    u32 mtd_offset = 0;
    u32 tmp_offset = 0;
    u32 block_size = 0;
    u32 write_off = 0;
    u32 true_len = 0;
    s32 fd = -1;
    s32 result = -1;
    s32 strue_len = -1;
    u8 *page = NULL;
    u8 *dst_buf = NULL;

    /*入参检查*/
    if ((NOR_FLASH != flash_id) && (NAND_FLASH != flash_id))
    {
        FLASH_DEBUG("flash_write Flash Type Error!\n");
        return ERR_FLASH_TYPE;
    }

    if ((NULL == buf) || (NULL == write_len))
    {
        return ERR_FLASH_NULL_POINTER;
    }

    /*获取MTD设备文件信息*/
    ret = get_mtd_info(flash_id, offset, &mtd_index, &mtd_offset);
    if (SUCCESS != ret)
    {
        FLASH_DEBUG("flash_write get_mtd_info Fail!\n");
        return ret;
    }

    if (FALSE == sg_mtd_info[mtd_index].mtd_use_flag)
    {
        FLASH_DEBUG("flash_write MtdUseFlag Error!\n");
        return ERR_FLASH_UNUSED;
    }

    if (mtd_offset + len > sg_mtd_info[mtd_index].mtd_size)
    {
        FLASH_DEBUG("flash_write mtd_offset(%08x) Error!\n", mtd_offset);
        return ERR_FLASH_OFFSET;
    }
#if 0
    printf("flash_write(%d, %d, %d) MtdIndex:%d Offset:%x MtdSize:%x MtdOffset:%x EraseSize:%x DevName:%s MtdName:%s\n"
            , flash_id
            , offset
            , len
            , mtd_index
            , mtd_offset
            , sg_mtd_info[mtd_index].mtd_size
            , sg_mtd_info[mtd_index].mtd_offset
            , sg_mtd_info[mtd_index].erase_size
            , sg_mtd_info[mtd_index].mtd_dev_name
            , sg_mtd_info[mtd_index].mtd_dev_name);
#endif
    /*打开文件*/
    fd = open(sg_mtd_info[mtd_index].mtd_dev_name, O_RDWR);
    if (fd < 0)
    {
        FLASH_DEBUG("flash_write open(mtf%d:%s) Fail!\n", mtd_index, sg_mtd_info[mtd_index].mtd_dev_name);
        return ERR_FLASH_MTDOPEN;
    }

    if (NOR_FLASH == flash_id)
    {
        /*移动文件指针,指向要操作的起始地址*/
        result = lseek(fd, mtd_offset, SEEK_SET); //SEEK_SET or SEEK_CUR or SEEK_END
        if (result < 0)
        {
            close(fd);

            FLASH_DEBUG("flash_write lseek(%08x) Fail!\n", mtd_offset);
            return ERR_FLASH_OFFSET;
        }

        strue_len = write(fd, buf, len);
        if (strue_len < 0)
        {
            FLASH_DEBUG("flash_write write(%d) fail %d (%x)\n", len, strue_len, errno);
            close(fd);
            return ERR_FLASH_WRITE;
        }
    }
    else
    {
        block_size = 0x800;
        tmp_offset = (mtd_offset / block_size) * block_size;                          // 整块对齐需要增加的偏移
        block_num = ceil((float)(len + mtd_offset - tmp_offset) / (float)block_size); // 整块对齐的写块数目
        true_len = block_num * block_size;                                           // 整块对齐的写长度
        write_off = mtd_offset - tmp_offset;                                          // 整块对齐偏移

        page = malloc(block_size * block_num);
        if (NULL == page)
        {
            FLASH_DEBUG("flash_write malloc buffer(%d) failed! true_len %d\n", block_size, true_len);
            close(fd);
            return ERR_FLASH_MALLOC;
        }
        // 定位偏移
        result = lseek(fd, tmp_offset, SEEK_SET); //SEEK_SET or SEEK_CUR or SEEK_END
        if (result < 0)
        {
            FLASH_DEBUG("flash_write tmp_offset(%x) fail\n", tmp_offset);
            free(page);
            close(fd);
            return ERR_FLASH_OFFSET;
        }

        //printf("flash_write nand read Offset:%x len:%x\n", tmp_offset, block_size*block_num);
        strue_len = read(fd, page, block_size * block_num);
        if (strue_len < 0)
        {
            FLASH_DEBUG("flash_write nand write(%d/%d) read fail(%d)\n", times, block_num, strue_len);
            close(fd);
            free(page);
            return ERR_FLASH_READ;
        }

        //printf("flash_write memcpy write_off:%x len:%x\n", write_off, len);
        dst_buf = page + write_off;
        memcpy(dst_buf, buf, len);

        // 定位偏移
        result = lseek(fd, tmp_offset, SEEK_SET); //SEEK_SET or SEEK_CUR or SEEK_END
        if (result < 0)
        {
            FLASH_DEBUG("flash_write tmp_offset(%x) fail---\n", tmp_offset);
            free(page);
            close(fd);
            return ERR_FLASH_OFFSET;
        }
        //printf("flash_write nand write Offset:%x len:%x\n", tmp_offset, block_size*block_num);
        strue_len = write(fd, page, block_size * block_num);
        if (block_size * block_num != strue_len)
        {
            FLASH_DEBUG("flash_write write(%d/%d) fail %d (%x)\n", times, block_num, strue_len, errno);
            free(page);
            close(fd);
            return ERR_FLASH_WRITE;
        }
    }

    /*写数据*/
    free(page);
    close(fd);
    (*write_len) = len;
    return SUCCESS;
}

/**
 * @brief 获取FLASH存储空间大小
 * 
 * @param pdwSize 
 * @return u32 
 */
u32 flash_size(u32 *pdwSize)
{
    return sg_nor_total_size;
}

/**
 * @brief 获取FLASH存储空间大小
 * 
 * @param flash_id 
 * @param pdwSize 
 * @return u32 
 */
u32 flash_totalsize(u32 flash_id, u32 *pdwSize)
{
    if (NOR_FLASH == flash_id)
    {
        *pdwSize = sg_nor_total_size;
        return SUCCESS;
    }
    else if (NAND_FLASH == flash_id)
    {
        *pdwSize = sg_nand_total_size;
        return SUCCESS;
    }
    else
    {
        return ERR_FLASH_TYPE;
    }
}
