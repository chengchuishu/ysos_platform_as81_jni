//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_CONNECTION_HPP
#define HTTP_SERVER3_CONNECTION_HPP

#include <vector>
#include <list>
#include <string>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_array.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/atomic.hpp>

//#include "reply.hpp"
//#include "request.hpp"
//#include "request_handler.hpp"
//#include "request_parser.hpp"
#include "sockethandledefine.h"

namespace http {
    namespace server3 {


        class connection_manager;

/// Represents a single connection from a client.
        class connection
                : public boost::enable_shared_from_this<connection>,
                  private boost::noncopyable {
        public:
            friend connection_manager;

            /// Construct a connection with the given io_service.
            explicit connection(boost::asio::io_service& io_service,
                                connection_manager& manager,
                    //std::string& session_id,
                                pfunHandleSocketStart fun_handle_start,
                                pfunHandleSocketRequestHeader fun_handle_request_header,
                                pfunHandleSocketRequestBody fun_handle_requst_body,
                                pfunHandleSocketClose fun_handle_close,
                                pfunHandleSocketError fun_handle_error,
                                const int request_message_header_size,
                                const int timeout_milliseconds
            );

            /// Get the socket associated with the connection.
            boost::asio::ip::tcp::socket& socket();

            /// Start the first asynchronous operation for the connection. 接收对方的数据
            void start();                                     ///< 服务端使用的函数


            /// Stop conn
            void close();

            void set_sessionid(std::string session_id);
            std::string get_sessionid();

            /// 判断连接是否有效，即双方是否还在连接
            bool is_conn_valid();

            /// 判断连接是否已超时
            bool is_conn_timeout();

            /// 发送数据，异步操作
            /// close_connection_after_send[Input] 发送后是否关闭连接
            int send_message_asyn(const unsigned char* reply_data, const int reply_data_length, const bool close_connection_after_send);
            int send_message_syn(const unsigned char* reply_data, const int reply_data_length, const bool close_connection_after_send);
            int read_asyn();


            /// 获取待发送数据的个数
            int get_write_size();

            //void handle_disconnection(const boost::system::error_code* pe);
            void handle_disconnection2();
            //private:
        protected:
            /// Handle completion of a read operation.
            void handle_read_header(const boost::system::error_code& e,
                                    std::size_t bytes_transferred,
                                    boost::shared_array<unsigned char> arr_buf);

            void handle_read_body(const boost::system::error_code& e,
                                  std::size_t bytes_transferred,
                                  boost::shared_array<unsigned char> arr_buf);

            /// Handle completion of a write operation.
            void handle_write(const boost::system::error_code& e);

            void handle_error(const boost::system::error_code* pe, const char* error_message);



            //std::string assembly_error_message(const boost::system::error_code& e, const char* error_messge);

        protected:
            boost::recursive_mutex recursive_mutex_member_;

            /// Strand to ensure the connection's handlers are not called concurrently.
            boost::asio::io_service::strand strand_;

            /// Socket for the connection.
            boost::asio::ip::tcp::socket socket_;
            boost::asio::io_service& io_service_;

            /// The manager for this connection.
            connection_manager& connection_manager_;
            pfunHandleSocketStart fun_handle_start_;
            pfunHandleSocketRequestHeader fun_handle_request_header_;
            pfunHandleSocketRequestBody fun_handle_requst_body_;
            pfunHandleSocketClose fun_handle_close_;
            pfunHandleSocketError fun_handle_error_;
            int request_message_header_size_;
            int timeout_milliseconds_;

            /// 连接的一些属性(需要作关发控制）
            boost::posix_time::ptime conn_begin_datatime_;   ///< 最开始连接时的时间
            boost::posix_time::ptime conn_latest_datatime_;   ///< 最近一次连接时的时间

            enum conntype {
                ct_not_known  =0,
                ct_keep_alive =1
            } conntype_;                            ///< 连接类型
            std::string session_id_;                 ///< 每个连接唯一的session id;

            boost::atomic<unsigned int> atomic_last_time_reread_sleep_milliseconds_;                ///< 最近一次读出错后sleep的时间,默认是0毫秒，最大是10秒

            std::list<std::vector<unsigned char>> list_reply_message_;
            boost::atomic<unsigned int> atomic_last_time_rewrite_sleep_milliseconds_;               ///< 最近一次重发的sleep时间，默认是0毫秒,最大是30秒
            boost::atomic<int> atomic_close_connection_after_send_;
        };

        typedef boost::shared_ptr<connection> connection_ptr;

    } // namespace server3
} // namespace http

#endif // HTTP_SERVER3_CONNECTION_HPP
