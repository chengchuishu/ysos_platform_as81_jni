##最低版本号要求##
cmake_minimum_required (VERSION 2.8)

##project info##
SET(PACKAGE_NAME httpclientconnection)

#------------------------------------------[0]----------------------------------------------------#
####库文件的默认输出路径，这里设置为工程目录下的lib目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
        ${PROJECT_SOURCE_DIR}/android/${ANDROID_ABI}/plugins)
#-------------------------------------------------------------------------------------------------#

#set compile flags
#添加-fPIC编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

##Enable C++11##
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

##LIB_DIR##
#set(LIB_DIR "/usr/local/lib")
#set(BASE_LIB_DIR ${PROJECT_SOURCE_DIR}/thirdparty/lib)
#set(PUB_LIB_DIR ${PROJECT_SOURCE_DIR}/linux/lib)

##add inlude include_directories and link_libraries ##
#include_directories(. /usr/local/include ./public/include)
include_directories(. ${PROJECT_SOURCE_DIR}/thirdparty/include)

######################################################################################
##add link_libraries_directories ##
####link_directories(${BASE_LIB_DIR}/log4cplus ${PUB_LIB_DIR})
#链接库
####link_libraries(log4cplus sys_framework_package os_hal_package core_help_package)
######################################################################################

#------------------------------------------[1]----------------------------------------------------#
####添加第三方库 log4cplus sys_framework_package os_hal_package core_help_package)
add_library(log4cplus SHARED IMPORTED)
add_library(curl STATIC IMPORTED)
add_library(core_help_package SHARED IMPORTED)
add_library(os_hal_package SHARED IMPORTED)
####添加库的路径
set_target_properties(curl
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/thirdparty/lib/curl/${ANDROID_ABI}/libcurl.a)
set_target_properties(log4cplus
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/thirdparty/lib/log4cplus/${ANDROID_ABI}/liblog4cplus.so)
set_target_properties(core_help_package
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/android/${ANDROID_ABI}/lib/libcore_help_package.so)
set_target_properties(os_hal_package
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/android/${ANDROID_ABI}/lib/libos_hal_package.so)
#-------------------------------------------------------------------------------------------------#

##查找当前目录下的所有源文件##
##并将名称保存到DIR_SRCS变量##
aux_source_directory(./src SRC_LIST)

#------------------------------------------[2]----------------------------------------------------#
####添加自己的so库test-lib
add_library( # Sets the name of the library.
        ${PACKAGE_NAME}

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        #src/ysos.cpp
        ${SRC_LIST})

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log )
#####添加链接
target_link_libraries( # Specifies the target library.
        ${PACKAGE_NAME}
        curl
        log4cplus
        core_help_package
        os_hal_package

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} )
#-------------------------------------------------------------------------------------------------#