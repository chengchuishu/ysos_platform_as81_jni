/**
 *@file autoselfstrategycallback.cpp
 *@brief the auto self strategy call back
 *@version 1.0
 *@author z.gg.
 *@date Created on: 2020-5-26 10:00:00
 *@copyright Copyright (c) 2020 YS Technology. All rights reserved.
 *
 */
/// Private Headers //  NOLINT
#include "../include/autoselfstrategycallback.h"

#include <stdio.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/locale.hpp>

#include "../../../public/include/common/commonenumdefine.h"

#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

#define INFINITE          0x7fffffff
#define MAX_MULTIPLIER    6
#ifdef _WIN32
const char *TASK_FILE           = "C:\\TDRobot\\YSOS\\data\\robottask\\task.xml";
#else
const char *TASK_FILE           = "../data/robottask/task.xml"; //WORK TASK DIRECTIOR
#endif

const char *TTS_MODULE          = "default@TtsExtModule";
const char *TTS_IOCTL_ID        = "8139";

const char *ROBOT_STATE           = "robot_state";
const char *ROBOT_STATE_NULL      = "null";
const char *ROBOT_CRUISE_SERVE    =  "cruise_serve";
const char *ROBOT_GO_HOME         = "go_home";
const char *ROBOT_CHARGING        = "charging";
const char *ROBOT_FAIL_CHARGE     = "failcharge";

const char *WORK_STATIC_SEARCH =  "WaitingService@TargetSearching@StaticSearching";
const char *WORK_LOCAL_SEARCH  =  "WaitingService@TargetSearching@LocalSearching";
const char *WORK_SPOT_SEARCH   =  "WaitingService@TargetSearching@SpotSearching";

enum ACTION_TYPE
{
    WORK_OFF      = 0,
    STATIC_SEARCH,
    LOCAL_SEARCH,
    SPOT_SEARCH
};

enum BATTERY_RANGE
{
    BATTERY_RANGE_VERY_LOW,
    BATTERY_RANGE_LOW,
    BATTERY_RANGE_MID,
    BATTERY_RANGE_HIGH
};

enum PRIORITY
{
    LOWEST  = 0,
    LOW,
    NORMAL,
    HIGH,
    HIGHEST
};

enum WORKMODE
{
    EVERYDAY,
    SPECIFIEDATE,
    FESTIVAL,
    WORKDAY,
    WEEKDAY
};

typedef struct
{
    int id;
    std::string name;
    std::string startTime;
}TASK_INFO;

typedef struct
{
    int id;
    std::string name;

    PRIORITY pri;
    std::string startDate;
    std::string endDate;
    std::string createTime;
    std::string version;

}WORK_PLAN_INFO;

#ifdef _WIN32
#else
/**
 *原型：char *itoa( int value, char *string,int radix);
 *value：欲转换的数据；string：目标字符串的地址；radix：转换后的进制数，可以是10进制、16进制等。
 *功能：把一个整数转换成字符串
 **/
static void itoa( int value, char *str, int radix) {
   sprintf(str, "%d", value);
   return;
}
#endif

