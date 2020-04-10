/**
 * @file http.c
 * @author zhao.wei (hw)
 * @brief http下载所需的接口进行封装，基于libcurl和cjson进行
 * @version 0.1
 * @date 2020-03-06
 * 
 * @copyright Copyright (c) 2020
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

/* 从http头部获取文件size*/
static size_t get_content_length_func(void *ptr, size_t size, size_t nmemb, void *stream)
{
    int r;
    long len = 0;

    /* _snscanf() is Win32 specific */
    // r = _snscanf(ptr, size * nmemb, "Content-Length: %ld\n", &len);
    r = sscanf(ptr, "Content-Length: %ld\n", &len);
    if (r) /* Microsoft: we don't read the specs */
        *((long *)stream) = len;

    return size * nmemb;
}

/* 保存下载文件 */
static size_t http_write_file(void *ptr, size_t size, size_t nmemb, void *stream)
{
    //zlog_debug(zc,"LEN %d ", size*nmemb);
    return fwrite(ptr, size, nmemb, stream);
}

/**
 * @brief 构造http发送的json串
 * 
 * @param id otaid
 * @param secret otasecret
 * @param num cpunum
 * @param version_type 版本类型
 * @return u8* 
 */
u8 *http_creat_json(char *id, char *secret, u8 num, u8 version_type)
{
    u8 *string = NULL;

    cJSON *head = cJSON_CreateObject();
    if (head == NULL)
    {
        return NULL;
    }
    cJSON_AddStringToObject(head, "otaId", id);
    cJSON_AddStringToObject(head, "otaSecret", secret);
    cJSON_AddNumberToObject(head, "ducCpuNum", num);
    if (VMP_VER_TYPE_BOOT == version_type)
    {
        cJSON_AddStringToObject(head, "mainType", "boot");
    }
    else if (VMP_VER_TYPE_SYS == version_type)
    {
        cJSON_AddStringToObject(head, "mainType", "sys");
    }
#ifdef VMP_MANAGE_APP_EN
    else if (VMP_VER_TYPE_APP == version_type)
    {
        cJSON_AddStringToObject(head, "mainType", "app");
    }
#endif
    else
    {
    }
    string = cJSON_Print(head);
    if (string == NULL)
    {
        return NULL;
    }

    cJSON_Delete(head);
    return string;
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
static size_t http_rcv_data(char *ptr, size_t size, size_t nmemb, void *userdata)
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
 * @brief 调用libcurl post发送json数据
 * 
 * @param url url
 * @param send_data 发送的json的数据
 * @param rcv_data 接收的json数据
 * @return int 
 */
int http_post_data(u8 *url, u8 *send_data, u8 *rcv_data)
{
    CURL *curl_handle = NULL;
    CURLcode curl_res;
    curl_res = curl_global_init(CURL_GLOBAL_ALL);

    if (curl_res == CURLE_OK)
    {
        curl_handle = curl_easy_init();
        if (curl_handle != NULL)
        {
            curl_easy_setopt(curl_handle, CURLOPT_URL, url);
            curl_easy_setopt(curl_handle, CURLOPT_POST, 1);
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, strlen(send_data));
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, (char *)send_data);
            curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
            curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0);
            curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 30);
            curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 10L);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, http_rcv_data);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (char *)rcv_data);
            curl_easy_setopt(curl_handle, CURLOPT_HEADER, 0L);

            struct curl_slist *pList = NULL;
            pList = curl_slist_append(pList, "Content-Type: application/json;charset=utf-8");

            curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, pList);
            curl_res = curl_easy_perform(curl_handle);
            if (curl_res != CURLE_OK)
            {
                printf("curl_easy_perform error, err_msg:[%ld]\n", curl_res);
                return ERR_HTTP_POST_DATA;
            }
            curl_easy_cleanup(curl_handle);
        }
    }
    else
    {
        printf("CURL ERROR : %s", curl_easy_strerror(curl_res));
        return ERR_HTTP_POST_DATA;
    }

    return SUCCESS;
}

/**
 * @brief http下载版本大文件
 * 
 * @param remotepath url
 * @param localpath 本地存储区
 * @param timeout 超时时间
 * @param tries 重试次数
 * @return int 
 */
