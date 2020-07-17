/**
 * @file ftp.c
 * @author zhao.wei (hw)
 * @brief ftp协议实现支撑函数,基于libcurl进行
 * @version 0.1
 * @date 2019-12-20
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include "pub.h"

static int progress_func(void *ptr, double TotalToDownload, double NowDownloaded,
                         double TotalToUpload, double NowUploaded)
{
    int totaldotz = 40;
    double fractiondownloaded = NowDownloaded / TotalToDownload;
    static int times = 0;
    int dotz = round(fractiondownloaded * totaldotz);

    int ii = 0;
    if (0 == times % 1000 || (NowDownloaded == TotalToDownload)) //减少一下刷新率
    {
        printf("%3.0f%% [", fractiondownloaded * 100);

        for (; ii < dotz; ii++)
        {
            printf("=");
        }

        for (; ii < totaldotz; ii++)
        {
            printf(" ");
        }

        printf("]\r");
        fflush(stdout);
    }
    times++;
    return 0;
}

/* parse headers for Content-Length */
static size_t get_content_length_func(void *ptr, size_t size, size_t nmemb, void *stream)
{
    int r;
    long len = 0;
    /* _snscanf() is Win32 specific */
    //r = _snscanf(ptr, size * nmemb, "Content-Length: %ld\n", &len);
    r = sscanf((const char *)ptr, "Content-Length: %ld\n", &len);
    if (r) /* Microsoft: we don't read the specs */
        *((long *)stream) = len;
    return size * nmemb;
}

//write data to upload
static size_t ftp_write_file(void *ptr, size_t size, size_t nmemb, void *stream)
{
    return fwrite(ptr, size, nmemb, (FILE *)stream);
}

/**
 * @brief ftp下载大文件，支持断点下载
 * 
 * @param remote_path ftp地址
 * @param user 用户名
 * @param passwd 密码
 * @param localpath 本地地址
 * @param timeout 超时时间
 * @param tries 重试次数
 * @return int 
 */
static int ftp_download_file(const char *remote_path, const char *user, const char *passwd,
                             const char *localpath, long timeout, long tries)
{
    FILE *file;
    curl_off_t local_file_len = -1;
    long filesize = 0;
    struct stat file_info;
    int use_resume = 0;
    CURL *curldwn = NULL;
    CURLcode ret = CURLE_GOT_NOTHING;
    char user_key[256] = {0};
    sprintf(user_key, "%s:%s", user, passwd);

    //获取本地文件大小信息
    if (stat(localpath, &file_info) == 0)
    {
        local_file_len = file_info.st_size;
        use_resume = 1;
    }
    //追加方式打开文件，实现断点续传
    file = fopen(localpath, "ab+");
    if (file == NULL)
    {
        zlog_debug(zc, "file open filed .");
        return ERR_FTP_OPEN_FILE;
    }
    ret = curl_global_init(CURL_GLOBAL_ALL);
    curldwn = curl_easy_init();

    curl_easy_setopt(curldwn, CURLOPT_URL, remote_path);
    curl_easy_setopt(curldwn, CURLOPT_USERPWD, user_key);
    //连接超时设置
    curl_easy_setopt(curldwn, CURLOPT_CONNECTTIMEOUT, timeout);
    //设置头处理函数
    curl_easy_setopt(curldwn, CURLOPT_HEADERFUNCTION, get_content_length_func);
    curl_easy_setopt(curldwn, CURLOPT_HEADERDATA, &filesize);
    // 设置断点续传
    curl_easy_setopt(curldwn, CURLOPT_RESUME_FROM_LARGE, use_resume ? local_file_len : 0);
    curl_easy_setopt(curldwn, CURLOPT_WRITEFUNCTION, ftp_write_file);
    curl_easy_setopt(curldwn, CURLOPT_WRITEDATA, file);

    curl_easy_setopt(curldwn, CURLOPT_PROGRESSFUNCTION, progress_func);
    curl_easy_setopt(curldwn, CURLOPT_PROGRESSDATA, NULL);
    curl_easy_setopt(curldwn, CURLOPT_NOPROGRESS, 0L);
    //curl_easy_setopt(curldwn, CURLOPT_VERBOSE, 1L);
    ret = curl_easy_perform(curldwn);
    fclose(file);
    if (ret == CURLE_OK)
        return SUCCESS;
    else
    {
        fprintf(stderr, "%s\n", curl_easy_strerror(ret));
        return ERR_FTP_TRANS_ERROR;
    }
}

