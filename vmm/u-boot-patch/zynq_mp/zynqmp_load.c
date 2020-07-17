#include <common.h>
#include <command.h>

#define IMAGE_A_PART 3 //zynqmp 的 image 分区标号
#define IMAGE_B_PART 4
#define ROOTFS_A_PART 5
#define ROOTFS_B_PART 6

#define DDR_ADDR 0x10000000 //内存地址
#define VMP_MANAGE_APP_EN
#define VMP_BACKUP_NUM 2               /**版本的最大主备数为2*/
#define VMP_MAX_SUB_VER_NUM 8          /**管理的子版本个数最大为8*/
#define VMP_DEFINE_NAME_LEN 64         /**定义名字占用的字节数*/
#define VMP_VER_INFO_MAX_NUM 128       /**累计可追溯的版本信息条目数*/
#define VMP_VER_CTRL_INFO_MAGIC 0x5a5a /**版本信息魔术字*/

/**
 * @brief 主版本信息
 * 
 */
struct vmp_subver_info
{
    u8 subver_version[VMP_DEFINE_NAME_LEN]; /**子版本号 */
    u8 subver_name[VMP_DEFINE_NAME_LEN];    /**子版本名字*/
    u8 md5[VMP_DEFINE_NAME_LEN];            /**md5校验值,版本文件采用MD5校验 */
    u64 size;                               /**版本大小 */
};

/**
 * @brief 主版本信息
 * 
 */
struct vmp_mainver_info
{
    u8 mainver_version[VMP_DEFINE_NAME_LEN]; /**主版本号 */
    u8 mainver_name[VMP_DEFINE_NAME_LEN];    /**主版本名字*/
    u8 is_use;                               /**目前是否使用该版本*/
    u8 is_update;                            /**设置更新标志*/
    u8 is_active;                            /**手动激活标志位*/
    u8 subver_num;                           /**子版本个数 */
    u32 pad;
    struct vmp_subver_info subver_info[VMP_MAX_SUB_VER_NUM];
    u16 crc; /**校验值 */
};

/**
 * @brief 版本控制存储数据结构
 * ver_info_store
 */
struct vmp_verinfo_store
{
    u32 magic;                                                /**魔术字 */
    struct vmp_mainver_info boot_mainver_info;                /**划归到 boot的只提供单版本的管理*/
    struct vmp_mainver_info sys_mainver_info[VMP_BACKUP_NUM]; /**划归到 sys的提供主备版本的管理*/
#ifdef VMP_MANAGE_APP_EN
    struct vmp_mainver_info app_mainver_info[VMP_BACKUP_NUM]; /**划归到 app的提供主备版本的管理*/
#endif
    u32 list_num; /**循环记录,该值是累加的，通过此值记录标志最新的一条版本信息*/
    u32 crc;      /**版本控制信息校验值 */
};

/**
 * @brief 获取bootcmd
 * 
 * @param *index load index
 * @param verinfo_store 版本存储信息
 * @param set_flag 是否重新设置版本信息标志
 * @return int 
 */
int zynqmp_get_bootcmd(char *index, struct vmp_verinfo_store *verinfo_store, char *set_flag)
{

    char load_index = 0; //加载索引

    if (0 == verinfo_store->sys_mainver_info[0].is_update &&
        0 == verinfo_store->sys_mainver_info[1].is_update)
    {
        //没有版本更新或 激活事件发生，正常进行加载引导
        load_index = (verinfo_store->sys_mainver_info[0].is_use) > 0 ? 0 : 1;
        *set_flag = 0;
    }
    else
    {
        //有版本更新或激活发生
        load_index = (verinfo_store->sys_mainver_info[0].is_update) > 0 ? 0 : 1;
        *set_flag = 1;
    }
    *index = load_index;
    return 0;
}
/**
 * @brief zynqmp的加载函数
 * 
 * @param cmdtp 
 * @param flag 
 * @param argc 参数个数
 * @param argv 第一个为sd设备编号 第二个ddr的暂存地址 第三个 镜像名字
 * @return int 
 */
