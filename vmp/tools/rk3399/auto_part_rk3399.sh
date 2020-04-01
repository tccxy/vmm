#!/bin/sh

# now 9 parted          all is 8g
#1st is spl             8000s
#2nd is uboot           8192s
#3rd is trust           8192s
#4th is kernel          262144s
#5th is rootfs          4984799s
#6th is version_info    20480s
#7th is APP_A           524288s
#8th is APP_B           524288s
#9th is data_part       the rest of ext4
#modiphy this data
dev_no=/dev/mmcblk1
mmc1=8000
mmc2=8192
mmc3=8192
mmc4=262144
mmc5=4984799
mmc6=20480
mmc7=524288
mmc8=524288
mmc9=



# don't modify!!!!
# Start partition  
start=64
RESERVED1_SIZE=128
RESERVED2_SIZE=8192

parted -s $dev_no mklabel gpt

mmc1_start=$start
mmc1_end=$(($start+$mmc1-1))
echo mmc1_end is $mmc1_end
parted -s $dev_no unit s mkpart loader1 $mmc1_start $mmc1_end

mmc2_start=$(($mmc1_end+1+$(($RESERVED1_SIZE+$RESERVED2_SIZE))))
mmc2_end=$(($mmc2_start+$mmc2-1))
echo mmc2_start is $mmc2_start
echo mmc2_end is $mmc2_end
parted -s $dev_no unit s mkpart loader2 $mmc2_start $mmc2_end

mmc3_start=$((mmc2_end+1))
mmc3_end=$(($mmc3_start+$mmc3-1))
echo mmc3_end is $mmc3_end
parted -s $dev_no unit s mkpart trust $mmc3_start $mmc3_end

mmc4_start=$((mmc3_end+1))
mmc4_end=$(($mmc4_start+$mmc4-1))
echo mmc4_end is $mmc4_end
parted -s $dev_no unit s mkpart boot $mmc4_start $mmc4_end

mmc5_start=$((mmc4_end+1))
mmc5_end=$(($mmc5_start+$mmc5-1))
echo mmc5_end is $mmc5_end
parted -s $dev_no unit s mkpart rootfs $mmc5_start $mmc5_end

mmc6_start=$((mmc5_end+1))
mmc6_end=$(($mmc6_start+$mmc6-1))
echo mmc6_end is $mmc6_end
parted -s $dev_no unit s mkpart versioninfo $mmc6_start $mmc6_end

mmc7_start=$((mmc6_end+1))
mmc7_end=$(($mmc7_start+$mmc7-1))
echo mmc7_end is $mmc7_end
parted -s $dev_no unit s mkpart app_A $mmc7_start $mmc7_end

mmc8_start=$((mmc7_end+1))
mmc8_end=$(($mmc8_start+$mmc8-1))
echo mmc8_end is $mmc8_end
parted -s $dev_no unit s mkpart app_B $mmc8_start $mmc8_end

mmc9_start=$((mmc8_end+1))
parted -s $dev_no unit s mkpart data $mmc9_start 100%

echo "set boot on"
parted -s $dev_no set 4 boot on



echo "set uuid"
ROOT_UUID="B921B045-1DF0-41C3-AF44-4C6F280D3FAE"

	gdisk ${dev_no} <<EOF
x
c
5
${ROOT_UUID}
w
y
EOF
# End partition 

# Start format partition
echo write loader1
dd if=./idbloader.img of=$dev_no"p1"
echo write uboot
dd if=./uboot.img of=$dev_no"p2"
echo write trust
dd if=./trust.img of=$dev_no"p3"
echo write boot
dd if=./boot.img of=$dev_no"p4"
echo write rootfs
dd if=./ubuntu16.04-rk3399.img of=$dev_no"p5"

info_p=$dev_no"p6"
dd if=/dev/zero of=$info_p

data_p=$dev_no"p9"
echo data_p is $data_p
mkfs.ext4 $data_p

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

