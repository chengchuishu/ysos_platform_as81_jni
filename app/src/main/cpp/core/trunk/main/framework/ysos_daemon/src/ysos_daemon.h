/*
# ysos_daemon.h
# Definition of Cysos_daemonApp
# Created on: 2017-01-19 14:23:23
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170119, created by JinChengZhe
*/

// ysos_daemon.h : main header file for the ysos_daemon application
//
#pragma once

//#ifndef __AFXWIN_H__
//#error "include 'stdafx.h' before including this file for PCH"
//#endif

#include <boost/thread.hpp>

/// Private Headers
//#include "resource.h"       // main symbols
/// Platform Headers
#include "../../../protect/include/sys_daemon_package/daemoninterface.h"

#include "../../../public/include/ysoslog/ysoslog.h"
extern log4cplus::Logger logger_;


class Cysos_daemonApp {
 public:
  Cysos_daemonApp();

  /// Custom Define Function
  #pragma region
  /**
  *@brief 设置YSOS 环境变量函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  bool SetYSOSEnvironmentVariable();
  /**
  *@brief 加载dll，并设置函数接口  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  bool LoadLibraryAndSetGetFunctionPtr();
  #pragma endregion

  /// Custom Define Variable
  #pragma region
  ysos::sys_daemon_package::FP_StartYSOSDaemonModule fp_start_ysos_daemon_ptr_;
  ysos::sys_daemon_package::FP_StopYSOSDaemonModule fp_stop_daemon_module_ptr_;
  #pragma endregion

 private:
  void* mutex_handle_;  ///< mutext handle，防止程序重复运行
  void* module_handle_;  ///< 动态加载dll的handle

 public:
  virtual bool InitInstance();
  virtual int ExitInstance();

};

extern Cysos_daemonApp theApp;
