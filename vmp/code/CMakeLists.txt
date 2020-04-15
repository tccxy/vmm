# cmake for vmp
cmake_minimum_required(VERSION 3.5.1)
project(vmp)
# name
set(PROJECT_NAME vmp)


option(ZYNQMP "option for zynqmp" OFF)
option(ZQ7020 "option for zq7020" OFF)
option(RK3399 "option for rk3399" OFF)


# set cmake cross compile
#set(CMAKE_CROSEECOMPILING  TRUE)
if(ZQ7020)
    set(CMKA_SYSTRM_NAME       Linux)
    set(CMAKE_C_COMPILER       arm-linux-gnueabihf-gcc)
    #set(CMAKE_CXX_COMPILER     arm-linux-gnueabihf-g++)
    set(LIB lib-32)
else()
    set(CMKA_SYSTRM_NAME       Linux)
    set(CMAKE_C_COMPILER       aarch64-linux-gnu-gcc)
    #set(CMAKE_CXX_COMPILER     aarch64-linux-gnu-g++)
    set(LIB lib)
endif(ZQ7020)

# add define value
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -rdynamic -O2 -Wall -Wno-pointer-to-int-cast\
                            -Wno-incompatible-pointer-types -Wno-pointer-sign\
                            -Wno-missing-braces -Wno-format")

# add message
message(STATUS "PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR})
message(STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})
#message(SEND_ERROR "this is a SEND_ERROR")
#message(FATAL_ERROR "this is a FATAL_ERROR")

# add src
aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRC)

# add include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# set lib dir
set(cjson_install ${PROJECT_SOURCE_DIR}/3rd_opensrc/cjson_install/${LIB})
set(curl_install ${PROJECT_SOURCE_DIR}/3rd_opensrc/curl_install/${LIB})
set(openssl_install ${PROJECT_SOURCE_DIR}/3rd_opensrc/openssl_install/${LIB})
set(zlog_install ${PROJECT_SOURCE_DIR}/3rd_opensrc/zlog_install/${LIB})

link_directories(${cjson_install} ${curl_install} ${openssl_install} ${zlog_install})

set(3rd_lib cjson crypto ssl curl zlog pthread m crypt dl rt)

# set lib include

set(cjson_include ${PROJECT_SOURCE_DIR}/3rd_opensrc/cjson_install/include)
set(curl_include ${PROJECT_SOURCE_DIR}/3rd_opensrc/curl_install/include)
set(openssl_include ${PROJECT_SOURCE_DIR}/3rd_opensrc/openssl_install/include)
set(zlog_include ${PROJECT_SOURCE_DIR}/3rd_opensrc/zlog_install/include)

include_directories(${cjson_include} ${curl_include} ${openssl_include} ${zlog_include})

add_definitions(-DZYNQMP)

add_executable(${PROJECT_NAME} ${DIR_SRC})
target_link_libraries(${PROJECT_NAME} ${3rd_lib})

execute_process(COMMAND mkdir -p ${PROJECT_BINARY_DIR}/release)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/release)
file(COPY ${openssl_install}/libssl.so.1.1 DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${openssl_install}/libcrypto.so.1.1 DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${zlog_install}/libzlog.so.1.1 DESTINATION ${EXECUTABLE_OUTPUT_PATH})