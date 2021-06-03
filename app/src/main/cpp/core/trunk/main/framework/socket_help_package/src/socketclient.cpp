/**   //NOLINT
  *@file TDHttpServer.cpp
  *@brief Definition of
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2017:6:23   14:48
  *@copyright Copyright(c) 2016 Tangdi Technology. All rights reserved.
  * http://www.fubaorobot.com/
  *@howto Usage:
  *@todo
  */


/// stl headers
#include <map>
#include <string>

/// boost headers;
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/chrono.hpp>

/// private headers
#include "../include/connection_manager.hpp"
//#include "connection.hpp"
#include "../include/clientconnection.h"
#include "../include/socketclient.h"


typedef http::server3::connection_ptr TDClientPtr;

class ClientMgr {
public:
    ClientMgr() {
        thread_io_run_ptr_ = NULL;
    }
    ~ClientMgr() {
        boost::lock_guard<boost::mutex> guard(mutex_member_);
        connection_manager_.stop_all();
        map_client_.clear();

        if (NULL != thread_io_run_ptr_) {
            thread_io_run_ptr_->interrupt();
            io_service_.stop();
            thread_io_run_ptr_ = NULL;
        }
    }

public:
    boost::mutex mutex_member_;
    //std::map<TDSocketHandle, TDClientPtr> map_client_;
    std::map<SocketHandle, std::string> map_client_;
    boost::asio::io_service io_service_;
    http::server3::connection_manager connection_manager_;
    boost::shared_ptr<boost::thread> thread_io_run_ptr_;
};


static ClientMgr g_client_mgr_;

void ThreadFunTest() {
    g_client_mgr_.io_service_.run();

#ifdef _DEBUG
    assert(false && "test for ioserver run finish");
#endif

    return;
}

SocketHandle SocketClient_Start(const char* ip, const char* port,
                                pfunHandleSocketStart fun_handle_start,
                                pfunHandleSocketRequestHeader fun_handle_request_header,
                                pfunHandleSocketRequestBody fun_handle_requst_body,
                                pfunHandleSocketClose fun_handle_close,
                                pfunHandleSocketError fun_handle_error,
                                const int request_message_header_size) {

    boost::lock_guard<boost::mutex> guard(g_client_mgr_.mutex_member_);

    TDClientPtr client_ptr(new http::server3::client_connection(g_client_mgr_.io_service_, g_client_mgr_.connection_manager_,
                                                                fun_handle_start,
                                                                fun_handle_request_header,
                                                                fun_handle_requst_body,
                                                                fun_handle_close,
                                                                fun_handle_error,
                                                                request_message_header_size));

    if (NULL == client_ptr) {
        return 0;
    }
    if (NULL == ip || NULL == port) {
        return 0;
    }
    int iret = ((http::server3::client_connection*)client_ptr.get())->connect(ip, port);
    if(0 != iret) { ///< 连不上
        client_ptr = NULL;
        return NULL;
    }
    //client_ptr->start();
    g_client_mgr_.connection_manager_.add_to_manager(client_ptr);
    // ((http::server3::client_connection*)client_ptr.get())->connect_asyn(ip, port);
    client_ptr->read_asyn();  ///< 开始异步读

    g_client_mgr_.map_client_.insert(std::make_pair<SocketHandle,std::string>((SocketHandle)client_ptr.get(), client_ptr->get_sessionid()));

    if(g_client_mgr_.thread_io_run_ptr_) {
        if(g_client_mgr_.thread_io_run_ptr_->try_join_for(boost::chrono::milliseconds(0))) { /// have finish
            g_client_mgr_.thread_io_run_ptr_ = NULL;
            g_client_mgr_.io_service_.reset();   ///< 重启事件循环
        }
    }
    if (NULL == g_client_mgr_.thread_io_run_ptr_) {
        /* g_client_mgr_.thread_io_run_ptr_.reset(new boost::thread(
        boost::bind(&boost::asio::io_service::run, &g_client_mgr_.io_service_)));*/
        g_client_mgr_.thread_io_run_ptr_.reset(new boost::thread(
                boost::bind(ThreadFunTest)));
    }

    return (SocketHandle)client_ptr.get();
}

void SocketClient_Stop(SocketHandle socket_handle) {
    boost::lock_guard<boost::mutex> guard(g_client_mgr_.mutex_member_);
    std::map<SocketHandle, std::string>::iterator it_find = g_client_mgr_.map_client_.find(socket_handle);
    if (it_find == g_client_mgr_.map_client_.end()) {
        return;
    }
    // stop
    g_client_mgr_.connection_manager_.stop_connection(it_find->second);
    g_client_mgr_.map_client_.erase(it_find);
    return;
}

int SocketClient_SendMessageAsyn(SocketHandle socket_handle, const unsigned char* reply_data, const int reply_data_length, const bool close_connection_after_send) {
    boost::lock_guard<boost::mutex> guard(g_client_mgr_.mutex_member_);
    std::map<SocketHandle, std::string>::iterator it_find = g_client_mgr_.map_client_.find(socket_handle);
    if (it_find == g_client_mgr_.map_client_.end()) {
        return -1;
    }
    return g_client_mgr_.connection_manager_.send_message_asyn(it_find->second, reply_data, reply_data_length, close_connection_after_send);
}

int SocketClient_SendMessageSyn(SocketHandle socket_handle, const unsigned char* reply_data, const int reply_data_length, const bool close_connection_after_send) {
    boost::lock_guard<boost::mutex> guard(g_client_mgr_.mutex_member_);
    std::map<SocketHandle, std::string>::iterator it_find = g_client_mgr_.map_client_.find(socket_handle);
    if (it_find == g_client_mgr_.map_client_.end()) {
        return -1;
    }
    return g_client_mgr_.connection_manager_.send_message_syn(it_find->second, reply_data, reply_data_length, close_connection_after_send);
}