/*
# ysos_daemon.cpp
# Definition of Cysos_daemonApp
# Created on: 2017-09-22 09:30:31
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170922, created by JinChengZhe
*/

// ysos_daemon.cpp : Defines the class behaviors for the application.
//

#ifdef _WIN32
  //#
#else
  #include <stdlib.h>
  #include <dlfcn.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <unistd.h> //work direc
  #include <boost/filesystem.hpp>
#endif

/// Private Headers
#include "ysos_daemon.h"
/// Platform Headers
#include "../../../protect/include/sys_daemon_package/externcommon.h"
//#include "../../../protect/include/sys_daemon_package/minidump.h"
using namespace ysos::extern_common;

static const unsigned int g_buffer_size = 8192;
#ifdef _DEBUG
static const char *g_sys_daemon_package_dll = _T("libsys_daemon_package_d.so");
#else
static const char *g_sys_daemon_package_dll = ("libsys_daemon_package.so");
#endif // _DEBUG

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Cysos_daemonApp
int MAX_PATH = 260;
log4cplus::Logger logger_;

//add for linux 
int GetModuleFileName( char* sModuleName, char* sFileName, int nSize)
{
 int ret = -1;
 //* one way
 char szBuf[nSize];
 int nCount;
 
 memset(szBuf, 0, sizeof(szBuf));
 nCount=readlink("/proc/self/exe", szBuf, nSize);
 if(nCount<0 || nCount>=nSize)
 {
    printf("failed\n");
 } else {
    ret = nCount;
 }
 szBuf[nCount]='\0';
 //printf("/proc/self/exe->[%s]\n", szBuf);
 //*/
 strncpy(sFileName,szBuf,nSize);
 return ret;
}

bool PathRemoveFileSpec (char* sMoudlePath)
{
    char* path;
    path = (char*)malloc(MAX_PATH + 1);
    char* p = strrchr(sMoudlePath,'/');
    ++p;
    char* it = sMoudlePath;
    int i = 0;
    for(i = 0; it < p-1; ++i, ++it)
        path[i] = *it;
    path[i] = '\0';
    //printf("..\\PathRemoveFileSpec() is :[%s]\n",path);
    strncpy(sMoudlePath,path,MAX_PATH);
    return true;
}

bool PathAppend(char* fpath, const char* tfilename)
{
    char fullpath[MAX_PATH];
    sprintf(fullpath,"%s%s", fpath, tfilename);
    strncpy(fpath,fullpath, MAX_PATH);
    return true;
}

bool PathAddExtension(char* fpath, const char* tfilename)
{
    char fullpath[MAX_PATH];
    sprintf(fullpath,"%s%s", fpath, tfilename);
    strncpy(fpath,fullpath, MAX_PATH);
    return true;
}

bool PathFileExists(const std::string& tfpath)
{
    bool ret = false;
    ret = boost::filesystem::exists(tfpath.c_str());
    return ret;
}

void PathRemoveExtension(char* tmpfpath)
{
    boost::filesystem::path path = tmpfpath; //add for linux
    strcpy(tmpfpath, path.replace_extension("").c_str());
    return;
}

bool CopyFile(const std::string& fromfpath, char* tofpath, boost::filesystem::copy_option coption, boost::system::error_code &ec)
{
    //TODO:can change other params for copy
    //boost::filesystem::copy_option option;
    //boost::system::error_code ec;
    boost::filesystem::copy_file ( fromfpath, tofpath, coption, ec);//add for linux  
    return true;
}

int GetEnvironmentVariable(const char* vname,char* system_path, int g_buffer_size)
{
    int ret = 0;
    char tgbuf[g_buffer_size];
    memset(tgbuf, 0, sizeof(tgbuf));
    char* vpath = getenv(vname);
    printf("%s\n",vpath);
    strncpy(system_path, tgbuf, g_buffer_size);
    ret = 1;
    return ret;
}


/// 全局变量声明
static const char* const g_guid = ("Global\\{77A3B103-6859-4A45-9372-EE538EF7FACF}");


// Cysos_daemonApp construction

