#!/bin/sh

# now 9 parted          all is 8g
#1st is BOOT.BIN        20m fat32
#2nd is version_info    8m none
#3rd is image.ub_A      50m fat32
#4th is image.ub_B      50m fat32
#5th is rootfs_A        2.5g ext4
#6th is rootfs_B        2.5g ext4
#7th is APP_A           256m ext4
#8th is APP_B           256m ext4
#9th is data_part       the rest of ext4
#modiphy this data
dev_no=/dev/mmcblk0
mmc1=20
mmc2=8
mmc3=50
mmc4=50
mmc5=2560
mmc6=2560
mmc7=256
mmc8=256
mmc9=



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
mmc6_end=$(($mmc6_start+$(($mmc6*512*4))-1))
echo mmc6_end is $mmc6_end
parted -s $dev_no unit s mkpart rootfs_B $mmc6_start $mmc6_end

mmc7_start=$((mmc6_end+1))
mmc7_end=$(($mmc7_start+$(($mmc7*512*4))-1))
echo mmc7_end is $mmc7_end
parted -s $dev_no unit s mkpart app_A $mmc7_start $mmc7_end

mmc8_start=$((mmc7_end+1))
mmc8_end=$(($mmc8_start+$(($mmc8*512*4))-1))
echo mmc8_end is $mmc8_end
parted -s $dev_no unit s mkpart app_B $mmc8_start $mmc8_end

mmc9_start=$((mmc8_end+1))
parted -s $dev_no unit s mkpart data $mmc9_start 100%


# End partition 

# Star ReWrite partition table for Fat
dd if=$dev_no of=./GPT.ori count=512 bs=1
dd conv=notrunc bs=1 count=16 skip=446 if=GPT.ori of=MBR.temp seek=462
dd if=MBR.temp of=$dev_no count=512 bs=1

# Start format partition
boot_p=$dev_no"p1"
echo boot_p is $boot_p
mkfs.vfat $boot_p

image_a_p=$dev_no"p3"
echo image_a_p is $image_a_p
mkfs.vfat $image_a_p

image_b_p=$dev_no"p4"
echo image_b_p is $image_b_p
mkfs.vfat $image_b_p

rootfs_a_p=$dev_no"p5"
echo rootfs_a_p is $rootfs_a_p
mkfs.ext4 $rootfs_a_p

rootfs_b_p=$dev_no"p6"
echo rootfs_b_p is $rootfs_b_p
mkfs.ext4 $rootfs_b_p

data_p=$dev_no"p9"
echo data_p is $data_p
mkfs.ext4 $data_p

info_p=$dev_no"p2"
dd if=/dev/zero of=$info_p


# Start copy
#mount_d=./tmp

#mkdir -p $mount_d

#echo start cp boot_p
#mount $boot_p $mount_d
#cp BOOT.BIN $mount_d -rf
#cp image.ub $mount_d -rf
#umount -l $mount_d

#sleep 3
#echo start tar rootfs.tar.gz
#mount $sys_p $mount_d
#tar -mzxf rootfs16.04-base.tar.gz -C $mount_d
#umount -l $mount_d

echo end please reboot...

