export 3RD_ORI_DIR	:=	$(3RD_DIR)/ori

export CURL_ORI 	:=	$(3RD_ORI_DIR)/curl-7.51.0.tar.gz
export OPENSSL_ORI	:=	$(3RD_ORI_DIR)/openssl-1.1.0e.tar.gz
export CJSON_ORI 	:=	$(3RD_ORI_DIR)/cjson-200908.tar.gz
export ZLOG_ORI 	:=	$(3RD_ORI_DIR)/zlog-1.2.15.zip

export 3RD_MODIFY_DIR	:=	$(3RD_DIR)/modify
export OPENSSL_UNPACK	:=	$(3RD_MODIFY_DIR)/openssl-1.1.0e
export OPENSSL_UPDATE	:=	$(3RD_MODIFY_DIR)/openssl-update
export CURL_UNPACK	    :=	$(3RD_MODIFY_DIR)/curl-7.51.0
export CURL_UPDATE   	:=	$(3RD_MODIFY_DIR)/curl-update
export CJSON_UNPACK	    :=	$(3RD_MODIFY_DIR)/cjson-200908
export CJSON_UPDATE   	:=	$(3RD_MODIFY_DIR)/cjson-update
export ZLOG_UNPACK	    :=	$(3RD_MODIFY_DIR)/zlog-1.2.15
export ZLOG_UPDATE   	:=	$(3RD_MODIFY_DIR)/zlog-update


.PHONY : all ready clean OPENSSL CURL CJSON ZLOG

all : ready clean OPENSSL CURL CJSON ZLOG

ready :

OPENSSL:
	mkdir -p ${OPENSSL_INSTALL}
	tar -xvf $(OPENSSL_ORI) -C $(3RD_MODIFY_DIR)
	cd $(OPENSSL_UPDATE) && chmod +x *.sh && ./update_openssl.sh

CURL :
	@echo "CURL_ORI =${CURL_ORI}"
	mkdir -p ${CURL_INSTALL}
	tar -zxvf $(CURL_ORI) -C $(3RD_MODIFY_DIR)
	cd $(CURL_UPDATE) && chmod +x *.sh && ./update_curl.sh

CJSON :
	@echo "CJSON_ORI =${CJSON_ORI}"
	mkdir -p ${CJSON_INSTALL}
	tar -zxvf $(CJSON_ORI) -C $(3RD_MODIFY_DIR)
	cd $(CJSON_UPDATE) && chmod +x *.sh && ./update_cjson.sh

ZLOG :
	@echo "ZLOG_ORI =${ZLOG_ORI}"
	mkdir -p ${ZLOG_INSTALL}
	unzip $(ZLOG_ORI) -d $(3RD_MODIFY_DIR)
	cd $(ZLOG_UPDATE) && chmod +x *.sh && ./update_zlog.sh

clean :
	@echo "clean"
	rm ${OPENSSL_UNPACK} -rf
	rm ${CURL_UNPACK} -rf
	rm ${CJSON_UNPACK} -rf
	rm ${ZLOG_UNPACK} -rf
	rm -rf $(3RD_INSATLL_DIR)