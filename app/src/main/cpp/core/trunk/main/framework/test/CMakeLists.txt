##最低版本号要求##
cmake_minimum_required (VERSION 2.8)

##project info##
set(EXE_NAME test_main)

#------------------------------------------[0]----------------------------------------------------#
####库文件的默认输出路径，这里设置为工程目录下的lib目录
set(EXECUTABLE_OUTPUT_PATH
        ${PROJECT_SOURCE_DIR}/android/${ANDROID_ABI}/bin)
#-------------------------------------------------------------------------------------------------#

#set compile flags
##set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wno-deprecated -pg")

## add compile params  ##
#### -DDEBUG  open   assert alias debug   ####
#### -DNDEBUG ignore assert alias release ####
add_definitions(-DNDEBUG)

##Enable C++11##
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

##LIB_DIR##
#set(LIB_DIR "/usr/local/lib")
####set(BASE_LIB_DIR ${PROJECT_SOURCE_DIR}/thirdparty/lib)
####set(PUB_LIB_DIR ${PROJECT_SOURCE_DIR}/linux/lib)

##add inlude include_directories and link_libraries ##
include_directories(. ${PROJECT_SOURCE_DIR}/thirdparty/include)

######################################################################################
##添加非标准的共享库搜索路径
####link_directories(${BASE_LIB_DIR}/log4cplus ${PUB_LIB_DIR})
######################################################################################
#------------------------------------------[1]----------------------------------------------------#
####添加第三方库 log4cplus ysoslog boost_system boost_chrono boost_thread boost_filesystem dl sys_platform_sdk)
add_library(socket_help_package SHARED IMPORTED)
#3.添加库的路径
set_target_properties(socket_help_package
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/android/${ANDROID_ABI}/lib/libsocket_help_package.so)
#-------------------------------------------------------------------------------------------------#

##查找当前目录下的所有源文件####并将名称保存到DIR_SRCS变量##
aux_source_directory(./src SRC_LIST)

#------------------------------------------[2]----------------------------------------------------#
####添加自己的so库test-lib
#add_library( # Sets the name of the library.
#        ${EXE_NAME}
#
#        # Sets the library as a shared library.
#        SHARED
#
#        # Provides a relative path to your source file(s).
#        #${SRC_LIST}
#        src/main.cpp
#        src/ysos.cpp
#        src/externutility.cpp)

add_executable(${EXE_NAME} src/test_main.cpp)

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log )
#####添加链接
target_link_libraries( # Specifies the target library.
        ${EXE_NAME}
        socket_help_package

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} )
#-------------------------------------------------------------------------------------------------#