int do_zynqmp_load(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int i = 0;
    char cmd[1024] = {0}; //cmd命令缓存
    int block_num = 0;    //需要从mmc中读取的block个数
    int info_num = 0;     //有效的信息条目
    char load_index = 0;
    u32 list_num_old = 0;
    u32 list_num = 0;
    char set_verflag = 0;
    struct vmp_verinfo_store *verinfo_store = NULL;
    struct vmp_verinfo_store *verinfo_write = NULL;
    struct vmp_verinfo_store verinfo_tmp = {0}; //版本信息临时存储结构体

    block_num = 1 + (sizeof(struct vmp_verinfo_store) * VMP_VER_INFO_MAX_NUM / 512);
    printf("do_zynqmp_load in block_num %d \n", block_num);
    if (argc < 4)
        return 0;
    sprintf(cmd, "mmc dev %s", argv[1]);
    //printf("** %s \n", cmd);
    run_command(cmd, 0);

    sprintf(cmd, "mmc read %x %s %x", DDR_ADDR, argv[2], block_num);
    //printf("** %s \n", cmd);
    run_command(cmd, 0);

    verinfo_store = (struct vmp_verinfo_store *)DDR_ADDR;
    for (i = 0; i < VMP_VER_INFO_MAX_NUM; i++)
    {
        //printf("magic %x \n", verinfo_store->magic);
        if (VMP_VER_CTRL_INFO_MAGIC == verinfo_store->magic) //只要包含5a5a就证明是一条有效信息
        {
            info_num++;
        }
        else
        {
            break;
        }
        list_num = verinfo_store->list_num;
        if (list_num < list_num_old)
        {
            info_num--;
            break; //有循环记录的数据产生
        }
        list_num_old = list_num;
        verinfo_store += 1;
    }

    if (info_num >= VMP_VER_INFO_MAX_NUM) //如果存储信息满了
    {
        verinfo_write = (struct vmp_verinfo_store *)DDR_ADDR;
    }
    else
    {
        verinfo_write = (struct vmp_verinfo_store *)DDR_ADDR + info_num;
    }

    if (info_num) //如果有版本信息存在,先将旧的信息拷贝出来
        memcpy((char *)&verinfo_tmp, (char *)((struct vmp_verinfo_store *)DDR_ADDR + (info_num - 1)), sizeof(struct vmp_verinfo_store));

    printf("do_zynqmp_load have %d info list_num_old %d\n", info_num, list_num_old);
    if ((0 == verinfo_tmp.sys_mainver_info[0].subver_num) && (0 == verinfo_tmp.sys_mainver_info[1].subver_num))
    {
        printf("** SYS will load default \n");
        sprintf(cmd, "setenv bootargs arlycon clk_ignore_unused root=/dev/mmcblk%sp%d rw rootwait rootfstype=ext4", argv[1], ROOTFS_A_PART);
        printf("** %s \n", cmd);
        run_command(cmd, 0);
        //更新控制字
        verinfo_tmp.magic = VMP_VER_CTRL_INFO_MAGIC;
        verinfo_tmp.list_num = list_num_old;
        verinfo_tmp.sys_mainver_info[0].is_use = 0;
        verinfo_tmp.sys_mainver_info[1].is_use = 0;
        verinfo_tmp.sys_mainver_info[0].is_update = 0;
        verinfo_tmp.sys_mainver_info[1].is_update = 0;
        verinfo_tmp.sys_mainver_info[0].is_use = 1;
        printf("do_zynqmp_load info_num %d verinfo_store %p \n", info_num, verinfo_store);

        //将信息放到写地址上去
        memcpy((char *)((struct vmp_verinfo_store *)DDR_ADDR + info_num), (char *)&verinfo_tmp, sizeof(struct vmp_verinfo_store));
        sprintf(cmd, "mmc write %x %s %x", DDR_ADDR, argv[2], block_num);
        //printf("cmd %s \n", cmd);
        run_command(cmd, 0);
        //执行默认引导 image_A + rootfs_a
        sprintf(cmd, "mmcinfo && fatload mmc %s:%d %x %s", argv[1], IMAGE_A_PART, DDR_ADDR, argv[3]);
        printf("** %s \n", cmd);
        run_command(cmd, 0);
    }
    else
    {
        zynqmp_get_bootcmd(&load_index, &verinfo_tmp, &set_verflag);
        printf("** SYS will load %d \n", load_index);
        if (1 == set_verflag)
        {

            //更新控制字
            printf("sys will update version index %d \n", load_index);
            verinfo_tmp.magic = VMP_VER_CTRL_INFO_MAGIC;
            verinfo_tmp.list_num = list_num_old + 1;
            verinfo_tmp.sys_mainver_info[0].is_use = 0;
            verinfo_tmp.sys_mainver_info[1].is_use = 0;
            verinfo_tmp.sys_mainver_info[0].is_update = 0;
            verinfo_tmp.sys_mainver_info[1].is_update = 0;
            verinfo_tmp.sys_mainver_info[load_index].is_use = 1;

            printf("do_zynqmp_load info_num %d verinfo_store %p \n", info_num, verinfo_store);
            memcpy((char *)((struct vmp_verinfo_store *)DDR_ADDR + info_num), (char *)&verinfo_tmp, sizeof(struct vmp_verinfo_store));

            sprintf(cmd, "mmc write %x %s %x", DDR_ADDR, argv[2], block_num);
            printf("** %s \n", cmd);
            run_command(cmd, 0);
        }
        if (0 == load_index)
        {
            sprintf(cmd, "setenv bootargs arlycon clk_ignore_unused root=/dev/mmcblk%sp%d rw rootwait rootfstype=ext4", argv[1], ROOTFS_A_PART);
            printf("** %s \n", cmd);
            run_command(cmd, 0);
            sprintf(cmd, "mmcinfo && fatload mmc %s:%d %x %s", argv[1], IMAGE_A_PART, DDR_ADDR, argv[3]);
        }
        else
        {
            sprintf(cmd, "setenv bootargs arlycon clk_ignore_unused root=/dev/mmcblk%sp%d rw rootwait rootfstype=ext4", argv[1], ROOTFS_B_PART);
            printf("** %s \n", cmd);
            run_command(cmd, 0);
            sprintf(cmd, "mmcinfo && fatload mmc %s:%d %x %s", argv[1], IMAGE_B_PART, DDR_ADDR, argv[3]);
        }
        printf("** %s \n", cmd);
        run_command(cmd, 0);
    }

    printf("do_zynqmp_load out \n");
    return 0;
}

U_BOOT_CMD(
    zynqmp_load, 4, 0, do_zynqmp_load,
    "zynqmp_load function\n",
    "\n"
    "    sdbootdev ver_infoaddr(emmc) kernel_name\n");