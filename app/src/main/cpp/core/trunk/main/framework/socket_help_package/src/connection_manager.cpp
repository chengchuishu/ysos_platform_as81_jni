//
// connection_manager.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "../include/connection_manager.hpp"

#include <algorithm>

#include <boost/thread/lock_guard.hpp>
#include <boost/bind.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>

namespace http {
    namespace server3 {

        void connection_manager::add_to_manager(connection_ptr conn_ptr) {
            /// create new session id;
            boost::uuids::random_generator rgen;
            boost::uuids::uuid uid_session = rgen();
            std::string str_session_id = boost::uuids::to_string(uid_session);
            if (str_session_id.length() <=0) {
                return;
            }
            conn_ptr->set_sessionid(str_session_id);
            {
                /// add to map
                boost::lock_guard<boost::mutex> guard(mutex_session_conn_);
                //map_session_conn_.insert(std::make_pair<std::string, connection_ptr>(str_session_id, conn_ptr));
                /**
                 * template< class T1, class T2 >
                 * std::pair<T1,T2> make_pair( T1 t, T2 u );//(C++11 前)
                 * template< class T1, class T2 >
                 * std::pair<V1,V2> make_pair( T1&& t, T2&& u );//(C++11 起)-(C++14 前)
                 * template< class T1, class T2 >
                 * constexpr std::pair<V1,V2> make_pair( T1&& t, T2&& u );//(C++14 起)
                 */
                map_session_conn_.insert(std::make_pair(str_session_id, conn_ptr));
            }
            boost::system::error_code myecc;
        }

//void connection_manager::stop(connection_ptr c)
//{
//  connections_.erase(c);
//  c->stop();
//}

        int connection_manager::stop_connection(std::string session_id) {
            assert(session_id.length() >0);
            if (session_id.length() <=0) {
                return -1;
            }

            connection_ptr conn_ptr =0;
            {
                boost::lock_guard<boost::mutex> guard(mutex_session_conn_);
                std::map<std::string, connection_ptr>::iterator it_find = map_session_conn_.find(session_id);
                if (it_find == map_session_conn_.end()) {
                    return -1;   ///< not find return -1
                }
                conn_ptr = it_find->second;
                map_session_conn_.erase(it_find);
            }

            conn_ptr->close();
            conn_ptr =0;

            return 0;
        }

        void connection_manager::notify_close_connection(std::string session_id) {
            assert(session_id.length() >0);
            if (session_id.length() <=0) {
                return ;
            }

            {
                boost::lock_guard<boost::mutex> guard(mutex_session_conn_);
                std::map<std::string, connection_ptr>::iterator it_find = map_session_conn_.find(session_id);
                if (it_find == map_session_conn_.end()) {
                    return ;   ///< not find return -1
                }
                map_session_conn_.erase(it_find);
            }


            return ;
        }

        void connection_manager::stop_all() {
            boost::lock_guard<boost::mutex> guard(mutex_session_conn_);
            std::map<std::string, connection_ptr>::iterator it = map_session_conn_.begin();
            for (it; it != map_session_conn_.end(); ++it) {
                it->second->close();
            }
            map_session_conn_.clear();
        }

        int connection_manager::send_message_asyn(std::string session_id, const unsigned char* reply_data, const int reply_data_length, const bool close_connection_after_send) {
            connection_ptr conn_ptr =0;

            {
                boost::lock_guard<boost::mutex> guard(mutex_session_conn_);
                std::map<std::string, connection_ptr>::iterator it_find = map_session_conn_.find(session_id);
                if (it_find == map_session_conn_.end()) {
                    return -1;
                }
                conn_ptr = it_find->second;
            }

            return conn_ptr->send_message_asyn(reply_data, reply_data_length, close_connection_after_send);
        }

        int connection_manager::broadcast_message_asyn(const unsigned char* reply_data, const int reply_data_length) {
            boost::lock_guard<boost::mutex> guard(mutex_session_conn_);
            std::map<std::string, connection_ptr>::iterator it = map_session_conn_.begin();
            for (it; it != map_session_conn_.end(); ++it) {
                it->second->send_message_asyn(reply_data, reply_data_length, 0);
            }
            return 0;
        }

        unsigned int connection_manager::get_connection_count() {
            boost::lock_guard<boost::mutex> guard(mutex_session_conn_);
            return map_session_conn_.size();
        }

        int connection_manager::send_message_syn(std::string session_id, const unsigned char* reply_data, const int reply_data_length, const bool close_connection_after_send) {
            connection_ptr conn_ptr =0;
            {
                boost::lock_guard<boost::mutex> guard(mutex_session_conn_);
                std::map<std::string, connection_ptr>::iterator it_find = map_session_conn_.find(session_id);
                if (it_find == map_session_conn_.end()) {
                    return -1;
                }
                conn_ptr = it_find->second;
            }

            return conn_ptr->send_message_syn(reply_data, reply_data_length, close_connection_after_send);
        }


        void connection_manager::clear_timeout_connection() {
            boost::lock_guard<boost::mutex> guard(mutex_session_conn_);
            std::list<connection_ptr> list_tmp_not_valid_conn_, list_tmp_timeout_conn_;

            std::map<std::string, connection_ptr>::iterator it = map_session_conn_.begin();
            for (it; it != map_session_conn_.end(); /*++it*/) {
                if (false == it->second->is_conn_valid()) {
                    std::cout<<"conn manager clear not valid conn  sessionid:" << it->first << std::endl;
                    it->second->close();
                    it = map_session_conn_.erase(it);
                    continue;
                } else if (it->second->is_conn_timeout()) {
                    std::cout<<"conn manager clear timeout conn , sessionid:" << it->first << std::endl;
                    it->second->close();
                    it = map_session_conn_.erase(it);
                    continue;
                }
                ++it;
            }
        }


    } // namespace server
} // namespace http
