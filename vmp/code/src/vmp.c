/**
 * @file vmp.c
 * @author zhao.wei (hw)
 * @brief vmp主程序
 * @version 0.1
 * @date 2019-12-17
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "pub.h"
//#include "vmp.conf"

static u8 tmp_buf[VMP_TRANS_FILEDATA_LEN] = {0}; /**定义一个512字节的临时buf 给其他模块使用*/

zlog_category_t *zc = NULL; /**日志描述符*/

#ifdef VMP_MANAGE_APP_EN
static u8 help[] =
    "\
        \r\n Usage   : \
        \r\n    vmp [options] <type> {[trans] [url] --netmsg=...}\
        \r\n    vmp [options] <type> {--number=<x>}\
        \r\n options\
        \r\n    -h,--help                          get app help\
        \r\n    -g,--get      <type>               get version info <boot | sys | app>\
        \r\n        vmp [options] <type> {--number=<x>}\
        \r\n    -o,--ota      <type>               ota version <boot | sys | app>\
        \r\n    -s,--set      <type>               set version info <boot | sys | app>\
        \r\n    -u,--update   <type>               update version <boot | sys | app>\
        \r\n        -s,-u     <type> [trans] <url> --netmsg=...\
        \r\n    -a,--active   <type>               active version <sys | app>\
        \r\n    -d,--deactive <type>               deactive version <sys | app>\
        \r\n    -l,--load     [mount dir]          load version just for app\
        \r\n type\
        \r\n    <boot | sys | app>\
        \r\n trans\
        \r\n    -F,--Ftp\
        \r\n         --netmsg=<filename>:<username>:<passwd>\
        \r\n    -H,--Http\
        \r\n         --netmsg=<cpunum>:<otaid>:<otasecret>\
        \r\n    eg-->:vmp -u boot -F ftp://192.168.5.196:21 --netmsg=version.dat:ab:ab\
        \r\n          vmp -u app -H http://192.168.130.33:8181/ota --netmsg=1:ab1mms:4ityv66uhak\
        ";
#else //for userImage
static u8 help[] =
    "\
        \r\n Usage   : \
        \r\n    vmp [options] <type> {[trans] [url] --netmsg=...}\
        \r\n    vmp [options] <type> {--number=<x>}\
        \r\n options\
        \r\n    -h,--help                          get app help\
        \r\n    -g,--get      <type>               get version info <boot | sys>\
        \r\n        vmp [options] <type> {--number=<x>}\
        \r\n    -o,--ota      <type>               ota version <boot | sys>\
        \r\n    -s,--set      <type>               set version info <boot | sys>\
        \r\n    -u,--update   <type>               update version <boot | sys>\
        \r\n        -s,-u     <type> [trans] <url> --netmsg=...\
        \r\n    -a,--active   <type>               active version <sys>\
        \r\n    -d,--deactive <type>               deactive version <sys>\
        \r\n type\
        \r\n    <boot | sys>\
        \r\n trans\
        \r\n    -F,--Ftp\
        \r\n         --netmsg=<filename>:<username>:<passwd>\
        \r\n    -H,--Http\
        \r\n         --netmsg=<cpunum>:<otaid>:<otasecret>\
        \r\n    eg-->:vmp -u boot -F ftp://192.168.5.196:21 --netmsg=version.dat:ab:ab\
        \r\n          vmp -u app -H http://192.168.130.33:8181/ota --netmsg=1:ab1mms:4ityv66uhak\
        ";
#endif

static u8 exit_prese_msg[] =
    "\
    \r\n Usage   : \
    \r\n    vmp [options] <type> {[trans] [version_name] --netmsg=<ip>:{port}:{username}:{pawsswd}:{path}}\
    \r\n    vmp [options] <type> {--number=<x>}\
    \r\nTry `vmp -h,--help' for more information.\
    ";

static void
print_usage(const u8 *version)
{
    printf("%s %s \r\n", version, help);
    exit(1);
}

void exit_usage(u8 leavel)
{
    switch (leavel)
    {
    case 0:
        printf("%s \r\n", exit_prese_msg);
        break;
    case 1:
        printf("%s \r\n", "version data format error please check .");
        zlog_error(zc, "%s \r\n", "version data format error please check .");
        break;
    default:
        printf("%s \r\n", exit_prese_msg);
        break;
    }

    //日志系统资源释放
    zlog_fini();
    exit(1);
}

static struct option long_options[] =
    {
        {"help", no_argument, NULL, 'h'},
        {"update", required_argument, NULL, 'u'},
        {"get", required_argument, NULL, 'g'},
        {"set", required_argument, NULL, 's'},
        {"active", required_argument, NULL, 'a'},
        {"deactive", required_argument, NULL, 'd'},
        {"Ftp", required_argument, NULL, 'F'},
        {"Http", required_argument, NULL, 'H'},
        {"netmsg", optional_argument, NULL, 'n'},
        {"number", optional_argument, NULL, 'N'},
        {"load", required_argument, NULL, 'l'},
        {"ota", required_argument, NULL, 'o'},
        {NULL, 0, NULL, 0},
};

/**
 * @brief 解析cmd的版本类型
 * 
 * @param data 
 * @param get_data 
 * @return int 
 */
int cmd_parse_vertype(u8 *data, struct vmp_get_data *get_data)
{
    zlog_debug(zc, " data is %s ", data);
    if (NULL == data)
        return ERR_VMP_NULL_POINTER;
    if (0 == strcmp("boot", (u8 *)data))
    {
        get_data->ver_type = VMP_VER_TYPE_BOOT;
    }
    else if (0 == strcmp("sys", (u8 *)data))
    {
        get_data->ver_type = VMP_VER_TYPE_SYS;
    }
#ifdef VMP_MANAGE_APP_EN
    else if (0 == strcmp("app", (u8 *)data))
    {
        get_data->ver_type = VMP_VER_TYPE_APP;
    }
#endif
    else
    {
        exit_usage(0);
    }
    return SUCCESS;
}

/**
 * @brief 版本激活
 * 
 * @param get_data 输入的描述信息
 * @return int 
 */
int vmp_active_ver(struct vmp_get_data *get_data)
{
    vmp_active_ver_deal(get_data->ver_type);
    return SUCCESS;
}

/**
 * @brief 版本的反激活
 * 
 * @param get_data 输入的描述信息
 * @return int 
 */
int vmp_deactive_ver(struct vmp_get_data *get_data)
{
    vmp_deactive_ver_deal(get_data->ver_type);
    return SUCCESS;
}

/**
 * @brief 获取版本的描述信息
 * 
 * @param get_data 输入的描述信息
 * @return int 
 */
int vmp_get_ver(struct vmp_get_data *get_data)
{
    u16 num = 0;
    sscanf(get_data->data, "%d", &num);
    if (num > VMP_VER_INFO_MAX_NUM)
        num = VMP_VER_INFO_MAX_NUM;

    num = num > 1 ? num : 1; //num最小为1条
    zlog_debug(zc, " num is %x ", num);
    vmp_get_ver_deal(get_data->ver_type, num);

    return SUCCESS;
}

/**
 * @brief ota升级接口
 * 
 * @return int 
 */
int vmp_ota_ver(struct vmp_get_data *get_data)
{
    u32 ret;
    struct vmp_mainver_info mainver_info = {0};

    ret = vmp_parse_getdata(get_data, &mainver_info);
    if (ret)
        return ret;

    zlog_debug(zc, "--%s ", mainver_info.mainver_version);
    //zlog_debug(zc,"--%s ", mainver_info.mainver_name);
    zlog_debug(zc, "--%x ", mainver_info.subver_num);
    zlog_debug(zc, "--%s ", mainver_info.subver_info[0].subver_name);
    zlog_debug(zc, "--%s ", mainver_info.subver_info[0].subver_version);
    zlog_debug(zc, "--%s ", mainver_info.subver_info[0].md5);
    zlog_debug(zc, "--%d ", mainver_info.subver_info[0].size);

    vmp_ota_ver_deal(get_data->ver_type, get_data->trans_type, &mainver_info);

    return SUCCESS;
}

/**
 * @brief 设置版本的描述信息
 * 
 * @param get_data 输入的描述信息
 * @return int 
 */
int vmp_set_ver(struct vmp_get_data *get_data)
{
    u32 ret;
    struct vmp_mainver_info mainver_info = {0};

    ret = vmp_parse_getdata(get_data, &mainver_info);
    if (ret)
        return ret;

    zlog_debug(zc, "--%s ", mainver_info.mainver_version);
    //zlog_debug(zc,"--%s ", mainver_info.mainver_name);
    zlog_debug(zc, "--%x ", mainver_info.subver_num);
    zlog_debug(zc, "--%s ", mainver_info.subver_info[0].subver_name);
    zlog_debug(zc, "--%s ", mainver_info.subver_info[0].subver_version);
    zlog_debug(zc, "--%s ", mainver_info.subver_info[0].md5);
    zlog_debug(zc, "--%d ", mainver_info.subver_info[0].size);

    vmp_set_ver_deal(get_data->ver_type, &mainver_info);
    return SUCCESS;
}

