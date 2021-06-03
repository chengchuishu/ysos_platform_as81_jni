//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "../include/connection.hpp"

#include <vector>

#include <boost/bind.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/shared_array.hpp>
#include <boost/thread/lock_guard.hpp>

#include "../include/connection_manager.hpp"

namespace http {
    namespace server3 {

#define SYSCODE_REMOTE_FORCE_CLOSE_CONN   10054 ///< 远程主机强迫关闭了一个现有的连接

        connection::connection(boost::asio::io_service& io_service,
                               connection_manager& manager,
                               pfunHandleSocketStart fun_handle_start,
                               pfunHandleSocketRequestHeader fun_handle_request_header,
                               pfunHandleSocketRequestBody fun_handle_requst_body,
                               pfunHandleSocketClose fun_handle_close,
                               pfunHandleSocketError fun_handle_error,
                               const int request_message_header_size,
                               const int timeout_milliseconds)
                : strand_(io_service),
                  socket_(io_service),
                  io_service_(io_service),
                  connection_manager_(manager),
                // session_id_(session_id),
                  fun_handle_start_(fun_handle_start),
                  fun_handle_request_header_(fun_handle_request_header),
                  fun_handle_requst_body_(fun_handle_requst_body),
                  fun_handle_close_(fun_handle_close),
                  fun_handle_error_(fun_handle_error),
                  request_message_header_size_(request_message_header_size),
                  timeout_milliseconds_(timeout_milliseconds) {
            conn_begin_datatime_ = boost::posix_time::second_clock::local_time();
            conn_latest_datatime_ = conn_begin_datatime_;

            //conn_timeout_seconds_ = 30; ///< 默认
            atomic_last_time_reread_sleep_milliseconds_ =0;
            atomic_last_time_rewrite_sleep_milliseconds_ =0;

            atomic_close_connection_after_send_ = false;

            //socket_.set_option(boost::asio::socket_base::keep_alive(true));
        }

        boost::asio::ip::tcp::socket& connection::socket() {
            return socket_;
        }

        void connection::start() {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            if (fun_handle_start_) {
                boost::asio::ip::tcp::endpoint remote_point = socket_.remote_endpoint();
                char tempbuf[64] = {'\0'};

//#ifdef _WIN32
//    sprintf_s(tempbuf, sizeof(tempbuf),"%d", remote_point.port());  //need update for linux
//#else
                sprintf(tempbuf,"%d", remote_point.port());   //need update for linux
                fun_handle_start_(session_id_.c_str(), remote_point.address().to_string().c_str(), tempbuf, "");
            }

            read_asyn();
        }

        int connection::read_asyn() {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            assert(fun_handle_request_header_);

            boost::shared_array<unsigned char> arr_buf(new unsigned char[request_message_header_size_]);
            if (NULL == arr_buf) {
                return -1;
            }
            socket_.async_read_some(boost::asio::buffer(arr_buf.get(), request_message_header_size_),
                                    strand_.wrap(
                                            boost::bind(&connection::handle_read_header, shared_from_this(),
                                                        boost::asio::placeholders::error,
                                                        boost::asio::placeholders::bytes_transferred,
                                                        arr_buf)));
            return 0;
        }

        void connection::close() {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            if (false == socket_.is_open()) {
                return;
            }
            socket_.close();
            boost::system::error_code ignored_ec;
            socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);

            io_service_.post(boost::bind(&connection::handle_disconnection2, shared_from_this()));
        }

