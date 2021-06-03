/**   //NOLINT
  *@file clientconnection.h
  *@brief Definition of
  *@version 0.1
  *@author Livisen.Wan
  *@date Created on: 2018:4:24   22:16
  *@copyright Copyright(c) 2016 Tangdi Technology. All rights reserved.
  * http://www.fubaorobot.com/
  *@howto Usage:
  *@todo
  */

/// windows headers

/// stl headers
#include <string>
#include <vector>

/// boost headers
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

/// private headers
#include "connection.hpp"

namespace http {
    namespace server3 {
        class client_connection: public connection {
        public:
            explicit client_connection(boost::asio::io_service& io_service,
                                       connection_manager& manager,
                    //std::string& session_id,
                                       pfunHandleSocketStart fun_handle_start,
                                       pfunHandleSocketRequestHeader fun_handle_request_header,
                                       pfunHandleSocketRequestBody fun_handle_requst_body,
                                       pfunHandleSocketClose fun_handle_close,
                                       pfunHandleSocketError fun_handle_error,
                                       const int request_message_header_size
                    //const int timeout_milliseconds
            );

            int connect(const char* ip, const char* port);  ////< 客户端才会使用的函数
            int connect_asyn(const char* ip, const char* port); ////< 客户端才会使用的函数

        protected:
            void handle_resolver(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
            void handle_connect(const boost::system::error_code& error);

        protected:
            boost::asio::ip::tcp::resolver resolver_;
            std::string conn_ip_;
            std::string conn_port_;
        };

    }
}