/**
 * @brief 更新版本
 * 
 * @param get_data 输入的描述信息
 * @return int 
 */
int vmp_update_ver(struct vmp_get_data *get_data)
{
    u32 ret;
    struct vmp_mainver_info mainver_info = {0};
    ret = vmp_parse_getdata(get_data, &mainver_info);
    if (ret)
        return ret;

    zlog_debug(zc, "--%s ", mainver_info.mainver_version);
    //zlog_debug(zc,"--%s \r\n", mainver_info.mainver_name);
    zlog_debug(zc, "--%x ", mainver_info.subver_num);
    zlog_debug(zc, "--%s ", mainver_info.subver_info[0].subver_name);
    zlog_debug(zc, "--%s ", mainver_info.subver_info[0].subver_version);
    zlog_debug(zc, "--%s ", mainver_info.subver_info[0].md5);
    zlog_debug(zc, "--%d ", mainver_info.subver_info[0].size);
    zlog_debug(zc, "--%s ", mainver_info.subver_info[1].subver_name);
    zlog_debug(zc, "--%s ", mainver_info.subver_info[1].subver_version);
    zlog_debug(zc, "--%s ", mainver_info.subver_info[1].md5);
    zlog_debug(zc, "--%u ", mainver_info.subver_info[1].size);
    /**根据版本类型进一步处理 */
    vmp_update_ver_deal(get_data->ver_type, get_data->trans_type, &mainver_info);

    return SUCCESS;
}

/**
 * @brief load 版本加载接口
 * 
 * @return int 
 */
int vmp_load_ver(struct vmp_get_data *get_data)
{
    u8 mount_dir[VMP_DEFINE_NAME_LEN] = {0};

    memcpy(mount_dir, (u8 *)get_data->data, strlen(get_data->data));
    zlog_debug(zc, "mount_dir %s ", mount_dir);
    vmp_load_ver_deal(mount_dir);

    return SUCCESS;
}

/**
 * @brief 主函数
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    u32 opt;
    u32 rc;
    u32 option_index = 0;
    u8 *string = "";
    struct vmp_get_data *get_data = NULL;

    get_data = (struct vmp_get_data *)tmp_buf; //指向临时buf

    rc = zlog_init("/usp/usp_log.conf");
    if (rc)
    {
        printf("init failed\n");
        return -1;
    }

    zc = zlog_get_category("vmp");
    if (!zc)
    {
        printf("get cat fail\n");
        zlog_fini();
        return -2;
    }

    //zlog_info(zc, "info Separator========================");
    while ((opt = getopt_long_only(argc, argv, string, long_options, &option_index)) != -1)
    {
        //printf("opt = %c\t\t", opt);
        //printf("optarg = %s\t\t", optarg);
        //printf("optind = %d\t\t", optind);
        //printf("argv[optind] =%s\t\t", argv[optind]);
        //printf("option_index = %d\n", option_index);
        switch (opt)
        {
        case 'u':
        case 'g':
        case 's':
        case 'a':
        case 'd':
        case 'h':
        case 'o':
            get_data->opt_type = opt;
            cmd_parse_vertype(optarg, get_data);
            break;
        case 'F':
        case 'H':
            get_data->trans_type = opt;
            if (strlen(optarg) > sizeof(get_data->url))
                exit_usage(0);
            memcpy(get_data->url, optarg, strlen(optarg)); //可以加一个长度的校验
            break;
        case 'n':
        case 'N':
            if (NULL == optarg)
                exit_usage(0);
            memcpy(get_data->data, optarg, strlen(optarg)); //可以加一个长度的校验
            break;
        case 'l':
            get_data->opt_type = opt;
            if (NULL == optarg)
                exit_usage(0);
            memcpy(get_data->data, optarg, strlen(optarg)); //可以加一个长度的校验
            break;
        default:
            exit_usage(0);
            break;
        }
    }
    switch (get_data->opt_type)
    {
    case 'h':
        print_usage(VMP_VER_DEV_TYPE);
        break;
    case 'u':
        vmp_update_ver(get_data);
        break;
    case 'g':
        vmp_get_ver(get_data);
        break;
    case 's':
        vmp_set_ver(get_data);
        break;
    case 'a':
        vmp_active_ver(get_data);
        break;
    case 'd':
        vmp_deactive_ver(get_data);
        break;
    case 'l':
        vmp_load_ver(get_data);
        break;
    case 'o':
        vmp_ota_ver(get_data);
        break;
    default:
        exit_usage(0);
        break;
    }
    zlog_fini();
    return 0;
}