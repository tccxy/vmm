OBJS := $(OBJ_DIR)/mmc.o \
		$(OBJ_DIR)/crc.o \
		$(OBJ_DIR)/md5.o \
		$(OBJ_DIR)/ftp.o \
		$(OBJ_DIR)/http.o \
		$(OBJ_DIR)/vmm_deal.o \
		$(OBJ_DIR)/vmm.o \



.PHONY : all

all : $(OBJS) 

$(OBJ_DIR)/mmc.o : ./mmc.c
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@

$(OBJ_DIR)/crc.o : ./crc.c
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@

$(OBJ_DIR)/md5.o : ./md5.c
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@

$(OBJ_DIR)/ftp.o : ./ftp.c
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@

$(OBJ_DIR)/http.o : ./http.c
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@

$(OBJ_DIR)/vmm_deal.o : ./vmm_deal.c
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@

$(OBJ_DIR)/vmm.o : ./vmm.c
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@