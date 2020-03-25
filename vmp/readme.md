<!-- TOC -->

- [版本管理](#版本管理)
    - [概述](#概述)
    - [版本划分](#版本划分)
        - [1. 嵌入式设备的启动流程](#1-嵌入式设备的启动流程)
    - [2.版本的定义](#2版本的定义)
    - [3.安全可控的版本引导流程](#3安全可控的版本引导流程)
        - [1）较为安全的方式A](#1较为安全的方式a)
        - [2）较为节省空间的方式B](#2较为节省空间的方式b)
    - [实现](#实现)

<!-- /TOC -->
# 版本管理
## 概述
实现极高安全可靠，高效率的版本管理系统，设备端支持基于本地(ldv)和基于云端(ota)的两种版本下载方式。为了提高版本管理系统的安全性，在设备侧对于关键数据的存储部分采用冗余双备份管理，ota传输过程采用基于分区的流传输加密技术，版本文件的完整性采用MD5校验方式，可以从技术源头确保不会因为版本升级过程中的断网，断电等各种异常情况导致设备宕机。为了提高版本的管理效率，将整个打的软件包依照升级频率的大原则进行细化切割，可以确保只利用最小的带宽资源和设备资源就对主要的业务部分完成升级。本地管理支持通过内部局域网完成各版本的下载，支持版本的回滚。
## 版本划分
版本管理功能是设备的重要的基础功能，版本管理首先面临的是如何划分版本的问题。
###1. 嵌入式设备的启动流程
嵌入式设备的启动流程如下图所示
```flow
//定义类型和描述
st=>start: 开始
e=>end: 结束
op=>operation: BootRom
op1=>operation: 一级loader
op2=>operation: Uboot
op3=>operation: Kernel
op4=>operation: Rootfs
op5=>operation: AppImage
  
st->op->op1->op2(right)->op3(right)->op4->op5->e

```

这是一个一般意义上的启动流程，即使是对于单片机或者cortex-m来说也是类似的，只是单片机没有复杂的
系统，或者是可能只包含一个rtos，那么他可能就是直接由一级loader引导应用启动了，更常见的对于单片
机或者cortex-m系列，可能厂家提供的只是一个bsp的包，(其实可以看看他的xx.S文件，类似于一个小的
loader)。

**再次分析或分类整个流程的各部分**

这里还是从一般意义上来讲，对于大部分设备来说，一级loader及其之前的部分是无法进行更改的，即使对于
少部分开源一级loader的芯片来说，其一级loader通常也是采用汇编来编写的，改动起来风险较高，对于一
个器件来说，一旦确定了他的启动方式，那么一级loader的位置基本上确定的，通常都是需要放在相应的存储
介质的起始位置，或者指定的偏移位上，我们依据版本存放位置是否方便更改的原则将启动流程进一步划分。

**将启动流程进一步划分**

```flow
//定义类型和描述
st=>start: 开始
e=>end: 结束
op=>operation: BootLoader
op1=>operation: UserImage
  
st->op(right)->op1->e

```
或者按照是否频繁更改的原则细化一下（通常系统版本不需要频繁的更改）
```flow
//定义类型和描述
st=>start: 开始
e=>end: 结束
op=>operation: BootLoader
op1=>operation: SysImage
op2=>operation: AppImage
  
st->op(right)->op1->op2->e

```
##2.版本的定义

我们将BootLoader定义为主版本，其包含的一级loader、uboot等定义为子版本，同理将SysImage定义为
主版本，其包含的kernel、rootfs定义为子版本，AppImage也可以包含appimage1、appiamge2等，如果
采用UserImage的方式，则其子版本包含kernel、rootfs、appimage等。
例如
我们以rk3399为例，可以看到其BootLoader包含loader1，uboot，trust三个子版本，如下，其loader1
不开源，且需要放在64扇区偏移的位置上。
```bash
Number  Start   End     Size    File system  Name     Flags
 1      32.8kB  4129kB  4096kB               loader1
 2      8389kB  12.6MB  4194kB               uboot
 3      12.6MB  16.8MB  4194kB               trust
 4      16.8MB  151MB   134MB   fat16        sys      boot, esp
 5      151MB   749MB   598MB   ext4         rootfs
 6      749MB   1331MB  583MB                app
 
(parted)
```
##3.安全可控的版本引导流程

版本的升级引导主要有以下两种方式，其中的优劣势都很明显
###1）较为安全的方式A

为了防止在版本升级过程中，发生断电等异常情况的产生，在存储介质中除了BootLoader外，其他的版本都采
用双版本的形式进行存储，通过版本控制字来记录版本的更新等信息，如果采用UserImage的方式进行存储，则
其正常的引导流程如下

```flow
//定义类型和描述
st=>start: 开始
e=>end: 结束
op=>operation: BootLoader
op1=>operation: UserImage_A
op2=>operation: UserImage_B
op3=>condition: 查询版本控制字
  
st->op(right)->op3
op3(yes)->op1->e
op3(no)->op2->e
```
如果以SysImage和AppImage的方法进行管理则其引导流程如下

```flow
//定义类型和描述
st=>start: 开始
e=>end: 结束
op=>operation: BootLoader
op1=>operation: SysImage_A
op2=>operation: SysImage_B
op3=>condition: 查询版本控制字
op4=>condition: 查询版本控制字
op5=>operation: AppImage_A
op6=>operation: AppImage_B
  
st->op(right)->op3
op3(yes)->op1->op4
op3(no)->op2->op4
op4(yes)->op5->e
op4(no)->op6->e
```
* 在升级的时候采用的方式同样如果当前为A版本，则向B版本的存储空间进行烧写，反之亦然，更新完成后同步更新版本控制字，进而达到重启后生效的目的。
```flow
//定义类型和描述
st=>start: 开始
e=>end: 结束
op=>operation: BootLoader
op1=>operation: UserImage_A
op2=>operation: UserImage_B
op3=>condition: 查询版本控制字
  
st->op1(right)->op2->op1
```
这种升级方式的优点是，在升级的过程中无需重启，而是向另一块存储空间写入，即使更新的过程中出现故障，并不会影响设备的运行，同时可以通过一些用户空间的逻辑去做一些版本的回退等安全机制

缺点就是占用的存储空间较大，是版本的2倍

###2）较为节省空间的方式B

较为节省空间的存储形式则是在存储介质中全部以单版本的形式进行存储，当有版本需要进行更新时，在用户
空间进行版本的拉取，然后更新版本控制字，版本的更新写入则由重启后的BootLoader执行
```flow
//定义类型和描述
st=>start: 开始
e=>end: 结束
op=>operation: BootLoader
op1=>operation: 更新版本
op2=>operation: UserImage
op3=>condition: 查询版本控制字
  
st->op(right)->op3
op3(yes)->op1->e
op3(no)->op2->e
```
这种升级方式的好处就是节省存储空间，但是在写入的整个过程中不能够出现意外的情况，而且要在用户空间
为待更新版本留够足够的空间，这也是类似于现在的安卓手机的版本更新方式。

## 实现
* 该部代码实现了1)较为安全的方式，在code目录执行make即可在output/release下获得可执行的文件和相应的动态库
* 编码风格采用了linux kernel的编码规范，注释采用了doxygen风格
* doc目录执行doxygen即可生成html的在线类说明,打开index.html即可
* 实现最多包含3个主版本，每个主版本内8个子版本的管理
* 网络传输协议支持Http/Https、Ftp两种
* 版本查询支持历史版本记录查询
* 支持本地版本管理操作
* 支持ota,当进行ota时，切记首先执行load操作
* 基于zlog的日志记录
* 完整的全部功能实现还需uboot下的部分代码支持,(主要是用来挂载系统版本)