Cysos_daemonApp::Cysos_daemonApp()
  : mutex_handle_(NULL),
    module_handle_(NULL),
    fp_start_ysos_daemon_ptr_(NULL),
    fp_stop_daemon_module_ptr_(NULL) {
  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::Cysos_daemonApp[Enter]\n");
  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::Cysos_daemonApp[Exit]\n");
}

// The one and only Cysos_daemonApp object

Cysos_daemonApp theApp;


// Cysos_daemonApp initialization

bool Cysos_daemonApp::InitInstance() {
  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Enter]\n");

  /// for debug
//  ::MessageBox(NULL, _T("debug"), _T("debug"), MB_OK);

  ///  判断是否已运行，防止重复运行
  //TODO:add for linux
  
  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Check Point][0]\n");

  ///  设置dump机制
  //TODO:add for linux

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Check Point][1]\n");

  ///  初始化本地日志属性
  char module_file_name[MAX_PATH] = {0};
  if (0 == GetModuleFileName(NULL, module_file_name, MAX_PATH)) {
    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Fail][The function[GetModuleFileName()] is failed][last_error_code][%s]\n", "GetLastError()");
    return false;
  }

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[module_file_name][%s]\n", module_file_name);

  if (true != PathRemoveFileSpec(module_file_name)) {
    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Fail][The function[PathRemoveFileSpec()] is failed][last_error_code][%s]\n", "GetLastError()");
    return false;
  }

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[module_file_name][%s]\n", module_file_name);

  if (true != PathRemoveFileSpec(module_file_name)) {
    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Fail][The function[PathRemoveFileSpec()] is failed][last_error_code][%s]\n", "GetLastError()");
    return false;
  }

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[module_file_name][%s]\n", module_file_name);

  if (true != PathAppend(module_file_name, "/log/ysos_daemon.log")) {
    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Fail][The function[PathRemoveFileSpec()] is failed][last_error_code][%s]\n", "GetLastError()");
    return false;
  }

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[module_file_name][%s]\n", module_file_name);

  if (true != ysos::log::InitLoggerExt(module_file_name, "ysos_daemon")) {
    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Fail]The function[ysos::log::InitLogger()] is failed\n");
    return false;
  }

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Check Point][2]\n");

  ///  判断logger对象是否可用
  logger_ = ysos::log::GetLogger("ysos_daemon");
  if (true != logger_.valid()) {
    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Fail]The function[ysos::log::GetLogger()] is failed\n");
    return false;
  }

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Check Point][3]\n");
  YSOS_LOG_DEBUG("Cysos_daemonApp::InitInstance[Check Point][2]");

  ///  设置进程运行环境变量
  if (true != SetYSOSEnvironmentVariable()) {
    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Fail][SetYSOSEnvironmentVariable()]\n");
    YSOS_LOG_DEBUG("Cysos_daemonApp::InitInstance[Fail]The function[SetYSOSEnvironmentVariable()] is failed");
    return false;
  }

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Check Point][4]\n");
  YSOS_LOG_DEBUG("Cysos_daemonApp::InitInstance[Check Point][3]");

  ///  加载sys_daemon_package.dll并获取公有函数指针
  if (true != LoadLibraryAndSetGetFunctionPtr()) {
    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Fail][LoadLibraryAndSetGetFunctionPtr()]\n");
    YSOS_LOG_DEBUG("Cysos_daemonApp::InitInstance[Fail]The function[LoadLibraryAndSetGetFunctionPtr()] is failed");
    return false;
  }

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Check Point][5]\n");
  YSOS_LOG_DEBUG("Cysos_daemonApp::InitInstance[Check Point][4]");

  ///  运行MainFrame
  #pragma region

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Check Point][6]\n");
  YSOS_LOG_DEBUG("Cysos_daemonApp::InitInstance[Check Point][5]");

  //TODO: add for linux
  ///  判断打开守护模块函数指针
  if (NULL == (fp_start_ysos_daemon_ptr_)) {
    printf("YSOS_DAEMON_EXE::CMainFrame::OnCreate[Fail][(dynamic_cast<Cysos_updateApp*>(AfxGetApp()))->fp_open_ptr_ is NULL]");
    YSOS_LOG_DEBUG("CMainFrame::OnCreate[Fail][(dynamic_cast<Cysos_updateApp*>(AfxGetApp()))->fp_open_ptr_ is NULL]");
    return false;
  }

  ///  打开守护模块
  if (0 != (fp_start_ysos_daemon_ptr_())) {
    printf("YSOS_DAEMON_EXE::CMainFrame::OnCreate[Fail][The function[(dynamic_cast<Cysos_updateApp*>(AfxGetApp()))->fp_open_ptr_()] is failed]");
    YSOS_LOG_DEBUG("CMainFrame::OnCreate[Fail]The function[(dynamic_cast<Cysos_updateApp*>(AfxGetApp()))->fp_open_ptr_()] is failed");
    return false;
  }


  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Check Point][7]\n");
  YSOS_LOG_DEBUG("Cysos_daemonApp::InitInstance[Check Point][6]");

  
  #pragma endregion

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::InitInstance[Exit]\n");
  YSOS_LOG_DEBUG("Cysos_daemonApp::InitInstance[Exit]");

  return true;
}

