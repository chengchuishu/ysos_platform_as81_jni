/**
 *@file main.cpp
 *@brief Definition of main
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

//  所有的单例均应在此文件中创建  //NOLINT

#include "../../../protect/include/sys_platform_sdk/sysinit.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"

//add for linux
/* #define BOOST_LOG_DYN_LINK 1 */

//  1, Enable; 0, Disable
#define ENABLE_DETECT_MEMORY_LEAK                               0
#define DETECT_MEMORY_LEAK_BY_VLD                               0

#if ENABLE_DETECT_MEMORY_LEAK
#if DETECT_MEMORY_LEAK_BY_VLD
#include "vld.h"
#else
#include <crtdbg.h>
#endif  //  DETECT_MEMORY_LEAK_BY_VLD
#endif  //  ENABLE_DETECT_MEMORY_LEAK

#include <boost/thread.hpp>

#include "../../../protect/include/sys_framework_package/platforminfo.h"
//#include "../../../../protect/include/sys_framework_package/baseappmanagerimpl.h"
#include "../../../public/include/sys_interface_package/callbackqueue.h"
//#include "../../../Components/modules/include/testmodule.h"

int main(int argc, char* argv[]) {
  // for debug
  boost::this_thread::sleep_for(boost::chrono::seconds(25));

  std::string ip_addr = "127.0.0.1";
  unsigned short port = 6002;
  GetSysInit()->Init("ysos.conf"/*, ip_addr, port*/);


  ysos::GetCallbackInterfaceManager()->Dump();
  std::cout << "-----------------------------------------------\n";
  ysos::CallbackInterfacePtr callback = ysos::GetCallbackInterfaceManager()->FindInterface("StateMachineCommonCallback");
  if (NULL == callback) {
    std::cout << "null\n";
  } else {
    callback->Callback(NULL, NULL);
    std::cout << "success\n";
  }

  std::cout << "-----------------------------------------------\n";
  ysos::GetDriverInterfaceManager()->Dump();
  std::cout << "-----------------------------------------------\n";

  //  01.先运行该Link, 睡眠约50秒://NOLINT
  while (1) {
    // boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(65000));
    boost::this_thread::sleep_for(boost::chrono::seconds(1000));
  }

  GetSysInit()->Uninit();

  return 0;
}