namespace ysos {
    DECLARE_PLUGIN_REGISTER_INTERFACE ( AutoSelfStrategyCallback,  CallbackInterface );  ///<  插件的入口，一定要加上 //  NOLINT
    AutoSelfStrategyCallback::AutoSelfStrategyCallback ( const std::string &strClassName ) : BaseStrategyCallbackImpl ( strClassName )
        , task_run_idx_ ( 0 )
        , today_task_num_ ( 0 )
        , today_task_ ( NULL )
        , task_day_()
        , nxt_work_day_()
        , breakDate_ ( "" )
        , data_ptr_ ( GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY ) )
    {
        memset ( &nxt_work_day_, 0,  sizeof ( nxt_work_day_ ) );
        logger_ = GetUtility()->GetLogger ( "ysos.autoselfstrategy" );
         
        thread_run_self_ptr_ = NULL;
        atomic_thread_run_self_stop_flag_ =0;
    }

    AutoSelfStrategyCallback::~AutoSelfStrategyCallback (  void )
    {
        /*
        boost::lock_guard<boost::recursive_mutex> guard(rmutex_member_);
        if (thread_run_self_ptr_) {
            atomic_thread_run_self_stop_flag_ =1;
            thread_run_self_ptr_->interrupt();
            if (thread_run_self_ptr_->joinable()) {
            YSOS_LOG_INFO("wait for thread exit begin");
            thread_run_self_ptr_->join();
            YSOS_LOG_INFO("wait for thread exit end");
            }
            thread_run_self_ptr_ = NULL;
        }*/
    }

    int AutoSelfStrategyCallback::Initialize(void *param)
    {
        YSOS_LOG_INFO("in Initialize");

        /*
        boost::lock_guard<boost::recursive_mutex> guard(rmutex_member_);
        if (thread_run_self_ptr_) {
            bool have_finish = thread_run_self_ptr_->try_join_for(boost::chrono::milliseconds(0));
            if (have_finish) {
            YSOS_LOG_INFO("thread_run_self_ptr_ is run finish");
            thread_run_self_ptr_ = NULL;
            }
        }
        if (NULL == thread_run_self_ptr_) {
            YSOS_LOG_INFO("create thread ThreadFunRunSelf");
            thread_run_self_ptr_.reset(new boost::thread(&AutoSelfStrategyCallback::ThreadFunRunSelf, this));
        }*/

        return BaseStrategyCallbackImpl::Initialize(param);
    }
    
    void AutoSelfStrategyCallback::ThreadFunRunSelf(CallbackInterface *context)
    {
        YSOS_LOG_INFO("ThreadFunRunSelf begin [Enter]");
        //TODO: 独立线程去处理时间计划任务
        //CallbackInterface *context;
        //while (true)
        {
            /* code */
            std::string cur_machine_state;
            data_ptr_->GetData("machine_state", cur_machine_state);
            
            if (cur_machine_state == "main@WaitingService" ) {
                if ( CheckTaskReadyBefore() ) {
                    //TODO:change state to main@WaitingService@BeforeWork
                    RequestService("default@TemplateModule", boost::lexical_cast<std::string>(CMD_TRANSFER_EVENT), "text", "{\"type\":\"time_before_work\",\"data\":{\"key\":\"value\"}}", context);
                } else if ( YSOS_ERROR_SUCCESS == "getTaskReady()" ) {
                    //TODO:change state to main@WaitingService@StartWork
                    if ( today_task_[task_run_idx_].action_type == WORK_OFF )
                    {
                        // case1: work off
                        RequestService("default@TemplateModule", boost::lexical_cast<std::string>(CMD_TRANSFER_EVENT), "text", "{\"type\":\"time_off_work\",\"data\":{\"key\":\"value\"}}", context);
                    }
                    else
                    {
                        std::string work_type;
                        switch ( today_task_[task_run_idx_].action_type )
                        {
                            case LOCAL_SEARCH:
                                {
                                    char param[64];
                                    sprintf ( param, "{\"still_rotate_name\":\"%s\"}", today_task_[task_run_idx_].action_place.c_str() );
                                    work_type = WORK_LOCAL_SEARCH;
                                    break;
                                }
                            case STATIC_SEARCH:
                                work_type = WORK_STATIC_SEARCH;
                                break;
                            case SPOT_SEARCH:
                                work_type = WORK_SPOT_SEARCH;
                                break;
                        }
                        // case2 :
                        RequestService("default@TemplateModule", boost::lexical_cast<std::string>(CMD_TRANSFER_EVENT), "text", "{\"type\":\"time_start_work\",\"data\":{\"work_type\":\"" + work_type + "\"}}", context);
                    }
                } else {
                    // special case: no task now
                    RequestService("default@TemplateModule", boost::lexical_cast<std::string>(CMD_TRANSFER_EVENT), "text", "{\"type\":\"time_start_work\",\"data\":{\"work_type\":\"WaitingService@TargetSearching@StaticSearching\"}}", context);
                }
            }
            
            
        }
        

        YSOS_LOG_INFO("ThreadFunRunSelf begin [End]");
        return;
    }
    int AutoSelfStrategyCallback::HandleMessage ( const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "HandleMessage: " << event_name );
        if ( data_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "data_ptr_ is NULL!" );
            return YSOS_ERROR_FAILED;
        }
        //TODO: when event arrive, you can do action 
        // common process
        if ( event_name == "start_work_plan" )
        {
            //TODO: check action time condition
            StartWorkPlan();
        }
        else if ( event_name == "stop_work_plan" )
        {
            //TODO: check action time condition
            StopWorkPlan();
        }
        else if ( event_name == "pause_work_plan" )
        {
            //TODO: check action time condition
            PauseWorkPlan();
        }
        else if ( event_name == "resume_work_plan" )
        {
            //TODO: check action time condition
            ResumeWorkPlan();
        }
        else if ( event_name == "switch_task" )
        {
            //TODO: check action time condition
            SwitchTask("");
        }
        else if ( event_name == "start_task" )
        {
            //TODO: check action time condition
            StartTask("");
        }
        else if ( event_name == "stop_task" )
        {
            //TODO: check action time condition
            StopTask("");
        }
        else if ( event_name == "pause_task" )
        {
            //TODO: check action time condition
            PauseTask("");
        }
        else if ( event_name == "resume_task" )
        {
            //TODO: check action time condition
            ResumeTask("");
        }
        else if ( event_name == "cannel_task" )
        {
            //TODO: check action time condition
            CancelTask("");
        }
        else if ( event_name == "cannel_all_task" )
        {
            //TODO: check action time condition
            CancelAllTask();
        }
        else if ( event_name == "switch_scene" )
        {
            //TODO: check action time condition
            SwitchScene();
        }
        else if ( event_name == "entrance_scene" )
        {
            //TODO: check action time condition
            EntranceScene();
        }
        else if ( event_name == "exit_scene" )
        {
            //TODO: check action time condition
            ExitScene();
        }
        else if (event_name == "network_status_event" )
        {
            ThreadFunRunSelf(context);
        }
        else if ( event_name == "work_plan_updat" )
        {
            LoadWorkPlan ( boost::posix_time::microsec_clock::local_time().date() );
        }
        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::Initialized (  const std::string &key,  const std::string &value )
    {
        YSOS_LOG_DEBUG ( "Initialized " << key << ": " << value );
        int n_return = YSOS_ERROR_SUCCESS;
        if ( key == "robot_init_mode" )
        {
            LoadWorkPlan ( boost::posix_time::microsec_clock::local_time().date() );
        }
        else if ( key == "breakDate" )
        {
            breakDate_ = value;
        }
        return n_return;
    }


    int AutoSelfStrategyCallback::LoadWorkPlan (  const boost::posix_time::ptime::date_type &day )
    {
        /* It is very bad not to read files in a new thread. However we do it first. */
        YSOS_LOG_DEBUG ( "LoadTask" );
        // record the task day
        task_day_ = day;
        // clear
        task_run_idx_ = 0;
        today_task_num_ = 0;
        if ( today_task_ != NULL )
        {
            delete []today_task_;
            today_task_ = NULL;
        }
        nxt_work_day_.tm_year = 0; // nxt_work_day_ is invaild
        // read and parse
        std::ifstream fi;
        fi.open ( TASK_FILE, std::ios_base::in );
        if ( !fi )
        {
            YSOS_LOG_WARN ( "Read the task file failed!" );
            return YSOS_ERROR_FAILED;
        }
        Json::Reader read;
        Json::Value value;
        bool rst = read.parse ( fi, value, false );
        fi.close();
        if ( !rst )
        {
            YSOS_LOG_WARN ( "Parse the task file failed!" );
            return YSOS_ERROR_FAILED;
        }
        int num = value.size();
        if ( num == 0 )
        {
            YSOS_LOG_WARN ( "Task num is invaild!" );
            return YSOS_ERROR_FAILED;
        }
        YSOS_LOG_INFO ( "Task num = " << num );
        // search today
        int i = -1;
        for ( int left = 0, right = num - 1;  left <= right; )
        {
            int year, month, day;
            int mid = ( left + right ) >> 1;
            std::string date = value[mid]["date"].asString();
            sscanf ( date.c_str(), "%d-%d-%d", &year, &month,  &day );
            boost::posix_time::ptime::date_type temp ( year, month, day );
            if ( temp == task_day_ )
            {
                YSOS_LOG_DEBUG ( "todya " << date );
                i = mid;
                break;
            }
            if ( temp < task_day_ )
            {
                left = mid + 1;
            }
            else
            {
                right = mid - 1;
            }
        }
        // read today tasks
        Json::Value task, tmp, tmp1;
        if ( i == -1 )
        {
            YSOS_LOG_WARN ( "Today task not found!" );
            //Today task no found , exec default task by wangjx 如果没有task则添加当前日期任务与明天任务，任务时间为默认时间与任务
            //return YSOS_ERROR_FAILED;
        }
        else
        {
            task  = value[i]["task"];
        }
        int task_num = task.size();
        //YSOS_LOG_DEBUG("task_num " << task_num );
        if ( task_num == 0 )
        {
            YSOS_LOG_WARN ( "Toady task num is invaild!" );
            return YSOS_ERROR_FAILED;
        }
        today_task_ = new Task[task_num];
        if ( today_task_ == NULL )
        {
            YSOS_LOG_WARN ( "new task array failed!" );
            return YSOS_ERROR_FAILED;
        }
        today_task_num_ = 0;
        for ( int j = 0, &k = today_task_num_;  j < task_num; ++j )
        {
            std::string tm = task[j]["time"].asString();
            sscanf ( tm.c_str(), "%d:%d", &today_task_[k].action_hour,  &today_task_[k].action_minute );
            std::string act = task[j]["action"].asString();
            if ( act == "WorkOff" )
            {
                if ( k > 0  && today_task_[k - 1].action_type == WORK_OFF )
                {
                    YSOS_LOG_WARN ( "    redundant WorkOff" );
                    continue;
                }
                today_task_[k].action_type = WORK_OFF;
            }
            else if ( act == "StaticSearch" )
            {
                today_task_[k].action_type = STATIC_SEARCH;
            }
            else if ( act == "LocalSearch" )
            {
                today_task_[k].action_type = LOCAL_SEARCH;
            }
            else if ( act == "SpotSearch" )
            {
                today_task_[k].action_type = SPOT_SEARCH;
            }
            else
            {
                YSOS_LOG_WARN ( "    unknown task " << act );
                continue;
            }
            YSOS_LOG_DEBUG ( "  got task: " << tm << " " << act << " " << today_task_[k].action_place );
            ++k;
        }

        return YSOS_ERROR_SUCCESS;
    }


    int AutoSelfStrategyCallback::StartWorkPlan()
    {
        YSOS_LOG_DEBUG ( "请求-开始计划 " );

        YSOS_LOG_DEBUG ( "通知-计划已开始 " );

        //TODO:遍历计划并顺序执行计划内容
        YSOS_LOG_DEBUG ( "执行计划内容 " );
        std::string tempTaskName;
        for( int i = 0; i < 5; i++)
        {
            tempTaskName = "";
        }
        int ret = StartTask(tempTaskName);
        if(!ret) {
            YSOS_LOG_DEBUG ( "计划已完成 " );
        } else {
            YSOS_LOG_DEBUG ( "计划未完成 " );
        }
        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::StopWorkPlan()
    {
        YSOS_LOG_DEBUG ( "请求-停止计划 " );

        YSOS_LOG_DEBUG ( "通知-计划已停止 " );

        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::PauseWorkPlan()
    {
        YSOS_LOG_DEBUG ( "请求-暂停计划 " );

        YSOS_LOG_DEBUG ( "通知-计划已暂停 " );

        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::ResumeWorkPlan()
    {
        YSOS_LOG_DEBUG ( "请求-恢复计划 " );

        YSOS_LOG_DEBUG ( "通知-计划已恢复 " );

        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::SwitchTask(std::string taskName)
    {
        YSOS_LOG_DEBUG ( "请求-切换任务 " );

        YSOS_LOG_DEBUG ( "通知-任务已切换 " );

        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::StartTask(std::string taskName)
    {
        YSOS_LOG_DEBUG ( "请求-开始任务 " );

        YSOS_LOG_DEBUG ( "通知-任务已开始 " );

        //TODO:执行任务内容 -- doSomething

        if(taskName == "")
        {
            YSOS_LOG_DEBUG("do action -> " << taskName);
        }
        else if(taskName =="") {
            YSOS_LOG_DEBUG("do action -> " << taskName);
        }
        else {
            YSOS_LOG_DEBUG("undo action -> " << taskName);
        }

        YSOS_LOG_DEBUG("do action is :" << taskName << " done");

        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::StopTask(std::string taskName)
    {
        YSOS_LOG_DEBUG ( "请求-停止任务 " );

        YSOS_LOG_DEBUG ( "通知-任务已停止 " );

        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::PauseTask(std::string taskName)
    {
        YSOS_LOG_DEBUG ( "请求-暂停任务 " );

        YSOS_LOG_DEBUG ( "通知-任务已暂停 " );

        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::ResumeTask(std::string taskName)
    {
        YSOS_LOG_DEBUG ( "请求-恢复任务 " );

        YSOS_LOG_DEBUG ( "通知-任务已恢复 " );

        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::CancelTask(std::string taskName)
    {
        YSOS_LOG_DEBUG ( "请求-取消任务 " );

        YSOS_LOG_DEBUG ( "通知-任务已取消 " );

        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::CancelAllTask()
    {
        YSOS_LOG_DEBUG ( "请求-取消所有任务 " );

        YSOS_LOG_DEBUG ( "通知-任务已全部取消 " );

        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::SwitchScene()
    {
        YSOS_LOG_DEBUG ( "请求-切换场景 " );

        YSOS_LOG_DEBUG ( "通知-场景已切换 " );

        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::EntranceScene()
    {
        YSOS_LOG_DEBUG ( "请求-进入场景 " );

        YSOS_LOG_DEBUG ( "通知-场景已进入 " );

        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::ExitScene()
    {
        YSOS_LOG_DEBUG ( "请求-退出场景 " );

        YSOS_LOG_DEBUG ( "通知-场景已退出 " );

        return YSOS_ERROR_SUCCESS;
    }
}