int Cysos_daemonApp::ExitInstance() {
  //TODO: handle additional resources you may have added

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::ExitInstance[Enter]\n");

  /// 判断dll句柄是否为空，不为空释放dll
  if (NULL != module_handle_) {
    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::ExitInstance[module_handle_ is not NULL]\n");
    
    //if (true !=  FreeLibrary(module_handle_)) {
    if ( true !=  dlclose ( module_handle_ ) ) {//add for linux
      printf("YSOS_DAEMON_EXE::Cysos_daemonApp::ExitInstance[Fail][FreeLibrary()][last_error_code][%s]\n", "GetLastError()");
    }

    module_handle_ = NULL;
    fp_start_ysos_daemon_ptr_ = NULL;
    fp_stop_daemon_module_ptr_ = NULL;
  }

  /// 判断mutex句柄是否为空，不为空关闭
  if (NULL != mutex_handle_) {
    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::ExitInstance[Check Point][mutex_handle_ is not NULL]\n");

    //if (true != CloseHandle(mutex_handle_)) 
    if ( true != dlclose ( mutex_handle_ ) )// add for linux
    {
      printf("YSOS_DAEMON_EXE::Cysos_daemonApp::ExitInstance[Fail][CloseHandle()][last_error_code][%s]\n", "GetLastError()");
    }

    mutex_handle_ = NULL;
  }

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::ExitInstance[Exit]\n");

  return 0;
}

// CAboutDlg dialog used for App About

// Cysos_daemonApp message handlers