/**
 * @brief 
 * 
 * @param ptr 
 * @param size 
 * @param nmemb 
 * @param userdata 
 * @return size_t 
 */
static u32 offset = 0;
static size_t ftp_rcv_data(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    u32 len;

    u8 *p = userdata;
    len = size * nmemb;
    zlog_debug(zc, "in ");
    zlog_debug(zc, "len is %d ", len);
    //zlog_debug(zc,"ptr is %s ", ptr);
    memcpy(p + offset, ptr, len);
    offset += len;
    return len;
}
/**
 * @brief ftp下载版本描述文件
 * 
 * @param remote_path ftp地址
 * @param user 用户名
 * @param passwd 密码
 * @param rcv_data 接受数据缓冲区
 * @return int 
 */
static int ftp_download_data(const char *remote_path, const char *user, const char *passwd,
                             const char *rcv_data)
{
    CURL *curl;
    CURLcode ret;
    char user_key[256] = {0};
    sprintf(user_key, "%s:%s", user, passwd);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, remote_path);

        curl_easy_setopt(curl, CURLOPT_USERPWD, user_key);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ftp_rcv_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (char *)rcv_data); //给相关函数的第四个参数传递一个结构体的指针
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);                   //CURLOPT_VERBOSE 这个选项很常用用来在屏幕上显示对服务器相关操作返回的信息

        ret = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (CURLE_OK != ret)
        {
            fprintf(stderr, "curl told us %d\n", ret);
            return ERR_FTP_TRANS_ERROR;
        }
    }
    curl_global_cleanup();

    return 0;
}

/**
 * @brief 解析版本描述文件
 * 
 * @param mainver_def conf中定义的版本描述文件
 * @param version_data 传入的待解析文件json格式
 * @param verinfo_store 临时存储版本信息的数据结构
 * 
 * @return int 解析结果0 为成功
 */
static int ftp_parse_version_description(u8 *version_data, struct vmm_mainver_info *mainver_info)
{
    cJSON *head = NULL;
    cJSON *main_ver = NULL;
    cJSON *sub_file = NULL;
    cJSON *sub_obj = NULL;
    cJSON *sub_name = NULL;
    cJSON *sub_ver = NULL;
    cJSON *sub_size = NULL;
    cJSON *sub_md5 = NULL;
    u32 size = 0, i = 0;

    head = cJSON_Parse(version_data); //获取整个大的句柄
    if (NULL == head)
    {
        return ERR_FTP_PARSE_JSON;
    }
    zlog_debug(zc, "head get success ");
    main_ver = cJSON_GetObjectItem(head, "mainVersion");
    if (main_ver->valuestring != NULL)
    {
        zlog_debug(zc, "MainVersion %s ", main_ver->valuestring);
    }
    memcpy(mainver_info->mainver_version, main_ver->valuestring, strlen(main_ver->valuestring));
    sub_file = cJSON_GetObjectItem(head, "subFiles");
    size = cJSON_GetArraySize(sub_file);
    zlog_debug(zc, "sub ver num  %d ", size);
    //加一个校验
    if (size > VMM_MAX_SUB_VER_NUM)
    {
        return ERR_FTP_SUBVER_NUM;
    }
    mainver_info->subver_num = size;

    for (i = 0; i < size; i++)
    {
        sub_obj = cJSON_GetArrayItem(sub_file, i);
        sub_name = cJSON_GetObjectItem(sub_obj, "subName");
        if (sub_name->valuestring != NULL)
        {
            zlog_debug(zc, "sub_name %s ", sub_name->valuestring);
        }
        memcpy(mainver_info->subver_info[i].subver_name, sub_name->valuestring, strlen(sub_name->valuestring));

        sub_ver = cJSON_GetObjectItem(sub_obj, "subVersion");
        if (sub_ver->valuestring != NULL)
        {
            zlog_debug(zc, "sub_ver %s ", sub_ver->valuestring);
        }
        memcpy(mainver_info->subver_info[i].subver_version, sub_ver->valuestring, strlen(sub_ver->valuestring));

        sub_size = cJSON_GetObjectItem(sub_obj, "subSize");
        if (sub_size->valuestring != NULL)
        {
            zlog_debug(zc, "sub_size %s ", sub_size->valuestring);
        }
        mainver_info->subver_info[i].size = atoi(sub_size->valuestring);

        sub_md5 = cJSON_GetObjectItem(sub_obj, "subMd5");
        if (sub_md5->valuestring != NULL)
        {
            zlog_debug(zc, "sub_md5 %s ", sub_md5->valuestring);
        }
        memcpy(mainver_info->subver_info[i].md5, sub_md5->valuestring, strlen(sub_md5->valuestring));
    }

    return SUCCESS;
}