        void connection::handle_read_header(const boost::system::error_code& e,
                                            std::size_t bytes_transferred,
                                            boost::shared_array<unsigned char> arr_buf) {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            if(false == socket_.is_open() ) {
                return;
            }

            if (bytes_transferred >0) {
                // update conn time
                conn_latest_datatime_ = boost::posix_time::second_clock::local_time();
            }
            assert(fun_handle_request_header_);

            if (!e) {
                ///读取成功
                assert(bytes_transferred == request_message_header_size_);
                int message_body_len =0;
                fun_handle_request_header_(session_id_.c_str(), arr_buf.get(), bytes_transferred, &message_body_len);

                if (message_body_len >0) {
                    arr_buf.reset(new unsigned char[message_body_len]);
                    if (NULL == arr_buf) {
                        return;
                    }
                    /// 再次读取
                    socket_.async_read_some(boost::asio::buffer(arr_buf.get(), message_body_len),
                                            strand_.wrap(
                                                    boost::bind(&connection::handle_read_body, shared_from_this(),
                                                                boost::asio::placeholders::error,
                                                                boost::asio::placeholders::bytes_transferred,
                                                                arr_buf)));
                } else {
                    /// 再次去读下一次数据
                    socket_.async_read_some(boost::asio::buffer(arr_buf.get(), request_message_header_size_),
                                            strand_.wrap(
                                                    boost::bind(&connection::handle_read_header, shared_from_this(),
                                                                boost::asio::placeholders::error,
                                                                boost::asio::placeholders::bytes_transferred,
                                                                arr_buf)));
                }
                atomic_last_time_reread_sleep_milliseconds_ =0;
            } else {
                /// 再次去读下一次数据
                if (0 == atomic_last_time_reread_sleep_milliseconds_) { ///< 这里只发送一次错误异常
                    handle_error(&e, (std::string("read error") + e.message()).c_str());
                }

                if (false == socket_.is_open()) {
                    return;  ///< 不再读
                }
                {
                    /// 本次read失败，则过一段时间再read
//#ifdef _WIN32
//      Sleep(atomic_last_time_reread_sleep_milliseconds_);
//#else
                    sleep(atomic_last_time_reread_sleep_milliseconds_/1000);
                    if (0 == atomic_last_time_reread_sleep_milliseconds_) {
                        atomic_last_time_reread_sleep_milliseconds_ =100;
                    } else {
                        atomic_last_time_reread_sleep_milliseconds_ = atomic_last_time_reread_sleep_milliseconds_.storage() *2;    ///< 2倍时间增长
                        if (atomic_last_time_reread_sleep_milliseconds_ > 20000) { ///< 最大20秒
                            atomic_last_time_reread_sleep_milliseconds_ = 20000;
                        }
                    }
                }
                if (is_conn_timeout()) { ///< 超时
                    close();
                    return;
                }
                if (0 == timeout_milliseconds_) { ///< 连接出错
                    close();
                    return;
                }
                socket_.async_read_some(boost::asio::buffer(arr_buf.get(), request_message_header_size_),
                                        strand_.wrap(
                                                boost::bind(&connection::handle_read_header, shared_from_this(),
                                                            boost::asio::placeholders::error,
                                                            boost::asio::placeholders::bytes_transferred,
                                                            arr_buf)));

            }
        }

        void connection::handle_read_body(const boost::system::error_code& e,
                                          std::size_t bytes_transferred,
                                          boost::shared_array<unsigned char> arr_buf) {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            if(false == socket_.is_open() ) {
                return;
            }

            if (bytes_transferred >0) {
                // update conn time
                conn_latest_datatime_ = boost::posix_time::second_clock::local_time();
            }

            assert(fun_handle_requst_body_);
            if (!e) {
                fun_handle_requst_body_(session_id_.c_str(), arr_buf.get(), bytes_transferred);
            }

            arr_buf.reset(new unsigned char[request_message_header_size_]);
            /// 再次去读下一次数据
            socket_.async_read_some(boost::asio::buffer(arr_buf.get(), request_message_header_size_),
                                    strand_.wrap(
                                            boost::bind(&connection::handle_read_header, shared_from_this(),
                                                        boost::asio::placeholders::error,
                                                        boost::asio::placeholders::bytes_transferred,
                                                        arr_buf)));
        }

        void connection::handle_write(const boost::system::error_code& e) {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            if(false == socket_.is_open()) {
                return;
            }

            if (!e) {
                atomic_last_time_rewrite_sleep_milliseconds_ =0;
                list_reply_message_.pop_front();                         ///< 发送成功，从队列中删除

                // boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);
                conn_latest_datatime_ = boost::posix_time::second_clock::local_time();  ///< 更新一下连接时间
            } else {
                /// 发送出错
                std::string error_message = std::string("handle write error:") +  e.message();
                handle_error(&e, error_message.c_str());

                if (false == socket_.is_open()) {
                    return;  ///< 不再写
                }
                if (0 == timeout_milliseconds_) { ///< 连接出错
                    close();
                    return;
                }
                /// 本次发送失败，则过一段时间再发送
//#ifdef _WIN32
//    Sleep(atomic_last_time_rewrite_sleep_milliseconds_);
//#else
                sleep(atomic_last_time_rewrite_sleep_milliseconds_/1000);

                if (0 == atomic_last_time_rewrite_sleep_milliseconds_) {
                    atomic_last_time_rewrite_sleep_milliseconds_ =100;
                } else {
                    atomic_last_time_rewrite_sleep_milliseconds_ = atomic_last_time_rewrite_sleep_milliseconds_.storage() *2;    ///< 2倍时间增长
                    if (atomic_last_time_rewrite_sleep_milliseconds_ > 20000) { ///< 最大20秒
                        atomic_last_time_rewrite_sleep_milliseconds_ = 20000;
                    }
                }
                /// 重发
            }

            /// 如果队列中还有消息待发送，则再发送(包含上一次没发送成功的，这次再发送一次）
            if (list_reply_message_.size() >0) {
                boost::system::error_code ignored_ec;
                std::vector<unsigned char>* psend_data = &(*list_reply_message_.begin());
                socket_.async_write_some(boost::asio::buffer(&(*psend_data)[0], psend_data->size()),
                                         strand_.wrap(
                                                 boost::bind(&connection::handle_write, shared_from_this(),
                                                             boost::asio::placeholders::error)));

            }

            if (atomic_close_connection_after_send_ && list_reply_message_.size() <=0) {
                //关闭连接
                close();
                connection_manager_.notify_close_connection(session_id_); ///< 通知管理器关闭连接了
            }

        }