// Custom Define Function
#pragma region
bool Cysos_daemonApp::SetYSOSEnvironmentVariable() {
  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::SetYSOSEnvironmentVariable[Enter]\n");

  bool result = false;
  int MAX_PATH = 260;
  do {
    /// 获取系统path
    char system_path[g_buffer_size] = {0};
    char* tval = getenv("PATH");
    printf("[%s]\n", getenv("PATH"));
    if ( NULL == tval ) //add for linux
    {
        printf( "YSOS_DAEMON_EXE::Cysos_daemonApp::SetYSOSEnvironmentVariable[Fail][GetEnvironmentVariable()][val][%s]\n",tval);
        break;
    } else {
        strcpy(system_path, tval);
    }

    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::SetYSOSEnvironmentVariable[Check Point][0][system_path][%s]\n", system_path);

    /// 获取模块文件路径
    char module_file_name[MAX_PATH*2] = {0};
    if (0 == GetModuleFileName(NULL, module_file_name, MAX_PATH*2)) {
      printf("YSOS_DAEMON_EXE::Cysos_daemonApp::SetYSOSEnvironmentVariable[Fail][GetModuleFileName()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::SetYSOSEnvironmentVariable[Check Point][1][module_file_name][%s]\n", module_file_name);

    if (true != PathRemoveFileSpec(module_file_name)) {
      printf("YSOS_DAEMON_EXE::Cysos_daemonApp::SetYSOSEnvironmentVariable[Fail][PathRemoveFileSpec()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::SetYSOSEnvironmentVariable[Check Point][2][module_file_name][%s]\n", module_file_name);

    //if (true != SetCurrentDirectory(module_file_name)) {
    if ( -1 == chdir ( module_file_name ) ) { //add for linux
      printf("YSOS_DAEMON_EXE::Cysos_daemonApp::SetYSOSEnvironmentVariable[Fail][SetCurrentDirectory()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::SetYSOSEnvironmentVariable[Check Point][3][module_file_name][%s]\n", module_file_name);

    char variable_name[] = ("path");
    char variable_value[g_buffer_size] = {0};
    //StringCchPrintf(variable_value, g_buffer_size, _T("%s;%s\\..\\os_lib;%s\\..\\lib;%s\\..\\extern_lib;%s;"), module_file_name, module_file_name, module_file_name, module_file_name, system_path);
    sprintf(variable_value, ("%s;%s/../lib;%s/../extern_lib;%s;"), module_file_name, module_file_name, module_file_name, system_path); //add for linux
    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::SetYSOSEnvironmentVariable[Check Point][4][variable_value][%s]\n", variable_value);

    //if (true != SetEnvironmentVariable(variable_name, variable_value)) {
    if ( 0 != setenv ( variable_name, variable_value, 1) ) { //add for linux
      printf("YSOS_DAEMON_EXE::Cysos_daemonApp::SetYSOSEnvironmentVariable[Fail][SetEnvironmentVariable()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::SetYSOSEnvironmentVariable[Check Point][End]\n");

    result = true;
  } while (0);

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::SetYSOSEnvironmentVariable[Exit]\n");

  return result;
}

bool Cysos_daemonApp::LoadLibraryAndSetGetFunctionPtr() {
  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::LoadLibraryAndSetGetFunctionPtr[Enter]\n");

  bool result = false;

  do {
    #ifdef _WIN32
      module_handle_ = LoadLibrary(g_sys_daemon_package_dll);
    #else
      //open so lib
      module_handle_ = dlopen ( g_sys_daemon_package_dll, RTLD_LAZY);
      printf("YSOS_DAEMON_EXE::Cysos_daemonApp::LoadLibraryAndSetGetFunctionPtr[module_handle_][%p]\n", module_handle_);
    #endif
    if (NULL == module_handle_) {
      printf("YSOS_DAEMON_EXE::Cysos_daemonApp::LoadLibraryAndSetGetFunctionPtr[Fail][module_handle_ is NULL][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::LoadLibraryAndSetGetFunctionPtr[Check Point][0]\n");
    #ifdef _WIN32 
      fp_start_ysos_daemon_ptr_ = (ysos::sys_daemon_package::FP_StartYSOSDaemonModule)GetProcAddress(module_handle_, "StartYSOSDaemonModule");
    #else
      fp_start_ysos_daemon_ptr_ = (ysos::sys_daemon_package::FP_StartYSOSDaemonModule)dlsym(module_handle_, "StartYSOSDaemonModule");
    #endif
    if (NULL == fp_start_ysos_daemon_ptr_) {
      printf("YSOS_DAEMON_EXE::Cysos_daemonApp::LoadLibraryAndSetGetFunctionPtr[Fail][fp_start_ysos_daemon_ptr_ is NULL][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::LoadLibraryAndSetGetFunctionPtr[Check Point][1]\n");
    #ifdef _WIN32
      fp_stop_daemon_module_ptr_ = (ysos::sys_daemon_package::FP_StopYSOSDaemonModule)GetProcAddress(module_handle_, "StopYSOSDaemonModule");
    #else
      fp_stop_daemon_module_ptr_ = (ysos::sys_daemon_package::FP_StopYSOSDaemonModule)dlsym(module_handle_, "StopYSOSDaemonModule");
    #endif  
    if (NULL == fp_stop_daemon_module_ptr_) {
      printf("YSOS_DAEMON_EXE::Cysos_daemonApp::LoadLibraryAndSetGetFunctionPtr[Fail][fp_stop_daemon_module_ptr_ is NULL][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("YSOS_DAEMON_EXE::Cysos_daemonApp::LoadLibraryAndSetGetFunctionPtr[Check Point][End]\n");

    result = true;
  } while (0);

  printf("YSOS_DAEMON_EXE::Cysos_daemonApp::LoadLibraryAndSetGetFunctionPtr[Exit]\n");

  return result;
}
#pragma endregion


#pragma region 

#pragma endregion