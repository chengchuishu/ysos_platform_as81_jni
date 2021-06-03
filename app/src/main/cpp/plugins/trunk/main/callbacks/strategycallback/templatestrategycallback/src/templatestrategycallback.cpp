/**
 *@file templatestrategycallback.cpp
 *@brief the manager self strategy call back
 *@version 1.0
 *@author z.gg
 *@date Created on: 2020-3-31 15:16:00
 *@copyright Copyright (c) 2020 YS Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/templatestrategycallback.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "../../../public/include/common/commonenumdefine.h"

#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"


#include <json/json.h>

namespace ysos {
    DECLARE_PLUGIN_REGISTER_INTERFACE ( TemplateStrategyCallback,  CallbackInterface );  ///<  插件的入口，一定要加上 //  NOLINT
    TemplateStrategyCallback::TemplateStrategyCallback ( const std::string &strClassName ) : BaseStrategyCallbackImpl ( strClassName )
            , data_ptr_ ( GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY ) )
    {
        logger_ = GetUtility()->GetLogger ( "ysos.templatestrategy" );
    }

    TemplateStrategyCallback::~TemplateStrategyCallback (  void )
    {
    }

    int TemplateStrategyCallback::HandleMessage ( const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "HandleMessage: " << event_name );
        if ( data_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "data_ptr_ is NULL!" );
            return YSOS_ERROR_FAILED;
        }
        /* TODO: test template change state
        // template_change_state result
        std::string template_result;
		std::string change_state;
        data_ptr_->GetData ( "template_result", template_result );
        if ( template_result != "0" )
        {
            YSOS_LOG_DEBUG ( "template_result is not finish: " <<  template_result );
            return YSOS_ERROR_FAILED;
        } else {
			data_ptr_->GetData ( "new_state", change_state );
		}
		YSOS_LOG_DEBUG ( "****** HandleMessage [change_state] = " << change_state );
		//< change state
	    {
			int indx = change_state.find("@");
			if (indx <= 0 || indx >= change_state.length()) {
				indx = 0;
			} else {
			    indx = indx + 1;
			}
			std::string tmp_ch_state = change_state.substr(indx);
			int rst = DoSwitchNotifyService ( tmp_ch_state, context );
			if ( YSOS_ERROR_SUCCESS != rst )
			{
				YSOS_LOG_DEBUG ( "execute DoSwitchNotifyService failed" );
			} else {
				YSOS_LOG_DEBUG ( "execute DoSwitchNotifyService success" );
				data_ptr_->SetData("machine_state", tmp_ch_state);
			}
        }
        */
        if ( event_name == "notify_event" )
        {
            std::string event_name;
            if ( YSOS_ERROR_SUCCESS == ParserNotifyEventInfo ( input_buffer, event_name ) )
            {
                if ( event_name.empty() )
                {
                    YSOS_LOG_WARN ( "notify_event's data.name is NULL!" );
                }
                else
                {
                    NotifyEventDoAction(event_name, context);
                }
            } else {
                YSOS_LOG_ERROR ( "ParserNotifyEventInfo is Failed!" );
                return YSOS_ERROR_FAILED;
            }
        }

        return YSOS_ERROR_SUCCESS;
    }

    int TemplateStrategyCallback::Initialized (  const std::string &key,  const std::string &value )
    {
        YSOS_LOG_DEBUG ( "Initialized " << key << ": " << value );
        int n_return = YSOS_ERROR_SUCCESS;
        return n_return;
    }

    int TemplateStrategyCallback::RequestService ( std::string service_name, std::string id,  std::string type, std::string json_value, CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "RequestService " << service_name << " " << id << " " << type << " " << json_value );
        int n_return = YSOS_ERROR_SUCCESS;
        ReqIOCtlServiceParam req_service;
        req_service.id = id;
        req_service.service_name = service_name;
        if ( !type.empty() )
        {
            req_service.type = type;
        }
        if ( !json_value.empty() )
        {
            req_service.value = json_value;
        }
        n_return = DoIoctlService ( req_service,  context );
        if ( YSOS_ERROR_SUCCESS != n_return )
        {
            YSOS_LOG_DEBUG ( "execute DoIoctlService failed, n_return = " << n_return );
        }
        YSOS_LOG_DEBUG ( "RequestService done and success" );
        return n_return;
    }

    int TemplateStrategyCallback::ParserNotifyEventInfo (  BufferInterfacePtr input_buffer,  std::string &event_name )
    {
        const char *buffer =  reinterpret_cast<const char *> ( GetBufferUtility()->GetBufferData ( input_buffer ) );
        if ( buffer == NULL )
        {
            YSOS_LOG_ERROR ( "buffer is NULL!" );
            return YSOS_ERROR_FAILED;
        }
        YSOS_LOG_DEBUG ( "input_buffer: " << buffer );
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( buffer, js_value,  false ) )
        {
            YSOS_LOG_ERROR ( "ParserMoveInfo failed!" );
            return YSOS_ERROR_FAILED;
        }
        js_value = js_value["data"];
        if ( js_value.isMember ( "name" ) )
        {
            event_name = js_value["name"].asString();
            return YSOS_ERROR_SUCCESS;
        }
        return YSOS_ERROR_FAILED;
    }

    int TemplateStrategyCallback::NotifyEventDoAction ( std::string event_name, CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "NotifyEventDoAction  event_name :" << event_name );
        int n_return = YSOS_ERROR_SUCCESS;
        do
        {
            /* code */
            std::string msg;
            if(event_name == "P005") {
                msg = "通知计划已开始";
            } else if(event_name == "P007") {
                msg = "通知计划已暂停";
            } else if(event_name == "P009") {
                msg = "通知计划已恢复";
            } else if(event_name == "P011") {
                msg = "通知计划被终止";
            } else if(event_name == "P012") {
                msg = "通知计划已完成";
            } else if(event_name == "P013") {
                msg = "通知计划已停止";
            } else if(event_name == "T006") {
                msg = "通知任务已切换";
            } else if(event_name == "T008") {
                msg = "通知任务已开始";
            } else if(event_name == "T010") {
                msg = "通知任务已暂停";
            } else if(event_name == "T012") {
                msg = "通知任务已恢复";
            } else if(event_name == "T014") {
                msg = "通知任务被终止";
            } else if(event_name == "T015") {
                msg = "通知任务已完成";
            } else if(event_name == "T016") {
                msg = "通知任务已停止";
            } else if(event_name == "T018") {
                msg = "通知任务已取消";
            } else if(event_name == "S006") {
                msg = "通知场景已切换";
            } else if(event_name == "S008") {
                msg = "通知场景已进入";
            } else if(event_name == "S010") {
                msg = "通知场景已退出";
            } else if(event_name == "C004") {
                msg = "通知内容已加载";
            } else if(event_name == "C006") {
                msg = "通知内容已开始";
            } else if(event_name == "C008") {
                msg = "通知内容已暂停";
            } else if(event_name == "C010") {
                msg = "通知内容已恢复";
            } else if(event_name == "C012") {
                msg = "通知内容被终止";
            } else if(event_name == "C013") {
                msg = "通知内容已完成";
            } else if(event_name == "C014") {
                msg = "通知内容已停止";
            } else if(event_name == "C016") {
                msg = "通知内容已卸载";
            } else if(event_name == "B012") {
                msg = "通知片段已切换";
            } else if(event_name == "B014") {
                msg = "通知片段已加载";
            } else if(event_name == "B016") {
                msg = "通知片段已就绪";
            } else if(event_name == "B018") {
                msg = "通知片段已开始";
            } else if(event_name == "B020") {
                msg = "通知片段已暂停";
            } else if(event_name == "B022") {
                msg = "通知片段已恢复";
            } else if(event_name == "B024") {
                msg = "通知片段已跳转";
            } else if(event_name == "B025") {
                msg = "通知片段已收到数据";
            } else if(event_name == "B028") {
                msg = "通知片段被终止";
            } else if(event_name == "B029") {
                msg = "通知片段已完成";
            } else if(event_name == "B030") {
                msg = "通知片段已停止";
            } else if(event_name == "B032") {
                msg = "通知片段已卸载";
            } else {
                msg = "unknown event name";
            }

            /*
            Json::FastWriter json_writer;
            Json::Value json_data;
            json_data["msg"] = msg;
            msg = json_writer.write(json_data);
            msg = GetUtility()->ReplaceAllDistinct ( msg, "\n", "" );*/

            char dstgbk[1024] = {0};
            strcpy(dstgbk, msg.c_str());
            YSOS_LOG_DEBUG ( "NotifyEventDoAction msg check[0]: " << msg);
            int etype = GetUtility()->GetEncoding((unsigned char*)dstgbk, strlen(dstgbk));
            YSOS_LOG_DEBUG ( "NotifyEventDoAction encode type check[0]: " << etype);
            msg = GetUtility()->Utf8ToAscii(msg);

            memset(dstgbk, 0, sizeof(dstgbk)/sizeof(char));
            strcpy(dstgbk, msg.c_str());
            etype = GetUtility()->GetEncoding((unsigned char*)dstgbk, strlen(dstgbk));
            YSOS_LOG_DEBUG ( "NotifyEventDoAction msg check[1]: " << msg);
            YSOS_LOG_DEBUG ( "NotifyEventDoAction encode type check[1]: " << etype);

            msg = "{\"msg\":\"" + msg + "\"}";

            memset(dstgbk, 0, sizeof(dstgbk)/sizeof(char));

            n_return = DoEventNotifyService ( event_name, event_name + "_callback", msg, context );

            if (YSOS_ERROR_SUCCESS != n_return) {
                YSOS_LOG_DEBUG("execute EventNotify failed, n_return = " << n_return);
                n_return = YSOS_ERROR_FAILED;
                break;
            }

            YSOS_LOG_DEBUG ( "NotifyEventDoAction event_name : " << event_name << " " << msg);
            YSOS_LOG_DEBUG ( "NotifyEventDoAction done and success" );

        } while (0);

        return n_return;
    }

}