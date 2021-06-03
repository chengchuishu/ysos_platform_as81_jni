#include <cstdio>

#include "ysos_daemon.h"

/**
 * main  
 **/
int main(int argc, char* argv[]) 
{
    printf("==========start service [check] ============\n");
    //TODO: first-  init ysoslog config and create all init log files
    try{
          int ret = theApp.InitInstance();
          printf("[********]mian[********][theApp.InitInstance()][ret][%d]\n", ret);
          //TODO: second- auto run all services
          if(!ret){
            printf("[********]mian[********][theApp.InitInstance()][fail][ret][%d]\n", ret);
          } else {
            //  01.先运行该Link, 睡眠约50秒://NOLINT
            while (1) {
              boost::this_thread::sleep_for(boost::chrono::seconds(1000));
            }
          }
    } catch(...) {
       printf("Catch Exceptions in this app!!!");
    }
    //TODO: exit app
    theApp.ExitInstance();
    printf("==========start service [enter] ============\n");
    return 0;
}