/**
 * @brief ftp获取版本描述信息
 * 
 * @param ftp_url ftp地址
 * @param ftp_data ftp的数据信息
 * @param mainver_info 版本描述信息
 * @return int 
 */
/**用来临时存储服务器信息*/
static u8 ftp_server_data[1024];
static u8 ftp_usr[1024];
static u8 ftp_passwd[1024];
int vmm_ftp_parse_verdata(struct vmm_get_data *get_data, struct vmm_mainver_info *mainver_info)
{
    u32 ret = SUCCESS;
    u8 filename[1024], user[1024], passwd[1024], url[1024];
    u8 *ftp_url = NULL;
    u8 *ftp_data = NULL;
    ftp_url = get_data->url;
    ftp_data = get_data->data;

    zlog_debug(zc, "ftp_url is %s ", ftp_url);

    sscanf(ftp_data, "%[^:]%*c%[^:]%*c%[^:] ", &filename, &user, &passwd);
    zlog_debug(zc, "filename %s user is %s passwd is %s ", filename, user, passwd);

    memcpy(ftp_server_data, ftp_url, strlen(ftp_url)); //暂存ftp服务信息
    memcpy(ftp_usr, user, strlen(user));               //暂存ftp服务信息
    memcpy(ftp_passwd, passwd, strlen(passwd));        //暂存ftp服务信息

    sprintf(url, "%s/%s", ftp_url, filename); //构造完整路径
    zlog_debug(zc, "url is %s ", url);
    ret = ftp_download_data(url, user, passwd, ga_trans_data);
    if (ret)
        return ret;
    zlog_debug(zc,"rcv_data %s ", ga_trans_data);
    ret = ftp_parse_version_description(ga_trans_data, mainver_info);
    if (ret)
        return ret;
    return SUCCESS;
}

/**
 * @brief ftp下载版本大文件
 * 
 * @param version_file 远端文件名
 * @param version_locat 本地存储扇区名
 * @param version_md5 md5校验值
 * @param version_size 版本大小
 * @return int 
 */
#define MD5_BUF_SIZE (10 * 1024 * 1024)
int vmm_ftp_version_file(u8 sub_index, u8 *version_file, u8 *version_locat, u8 *version_md5, u64 version_size)
{
    u32 ret = SUCCESS;
    u8 url[1024];
    u8 md5_value[VMM_DEFINE_NAME_LEN] = {0};
    u8 md5_cmd[VMM_DEFINE_NAME_LEN] = {0};
    FILE *fp;

    sprintf(url, "%s/%s", ftp_server_data, version_file); //构造完整路径
    zlog_debug(zc, "url is %s ", url);
    ret = ftp_download_file(url, ftp_usr, ftp_passwd, version_locat, 5, 3);
    if (ret)
        return ret;
    printf("\r\ndownload success \r\n");

    zlog_debug(zc, "start pop md5");
    sprintf(md5_cmd, "md5sum %s", version_locat);
    zlog_debug(zc, "md5_cmd %s", md5_cmd);
    fp = popen(md5_cmd, "r");
    if (fp != NULL)
    {
        fgets(md5_value, sizeof(md5_value), fp);
        zlog_debug(zc, "pop calc md5 %s ", md5_value);
        pclose(fp);
    }
    zlog_debug(zc, "end pop md5");
    zlog_debug(zc, "calc md5 %s ", md5_value);
    zlog_debug(zc, "input md5 %s ", version_md5);
    if (strncmp(md5_value, version_md5, 32))
    {
        printf("md5 check error .");
        ret = ERR_FTP_FILE_MD5;
    }

    return ret;
}