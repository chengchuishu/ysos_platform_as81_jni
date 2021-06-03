/**   //NOLINT
  *@file sockethandledefine.h
  *@brief Definition of
  *@version 0.1
  *@author Livisen.Wan
  *@date Created on: 2018:4:23   10:55
  *@copyright Copyright(c)
  * http://www.fubaorobot.com/
  *@howto Usage:
  *@todo
  */

#pragma once

/**
 *@brief socket 开启的函数指针定义
 *@param session_id[Input] socket的sessionid
 *@return 0 handle success
 *        -1 cant handle
 */
typedef int (*pfunHandleSocketStart)(const char* session_id, const char* remote_ip, const char* remote_port, const char* protocol);


/**
 *@brief socket 处理客户端发来的请求
 *@param session_id[Input] socket的sessionid
 *@param request_data[Input] 客户端发来的数据
 *@param request_length[Input] 客户端发来的数据的长度
 *@param decode_msg_body_len[Output] 解决头后消息体的长度
 *@return 0 handle success
 *        -1 cant handle
 */
typedef int (*pfunHandleSocketRequestHeader)(const char* sessio_id, const unsigned char* request_data, const int request_length, int* decode_msg_body_len);

/**
 *@brief socket 处理客户端发来的请求
 *@param session_id[Input] socket的sessionid
 *@param request_data[Input] 客户端发来的数据
 *@param request_length[Input] 客户端发来的数据的长度
 *@return 0 handle success
 *        -1 cant handle
 */
typedef int (*pfunHandleSocketRequestBody)(const char* sessio_id, const unsigned char* request_data, const int request_length);

/**
 *@brief socket 关闭的函数指针定义
 *@param session_id[Input] socket的sessionid
 *@return 0 handle success
 *        -1 cant handle
 */
typedef int (*pfunHandleSocketClose)(const char* session_id);


/**
 *@brief socket 关闭的函数指针定义
 *@param session_id[Input] socket的sessionid
 *@param error_message[Input] socket的出错处理
 *@return 0 handle success
 *        -1 cant handle
 */
typedef int (*pfunHandleSocketError)(const char* session_id, const char* error_message);