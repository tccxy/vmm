#!/bin/sh
export PREFIX=${CURL_INSTALL}
export SRC_DIR=${CURL_UNPACK}
export _3RD_CROSS_COMPILE=${CROSS_COMPILE}
export CROSS_COMPILE=${_3RD_CROSS_COMPILE}
export AR=${_3RD_CROSS_COMPILE}ar
export AS=${_3RD_CROSS_COMPILE}as
export LD=${_3RD_CROSS_COMPILE}ld
export RANLIB=${_3RD_CROSS_COMPILE}ranlib
export CC=${_3RD_CROSS_COMPILE}gcc
export NM=${_3RD_CROSS_COMPILE}nm

cd ${SRC_DIR}
#chmod +x configure

./configure --host=arm-linux \
			--prefix=${PREFIX}\
			--disable-ares \
			--disable-dict \
			--enable-file \
			--disable-gopher \
			--disable-idn \
			--disable-imap \
			--disable-ipv6 \
			--disable-largefile \
			--disable-ldap \
			--disable-manual \
			--disable-pop3 \
			--disable-proxy \
			--disable-rt \
			--disable-rtsp \
			--disable-smb \
			--disable-smtp \
			--disable-sspi \
			--disable-shared \
			--disable-telnet \
			--enable-tftp \
			--enable-static \
			--without-axtls \
			--without-cyassl \
			--without-darwinssl \
			--without-gnutls \
			--without-libidn2 \
			--without-librtmp \
			--without-libssh2 \
			--without-mbedtls \
			--without-nghttp2 \
			--without-nss \
			--without-polarssl \
			--without-ssl \
			--without-winssl \
			--without-zlib

cd 	${SRC_DIR}
pwd
touch *

touch ${SRC_DIR}/*
sudo make install
