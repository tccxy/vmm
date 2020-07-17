/**
 * @file vmm.h
 * @author zhao.wei (hw)
 * @brief vmm主程序
 * @version 0.1
 * @date 2019-12-17
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef __VMM_H__
#define __VMM_H__

#define VMM_MANAGE_APP_EN     /**APP版本 是否进行单独管理*/
#define VMM_BACKUP_NUM 2      /**版本的最大主备数为2*/
#define VMM_MAX_SUB_VER_NUM 8 /**管理的子版本个数最大为8*/

#define VMM_CRC_POLY 0x04c11db7 /**初始化crc表时候需要用到的*/

#define VMM_VER_DEV_TYPE (u8 *)"VMM_ABU" /**可自定义的设备类型*/
#define VMM_VER_INFO_MAX_NUM 128         /**累计可追溯的版本信息条目数*/
#define VMM_DEFINE_NAME_LEN 64           /**定义名字占用的字节数*/
#define VMM_TRANS_FILEDATA_LEN 2048      /**版本描述文件的最大长度*/
#define VMM_STORE_TYPE_ADDR 0            /**版本存在flash中使用该形式*/
#define VMM_STORE_TYPE_FILE 1            /**版本存在mmc设备中使用该形式*/

#define VMM_VER_TYPE_BOOT 0            /**Boot版本 */
#define VMM_VER_TYPE_SYS 1             /**系统版本 */
#define VMM_VER_TYPE_APP 2             /**应用版本 */
#define VMM_VER_CTRL_INFO_MAGIC 0x5a5a /**版本信息魔术字*/
#define VMM_DOWNLOAD_TIMEOUT 60        /**版本下载超时时间(单位: 秒) */

//#define VMM_DEBUG_EN
#ifdef VMM_DEBUG_EN
#define VMM_DEBUG(format, ...) printf("File: "__FILE__             \
                                      ",(%s) Line: %05d: " format, \
                                      __func__, __LINE__, ##__VA_ARGS__)

#else
#define VMM_DEBUG(format, ...)
#endif

/**用来临时存储传输文件*/
extern u8 ga_trans_data[VMM_TRANS_FILEDATA_LEN];

/**
 * @brief vmm获取输入的信息
 * 
 */
struct vmm_get_data
{
    u8 opt_type;   /**操作类型 升级 查询 激活 反激活*/
    u8 ver_type;   /**版本类型 boot sys app*/
    u8 trans_type; /**传输类型 ftp tftp*/
    u8 pad;
    u8 url[VMM_DEFINE_NAME_LEN]; /**文件名字*/
    u8 data[0];                  /**other data*/
};

/**
 * @brief 主版本信息
 * 
 */
struct vmm_subver_info
{
    u8 subver_version[VMM_DEFINE_NAME_LEN]; /**子版本号 */
    u8 subver_name[VMM_DEFINE_NAME_LEN];    /**子版本名字*/
    u8 md5[VMM_DEFINE_NAME_LEN];            /**md5校验值,版本文件采用MD5校验 */
    u64 size;                               /**版本大小 */
};

/**
 * @brief 主版本信息
 * 
 */
struct vmm_mainver_info
{
    u8 mainver_version[VMM_DEFINE_NAME_LEN]; /**主版本号 */
    u8 mainver_name[VMM_DEFINE_NAME_LEN];    /**主版本名字*/
    u8 is_use;                               /**目前是否使用该版本*/
    u8 is_update;                            /**设置更新标志*/
    u8 is_active;                            /**手动激活标志位*/
    u8 subver_num;                           /**子版本个数 */
    u32 pad;
    struct vmm_subver_info subver_info[VMM_MAX_SUB_VER_NUM];
    u16 crc; /**校验值 */
};

/**
 * @brief 版本控制存储数据结构
 * ver_info_store
 */
struct vmm_verinfo_store
{
    u32 magic;                                                /**魔术字 */
    struct vmm_mainver_info boot_mainver_info;                /**划归到 boot的只提供单版本的管理*/
    struct vmm_mainver_info sys_mainver_info[VMM_BACKUP_NUM]; /**划归到 sys的提供主备版本的管理*/
#ifdef VMM_MANAGE_APP_EN
    struct vmm_mainver_info app_mainver_info[VMM_BACKUP_NUM]; /**划归到 app的提供主备版本的管理*/
#endif
    u32 list_num; /**循环记录,该值是累加的，通过此值记录标志最新的一条版本信息*/
    u32 crc; /**版本控制信息校验值 */
};

/**
 * @brief 版本按文件存储的数据结构
 * 
 */
struct vmm_ver_store_location
{
    u8 *location;
};

/*
 * @brief 子版本定义数据结构
 * 
 */
struct vmm_subver_def
{
    u8 store_type;                                           /**版本存储方式 */
    struct vmm_ver_store_location ver_store[VMM_BACKUP_NUM]; /**版本存储位置 为了适配主备版本采用双数据存储 */
};
/**
 * @brief 主版本定义数据结构
 * 
 */
struct vmm_mainver_def
{
    u8 *mainver_name;                                      /**主版本的名字 */
    u8 is_backup;                                          /**是否为主备版本 */
    u8 subver_num;                                         /**包含的子版本个数 */
    struct vmm_subver_def subver_def[VMM_MAX_SUB_VER_NUM]; /**子版本定义 */
};

void exit_usage(u8 leavel);
void init_crc32_table(u32 poly);
u32 count_crc32(const u8 *pdata, u32 len, u32 crc);
int cmd_parse_vertype(u8 *data, struct vmm_get_data *get_data);
int vmm_ftp_parse_verdata(struct vmm_get_data *get_data, struct vmm_mainver_info *mainver_info);
int vmm_ftp_version_file(u8 sub_index, u8 *version_file, u8 *version_locat, u8 *version_md5, u64 version_size);
int vmm_http_parse_verdata(struct vmm_get_data *get_data, struct vmm_mainver_info *mainver_info);
int vmm_http_version_file(u8 sub_index, u8 *version_file, u8 *version_locat, u8 *version_md5, u64 version_size);
int vmm_parse_getdata(struct vmm_get_data *get_data, struct vmm_mainver_info *mainver_info);
int vmm_puts_verinfo(u8 ver_type, struct vmm_verinfo_store *verinfo_store, u16 num);
int vmm_set_store_value(u8 ver_type, struct vmm_verinfo_store *verinfo_store,
                        struct vmm_mainver_info *mainver_info);
int vmm_set_verctrl_info(u8 *dev_info, u8 *data);
int vmm_get_verctrl_info(u8 *dev_info, u16 info_num, u8 *data, u16 *true_num);
int vmm_get_ver_deal(u8 ver_type, u16 num);
int vmm_set_ver_deal(u8 ver_type, struct vmm_mainver_info *mainver_info);
int vmm_update_ver_deal(u8 ver_type, u8 trans_type, struct vmm_mainver_info *mainver_info);
int vmm_load_ver_deal(u8 *mount_dir);
int vmm_ota_ver_deal(u8 ver_type, u8 trans_type, struct vmm_mainver_info *mainver_info);
int vmm_active_ver_deal(u8 ver_type);
int vmm_deactive_ver_deal(u8 ver_type);

#endif