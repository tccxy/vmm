BUILD_DIR = ./
BUILD_EXEC_DIR = ${CJSON_INSTALL}
SRC_DIR = ${CJSON_UNPACK}/cJSON
CFLAGS = 

LDFLAGS = 
OBJS = ${SRC_DIR}/cJSON.o 
LIB = ${BUILD_EXEC_DIR}/lib/libcjson.a
#TARGET = cjson
all:${LIB} 
	

${SRC_DIR}/%.o: ${SRC_DIR}/%.c
	${CC} -c $< -o $@ ${CFLAGS} -I${SRC_DIR} ${LDFLAGS}

${LIB}: ${OBJS}
	rm -f $@
	mkdir -p ${BUILD_EXEC_DIR}/lib
	${AR} crs ${LIB} ${OBJS}
	#mv ${LIB} ${BUILD_EXEC_DIR}
	
#${TARGET}: ${OBJS}
#	${CC} -o $@ ${OBJS}
	
install:
	mkdir -p ${BUILD_EXEC_DIR}/include
	cp ${SRC_DIR}/cJSON.h ${BUILD_EXEC_DIR}/include

uninstall:

