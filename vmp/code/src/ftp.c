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
    if (0 == times % 1000 || (NowDownloaded == TotalToDownload))//减少一下刷新率
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
int ftp_download_file(const char *remote_path, const char *user, const char *passwd,
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
        zlog_debug(zc,"file open filed .");
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
    zlog_debug(zc,"in ");
    zlog_debug(zc,"len is %d ", len);
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
int ftp_download_data(const char *remote_path, const char *user, const char *passwd,
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
 * @brief 分解字符串中的键值和数据
 * 
 * @param line 
 * @param key 
 * @param value 
 * @return u32 
 */
static u32 ftp_parse_keyvalue(char *line, char **key, char **value)
{
    //获取Key
    while (' ' == *line || '\t' == *line)
    {
        line++;
    }
    *key = line;
    while (0 != *line && ' ' != *line && '\t' != *line && '=' != *line)
    {
        line++;
    }
    if ('\0' == *line)
    {
        return ERR_VMP_STRING_FORMAT;
    }
    *line++ = '\0';

    //判断格式是否正确
    while (' ' == *line || '\t' == *line)
    {
        line++;
    }
    if ('=' != *line)
    {
        return ERR_VMP_STRING_FORMAT;
    }
    line++;

    //获取Value
    while (' ' == *line || '\t' == *line)
    {
        line++;
    }
    *value = line;
    while (0 != *line && ' ' != *line && '\t' != *line)
    {
        line++;
    }
    *line = '\0';

    return SUCCESS;
}

/**
 * @brief 解析版本描述文件
 * 
 * @param mainver_def conf中定义的版本描述文件
 * @param version_data 传入的待解析文件
 * @param verinfo_store 临时存储版本信息的数据结构
 * 
 * @return int 解析结果0 为成功
 */
static int ftp_parse_version_description(u8 *version_data, struct vmp_mainver_info *mainver_info)
{
    u8 line_buf[128];
    u8 *line = NULL;
    u8 *key = NULL;
    u8 iname = 0;
    u8 iversion = 0;
    u8 imd5 = 0;
    u8 isize = 0;
    u8 *value;

    if (NULL == version_data)
        return ERR_VMP_NULL_POINTER;

    while (0 != *version_data)
    {
        //获取行数据
        line = line_buf;
        while ('\0' != *version_data && '\r' != *version_data && '\n' != *version_data)
        {
            *line++ = *version_data++;
        }
        version_data++;
        *line = '\0';

        if ('\0' == line_buf[0] || '#' == line_buf[0])
        {
            continue;
        }
        //分解键值和数据
        if (SUCCESS != ftp_parse_keyvalue(line_buf, &key, &value))
        {
            continue;
        }
        //判断数据是否超长
        if (VMP_DEFINE_NAME_LEN <= strlen((char *)value))
        {
            return ERR_VMP_VERSION_INFO_STRING;
        }
        zlog_debug(zc,"key = '%s', value = '%s' ", key, value);
        if (NULL != strstr(key, "mainVersion"))
            memcpy(mainver_info->mainver_version, value, strlen(value));
        //if (NULL != strstr(key, "mainName"))
        //memcpy(mainver_info->mainver_name, value, strlen(value));

        if (NULL != strstr(key, "sub") && NULL != strstr(key, "Name"))
            memcpy(mainver_info->subver_info[iname++].subver_name, value, strlen(value));

        if (NULL != strstr(key, "sub") && NULL != strstr(key, "Version"))
            memcpy(mainver_info->subver_info[iversion++].subver_version, value, strlen(value));

        if (NULL != strstr(key, "sub") && NULL != strstr(key, "Md5"))
            memcpy(mainver_info->subver_info[imd5++].md5, value, strlen(value));

        if (NULL != strstr(key, "sub") && NULL != strstr(key, "Size"))
            mainver_info->subver_info[isize++].size = atoi(value);
    }
    if (imd5 == iversion && iversion == iname)
    {
        mainver_info->subver_num = iname;
        return SUCCESS;
    }
    else
        return ERR_FTP_SUBVER_NUM;
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
int vmp_ftp_parse_verdata(struct vmp_get_data *get_data, struct vmp_mainver_info *mainver_info)
{
    u32 ret = SUCCESS;
    u8 filename[1024], user[1024], passwd[1024], url[1024];
    u8 *ftp_url = NULL;
    u8 *ftp_data = NULL;
    ftp_url = get_data->url;
    ftp_data = get_data->data;

    zlog_debug(zc,"ftp_url is %s ", ftp_url);

    sscanf(ftp_data, "%[^:]%*c%[^:]%*c%[^:] ", &filename, &user, &passwd);
    zlog_debug(zc,"filename %s user is %s passwd is %s ", filename, user, passwd);

    memcpy(ftp_server_data, ftp_url, strlen(ftp_url)); //暂存ftp服务信息
    memcpy(ftp_usr, user, strlen(user));               //暂存ftp服务信息
    memcpy(ftp_passwd, passwd, strlen(passwd));        //暂存ftp服务信息

    sprintf(url, "%s/%s", ftp_url, filename); //构造完整路径
    zlog_debug(zc,"url is %s ", url);
    ret = ftp_download_data(url, user, passwd, ga_trans_data);
    if (ret)
        return ret;
    //zlog_debug(zc,"rcv_data %s ", ga_trans_data);
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
int vmp_ftp_version_file(u8 sub_index, u8 *version_file, u8 *version_locat, u8 *version_md5, u32 version_size)
{
    u32 ret = SUCCESS;
    u8 url[1024];
    MD5_CTX md5;
    s32 locatfd = -1;
    u32 read_len = 0;
    u32 read_offset = 0, i;
    u8 md5_value[VMP_DEFINE_NAME_LEN] = {0};
    u8 md5_str[16] = {0};

    sprintf(url, "%s/%s", ftp_server_data, version_file); //构造完整路径
    zlog_debug(zc,"url is %s ", url);

    ret = ftp_download_file(url, ftp_usr, ftp_passwd, version_locat, 1, 3);
    if (ret)
        return ret;
    printf("\r\ndownload success \r\n");

    MD5Init(&md5);
    locatfd = mmc_open(version_locat);
    while (0 < version_size)
    {

        read_len = mmc_read(locatfd, read_offset, sizeof(ga_trans_data), ga_trans_data);

        MD5Update(&md5, ga_trans_data, read_len);
        version_size -= read_len;
        read_offset += read_len;
    }
    mmc_close(locatfd);
    MD5Final(&md5, md5_str);
    for (i = 0; i < 16; i++)
    {
        snprintf(md5_value + i * 2, 2 + 1, "%02x", md5_str[i]);
    }
    zlog_debug(zc,"calc md5 %s ", md5_value);
    zlog_debug(zc,"input md5 %s ", version_md5);

    if (strcmp(md5_value, version_md5))
    {
        printf("md5 check error .");
        ret = ERR_FTP_FILE_MD5;
    }

    return ret;
}