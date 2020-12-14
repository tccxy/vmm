#!/bin/sh

export PREFIX=${OPENSSL_INSTALL}
export SRC_DIR=${OPENSSL_UNPACK}
export _3RD_CROSS_COMPILE=${CROSS_COMPILE}
#export CROSS_COMPILE=""
#export CFLAGS=-Wall-O3-pthread-DL_ENDIAN
#export _3RD_CROSS_COMPILE=/opt/gcc-linaro-4.9.4-2017.01-i686_aarch64-linux-gnu/bin/aarch64-linux-gnu-
export AR=ar
export AS=as
export LD=ld
export RANLIB=ranlib
export CC=gcc
export NM=nm

cd ${SRC_DIR}
./config no-asm shared\
	--prefix=${PREFIX}\
	--cross-compile-prefix=${CROSS_COMPILE}\

sed -i "s/-m64/ /g" ${SRC_DIR}/Makefile
#cd ${_USP_OPENSSL_UPDATE}
#cp Makefile ${_USP_OPENSSL_UNPACK}
#cd ${_USP_OPENSSL_UNPACK}
make
make install
#rm ${_USP_OPENSSL_INSTALL}/include/openssl -r
#cp -r ${_USP_OPENSSL_UPDATE}/openssl ${_USP_OPENSSL_INSTALL}/include
export CROSS_COMPILE=${_3RD_CROSS_COMPILE}


		  

