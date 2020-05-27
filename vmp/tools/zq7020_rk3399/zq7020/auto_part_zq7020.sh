#!/bin/sh

# now 5 parted          all is 8g
#1st is image           50m fat
#2nd is rootfs          3g  ext4
#3rd is version_info    8m  none
#4th is APP_A           256m ext4
#5th is APP_B           256m ext4
#6th is data_part       the rest of ext4
#modiphy this data
dev_no=/dev/mmcblk1
mmc1=50
mmc2=3072
mmc3=8
mmc4=256
mmc5=256
mmc6=



# don't modify!!!!
# Start partition  
start=2048


parted -s $dev_no mklabel gpt

mmc1_start=$start
mmc1_end=$(($start+$(($mmc1*512*4))-1))
echo mmc1_end is $mmc1_end
parted -s $dev_no unit s mkpart boot $mmc1_start $mmc1_end

mmc2_start=$((mmc1_end+1))
mmc2_end=$(($mmc2_start+$(($mmc2*512*4))-1))
echo mmc2_end is $mmc2_end
parted -s $dev_no unit s mkpart verinfo $mmc2_start $mmc2_end

mmc3_start=$((mmc2_end+1))
mmc3_end=$(($mmc3_start+$(($mmc3*512*4))-1))
echo mmc3_end is $mmc3_end
parted -s $dev_no unit s mkpart image_A $mmc3_start $mmc3_end

mmc4_start=$((mmc3_end+1))
mmc4_end=$(($mmc4_start+$(($mmc4*512*4))-1))
echo mmc4_end is $mmc4_end
parted -s $dev_no unit s mkpart image_B $mmc4_start $mmc4_end

mmc5_start=$((mmc4_end+1))
mmc5_end=$(($mmc5_start+$(($mmc5*512*4))-1))
echo mmc5_end is $mmc5_end
parted -s $dev_no unit s mkpart rootfs_A $mmc5_start $mmc5_end

mmc6_start=$((mmc5_end+1))
parted -s $dev_no unit s mkpart data $mmc6_start 100%


# End partition 

# Star ReWrite partition table for Fat
dd if=$dev_no of=./GPT.ori count=512 bs=1
dd conv=notrunc bs=1 count=16 skip=446 if=GPT.ori of=MBR.temp seek=462
dd if=MBR.temp of=$dev_no count=512 bs=1

# Start format partition
image_p=$dev_no"p1"
echo image_p is $image_p
mkfs.vfat $image_p

rootfs_p=$dev_no"p2"
echo rootfs_p is $rootfs_p
mkfs.ext4 $rootfs_p

data_p=$dev_no"p6"
echo data_p is $data_p
mkfs.ext4 $data_p

info_p=$dev_no"p3"
dd if=/dev/zero of=$info_p


# Start copy
mount_d=./tmp

mkdir -p $mount_d

echo start cp boot_p
flash_eraseall /dev/mtd0
flashcp BOOT.BIN /dev/mtd0


echo start cp image
mount $image_p $mount_d
cp image.ub $mount_d -rf
umount -l $mount_d

sleep 3
echo start tar rootfs.tar.gz
mount $rootfs_p $mount_d
tar -mzxf ubuntu16.04-base.tar.gz -C $mount_d
umount -l $mount_d

echo end please reboot...