int http_download_file(const char *remotepath, const char *localpath, long timeout, long tries)
{
    FILE *file;
    curl_off_t local_file_len = -1;
    long filesize = 0;
    CURLcode ret = CURLE_GOT_NOTHING;
    struct stat file_info;
    int use_resume = 0;
    CURL *curlhandle = NULL;

    curl_global_init(CURL_GLOBAL_ALL);
    curlhandle = curl_easy_init();
    /* 得到本地文件大小 */
    if (stat(localpath, &file_info) == 0)
    {
        local_file_len = file_info.st_size;
        use_resume = 1;
    }
    //采用追加方式打开文件，便于实现文件断点续传工作
    file = fopen(localpath, "ab+");
    if (file == NULL)
    {
        perror(NULL);
        return 0;
    }

    //curl_easy_setopt(curlhandle, CURLOPT_UPLOAD, 1L);

    curl_easy_setopt(curlhandle, CURLOPT_URL, remotepath);

    curl_easy_setopt(curlhandle, CURLOPT_CONNECTTIMEOUT, timeout); // 设置连接超时，单位秒
    //设置http 头部处理函数
    curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION, get_content_length_func);
    curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, &filesize);
    // 设置文件续传的位置给libcurl
    curl_easy_setopt(curlhandle, CURLOPT_RESUME_FROM_LARGE, use_resume ? local_file_len : 0);

    curl_easy_setopt(curlhandle, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, http_write_file);

    curl_easy_setopt(curlhandle, CURLOPT_PROGRESSFUNCTION, progress_func);
    curl_easy_setopt(curlhandle, CURLOPT_PROGRESSDATA, NULL);

    curl_easy_setopt(curlhandle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curlhandle, CURLOPT_SSL_VERIFYHOST, 0);  //当没有设置这个选项为0的时候，出现上述的错误。当然这个错误也是不一定会出现的，这取决于证书是否正确

    //curl_easy_setopt(curlhandle, CURLOPT_READFUNCTION, readfunc);
    //curl_easy_setopt(curlhandle, CURLOPT_READDATA, f);
    curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, 0L);
    //curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 1L);

    ret = curl_easy_perform(curlhandle);

    fclose(file);

    if (ret == CURLE_OK)
        return SUCCESS;
    else
    {
        fprintf(stderr, "%s\n", curl_easy_strerror(ret));
        return ERR_HTTP_DOWNLOAD_FILE;
    }
}

/**
 * @brief http解析版本描述文件
 * 
 * @param json_data 源json格式数据
 * @param mainver_info 版本描述文件
 * @return int 
 */
static u8 download_url[VMP_MAX_SUB_VER_NUM][1024] = {0}; /**用于存储下载版本的url*/
int http_prase_verdata(u8 *json_data, struct vmp_mainver_info *mainver_info)
{
    cJSON *head = NULL;
    cJSON *data = NULL;
    cJSON *main_ver = NULL;
    cJSON *sub_file = NULL;
    cJSON *sub_obj = NULL;
    cJSON *sub_name = NULL;
    cJSON *sub_ver = NULL;
    cJSON *sub_size = NULL;
    cJSON *sub_md5 = NULL;
    cJSON *sub_file_url = NULL;
    u32 size = 0, i = 0;

    head = cJSON_Parse(json_data); //获取整个大的句柄
    if (NULL == head)
    {
        return ERR_HTTP_PARSE_JSON;
    }
    //获取data节点
    data = cJSON_GetObjectItem(head, "data");
    if (NULL == data)
    {
        return ERR_HTTP_PARSE_JSON;
    }
    main_ver = cJSON_GetObjectItem(data, "mainVersion");
    if (main_ver->valuestring != NULL)
    {
        zlog_debug(zc,"appMainVersion %s ", main_ver->valuestring);
    }
    memcpy(mainver_info->mainver_version, main_ver->valuestring, strlen(main_ver->valuestring));
    sub_file = cJSON_GetObjectItem(data, "subFiles");
    size = cJSON_GetArraySize(sub_file);
    zlog_debug(zc,"sub ver num  %d ", size);
    //加一个校验
    if (size > VMP_MAX_SUB_VER_NUM)
    {
        return ERR_HTTP_SUB_NUM;
    }
    mainver_info->subver_num = size;
    for (i = 0; i < size; i++)
    {
        sub_obj = cJSON_GetArrayItem(sub_file, i);
        sub_name = cJSON_GetObjectItem(sub_obj, "subName");
        if (sub_name->valuestring != NULL)
        {
            zlog_debug(zc,"sub_name %s ", sub_name->valuestring);
        }
        memcpy(mainver_info->subver_info[i].subver_name, sub_name->valuestring, strlen(sub_name->valuestring));

        sub_ver = cJSON_GetObjectItem(sub_obj, "subVersion");
        if (sub_ver->valuestring != NULL)
        {
            zlog_debug(zc,"sub_ver %s ", sub_ver->valuestring);
        }
        memcpy(mainver_info->subver_info[i].subver_version, sub_ver->valuestring, strlen(sub_ver->valuestring));

        sub_size = cJSON_GetObjectItem(sub_obj, "subSize");
        if (sub_size->valuestring != NULL)
        {
            zlog_debug(zc,"sub_size %s ", sub_size->valuestring);
        }
        mainver_info->subver_info[i].size = atoi(sub_size->valuestring);

        sub_md5 = cJSON_GetObjectItem(sub_obj, "subMd5");
        if (sub_md5->valuestring != NULL)
        {
            zlog_debug(zc,"sub_md5 %s ", sub_md5->valuestring);
        }
        memcpy(mainver_info->subver_info[i].md5, sub_md5->valuestring, strlen(sub_md5->valuestring));

        sub_file_url = cJSON_GetObjectItem(sub_obj, "subFileUrl");
        if (sub_file_url->valuestring != NULL)
        {
            zlog_debug(zc,"sub_file_url %s ", sub_file_url->valuestring);
        }
        memcpy((u8 *)&download_url[i][0], sub_file_url->valuestring, strlen(sub_file_url->valuestring));
    }

    return SUCCESS;
}
/**
 * @brief http获取版本描述信息
 * 
 * @param http_url url
 *  * @param http_data 数据信息
 * @return int 
 */
int vmp_http_parse_verdata(struct vmp_get_data *get_data, struct vmp_mainver_info *mainver_info)
{
    u32 ret = SUCCESS;
    u8 ota_id[1024], ota_secret[1024];
    u8 cpu_num = 0;
    u8 *send_data = NULL;
    u8 *http_url = NULL;
    u8 *http_data = NULL;
    http_url = get_data->url;
    http_data = get_data->data;

    zlog_debug(zc,"http data is %s ", http_data);
    sscanf(http_data, "%d:%[^:]%*c%[^:] ", &cpu_num, &ota_id, &ota_secret);
    zlog_debug(zc,"ota_id is %s ota_secret is %s cpu_num %d ", ota_id, ota_secret, cpu_num);
    zlog_debug(zc,"http url is %s ", http_url);
    //ERR_HTTP_URL后续可以加一个url格式判别
    send_data = http_creat_json(ota_id, ota_secret, cpu_num, get_data->ver_type); //该内存区域要手动释放
    if (NULL == send_data)
    {
        return ERR_HTTP_SEND_JSON;
    }
    zlog_debug(zc,"send_data %s ", send_data);

    //发送数据并获取版本信息
    http_post_data(http_url, send_data, ga_trans_data);
    zlog_debug(zc,"rcv_data %s ", ga_trans_data);
    free(send_data);
    //解析数据
    ret = http_prase_verdata(ga_trans_data, mainver_info);
    return ret;
}

/**
 * @brief http下载版本大文件
 * 
 * @param version_file 远端文件名
 * @param version_locat 本地存储的扇区位置
 * @param version_md5 md5校验值
 * @param version_size 版本大小
 * @return int 
 */
int vmp_http_version_file(u8 sub_index, u8 *version_file, u8 *version_locat, u8 *version_md5, u64 version_size)
{
    u32 ret = SUCCESS;
    u8 md5_value[VMP_DEFINE_NAME_LEN] = {0};
    u8 md5_cmd[VMP_DEFINE_NAME_LEN] = {0};
    FILE *fp;

    zlog_debug(zc,"url is %s ", (u8 *)&download_url[sub_index][0]);

    ret = http_download_file((u8 *)&download_url[sub_index][0], version_locat, 5, 3);
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
        ret = ERR_HTTP_FILE_MD5;
    }

    return SUCCESS;
}