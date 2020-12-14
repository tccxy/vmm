#!/bin/sh


UBOOT_DIR=/home/ab64
WORK_DIR=/home/ab64
# now 6 parted          all is 8g
#1st is boot
#2nd is rootfs           8192s
#3rd is version_info     20480s
#4th is APP_A            262144s
#5th is APP_B            4984799s
#6th is data_part        the rest of ext4

#modiphy this data
dev_no=/dev/mmcblk2
mmc1=516096
mmc2=4984799
#mmc3 #extend 
mmc4=20480
mmc5=524288
mmc6=524288
mmc7=



# don't modify!!!!
# Start partition  
start=8192

dd bs=1 seek=446 count=64 if=/dev/zero of="$dev_no" >/dev/null 2>&1

parted -s $dev_no -- mklabel msdos

mmc1_start=$start
mmc1_end=$(($start+$mmc1-1))
echo mmc1_end is $mmc1_end
parted -s $dev_no -- mkpart primary fat32 ${mmc1_start}s ${mmc1_end}s

mmc2_start=$(($mmc1_end+1))
mmc2_end=$(($mmc2_start+$mmc2-1))
echo mmc2_start is $mmc2_start
echo mmc2_end is $mmc2_end
parted -s $dev_no -- mkpart primary ext4 ${mmc2_start}s ${mmc2_end}s

##extend part 
mmc3_start=$((mmc2_end+1))
parted -s $dev_no -- mkpart extend ${mmc3_start}s 100%

#logic  part  
mmc4_start=$((mmc3_start+1))
mmc4_end=$(($mmc4_start+$mmc4-1))
echo mmc4_end is $mmc4_end
parted -s $dev_no -- mkpart logic ${mmc4_start}s ${mmc4_end}s

#logic part
mmc5_start=$((mmc4_end+2))
mmc5_end=$(($mmc5_start+$mmc5-1))
echo mmc5_end is $mmc5_end
parted -s $dev_no -- mkpart logic ${mmc5_start}s ${mmc5_end}s

#logic part
mmc6_start=$((mmc5_end+2))
mmc6_end=$(($mmc6_start+$mmc6-1))
echo mmc6_end is $mmc6_end
parted -s $dev_no -- mkpart logic ${mmc6_start}s ${mmc6_end}s

mmc7_start=$((mmc6_end+2))
parted -s $dev_no -- mkpart logic ${mmc7_start}s 100%

parted -s "$dev_no" set 1 boot on
partprobe "$dev_no"


sleep 2

mkfs.vfat "$dev_no"'p1'
mkfs.ext4 "$dev_no"'p2'

emmcrootuuid=$(blkid -o export "$dev_no"'p2' | grep -w UUID)
emmcbootuuid=$(blkid -o export "$dev_no"'p1' | grep -w UUID)

echo "emmcbootuuid $emmcbootuuid"
echo "emmcrootuuid $emmcrootuuid"

write_uboot_platform()
{
	echo "will write uboot"
	dd if="$UBOOT_DIR/u-boot.bin.sd.bin" of="$dev_no" conv=fsync,notrunc bs=442 count=1 > /dev/null 2>&1
	dd if="$UBOOT_DIR/u-boot.bin.sd.bin" of="$dev_no" conv=fsync,notrunc bs=512 skip=1 seek=1 > /dev/null 2>&1
}

replace_uuid()
{
	if [[ -f $WORK_DIR/emmc/env.txt ]]; then
		sed -e 's,rootdev=.*,rootdev='"$emmcrootuuid"',g' -i $WORK_DIR/emmc//env.txt
		grep -q '^rootdev' $WORK_DIR/emmc/env.txt || echo "rootdev=$emmcrootuuid" >> $WORK_DIR/emmc//env.txt
	fi
}
write_uboot_platform

mkdir -p $WORK_DIR/sd
mkdir -p $WORK_DIR/emmc

mount /dev/mmcblk1p1 $WORK_DIR/sd
slepp1
mount /dev/mmcblk2p1 $WORK_DIR/emmc
slepp1

echo "cp boot file"
cp $WORK_DIR/sd/* $WORK_DIR/emmc/* -Rdf
cp $WORK_DIR/sd/.next $WORK_DIR/emmc/ -Rdf

sleep1
echo "umont please wait"
umount -l $WORK_DIR/sd
umount -l $WORK_DIR/emmc


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

