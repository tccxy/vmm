# 自动化分区脚本
* 以板级作为目录，目前共有两种板子支持
* auto_part*.sh为自动化分区脚本工具，采用gpt的方式进行分区
* 此分区针对的对象是emmc/sd
* 因采用的gpt分区，要求宿主机安装parted工具
* zynq系列对gpt分区支持，不好，需作更改
* usp_log.conf 为zlog的日志记录模板

# user_manual.md为使用说明

# version.dat 为版本描述信息模板