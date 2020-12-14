/**
 * @file vmm_deal.c
 * @author zhao.wei (hw)
 * @brief vmm实体处理函数集
 * @version 0.1
 * @date 2019-12-23
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include "pub.h"
#ifdef ZYNQMP
#include "vmm_zynqmp.conf"
#endif
#ifdef RK3399
#include "vmm_rk3399.conf"
#endif
#ifdef ZQ7020
#include "vmm_zq7020.conf"
#endif
#ifdef A311D
#include "vmm_a311d.conf"
#endif
/**用来临时存储传输文件*/
u8 ga_trans_data[VMM_TRANS_FILEDATA_LEN];

/**
 * @brief 字符串转16进制 去掉小数点
 * 
 * @param s 
 * @return int 
 */
static int htoi(u8 *s)
{
    u32 i;
    u32 n = 0;
    u32 len = strlen(s);
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
    {
        i = 2;
    }
    else
    {
        i = 0;
    }
    for (; i < len; ++i)
    {
        if ((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'))
        {
            if (tolower(s[i]) > '9')
            {
                n = 16 * n + (10 + tolower(s[i]) - 'a');
            }
            else
            {
                n = 16 * n + (tolower(s[i]) - '0');
            }
        }
        else
        {
        }
    }
    return n;
}

/**
 * @brief 获取当前总共有多少条版本控制信息
 * 
 * @param dev_info 版本信息存储位置
 * @param locat_num 版本存储的拐点条目数，采用循环存储，从该值处倒序查找即可
 * @param u32 *list总的条目数，如果大于VMM_VER_INFO_MAX_NUM说明有覆盖的情况发生
 * @return int 
 */
int vmm_get_verctrl_locat_list(u8 *dev_info, u16 *locat_num, u32 *list)
{
    u16 loop = 0;
    u32 offset = 0;
    u16 num = 0;
    s32 locatfd = -1;
    u32 list_num_old = 0;
    u32 list_num = 0;
    u32 crc_value = 0;
    struct vmm_verinfo_store verinfo_store;

    locatfd = mmc_open(dev_info);
    for (loop = 0; loop < VMM_VER_INFO_MAX_NUM; loop++) //遍历查找
    {
        if (0 > mmc_read(locatfd, offset, sizeof(struct vmm_verinfo_store),
                         (u8 *)&verinfo_store))
        {
            return ERR_VMM_INFO_READ;
        }
        else
        {
            //计算存储控制字的有效数据位的crc值用于校验，防止由于异常断电等写入无效的控制字
            crc_value = count_crc32((u8 *)&verinfo_store, sizeof(struct vmm_verinfo_store) - 4, 0);
            zlog_debug(zc, "vmm_get_verctrl_locat_list crc_value %x ", crc_value);
            if (VMM_VER_CTRL_INFO_MAGIC == verinfo_store.magic &&
                crc_value == verinfo_store.crc)
            {
                num++;
            }
            else
            {
                break;
            }
            list_num = verinfo_store.list_num;
            if (list_num < list_num_old)
            {
                num--;
                break; //有循环记录的数据产生
            }
            list_num_old = list_num;
        }
        offset += sizeof(struct vmm_verinfo_store);
    }
    *locat_num = num;
    *list = list_num_old;
    mmc_close(locatfd);
    return SUCCESS;
}

/**
 * @brief 获取版本信息的数据
 * 
 * @param dev_info 版本信息存储位置
 * @param offset 最新一条信息的偏移
 * @param info_num 倒序查找的数量
 * @param data 返回的数据
 * @return int 
 */
int vmm_get_verctrl_infodata(u8 *dev_info, u32 offset, u16 info_num, u8 *data)
{
    u16 loop = 0;
    u8 *data_offset = NULL;
    s32 locatfd = -1;

    locatfd = mmc_open(dev_info);
    data_offset = data;
    for (loop = 0; loop < info_num; loop++)
    {
        if (0 > mmc_read(locatfd, offset, sizeof(struct vmm_verinfo_store),
                         data_offset))
        {
            return ERR_VMM_INFO_READ;
        }
        else
        {
            offset -= sizeof(struct vmm_verinfo_store); //倒序查找
            data_offset += sizeof(struct vmm_verinfo_store);
        }
    }

    mmc_close(locatfd);
    return SUCCESS;
}

/**
 * @brief 从版本描述文件中解析出主版本数据结构
 * 
 * @param get_data 输入的描述信息
 * @param mainver_info 主版本数据结构
 * @return int 
 */
int vmm_parse_getdata(struct vmm_get_data *get_data, struct vmm_mainver_info *mainver_info)
{
    u32 ret = SUCCESS;
    zlog_debug(zc, " in ");

    //传入版本描述文件
    switch (get_data->trans_type)
    {
    case 'F':
        //cmd 解析
        ret = vmm_ftp_parse_verdata(get_data, mainver_info);
        break;
    case 'H':
        //url -->httpurl   data--> otaid otasecret
        //http 传输的文件类型为json 在http.c中解析
        ret = vmm_http_parse_verdata(get_data, mainver_info);
        break;
    default:
        break;
    }
    if (ret)
        exit_usage(1);
    return SUCCESS;
}

/**
 * @brief 对外的格式化输出函数
 * 
 * @param ver_type 版本类型
 * @param verinfo_store 查询的版本存储结构
 * @return int 
 */
int vmm_puts_verinfo(u8 ver_type, struct vmm_verinfo_store *verinfo_store, u16 num)
{
    u8 loop = 0, i = 0;
    u8 printf_info[1024] = {0};
    u8 string_use[20] = {0};
    u8 string_active[20] = {0};

    printf("\r\n Version information %d \r\n", num + 1);
    //zlog_info(zc, "version info %d ", num + 1);
    if (VMM_VER_TYPE_BOOT == ver_type)
    {
        sprintf(printf_info, "--%10s:%10s%15s:%10s%15s:%2d", "MainName",
                "BOOT", "MainVersion",
                verinfo_store->boot_mainver_info.mainver_version, "SubNumber",
                verinfo_store->boot_mainver_info.subver_num);
        printf("%s\r\n", printf_info);
        //zlog_info(zc, "%s ", printf_info); //记录info日志
        for (loop = 0; loop < verinfo_store->boot_mainver_info.subver_num; loop++)
        {
            sprintf(printf_info, "**%10s:%20s%15s:%10s", "SubName",
                    verinfo_store->boot_mainver_info.subver_info[loop].subver_name, "SubVersion",
                    verinfo_store->boot_mainver_info.subver_info[loop].subver_version);
            printf("%s\r\n", printf_info);
            //zlog_info(zc, "%s ", printf_info);
        }
    }
    if (VMM_VER_TYPE_SYS == ver_type)
    {
        for (i = 0; i < VMM_BACKUP_NUM; i++)
        {
            if (verinfo_store->sys_mainver_info[i].is_use)
                strcpy(string_use, "UseVersion");
            if (verinfo_store->sys_mainver_info[i].is_update)
                strcpy(string_use, "UpdateVersion");
            if (verinfo_store->sys_mainver_info[i].is_active)
                strcpy(string_active, "Active");

            sprintf(printf_info, "--%10s:%10s%15s:%10s%15s:%2d%15s%15s", "MainName",
                    "SYS", "MainVersion",
                    verinfo_store->sys_mainver_info[i].mainver_version, "SubNumber",
                    verinfo_store->sys_mainver_info[i].subver_num, string_use, string_active);
            printf("%s\r\n", printf_info);
            //zlog_info(zc, "%s ", printf_info);
            for (loop = 0; loop < verinfo_store->sys_mainver_info[i].subver_num; loop++)
            {
                sprintf(printf_info, "**%10s:%20s%15s:%10s", "SubName",
                        verinfo_store->sys_mainver_info[i].subver_info[loop].subver_name, "SubVersion",
                        verinfo_store->sys_mainver_info[i].subver_info[loop].subver_version);
                printf("%s\r\n", printf_info);
                //zlog_info(zc, "%s ", printf_info);
            }
            memset(string_use, 0, sizeof(string_use));
            memset(string_use, 0, sizeof(string_active));
        }
    }
#ifdef VMM_MANAGE_APP_EN
    if (VMM_VER_TYPE_APP == ver_type)
    {
        for (i = 0; i < VMM_BACKUP_NUM; i++)
        {
            if (verinfo_store->app_mainver_info[i].is_use)
                strcpy(string_use, "UseVersion");
            if (verinfo_store->app_mainver_info[i].is_update)
                strcpy(string_use, "UpdateVersion");
            if (verinfo_store->app_mainver_info[i].is_active)
                strcpy(string_active, "Active");

            sprintf(printf_info, "--%10s:%20s%15s:%10s%15s:%2d%15s%15s", "MainName",
                    "APP", "MainVersion",
                    verinfo_store->app_mainver_info[i].mainver_version, "SubNumber",
                    verinfo_store->app_mainver_info[i].subver_num, string_use, string_active);
            printf("%s\r\n", printf_info);
            //zlog_info(zc, "%s ", printf_info);
            for (loop = 0; loop < verinfo_store->app_mainver_info[i].subver_num; loop++)
            {
                sprintf(printf_info, "**%10s:%20s%15s:%10s", "SubName",
                        verinfo_store->app_mainver_info[i].subver_info[loop].subver_name, "SubVersion",
                        verinfo_store->app_mainver_info[i].subver_info[loop].subver_version, string_use);
                printf("%s\r\n", printf_info);
                //zlog_info(zc, "%s ", printf_info);
            }
            memset(string_use, 0, sizeof(string_use));
            memset(string_use, 0, sizeof(string_active));
        }
    }
#endif
    return SUCCESS;
}

/**
 * @brief 设置存储的版本信息的值，主备版本同时存在的将二者的控制信息同时更新，并将第一个版本置为可用的
 * 
 * @param ver_type 版本类型
 * @param verinfo_store 版本存储信息
 * @param mainver_info 解析的版本数据结构
 * @return int 
 */
int vmm_set_store_value(u8 ver_type, struct vmm_verinfo_store *verinfo_store,
                        struct vmm_mainver_info *mainver_info)
{
    verinfo_store->magic = VMM_VER_CTRL_INFO_MAGIC;
    if (VMM_VER_TYPE_BOOT == ver_type)
    {
        memcpy((u8 *)&(verinfo_store->boot_mainver_info), (u8 *)mainver_info,
               sizeof(struct vmm_mainver_info));
    }
    if (VMM_VER_TYPE_SYS == ver_type)
    {
        memcpy((u8 *)&(verinfo_store->sys_mainver_info[0]), (u8 *)mainver_info,
               sizeof(struct vmm_mainver_info));
        memcpy((u8 *)&(verinfo_store->sys_mainver_info[1]), (u8 *)mainver_info,
               sizeof(struct vmm_mainver_info));
        verinfo_store->sys_mainver_info[0].is_use = 1;
    }
#ifdef VMM_MANAGE_APP_EN
    if (VMM_VER_TYPE_APP == ver_type)
    {
        memcpy((u8 *)&(verinfo_store->app_mainver_info[0]), (u8 *)mainver_info,
               sizeof(struct vmm_mainver_info));
        memcpy((u8 *)&(verinfo_store->app_mainver_info[1]), (u8 *)mainver_info,
               sizeof(struct vmm_mainver_info));
        verinfo_store->app_mainver_info[0].is_use = 1;
    }
#endif
    return SUCCESS;
}

/**
 * @brief 获取版本信息，支持获取历史信息
 * 
 * @param dev_info 版本信息的存储位置
 * @param info_num 获取的条目数
 * @param data 版本信息索引
 * @param u16 *true_num
 */
int vmm_get_verctrl_info(u8 *dev_info, u16 info_num, u8 *data, u16 *true_num)
{
    u16 info_locat_num = 0;
    u16 info_all_num = 0; //版本总的条目数
    u32 ret = 0;
    u32 offset = 0;
    u32 list;
    u8 *ver_data = data;
    if (info_num > VMM_VER_INFO_MAX_NUM)
    {
        return ERR_VMM_INFO_MAX_NUM;
    }
    ret = vmm_get_verctrl_locat_list(dev_info, &info_locat_num, &list);
    if (ret)
        return ret;

    if (list < VMM_VER_INFO_MAX_NUM) //如果没有循环发生，则拐点就是当前条目总数
        info_all_num = info_locat_num;
    else
        info_all_num = VMM_VER_INFO_MAX_NUM;

    if (info_num > info_all_num)
        info_num = info_all_num; //如果获取条数大于总数，则获取最大数

    *true_num = info_all_num; //真实的条目数

    if (0 == info_all_num)
        offset = 0; //没有信息的时候返回最开始的信息
    else
        offset = (info_locat_num - 1) * sizeof(struct vmm_verinfo_store); //最新一条的偏移

    if (info_num < info_locat_num) //小于拐点，直接读取就可以
    {
        ret = vmm_get_verctrl_infodata(dev_info, offset, info_num, ver_data);
    }
    else //先倒序读取前半段，再倒序读取后半段
    {
        ret = vmm_get_verctrl_infodata(dev_info, offset, info_locat_num, ver_data);
        offset = (VMM_VER_INFO_MAX_NUM - 1) * sizeof(struct vmm_verinfo_store); //最末尾一条的偏移
        ver_data += sizeof(struct vmm_verinfo_store) * info_locat_num;
        ret = vmm_get_verctrl_infodata(dev_info, offset, (info_num - info_locat_num), ver_data);
    }
    if (ret)
        return ret;

    return SUCCESS;
}

/**
 * @brief 设置版本信息
 * 
 * @param dev_info 版本信息的存储位置
 * @param data 数据区
 * @return int 
 */
int vmm_set_verctrl_info(u8 *dev_info, u8 *data)
{
    u16 info_locat_num = 0;
    u32 ret = 0;
    u32 offset = 0;
    s32 locatfd = -1;
    u32 list;
    locatfd = mmc_open(dev_info);
    u32 crc_value = 0;
    struct vmm_verinfo_store *write_data = NULL;

    write_data = (struct vmm_verinfo_store *)data;
    ret = vmm_get_verctrl_locat_list(dev_info, &info_locat_num, &list);
    if (ret)
        return ret;

    zlog_debug(zc, "info_locat_num %d info", info_locat_num);
    if (info_locat_num >= VMM_VER_INFO_MAX_NUM)
        offset = 0;
    else
        offset = info_locat_num * sizeof(struct vmm_verinfo_store); //获取偏移

    write_data->list_num = list + 1;
    crc_value = count_crc32((u8 *)write_data, sizeof(struct vmm_verinfo_store) - 4, 0); //计算crc的值用于校验
    write_data->crc = crc_value;
    zlog_debug(zc, "vmm_set_verctrl_info crc_value %x ", crc_value);

    zlog_info(zc, "write_data->list_num %d ", write_data->list_num);
    if (0 > mmc_write(locatfd, offset, sizeof(struct vmm_verinfo_store),
                      data))
        return ERR_VMM_INFO_WRITE;

    mmc_close(locatfd);
    return SUCCESS;
}

/**
 * @brief 下载子版本
 * 
 * @param download_index 对于主备版本的下载索引
 * @param download_type 下载类型ftp/http/tftp
 * @param subver_info 从版本描述文件中解析的版本信息
 * @param subver_def 版本预定义的数据结构
 * @return int 
 */
int vmm_download_subver(u8 sub_index, u8 download_index, u8 download_type, struct vmm_subver_info *subver_info, struct vmm_subver_def *subver_def)
{
    u32 ret = 0;
    zlog_debug(zc, "version %s name %s size %u md5 %s ", subver_info->subver_version,
               subver_info->subver_name, subver_info->size, subver_info->md5);
    zlog_debug(zc, "dowanload locat %s  ", subver_def->ver_store[download_index].location);
    zlog_debug(zc, "download_type %c ", download_type);
    //boot 版本不分主备版本，直接下载到存储的0位置上
    if (download_index == 0xff)
    {
        download_index = 0;
    }
    if (download_type == 'F')
    {
        ret = vmm_ftp_version_file(sub_index, subver_info->subver_name, subver_def->ver_store[download_index].location, subver_info->md5, subver_info->size);
    }
    else if (download_type == 'H')
    {
        ret = vmm_http_version_file(sub_index, subver_info->subver_name, subver_def->ver_store[download_index].location, subver_info->md5, subver_info->size);
    }
    else
    {
    }
    if (!ret)
    {
        zlog_info(zc, "version %s name %s size %d md5 %s ", subver_info->subver_version,
                  subver_info->subver_name, subver_info->size, subver_info->md5);
        zlog_info(zc, "dowanload locat %s  ", subver_def->ver_store[download_index].location);
        zlog_info(zc, "download_type %c ", download_type);
    }
    return ret;
}

/**
 * @brief 下载版本
 * 
 * @param ver_type 版本类型 
 * @param trans_type 传输类型
 * @param mainver_info 版本描述文件
 * @return int 
 */
int vmm_update_ver_deal(u8 ver_type, u8 trans_type, struct vmm_mainver_info *mainver_info)
{
    u8 loop = 0;
    u32 ret = 0;
    u16 true_num = 0;
    u8 downloa_index = 0; //下载版本的索引
    struct vmm_verinfo_store verinfo_store = {0};

    //如果解析的子版本个数 与 定义的子版本个数不一致 则返回错误值
    if (mainver_info->subver_num != sat_mainver_def[ver_type].subver_num)
    {
        zlog_error(zc, "error code %x \r\n", ERR_VMM_INFO_MAX_NUM);
        return ERR_VMM_SUBNUM_CHECK;
    }
    //获取当前的版本信息
    ret = vmm_get_verctrl_info(VMM_VER_INFO_STORE_LOCATION, 1, &verinfo_store, &true_num);
    if (ret)
    {
        zlog_error(zc, "%s error code %x \r\n", __func__, ret);
        return ret;
    }

    if (VMM_VER_TYPE_BOOT == ver_type)
    {
        downloa_index = 0xff;
    }
    verinfo_store.magic = VMM_VER_CTRL_INFO_MAGIC;
    //如果第一块存储被使用 则下载到另一块 反之亦然
    if (VMM_VER_TYPE_SYS == ver_type)
    {
        //正在使用的版本该值为1 备用版本该值为0
        downloa_index = (verinfo_store.sys_mainver_info[0].is_use) > 0 ? 1 : 0;
        zlog_debug(zc, "downloa_index %x ", downloa_index);
        memcpy((u8 *)&(verinfo_store.sys_mainver_info[downloa_index]), (u8 *)mainver_info,
               sizeof(struct vmm_mainver_info));
        //更新标志位
        verinfo_store.sys_mainver_info[downloa_index].is_update = 1;
    }
#ifdef VMM_MANAGE_APP_EN
    if (VMM_VER_TYPE_APP == ver_type)
    {
        downloa_index = (verinfo_store.app_mainver_info[0].is_use) > 0 ? 1 : 0;
        zlog_debug(zc, "downloa_index %x ", downloa_index);
        memcpy((u8 *)&(verinfo_store.app_mainver_info[downloa_index]), (u8 *)mainver_info,
               sizeof(struct vmm_mainver_info));
        //更新标志位
        verinfo_store.app_mainver_info[downloa_index].is_update = 1;
    }

#endif
    zlog_info(zc, "update mainversion %s ", mainver_info->mainver_version); //记录日志
    for (loop = 0; loop < mainver_info->subver_num; loop++)
    {
        ret = vmm_download_subver(loop, downloa_index, trans_type, &(mainver_info->subver_info[loop]), &(sat_mainver_def[ver_type].subver_def[loop]));
    }

    //设置版本控制字
    if (ret)
    {
        zlog_error(zc, "%s error code %x \r\n", __func__, ret);
        return ret;
    }
    ret = vmm_set_verctrl_info(VMM_VER_INFO_STORE_LOCATION, &verinfo_store);
    if (ret)
    {
        zlog_error(zc, "%s error code %x \r\n", __func__, ret);
        return ret;
    }

    return SUCCESS;
}

/**
 * @brief 获取版本
 * 
 * @param ver_type 版本类型
 * @param num 版本数量
 * @return int 
 */
int vmm_get_ver_deal(u8 ver_type, u16 num)
{
    u32 ret = SUCCESS;
    u16 loop = 0;
    u16 true_num = 0; //实际返回的条目数
    struct vmm_verinfo_store verinfo_store[VMM_VER_INFO_MAX_NUM] = {0};

    ret = vmm_get_verctrl_info(VMM_VER_INFO_STORE_LOCATION, num, verinfo_store, &true_num);

    if (ret)
    {
        zlog_error(zc, "%s error code %x \r\n", __func__, ret);
        return ret;
    }

    if (true_num)
    {
        num = num > true_num ? true_num : num; //最多返回真实的条目
        printf("Version information Item Is %d \r\n", num);
        //zlog_info(zc, "get version info item %d ", num); //记录info日志
        for (loop = 0; loop < num; loop++)
        {
            zlog_debug(zc, "verinfo_store.list_num %d", verinfo_store[loop].list_num);
            vmm_puts_verinfo(ver_type, &verinfo_store[loop], loop);
        }
    }
    else
    {
        printf("Version information Item Is 0 \r\n");
    }

    return ret;
}

/**
 * @brief 设置版本信息
 * 
 * @param ver_type 版本类型
 * @param mainver_info 版本描述文件
 * @return int 
 */
int vmm_set_ver_deal(u8 ver_type, struct vmm_mainver_info *mainver_info)
{
    u32 ret = 0;
    u16 true_num = 0;
    struct vmm_verinfo_store verinfo_store = {0};

    ret = vmm_get_verctrl_info(VMM_VER_INFO_STORE_LOCATION, 1, &verinfo_store, &true_num);
    if (ret)
    {
        zlog_error(zc, "%s error code %x \r\n", __func__, ret);
        return ret;
    }

    vmm_set_store_value(ver_type, &verinfo_store, mainver_info);

    ret = vmm_set_verctrl_info(VMM_VER_INFO_STORE_LOCATION, &verinfo_store);
    if (ret)
    {
        zlog_error(zc, "%s error code %x \r\n", __func__, ret);
        return ret;
    }

    return SUCCESS;
}

/**
 * @brief 加载版本,暂时不考虑版本回滚，后期可以加上
 * 
 * @param mount_dir 挂载目录，如果后续传入多个分区，这里可以通过文件解析 
 * @return int 
 */
int vmm_load_ver_deal(u8 *mount_dir)
{
    u32 ret = SUCCESS;
    u16 true_num = 0;
    u8 load_index = 0; //加载索引
    u8 locat_def[VMM_DEFINE_NAME_LEN] = {0};
    u8 cmd[VMM_DEFINE_NAME_LEN * 2] = {0};
    u8 loop = 0;
    struct vmm_verinfo_store verinfo_store = {0};

    ret = vmm_get_verctrl_info(VMM_VER_INFO_STORE_LOCATION, 1, &verinfo_store, &true_num);
    zlog_debug(zc, "verinfo_store.list_num %d", verinfo_store.list_num);
    if (ret)
    {
        zlog_error(zc, "%s error code %x \r\n", __func__, ret);
        return ret;
    }
    if (0 == true_num)
    {
        printf("No Version Can Load .\r\n");
        return ret;
    }
    zlog_debug(zc, "is_update %x %x-", verinfo_store.app_mainver_info[0].is_update,
               verinfo_store.app_mainver_info[1].is_update);
    zlog_debug(zc, "is_use %x %x *", verinfo_store.app_mainver_info[0].is_use,
               verinfo_store.app_mainver_info[1].is_use);
    if (0 == verinfo_store.app_mainver_info[0].is_update &&
        0 == verinfo_store.app_mainver_info[1].is_update)
    {
        //没有版本更新或 激活事件发生，正常进行加载引导
        load_index = (verinfo_store.app_mainver_info[0].is_use) > 0 ? 0 : 1;
        for (loop = 0; loop < verinfo_store.app_mainver_info[load_index].subver_num; loop++) //后续可扩展多分区
        {
            memcpy(locat_def, sat_mainver_def[VMM_VER_TYPE_APP].subver_def[loop].ver_store[load_index].location,
                   strlen(sat_mainver_def[VMM_VER_TYPE_APP].subver_def[loop].ver_store[load_index].location));
            zlog_debug(zc, "load def %s ", locat_def);
        }
        sprintf(cmd, "mount -t ext4 %s %s", locat_def, mount_dir);
    }
    else
    {
        //有版本更新或激活发生
        load_index = (verinfo_store.app_mainver_info[0].is_update) > 0 ? 0 : 1;
        for (loop = 0; loop < verinfo_store.app_mainver_info[load_index].subver_num; loop++) //后续可扩展多分区
        {
            memcpy(locat_def, sat_mainver_def[VMM_VER_TYPE_APP].subver_def[loop].ver_store[load_index].location,
                   strlen(sat_mainver_def[VMM_VER_TYPE_APP].subver_def[loop].ver_store[load_index].location));
            zlog_debug(zc, "load def %s ", locat_def);
        }

        sprintf(cmd, "mount -t ext4 %s %s", locat_def, mount_dir);
        //重写版本控制字

        verinfo_store.app_mainver_info[0].is_update = 0;
        verinfo_store.app_mainver_info[1].is_update = 0;
        verinfo_store.app_mainver_info[0].is_use = 0;
        verinfo_store.app_mainver_info[1].is_use = 0;
        verinfo_store.app_mainver_info[load_index].is_use = 1;

        ret = vmm_set_verctrl_info(VMM_VER_INFO_STORE_LOCATION, &verinfo_store);
        if (ret)
        {
            zlog_error(zc, "%s error code %x \r\n", __func__, ret);
            return ret;
        }
    }

    zlog_info(zc, "load version cmd is %s ", cmd); //记录info日志
    system(cmd);
    return ret;
}

/**
 * @brief ota处理函数,

 * 
 * @param ver_type 版本类型
 * @param trans_type 传输类型
 * @param mainver_info 版本描述信息
 * @return int 
 */
int vmm_ota_ver_deal(u8 ver_type, u8 trans_type, struct vmm_mainver_info *mainver_info)
{
    u8 loop = 0;
    u32 ret = 0;
    u16 true_num = 0;
    u8 downloa_index = 0; //下载版本的索引
    u8 download_flag = 0;
    u8 load_index = 0; //加载索引
    //u8 cmd[VMM_DEFINE_NAME_LEN * 2] = {0};
    struct vmm_verinfo_store verinfo_store = {0};

    /**重要 ！！！！此接口应当在load结束后再进行调用！！！！！，\
    如果后期加入版本回滚机制则需在版本全部正常启动后再进行调用！！！*/

    //printf("\r\nota version is %s \r\n", mainver_info->mainver_version);

    zlog_info(zc, "ota version is %s ", mainver_info->mainver_version);
    ret = vmm_get_verctrl_info(VMM_VER_INFO_STORE_LOCATION, 1, &verinfo_store, &true_num);
    zlog_debug(zc, "verinfo_store.list_num %d", verinfo_store.list_num);
    if (ret)
    {
        zlog_error(zc, "%s error code %x \r\n", __func__, ret);
        return ret;
    }
    if (VMM_VER_TYPE_BOOT == ver_type)
    {
        downloa_index = 0xff;
    }
    verinfo_store.magic = VMM_VER_CTRL_INFO_MAGIC;
    //如果第一块存储被使用 则下载到另一块 反之亦然
    if (VMM_VER_TYPE_SYS == ver_type)
    {
        //正在使用的版本该值为1 备用版本该值为0
        downloa_index = (verinfo_store.sys_mainver_info[0].is_use) > 0 ? 1 : 0;
        load_index = (verinfo_store.sys_mainver_info[0].is_use) > 0 ? 0 : 1;
        zlog_debug(zc, "downloa_index %x load_index %x", downloa_index, load_index);

        zlog_info(zc, "load version is %s ", verinfo_store.sys_mainver_info[load_index].mainver_version);
        if (1 == verinfo_store.sys_mainver_info[load_index].is_active) //有激活的情况发生
        {
            //被激活的版本将不会与服务器校验本版本，而是 校验另一版本
            printf("now is active state \r\n");
            printf("back version is %s \r\n", verinfo_store.sys_mainver_info[downloa_index].mainver_version);
            if ((load_index != downloa_index) &&
                (htoi(verinfo_store.sys_mainver_info[downloa_index].mainver_version) < htoi(mainver_info->mainver_version)))
            {
                memcpy((u8 *)&(verinfo_store.sys_mainver_info[downloa_index]), (u8 *)mainver_info,
                       sizeof(struct vmm_mainver_info));
                //更新标志位
                verinfo_store.sys_mainver_info[downloa_index].is_update = 1;
                download_flag = 1;
            }
        }
        else
        {
            if ((load_index != downloa_index) &&
                (htoi(verinfo_store.sys_mainver_info[load_index].mainver_version) < htoi(mainver_info->mainver_version)))
            {
                memcpy((u8 *)&(verinfo_store.sys_mainver_info[downloa_index]), (u8 *)mainver_info,
                       sizeof(struct vmm_mainver_info));
                //更新标志位
                verinfo_store.sys_mainver_info[downloa_index].is_update = 1;
                download_flag = 1;
            }
        }
    }
#ifdef VMM_MANAGE_APP_EN
    if (VMM_VER_TYPE_APP == ver_type)
    {
        downloa_index = (verinfo_store.app_mainver_info[0].is_use) > 0 ? 1 : 0;
        load_index = (verinfo_store.app_mainver_info[0].is_use) > 0 ? 0 : 1;
        zlog_debug(zc, "downloa_index %x load_index %x", downloa_index, load_index);
        //printf("load version is %s \r\n", verinfo_store.app_mainver_info[load_index].mainver_version);
        zlog_info(zc, "load version is %s ", verinfo_store.app_mainver_info[load_index].mainver_version);
        if (1 == verinfo_store.app_mainver_info[load_index].is_active) //有激活的情况发生
        {
            //被激活的版本将不会与服务器校验本版本，而是 校验另一版本
            printf("now is active state \r\n");
            printf("back version is %s \r\n", verinfo_store.app_mainver_info[downloa_index].mainver_version);
            if ((load_index != downloa_index) &&
                (htoi(verinfo_store.app_mainver_info[downloa_index].mainver_version) < htoi(mainver_info->mainver_version)))
            {
                memcpy((u8 *)&(verinfo_store.app_mainver_info[downloa_index]), (u8 *)mainver_info,
                       sizeof(struct vmm_mainver_info));
                //更新标志位
                verinfo_store.app_mainver_info[downloa_index].is_update = 1;
                download_flag = 1;
            }
        }
        else
        {
            if ((load_index != downloa_index) &&
                (htoi(verinfo_store.app_mainver_info[load_index].mainver_version) < htoi(mainver_info->mainver_version)))
            {
                memcpy((u8 *)&(verinfo_store.app_mainver_info[downloa_index]), (u8 *)mainver_info,
                       sizeof(struct vmm_mainver_info));
                //更新标志位
                verinfo_store.app_mainver_info[downloa_index].is_update = 1;
                download_flag = 1;
            }
        }
    }

#endif

    if (download_flag)
    {
        printf("start ota .\r\n");
        //sprintf(cmd, "aplay /home/ab64/ab_data/voice/ota01.wav");
        //system(cmd);
        for (loop = 0; loop < mainver_info->subver_num; loop++)
        {
            ret = vmm_download_subver(loop, downloa_index, trans_type, &(mainver_info->subver_info[loop]), &(sat_mainver_def[ver_type].subver_def[loop]));
        }
        //设置版本控制字
        if (ret)
        {
            zlog_error(zc, "%s error code %x \r\n", __func__, ret);
            return ret;
        }
        ret = vmm_set_verctrl_info(VMM_VER_INFO_STORE_LOCATION, &verinfo_store);
        if (ret)
        {
            zlog_error(zc, "%s error code %x \r\n", __func__, ret);
            return ret;
        }
        //sprintf(cmd, "aplay /home/ab64/ab_data/voice/ota02.wav");
        //system(cmd);
    }
    printf("ota end .\r\n");
    return SUCCESS;
}

/**
 * @brief 版本激活处理函数
 * 
 * @param ver_type 版本类型
 * @return int 
 */
int vmm_active_ver_deal(u8 ver_type)
{
    u32 ret = 0;
    u16 true_num = 0;
    struct vmm_verinfo_store verinfo_store = {0};
    struct vmm_mainver_info *mainver_info = NULL;
    u8 active_index = 0;
    /**重要 ！！！！版本激活只是用于将****备份的有效版本激活****，备份版本的有效指的是含有版本信息的非当前使用版
     * 一旦备份版本被激活后，ota将不会用于等同于主版本号的版本，但不影响手动更新
     * eg ：主：1.00.05 备：1.00.04 当1.00.04版本被激活后，不会再重新ota1.00.05，直至1.00.06版本的更新发生
    */
    ret = vmm_get_verctrl_info(VMM_VER_INFO_STORE_LOCATION, 1, &verinfo_store, &true_num); //获取最新的一条版本信息
    zlog_debug(zc, "verinfo_store.list_num %d", verinfo_store.list_num);
    if (ret)
        return ret;

    if (VMM_VER_TYPE_SYS == ver_type)
    {
        active_index = (verinfo_store.sys_mainver_info[0].is_use) > 0 ? 1 : 0;
        mainver_info = &(verinfo_store.sys_mainver_info[active_index]);
    }
#ifdef VMM_MANAGE_APP_EN
    if (VMM_VER_TYPE_APP == ver_type)
    {
        active_index = (verinfo_store.app_mainver_info[0].is_use) > 0 ? 1 : 0;
        mainver_info = &(verinfo_store.app_mainver_info[active_index]);
    }
#endif
    //zlog_debug(zc, "ver_type %x", ver_type);
    zlog_debug(zc, "ver_A use %x", verinfo_store.app_mainver_info[0].is_use);
    zlog_debug(zc, "ver_B use %x", verinfo_store.app_mainver_info[1].is_use);

    //zlog_debug(zc, "active_index %x", active_index);
    zlog_info(zc, "active type %x active index %x ", ver_type, active_index); //记录info日志
    if (mainver_info->subver_num != 0)
    {
        mainver_info->is_active = 1; //设置版本激活标志位
        mainver_info->is_update = 1; //设置版本激活标志值位

        ret = vmm_set_verctrl_info(VMM_VER_INFO_STORE_LOCATION, &verinfo_store);
        if (ret)
        {
            zlog_error(zc, "%s error code %x \r\n", __func__, ret);
            return ret;
        }
    }
    else
    {
        //无有效的激活版本
    }

    return SUCCESS;
}

/**
 * @brief 版本去激活处理函数
 * 
 * @param ver_type 版本类型
 * @return int 
 */
int vmm_deactive_ver_deal(u8 ver_type)
{
    u32 ret = 0, i;
    u16 true_num = 0;
    struct vmm_verinfo_store verinfo_store = {0};
    struct vmm_mainver_info *mainver_info = NULL;
    u8 load_index = 0;
    /**重要 ！！！！版本去激活是清除掉版本激活的标志位，使已激活的版本进行正常的ota活动
    */
    ret = vmm_get_verctrl_info(VMM_VER_INFO_STORE_LOCATION, 1, &verinfo_store, &true_num); //获取最新的一条版本信息
    zlog_debug(zc, "verinfo_store.list_num %d", verinfo_store.list_num);
    if (ret)
    {
        zlog_error(zc, "deactive error code %x \r\n", ret);
        return ret;
    }
    //去版本激活会将主备版本的激活标志都清除掉
    for (i = 0; i < VMM_BACKUP_NUM; i++)
    {
        if (VMM_VER_TYPE_SYS == ver_type)
        {
            mainver_info = &(verinfo_store.sys_mainver_info[i]);
        }
#ifdef VMM_MANAGE_APP_EN
        if (VMM_VER_TYPE_APP == ver_type)
        {
            mainver_info = &(verinfo_store.app_mainver_info[i]);
        }
#endif
        zlog_debug(zc, "load_index %x", load_index);
        zlog_info(zc, "deactive type %x ", ver_type); //记录info日志
        if (mainver_info->subver_num != 0)
        {
            zlog_debug(zc, "deactive version index  %x", load_index);
            mainver_info->is_active = 0; //清除版本激活标志位
            mainver_info->is_update = 0;
            ret = vmm_set_verctrl_info(VMM_VER_INFO_STORE_LOCATION, &verinfo_store);
            if (ret)
            {
                zlog_error(zc, "%s error code %x \r\n", __func__, ret);
                return ret;
            }
        }
        else
        {
            //无有效的激活版本
        }
    }
    return SUCCESS;
}