##最低版本号要求##
cmake_minimum_required (VERSION 2.8)

##project info##
SET(PACKAGE_NAME tests)

#set compile flags
#添加-fPIC编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

##Enable C++11##
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

##LIB_DIR##
#set(LIB_DIR "/usr/local/lib")
set(BASE_LIB_DIR ${PROJECT_SOURCE_DIR}/thirdparty/lib)
set(PUB_LIB_DIR ${PROJECT_SOURCE_DIR}/linux/lib)
set(EXT_LIB_DIR ${PROJECT_SOURCE_DIR}/linux/extern_lib)

##add inlude include_directories and link_libraries ##
#include_directories(. /usr/local/include ./public/include)
include_directories(. ${PROJECT_SOURCE_DIR}/thirdparty/include)

##add link_libraries_directories ##
link_directories(${BASE_LIB_DIR}/log4cplus ${BASE_LIB_DIR}/boost ${PUB_LIB_DIR} ${EXT_LIB_DIR})

#链接库
link_libraries(log4cplus json sys_framework_package os_hal_package core_help_package httpclient)

##查找当前目录下的所有源文件##
##并将名称保存到DIR_SRCS变量##
aux_source_directory(./src SRC_LIST)

##指定生成目标##
add_executable(${PACKAGE_NAME} ./src/main.cpp )

##把目标文件与库文件进行链接
target_link_libraries(${PACKAGE_NAME} json boost_system boost_chrono boost_thread boost_locale boost_filesystem)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/linux/bin)