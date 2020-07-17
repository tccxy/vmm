/**
 * @file error.h
 * @author zhao.wei (hw)
 * @brief 错误码定义
 * @version 0.1
 * @date 2019-12-17
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#define SUCCESS 0 /**成功*/
#define ERROR 1   /**错误*/
#define TRUE (1)
#define FALSE (0)

#define ERR_MMC_START (0xFF000100)  /**mmc模块 错误码基值*/
#define ERR_HTTP_START (0xFF000300) /**HTTP模块 错误码基值*/
#define ERR_FTP_START (0xFF000400)  /**FTP模块 错误码基值*/
#define ERR_VMM_START (0xFF001000)  /**vmm模块 错误码基值*/

/***********************************************************
*                          MMC模块错误码                   *
***********************************************************/
#define ERR_MMC_NULL_POINTER (ERR_MMC_START + 1) /**mmc设备空指针其他错误*/
#define ERR_MMC_OPEN_FILE (ERR_MMC_START + 2)    /**mmc设备打开文件错误*/
#define ERR_MMC_OFFSET (ERR_MMC_START + 3)       /**mmc设备偏移地址错误*/
#define ERR_MMC_READ (ERR_MMC_START + 4)         /**mmc设备读错误*/
#define ERR_MMC_WRITE (ERR_MMC_START + 5)        /**mmc设备写错误*/

/***********************************************************
*   FTP错误码                                             *
***********************************************************/
#define ERR_FTP_OPEN_FILE (ERR_FTP_START + 1)    /**FTP打开文件错误 */
#define ERR_FTP_TRANS_ERROR (ERR_FTP_START + 2)  /**传输错误 */
#define ERR_FTP_FILE_MD5 (ERR_FTP_START + 3)     /**md5校验错误 */
#define ERR_FTP_SUBVER_NUM (ERR_FTP_START + 4)   /**子版本个数错误 */
#define ERR_FTP_PARSE_JSON (ERR_FTP_START + 5) /**空指针错误*/
/***********************************************************
*   HTTP错误码                                             *
***********************************************************/
#define ERR_HTTP_URL (ERR_HTTP_START + 1)           /**url格式错误 */
#define ERR_HTTP_SEND_JSON (ERR_HTTP_START + 2)     /**构造发送json格式错误 */
#define ERR_HTTP_PARSE_JSON (ERR_HTTP_START + 3)    /**解析json格式错误 */
#define ERR_HTTP_POST_DATA (ERR_HTTP_START + 4)     /**构造发送json格式错误 */
#define ERR_HTTP_SUB_NUM (ERR_HTTP_START + 5)       /**子版本个数错误 */
#define ERR_HTTP_DOWNLOAD_FILE (ERR_HTTP_START + 6) /**下载文件错误 */
#define ERR_HTTP_FILE_MD5 (ERR_HTTP_START + 7)      /**md5校验错误 */
/***********************************************************
*                          VMM模块错误码                   *
***********************************************************/
#define ERR_VMM_NULL_POINTER (ERR_VMM_START + 1)        /**vmm空指针错误*/
#define ERR_VMM_STRING_FORMAT (ERR_VMM_START + 2)       /**vmm字符串格式错误*/
#define ERR_VMM_VERSION_INFO_STRING (ERR_VMM_START + 3) /**vmm版本信息格式错误*/
#define ERR_VMM_DATA_FORMAT (ERR_VMM_START + 4)         /**vmm cmd数据格式错误*/
#define ERR_VMM_FTP_OPEN (ERR_VMM_START + 5)            /**vmm ftp打开错误*/
#define ERR_VMM_FTP_RECV (ERR_VMM_START + 6)            /**vmm ftp接收错误*/
#define ERR_VMM_SUBVER_NUM (ERR_VMM_START + 7)          /**vmm 子版本数量比较错误*/
#define ERR_VMM_INFO_MAX_NUM (ERR_VMM_START + 8)        /**vmm 子版本数量过多错误*/
#define ERR_VMM_INFO_READ (ERR_VMM_START + 9)           /**vmm 版本信息读取错误*/
#define ERR_VMM_INFO_WRITE (ERR_VMM_START + 10)         /**vmm 版本信息写入错误*/
#define ERR_VMM_SUBNUM_CHECK (ERR_VMM_START + 11)       /**vmm 子版本校验错误*/
#define ERR_VMM_FILE_WRITE (ERR_VMM_START + 12)         /**vmm 版本信息写入错误*/
#define ERR_VMM_FILE_MD5 (ERR_VMM_START + 13)           /**vmm 版本md5校验错误*/