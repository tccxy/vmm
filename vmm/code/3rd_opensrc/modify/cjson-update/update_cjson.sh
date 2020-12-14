#!/bin/sh
export PREFIX=${CJSON_INSTALL}
export SRC_DIR=${CJSON_UNPACK}
export _3RD_CROSS_COMPILE=${CROSS_COMPILE}
export CROSS_COMPILE=${_3RD_CROSS_COMPILE}
export AR=${_3RD_CROSS_COMPILE}ar
export AS=${_3RD_CROSS_COMPILE}as
export LD=${_3RD_CROSS_COMPILE}ld
export RANLIB=${_3RD_CROSS_COMPILE}ranlib
export CC=${_3RD_CROSS_COMPILE}gcc
export NM=${_3RD_CROSS_COMPILE}nm


make all install

