<!-- TOC -->

- [vmp版本管理使用说明书](#vmp版本管理使用说明书)
    - [版本查询](#版本查询)
    - [版本升级](#版本升级)
    - [版本参数设置](#版本参数设置)
    - [版本ota升级](#版本ota升级)
    - [版本激活](#版本激活)
    - [版本去激活](#版本去激活)
    - [版本加载功能](#版本加载功能)

<!-- /TOC -->
# vmp版本管理使用说明书
* vmp -h已经列出了使用说明，下面一一详细说明
```
root@localhost:/usp/app# ./vmp -h
VMP_ABU         
 Usage   :         
    vmp [options] <type> {[trans] [url] --netmsg=...}        
    vmp [options] <type> {--number=<x>}        
 options        
    -h,--help                          get app help        
    -g,--get      <type>               get version info <boot | sys | app>        
        vmp [options] <type> {--number=<x>}        
    -o,--ota      <type>               ota version <boot | sys | app>        
    -s,--set      <type>               set version info <boot | sys | app>        
    -u,--update   <type>               update version <boot | sys | app>        
        -s,-u     <type> [trans] <url> --netmsg=...        
    -a,--active   <type>               active version <sys | app>        
    -d,--deactive <type>               deactive version <sys | app>        
    -l,--load     [mount dir]          load version just for app        
 type        
    <boot | sys | app>        
 trans        
    -F,--Ftp        
         --netmsg=<filename>:<username>:<passwd>        
    -H,--Http        
         --netmsg=<cpunum>:<otaid>:<otasecret>        
    eg-->:vmp -u boot -F ftp://192.168.5.196:21 --netmsg=version.dat:ab:ab        
          vmp -u app -H http://192.168.130.33:8181/ota --netmsg=1:abmms:4it
```


## 版本查询
vmp支持版本查询，并提供历史记录查询,如下

```bash
#只查询最新一条记录
root@localhost:/usp/app# ./vmp -g app
Version information Item Is 1 

 Version information 1 
--  MainName:                 APP    MainVersion:   1.00.05      SubNumber: 1     UseVersion               #表示当前使用的版本
**   SubName:     zq_1.00.00.ext4     SubVersion:   1.00.00
--  MainName:                 APP    MainVersion:   1.00.06      SubNumber: 1  UpdateVersion               #表示有更新的版本
**   SubName:     zq_1.00.00.ext4     SubVersion:   1.00.00
#如果查询多条--number=N即可
root@localhost:/usp/app# ./vmp -g app --number=2
Version information Item Is 2 

 Version information 1 
--  MainName:                 APP    MainVersion:   1.00.05      SubNumber: 1     UseVersion               
**   SubName:     zq_1.00.00.ext4     SubVersion:   1.00.00
--  MainName:                 APP    MainVersion:   1.00.06      SubNumber: 1  UpdateVersion         Active #表示有激活的版本
**   SubName:     zq_1.00.00.ext4     SubVersion:   1.00.00

 Version information 2 
--  MainName:                 APP    MainVersion:   1.00.05      SubNumber: 1     UseVersion               
**   SubName:     zq_1.00.00.ext4     SubVersion:   1.00.00
--  MainName:                 APP    MainVersion:   1.00.06      SubNumber: 1  UpdateVersion               
**   SubName:     zq_1.00.00.ext4     SubVersion:   1.00.00
```

## 版本升级
vmp支持版本下载，下载协议支持Ftp Http/Https，各参数的指代意义都在help中列出来了，版本升级可以用于任何有效的版本进行升级，（不校验版本号）,
-F后接Ftp的url ,netmsg的参数依次为版本描述文件名字，ftp用户名和密码
-H后接Htpp的url,netmsg的参数依次为cpuid，otaid，otasecret
ftp服务器部署可以采用多种软件，这里就不赘述了
```bash
#ftp方式进行下载
root@localhost:/usp/app# ./vmp -u app -F ftp://192.168.5.196:21/ --netmsg=version.dat:zw:zw
100% [========================================]
download success 

#Http方式进行下载
root@localhost:/usp/app# /usp/app/vmp -u app -H http://40.73.46.255:8181/cloud-admin/ota/upgradeOta --netmsg=1:abx65j6gh5u04zat6bq2:1y0akyyt4lnkm516i9tt66aapqc3i7b6wwf8ayctxyhdju4siks107b08xr569lw 
100% [========================================]
download success .
```
版本描述文件如下所示
```bash
#设备类型
dcuCpuNum = 1
#主版本号
mainVersion =1.00.07
#子版本名字 与conf中对应 如有多个 按照conf的顺序罗列就可 
sub1Name = app_pl_1.00.00.ext4
#子版本号
sub1Version =1.00.00
#子版本大小单位为BYTE
sub1Size = 266338304
#子版本Md5校验值
sub1Md5 = 07308683b76e68497d67569591b12229
```
## 版本参数设置
vmp可以设置版本参数到版本参数区，传入的是版本描述文件
## 版本ota升级
vmp支持版本下载，下载协议支持Ftp Http/Https，各参数的指代意义都在help中列出来了，与版本升级不同的是，ota会自动连接服务器，只有当服务器的版本号大于当前运行的版本号时，会进行升级操作。
```bash
#ftp方式进行下载
root@localhost:/usp/app# ./vmp -o app -F ftp://192.168.5.196:21/ --netmsg=version.dat:zw:zw 

ota version is 1.00.07 
load version is 1.00.05 
start ota .
100% [========================================]
download success 
ota end .

#Http方式进行下载
root@localhost:/usp/app# /usp/app/vmp -u app -H http://40.73.46.255:8181/cloud-admin/ota/upgradeOta --netmsg=1:abx65j6gh5u04zat6bq2:1y0akyyt4lnkm516i9tt66aapqc3i7b6wwf8ayctxyhdju4siks107b08xr569lw 
ota version is 1.00.07 
load version is 1.00.05 
start ota .
100% [========================================]
download success 
ota end .
```
## 版本激活
vmp支持版本激活功能，该功能主要用于测试，一旦有效的版本被激活后（有效版本指的是版本信息完整的版本包），下一次重启则会启用激活版本，并且只有当其备用版本比服务器高时才会启动ota，
应用的典型场景如下：
A:当前版本为v1.00.03 ota服务器版本为v1.00.04,升级后设备自动运行v1.00.04,测试人员发现该版本并没有03版本好用，此时执行active操作，则设备再次上电后会运行03版本，并且直至服务器上有大于04版本的更新时才启动ota升级
B:当服务器上以正常节奏进行版本发布时，设备此时与服务器版本相同，如v1.0.07，我们的测试人员有一个小的功能需要进行测试，于是研发人员临时构建了v0.00.00的版本包，通过本地的ftp进行升级，激活两个操作后，设备就不再更新ota服务器的版本了，直至v1.00.08版本发布。
```bash
root@localhost:/usp/app# ./vmp -g app
Version information Item Is 1 

 Version information 1 
--  MainName:                 APP    MainVersion:   1.00.05      SubNumber: 1     UseVersion               
**   SubName:     zq_1.00.00.ext4     SubVersion:   1.00.00
--  MainName:                 APP    MainVersion:   1.00.07      SubNumber: 1  UpdateVersion               
**   SubName: app_pl_1.00.00.ext4     SubVersion:   1.00.00
root@localhost:/usp/app# ./vmp -a app 
root@localhost:/usp/app# ./vmp -g app
Version information Item Is 1 

 Version information 1 
--  MainName:                 APP    MainVersion:   1.00.05      SubNumber: 1     UseVersion               
**   SubName:     zq_1.00.00.ext4     SubVersion:   1.00.00
--  MainName:                 APP    MainVersion:   1.00.07      SubNumber: 1  UpdateVersion         Active
**   SubName: app_pl_1.00.00.ext4     SubVersion:   1.00.00
root@localhost:/usp/app# 
```
## 版本去激活
vmp支持版本去激活，如题，会将激活的版本权限取消，该版本再重启后会正常进行ota的升级操作
```bash
root@localhost:/usp/app# ./vmp -g app
Version information Item Is 1 

 Version information 1 
--  MainName:                 APP    MainVersion:   1.00.05      SubNumber: 1                              
**   SubName:     zq_1.00.00.ext4     SubVersion:   1.00.00
--  MainName:                 APP    MainVersion:   1.00.07      SubNumber: 1     UseVersion         Active
**   SubName: app_pl_1.00.00.ext4     SubVersion:   1.00.00
root@localhost:/usp/app# ./vmp -d app 
root@localhost:/usp/app# ./vmp -g app
Version information Item Is 1 

 Version information 1 
--  MainName:                 APP    MainVersion:   1.00.05      SubNumber: 1                              
**   SubName:     zq_1.00.00.ext4     SubVersion:   1.00.00
--  MainName:                 APP    MainVersion:   1.00.07      SubNumber: 1     UseVersion               
**   SubName: app_pl_1.00.00.ext4     SubVersion:   1.00.00
root@localhost:/usp/app# 
```
## 版本加载功能
vmp的版本加载功能，会将应当运行的分区挂载到相应的目录,参数在help中已经说明
```bash
root@localhost:/usp/app# ./vmp -l /home/ab64/ab_app/
root@localhost:/usp/app# df
Filesystem     1K-blocks    Used Available Use% Mounted on
/dev/root        3030800 1889756    967376  67% /
devtmpfs          891648       0    891648   0% /dev
tmpfs            1023136       0   1023136   0% /dev/shm
tmpfs            1023136    9700   1013436   1% /run
tmpfs               5120       0      5120   0% /run/lock
tmpfs            1023136       0   1023136   0% /sys/fs/cgroup
tmpfs            1023136       0   1023136   0% /var/volatile
tmpfs             204628       0    204628   0% /run/user/1001
/dev/mmcblk0p6    247791    2063    228628   1% /home/ab64/ab_app
```
