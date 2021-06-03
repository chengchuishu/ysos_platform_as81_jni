/**   //NOLINT
  *@file tdsocketclient.h
  *@brief Definition of
  *@version 0.1
  *@author Livisen.Wan
  *@date Created on:
  *@copyright Copyright(c)
  * http://www.fubaorobot.com/
  *@howto Usage:
  *@todo
  */

#pragma once

#include "sockethandledefine.h"


#ifndef DLL_EXPORT
#ifdef LIBSOCKETSERVER_EXPORTS
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT /*__declspec(dllimport)*/  // update for linux
#endif
#endif

#ifndef EXTERN_C
#define EXTERN_C extern "C"
#endif




typedef void* SocketHandle;

/**
 *@brief 开启socket服务
 *@bindip[Input] 绑定的ip地址
 *@param[Input] 绑定的bindport地址
 *@param fun_handle_start[Input] 一个新的socket连接通知
 *@param fun_handle_requst[Input] 接收sock数据后的处理函数地址
 *@param thread_pool_size[Input] 处理的线程池大小,默认可为5
 *@return 创建的server的句柄 TDSocketHandle if sucess, or turn null
 *
 */
EXTERN_C DLL_EXPORT SocketHandle SocketClient_Start(const char* ip,
                                                    const char* port,
                                                    pfunHandleSocketStart fun_handle_start,
                                                    pfunHandleSocketRequestHeader fun_handle_request_header,
                                                    pfunHandleSocketRequestBody fun_handle_requst_body,
                                                    pfunHandleSocketClose fun_handle_close,
                                                    pfunHandleSocketError fun_handle_error,
                                                    const int request_message_header_size);

/**
 *@brief 关闭socket服务
 *@param socket_handle[Input] socket server的句柄
 *@return
 */
EXTERN_C DLL_EXPORT void SocketClient_Stop(SocketHandle socket_handle);

/**
 *@brief 向某一个sessionid的连接发送数据，异步发送，如果出错，则会隔段时间重发（时间间隔由内部决定）
 *@param session_id[Input] 连接的sessionid
 *@param reply_data[Input] 发送的数据开始地址
 *@param reply_data_length[Input] 发送数据的长度
 *@param close_connection_after_send[Input] 发送数据后是否关闭连接
 *@return 0 sucess
 *        -1 fail
 */
EXTERN_C DLL_EXPORT int SocketClient_SendMessageAsyn(SocketHandle socket_handle, const unsigned char* reply_data, const int reply_data_length, const bool close_connection_after_send);


/**
 *@brief 向某一个sessionid的连接发送数据,同步发送，如果出错，则不重发
 *@param session_id[Input] 连接的sessionid
 *@param reply_data[Input] 发送的数据开始地址
 *@param reply_data_length[Input] 发送数据的长度
 *@param close_connection_after_send[Input] 发送数据后是否关闭连接
 *@return 0 sucess
 *        -1 fail
 */
EXTERN_C DLL_EXPORT int SocketClient_SendMessageSyn(SocketHandle socket_handle, const unsigned char* reply_data, const int reply_data_length, const bool close_connection_after_send);