        void connection::set_sessionid(std::string session_id) {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            session_id_ = session_id;
        }
        std::string connection::get_sessionid() {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            return session_id_;
        }
        bool connection::is_conn_valid() {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            return socket_.is_open();
        }

        bool connection::is_conn_timeout() {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            if (timeout_milliseconds_ <=0) { ///< -1 表示没有超时限定, 0 表示只有出错时就不保持连接
                return false;
            }

            boost::posix_time::ptime tmp_conn_last_time;
            {
                tmp_conn_last_time = conn_latest_datatime_;
            }
            boost::posix_time::ptime now_time = boost::posix_time::second_clock::local_time();
            boost::posix_time::time_duration td = now_time - tmp_conn_last_time;
            if (td.total_milliseconds() > timeout_milliseconds_) {
                std::string error_message = std::string("connect time out");
                boost::system::error_code myecc /*= boost::system::errc::timed_out*/;
                handle_error(&myecc, error_message.c_str());
                return true;
            } else {
                return false;
            }
        }


        int connection::send_message_asyn(const unsigned char* reply_data, const int reply_data_length, const bool close_connection_after_send) {
            assert(reply_data && reply_data_length >0);
            if (NULL == reply_data || reply_data_length <=0) {
                return -1;
            }
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);
            if(false == socket_.is_open()) {
                return -1;
            }

            atomic_close_connection_after_send_ = close_connection_after_send;  ///< 注意，发送后是否关闭连接

            std::vector<unsigned char> reply_message(reply_data, reply_data+reply_data_length);
            if (list_reply_message_.size() >0) {
                list_reply_message_.push_back(reply_message);
                atomic_last_time_rewrite_sleep_milliseconds_ =0;
                return true;
            }

            list_reply_message_.push_back(reply_message);             ///< 先放到阵列里，待发送成功后再删除
            /*boost::asio::async_write(socket_, boost::asio::buffer(&(reply_message[0]), reply_message.size()),
            strand_.wrap(
            boost::bind(&connection::handle_write, shared_from_this(),
            boost::asio::placeholders::error)));*/
            socket_.async_write_some(boost::asio::buffer(&(reply_message[0]), reply_message.size()),
                                     strand_.wrap(
                                             boost::bind(&connection::handle_write, shared_from_this(),
                                                         boost::asio::placeholders::error)));

            return 0;
        }

        int connection::send_message_syn(const unsigned char* reply_data, const int reply_data_length, const bool close_connection_after_send) {
            assert(reply_data && reply_data_length >0);
            if (NULL == reply_data || reply_data_length <=0) {
                return -1;
            }
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);
            if(false == socket_.is_open()) {
                return -1;
            }

            atomic_close_connection_after_send_ = close_connection_after_send;  ///< 注意，发送后是否关闭连接

            boost::system::error_code myecc;
            socket_.write_some(boost::asio::buffer(reply_data, reply_data_length), myecc);

            if (atomic_close_connection_after_send_) {
                //关闭连接
                close();
            }

            if (!myecc) {
                return 0;
            } else {
                return -1;
            }
        }


        int connection::get_write_size() {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            return list_reply_message_.size();
        }

        void connection::handle_error(const boost::system::error_code* pe, const char* error_message) {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            if(false == socket_.is_open()) {
                return;
            }

            if (pe && SYSCODE_REMOTE_FORCE_CLOSE_CONN == pe->value()) {
                close();
            }
            if (fun_handle_error_) {
                std::string emsg = std::string(error_message) ;
                if (pe && boost::system::errc::success != *pe) {
                    emsg += (pe ? pe->message() : "");
                }
                fun_handle_error_(session_id_.c_str(), emsg.c_str());
            }
        }

//void connection::handle_disconnection(const boost::system::error_code* pe) {
//  boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);
//
//  if(false == socket_.is_open()) {
//    return;
//  }
//
//
//}

        void connection::handle_disconnection2() {
            boost::lock_guard<boost::recursive_mutex> guard(recursive_mutex_member_);

            if (fun_handle_close_) {
                fun_handle_close_(session_id_.c_str());
            }
            //connection_manager_.notify_close_connection(session_id_); ///< 通知管理器关闭连接了，这里要异常操作，防止死锁
            io_service_.post(boost::bind(&connection_manager::notify_close_connection, &connection_manager_, session_id_));
            return;
        }

    } // namespace server3
} // namespace http
