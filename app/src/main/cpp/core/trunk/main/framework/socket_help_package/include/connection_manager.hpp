//
// connection_manager.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_CONNECTION_MANAGER_HPP
#define HTTP_CONNECTION_MANAGER_HPP

//#include <set>
#include <map>

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/noncopyable.hpp>

#include "connection.hpp"

namespace http {
    namespace server3 {

/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
        class connection_manager
                : private boost::noncopyable
        {
        public:
            /// Add the specified connection to the manager and start it.
            void add_to_manager(connection_ptr c);

            /// Stop the specified connection.
            //void stop(connection_ptr c);
            int stop_connection(std::string session_id);

            /// Stop all connections.
            void stop_all();

            /// 通知管理器，某一个连接关掉了, 管理器从map中删除记录
            void notify_close_connection(std::string session_id);

            /// 清理超时的连接
            void clear_timeout_connection();

            int send_message_asyn(std::string session_id, const unsigned char* reply_data, const int reply_data_length, const bool close_connection_after_send);
            int send_message_syn(std::string session_id, const unsigned char* reply_data, const int reply_data_length, const bool close_connection_after_send);
            int broadcast_message_asyn(const unsigned char* reply_data, const int reply_data_length);

            /// 得到客户端连接的数量
            unsigned int get_connection_count();
        private:
            /// The managed connections.
            //std::set<connection_ptr> connections_;
            boost::mutex mutex_session_conn_;
            std::map<std::string, connection_ptr> map_session_conn_;         ///< 所有的socket连接，根据session来映射
        };

    } // namespace server
} // namespace http

#endif // HTTP_CONNECTION_MANAGER_HPP
