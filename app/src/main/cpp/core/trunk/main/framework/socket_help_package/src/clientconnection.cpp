/**   //NOLINT
  *@file clientconnection.cpp
  *@brief Definition of
  *@version 0.1
  *@author Livisen.Wan
  *@date Created on: 2018:4:24   22:15
  *@copyright Copyright(c) 2016 Tangdi Technology. All rights reserved.
  * http://www.fubaorobot.com/
  *@howto Usage:
  *@todo
  */

/// self headers
#include "../include/clientconnection.h"

/// boost headers
#include <boost/thread/lock_guard.hpp>

namespace http {
    namespace server3 {

        client_connection::client_connection(boost::asio::io_service& io_service,
                                             connection_manager& manager,
                //std::string& session_id,
                                             pfunHandleSocketStart fun_handle_start,
                                             pfunHandleSocketRequestHeader fun_handle_request_header,
                                             pfunHandleSocketRequestBody fun_handle_requst_body,
                                             pfunHandleSocketClose fun_handle_close,
                                             pfunHandleSocketError fun_handle_error,
                                             const int request_message_header_size
                //const int timeout_milliseconds
        )
                :connection(io_service,
                            manager,
                            fun_handle_start,
                            fun_handle_request_header,
                            fun_handle_requst_body,
                            fun_handle_close,
                            fun_handle_error,
                            request_message_header_size,
                            0),
                 resolver_(io_service)
        {

        }

        int  client_connection::connect_asyn(const char* ip, const char* port) {
            /*boost::asio::ip::tcp::endpoint conn_point(boost::asio::ip::address::from_string(ip), atoi(port));
            socket_.async_connect(conn_point,
                                  strand_.wrap(boost::bind(&connection::handle_connect, this,
                                               boost::asio::placeholders::error)));
            return 0;*/
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            conn_ip_ = ip;
            conn_port_ = port;

            boost::asio::ip::tcp::resolver::query query(ip, port);
            //boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver_.resolve(query);

            resolver_.async_resolve(query,
                                    strand_.wrap(boost::bind(&client_connection::handle_resolver,
                                                             this,
                                                             boost::asio::placeholders::error,
                                                             boost::asio::placeholders::iterator)));
            return 0;
        }

        int client_connection::connect(const char* ip, const char* port) {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            boost::asio::ip::tcp::endpoint conn_point(boost::asio::ip::address::from_string(ip), atoi(port));

            boost::system::error_code myecc;
            //boost::asio::connect(socket_, endpoint_iterator, myecc);
            socket_.connect(conn_point, myecc);
            //handle_connect(myecc);

            if (!myecc) {
                return 0;
            } else {
                return -1;
            }
        }



        void client_connection::handle_resolver(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);
            if(!error) {
                boost::asio::async_connect(socket_,
                                           endpoint_iterator,
                                           strand_.wrap(boost::bind(&client_connection::handle_connect,
                                                                    this,
                                                                    boost::asio::placeholders::error)));
            }else {
                handle_error(&error, "client cnst resolver ip address");
            }
        }


        void client_connection::handle_connect(const boost::system::error_code& error) {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);
            if (!error) {
                /// 连上后台
                std::cout<<"connect success" << std::endl;
                if (fun_handle_start_) {
                    /*boost::asio::ip::tcp::endpoint remote_point = socket_.remote_endpoint();
                    char tempbuf[64] = {'\0'};
                    sprintf_s(tempbuf, sizeof(tempbuf),"%d", remote_point.port());
                    fun_handle_start_(session_id_.c_str(), remote_point.address().to_string().c_str(), tempbuf, "");*/
                    fun_handle_start_(session_id_.c_str(), conn_ip_.c_str(), conn_port_.c_str(), "");
                }
                read_asyn();  ///< 异步读，连上后
            } else {
                std::cout<<"connect fail" <<std::endl;
                handle_error(&error, "connect fial");
            }
        }





//int connection::read_syn(std::vector<unsigned char>& vec_data) {
//  if (!socket_.available()) {
//    return -1;
//  }
//
//  //vec_data.clear();
//  //if(vec_buffer_.size() < socket_.available()) {
//  //  vec_buffer_.reserve(socket_.available() +1);
//  //}
//
//  boost::system::error_code error;
//  //boost::array<char, 8192> tmp_array;
//  int recv_count =socket_.read_some(boost::asio::buffer(buffer_), error);
//  //std::string s(tmp_array.begin(), tmp_array.end());
//  //std::string s2(tmp_array.data(), tmp_array.size());
//  //std::cout<<s <<std::endl;
//  vec_data.insert(vec_data.end(), buffer_.begin(), buffer_.end());
//  return 0;
//}
    }
}