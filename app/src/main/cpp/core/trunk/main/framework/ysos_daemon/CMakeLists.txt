##最低版本号要求##
cmake_minimum_required (VERSION 2.8)

##project info##
set(EXE_NAME ysos_daemon)

#set compile flags
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wno-deprecated -pg -llog4cplus -lysoslog -lboost_system -lboost_chrono -lboost_thread -lboost_filesystem -ldl -lsys_platform_sdk -lsys_framework_package -lprotocol_help_package -los_hal_package -lcore_help_package")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wno-deprecated -pg")

##Enable C++11##
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
##LIB_DIR##
#set(LIB_DIR "/usr/local/lib")
set(BASE_LIB_DIR ${PROJECT_SOURCE_DIR}/thirdparty/lib)
set(PUB_LIB_DIR ${PROJECT_SOURCE_DIR}/linux/lib)

##add inlude include_directories and link_libraries ##
#include_directories(. /usr/local/include)
include_directories(. ${PROJECT_SOURCE_DIR}/thirdparty/include)

##添加非标准的共享库搜索路径
link_directories(${BASE_LIB_DIR} ${PUB_LIB_DIR})

##查找当前目录下的所有源文件####并将名称保存到DIR_SRCS变量##
aux_source_directory(./src SRC_LIST)

##指定生成目标##
add_executable(${EXE_NAME} ${SRC_LIST})

##把目标文件与库文件进行链接
target_link_libraries(${EXE_NAME} log4cplus ysoslog boost_system boost_chrono boost_thread boost_filesystem dl sys_daemon_package)
#target_link_libraries(${EXE_NAME} log4cplus tinyxml2 pthread -lstdc++)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/linux/bin)