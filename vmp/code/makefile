######################################################################
#                  Source path information  1                        #
######################################################################
#export CC = arm-linux-gnueabihf-gcc
export CC = aarch64-linux-gnu-gcc
export ROOT_DIR      :=  $(shell pwd)
export SRC_DIR       :=  $(ROOT_DIR)/src
export INC_DIR       :=  $(ROOT_DIR)/include
export OBJ_DIR       :=  $(ROOT_DIR)/out_put/obj
export REL_DIR       :=  $(ROOT_DIR)/out_put/release
export 3RD_DIR       :=  $(ROOT_DIR)/3rd_opensrc
export CPU_TYPE      :=  ZYNQMP
######################################################################
#                     add header                                 #
######################################################################
ifneq ($(findstring arm,$(CC)),)
  LIB               :=  lib-32
endif
ifneq ($(findstring aarch64,$(CC)),)
  LIB               :=  lib
endif
COMMON_CFLAG_OPTION      := -rdynamic -O2 -Wall -Wno-pointer-to-int-cast\
                            -Wno-incompatible-pointer-types -Wno-pointer-sign\
                            -Wno-missing-braces -Wno-format -D$(CPU_TYPE)
export CFLAGS            += -I$(INC_DIR) $(COMMON_CFLAG_OPTION) -I$(3RD_DIR)\
                            -I$(3RD_DIR)/curl_install/include\
                            -I$(3RD_DIR)/cjson_install/include\
                            -I$(3RD_DIR)/openssl_install/include\
                            -I$(3RD_DIR)/zlog_install/include\
######################################################################
#                     add lib                                    #
######################################################################
export DFLAGS            := -lpthread -lm -lcrypt -ldl -lrt\
                            -L$(3RD_DIR)/cjson_install/$(LIB) -lcjson\
                            -L$(3RD_DIR)/openssl_install/$(LIB) -lcrypto -lssl\
                            -L$(3RD_DIR)/curl_install/$(LIB) -lcurl\
                            -L$(3RD_DIR)/zlog_install/$(LIB) -lzlog\

.PHONY : all clean CREAT_DIR MAKE_OBJ LIB_CP BIN

all : clean CREAT_DIR MAKE_OBJ LIB_CP BIN


CREAT_DIR :
	@echo creat destination directory!$(CC)! $(shell date)
	-mkdir -p $(OBJ_DIR)
	-mkdir -p $(REL_DIR)
MAKE_OBJ :
	@echo creat destination directory!$(CC)! $(shell date)
	make -C $(SRC_DIR)
LIB_CP :
	@echo creat destination directory!$(CC)! $(shell date)
	cp $(ROOT_DIR)/3rd_opensrc/openssl_install/$(LIB)/lib*.so* $(REL_DIR) -Rf
	cp $(ROOT_DIR)/3rd_opensrc/zlog_install/$(LIB)/lib*.so* $(REL_DIR) -Rf
BIN :
	@echo creat destination directory!$(CC)! $(shell date)
	$(CC) $(OBJ_DIR)/*.o -o $(REL_DIR)/vmp $(CFLAGS) $(DFLAGS)
clean :
	rm -rf $(OBJ_DIR)
	rm -rf $(REL_DIR)