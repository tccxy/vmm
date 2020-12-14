######################################################################
#                  Source path information  1                        #
######################################################################
export ROOT_DIR      :=  $(shell pwd)
export SRC_DIR       :=  $(ROOT_DIR)/src
export INC_DIR       :=  $(ROOT_DIR)/include
export OBJ_DIR       :=  $(ROOT_DIR)/out_put/obj
export REL_DIR       :=  $(ROOT_DIR)/out_put/release
export 3RD_DIR       :=  $(ROOT_DIR)/3rd_opensrc
export COMPILE       :=  $(ROOT_DIR)/compile
export CPU_TYPE      :=  A311D

######################################################################
#                     3rd  install                                   #
######################################################################
export 3RD_INSATLL_DIR	:=	$(3RD_DIR)/install
export OPENSSL_INSTALL	:=	$(3RD_INSATLL_DIR)/openssl-1.1.0e
export CURL_INSTALL 	:=	$(3RD_INSATLL_DIR)/curl-7.51.0
export CJSON_INSTALL 	:=	$(3RD_INSATLL_DIR)/cjson-200908
export ZLOG_INSTALL 	:=	$(3RD_INSATLL_DIR)/zlog-1.2.15
######################################################################
#                     gcc compile                                   #
######################################################################
export C_V               := gcc-linaro-7.2.1-2017.11-x86_64_aarch64-linux-gnu
export CROSS_COMPILE      :=  $(COMPILE)/$(C_V)/bin/aarch64-linux-gnu-
export CC = ${CROSS_COMPILE}gcc
#export CPP = ${CROSS_COMPILE}g++
export AR = ${CROSS_COMPILE}ar
export STRIP = ${CROSS_COMPILE}strip
export AS = ${CROSS_COMPILE}as
export LD = ${CROSS_COMPILE}lr
export RANLIB = ${CROSS_COMPILE}ranlib
export NM = ${CROSS_COMPILE}nm
######################################################################
#                     add header                                 #
######################################################################
LIB               :=  lib

COMMON_CFLAG_OPTION      := -rdynamic -O2 -Wall -Wno-pointer-to-int-cast\
                            -Wno-incompatible-pointer-types -Wno-pointer-sign\
                            -Wno-missing-braces -Wno-format -D$(CPU_TYPE)
export CFLAGS            += -I$(INC_DIR) $(COMMON_CFLAG_OPTION) -I$(3RD_DIR)\
                            -I$(CURL_INSTALL)/include\
                            -I$(CJSON_INSTALL)/include\
                            -I$(OPENSSL_INSTALL)/include\
                            -I$(ZLOG_INSTALL)/include\
######################################################################
#                     add lib                                    #
######################################################################
export DFLAGS            := -lpthread -lm -lcrypt -ldl -lrt\
                            -L$(CJSON_INSTALL)/$(LIB) -lcjson\
                            -L$(OPENSSL_INSTALL)/$(LIB) -lcrypto -lssl\
                            -L$(CURL_INSTALL)/$(LIB) -lcurl\
                            -L$(ZLOG_INSTALL)/$(LIB) -lzlog\

.PHONY : all clean CREAT_DIR 3RD MAKE_OBJ LIB_CP BIN

all : clean CREAT_DIR 3RD MAKE_OBJ LIB_CP BIN

3RD :
	@echo build $@ start! $(shell date)
	$(MAKE) -C $(3RD_DIR)
	@echo build $@ success! $(shell date)

CREAT_DIR :
	@echo creat destination directory!$(CC)! $(shell date)
	-mkdir -p $(OBJ_DIR)
	-mkdir -p $(REL_DIR)
	-rm -rf $(COMPILE)/$(C_V)
	cat $(COMPILE)/$(C_V).tar.gz0* | tar zx -C $(COMPILE)
#	-tar -zxvf $(COMPILE)/$(C_V).tar.gz -C $(COMPILE)

MAKE_OBJ :
	@echo creat destination directory!$(CC)! $(shell date)
	make -C $(SRC_DIR)

LIB_CP :
	@echo creat destination directory!$(CC)! $(shell date)
	cp $(OPENSSL_INSTALL)/lib/lib*.so* $(REL_DIR) -Rf
	cp $(ZLOG_INSTALL)/lib/lib*.so* $(REL_DIR) -Rf

BIN :
	@echo creat destination directory!$(CC)! $(shell date)
	$(CC) $(OBJ_DIR)/*.o -o $(REL_DIR)/vmm $(CFLAGS) $(DFLAGS)

clean :
	rm -rf $(OBJ_DIR)
	rm -rf $(REL_DIR)
