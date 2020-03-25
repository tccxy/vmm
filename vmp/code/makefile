######################################################################
#                  Source path information  1                        #
######################################################################
export CC = aarch64-linux-gnu-gcc
export ROOT_DIR      :=  $(shell pwd)
export SRC_DIR       :=  $(ROOT_DIR)/src
export INC_DIR       :=  $(ROOT_DIR)/include
export OBJ_DIR       :=  $(ROOT_DIR)/out_put/obj
export REL_DIR       :=  $(ROOT_DIR)/out_put/release
export 3RD_DIR       :=  $(ROOT_DIR)/3rd_opensrc

######################################################################
#                     add header                                 #
######################################################################
COMMON_CFLAG_OPTION      := -rdynamic -O2 -Wall -Wno-pointer-to-int-cast\
                            -Wno-incompatible-pointer-types -Wno-pointer-sign\
                            -Wno-missing-braces -Wno-format
export CFLAGS            += -I$(INC_DIR) $(COMMON_CFLAG_OPTION) -I$(3RD_DIR)\
                            -I$(3RD_DIR)/curl_install/include\
                            -I$(3RD_DIR)/cjson_install/include\
                            -I$(3RD_DIR)/openssl_install/include\
                            -I$(3RD_DIR)/zlog_install/include\
######################################################################
#                     add lib                                    #
######################################################################
export DFLAGS            := -lpthread -lm -lcrypt -ldl -lrt\
                            -L$(3RD_DIR)/cjson_install/lib -lcjson\
                            -L$(3RD_DIR)/openssl_install/lib -lcrypto -lssl\
                            -L$(3RD_DIR)/curl_install/lib -lcurl\
                            -L$(3RD_DIR)/zlog_install/lib -lzlog\

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
	cp $(ROOT_DIR)/3rd_opensrc/openssl_install/lib/lib*.so* $(REL_DIR) -Rf
	cp $(ROOT_DIR)/3rd_opensrc/zlog_install/lib/lib*.so* $(REL_DIR) -Rf
BIN :
	@echo creat destination directory!$(CC)! $(shell date)
	$(CC) $(OBJ_DIR)/*.o -o $(REL_DIR)/vmp $(CFLAGS) $(DFLAGS)
clean :
	rm -rf $(OBJ_DIR)