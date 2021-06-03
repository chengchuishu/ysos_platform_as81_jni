#include <iostream>
#include <vector>
#include <string>

//#include <boost/locale.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

//#include <json/json.h>

#include "../include/socketclient.h"
#include "../include/socketmessageheader.h"
//#include <common/stringext.h>


SocketHandle g_client_handle_= NULL;


std::string& replace_all_distinct(std::string& str,const std::string& old_value,const std::string& new_value)
{
    for(std::string::size_type pos(0); pos!=std::string::npos; pos+=new_value.length())
    {
        if((pos=str.find(old_value,pos))!=std::string::npos)
        {
            str.replace(pos,old_value.length(),new_value);
        } else {
            break;
        }
    }
    return str;
}

//////////////////////////////////////////////////////////////////////////
int HandleSocketStart(const char* session_id, const char* remote_ip, const char* remote_port, const char* protocol) {
    //std::cout<<"-->handle socket start....  "  << session_id << "remote ip(port):" << remote_ip <<"(" << remote_port << ")" <<  std::endl;
    std::string temps = std::string("-->handle socket start.... ") + "remote ip(port):" +  remote_ip + "(" + remote_port + ")";
    std::cout << temps << std::endl;
    return 0;
}
int HandleSocketClose(const char* session_id) {
    //std::cout<<"-->handle socket close...  " << session_id << std::endl;
    std::string temps = std::string("-->handle socket close... ");

    std::cout << temps << std::endl;
    return 0;
}

int HandleSocketRequestHeader(const char* sessio_id, const unsigned char* request_data, const int request_length, int* decode_msg_body_len) {
    //std::cout<<"-->handle rquest header: ssession id:" << sessio_id <<std::endl;
    std::string str_rquestdata((const char*)request_data, request_length);
    std::string temps = std::string("-->handle rquest header:") + str_rquestdata.c_str();//boost::locale::conv::from_utf<char>(str_rquestdata.c_str(), "gbk");

    std::cout << temps << std::endl;

    /* assert(request_length == sizeof(TDSocketMessageHeader));
    assert(decode_msg_body_len);*/
    if(decode_msg_body_len) {
        *decode_msg_body_len =0;
    }
    //if(request_length != sizeof(TDSocketMessageHeader)) {
    //  return -1;
    //}
    //TDSocketMessageHeader* pheader = reinterpret_cast<TDSocketMessageHeader*>((unsigned char*)request_data);
    ////assert(pheader->message_len == (pheader->message_body_len + sizeof(TDSocketMessageHeader)));
    ////if(pheader->message_len != (pheader->message_body_len + sizeof(TDSocketMessageHeader))) {
    ////  return -1;
    ////}
    //assert(1 == pheader->message_count && 0 == pheader->message_index);
    //if(1 != pheader->message_count || 0 != pheader->message_index) {
    //  std::cout<<"error, decodee socket rquest header error" <<std::endl;
    //  return -1;
    //}

    //*decode_msg_body_len = pheader->message_body_len;

    SocketMessageHeader socket_headers;
    std::string error_message;
    int det_ret = DecodeSocketHeader(request_data, request_length, &socket_headers, error_message);
    if(0 != det_ret) {
        std::cout<<error_message <<std::endl;
        return -1;
    }
    *decode_msg_body_len = socket_headers.message_body_len;
    return 0;
}

int HandleSocketRequestBody(const char* sessio_id, const unsigned char* request_data, const int request_length) {
    std::string str_rquestdata((const char*)request_data, request_length);
    std::cout<<"-->data[len:" << request_length << "]:" << str_rquestdata << std::endl;

#ifdef TDSOCKET_VERSION_TDOS_1  ///< TDOS平台
    //str_rquestdata = boost::locale::conv::to_utf<char>(str_rquestdata.c_str(), "gbk");
#endif

    if(request_length>0 && '{' == request_data[0])
    {
        // 为了显示换行，这里处理一下。
        try {
            std::istringstream in_stream(str_rquestdata);
            boost::property_tree::ptree json_tree;
            boost::property_tree::read_json(in_stream, json_tree);

            std::ostringstream out_stream;
            boost::property_tree::write_json(out_stream, json_tree);
            str_rquestdata = out_stream.str().c_str();//boost::locale::conv::from_utf<char>(out_stream.str().c_str(), "gbk");
            str_rquestdata = replace_all_distinct(str_rquestdata, "\n", "\r\n");

            //Json::Value root_value;
            //Json::Reader json_reader;
            //std::string ansi_string = boost::locale::conv::from_utf<char>(str_rquestdata.c_str(), "gbk");
            //json_reader.parse(ansi_string, root_value, 0);
            //Json::FastWriter json_writer;
            //str_rquestdata = json_writer.write(root_value, true);

        }catch(...) {

        }
    }else {
        str_rquestdata = str_rquestdata.c_str();//boost::locale::conv::from_utf<char>(str_rquestdata.c_str(), "gbk");
    }

    std::string temps = std::string("-->handle rquest body:\r\n") + str_rquestdata;
    std::cout << temps << std::endl;

    return 0;
}

int HandleSocketError(const char* session_id, const char* error_message) {
    //std::cout<<"-->handle socket error... (" << session_id << ") " << error_message << std::endl;
    std::string temps = std::string("--->handle socket error...") + error_message;
    std::cout << temps << std::endl;
    return 0;
}


bool connnect(std::string ip, std::string port)
{
    if(NULL == g_client_handle_) {

        g_client_handle_ = SocketClient_Start(ip.c_str(), port.c_str(),
                                              HandleSocketStart,
                                              HandleSocketRequestHeader,
                                              HandleSocketRequestBody,
                                              HandleSocketClose,
                                              HandleSocketError,
                                              sizeof(SocketMessageHeader));

        if(NULL == g_client_handle_) {
            std::cout << "error : 连接不上" << std::endl;
            return false;
        }
    } else {
        SocketClient_Stop(g_client_handle_);
        g_client_handle_ = NULL;
    }
    return true;
}

void send(std::string message)
{
    if(NULL == g_client_handle_) {
        std::cout << "error : please conn first" << std::endl;
        return;
    }

    /// 转成utf9发送
    std::string utf8_string = message.c_str();//boost::locale::conv::to_utf<char>(message.c_str(), "gbk");

#ifdef TDSOCKET_VERSION_TDOS_1
    //utf8_string = message; ///< 直接传GBK数据
#endif

    std::vector<unsigned char> vec_send;
    EncodeSocketMessage((const unsigned char*)utf8_string.c_str(), utf8_string.length(), vec_send);
    int send_ret = SocketClient_SendMessageAsyn(g_client_handle_,&vec_send[0], vec_send.size(), 0);
    if(0 != send_ret) {
        std::cout << "error : send error" << std::endl;
        return;
    } else {
        std::cout << "debug : send_ret =" << send_ret << std::endl;
    }
}

bool disconnnect()
{
    if(g_client_handle_){
        SocketClient_Stop(g_client_handle_);
        g_client_handle_ = NULL;
    }
}

int main(int, char *[])
{
    std::string ip = "10.8.146.140";
    std::string port = "6002";

    bool ret = connnect(ip, port);

    std::cout << ret <<std::endl;

    std::string message = "{\"header\":{\"tag\":\"tdos\",\"version\":\"1.0.0.1\",\"type\":\"31018\",\"time_stamp\":{\"week\":2,\"day\":\"2021-03-30\"},\"session_id\":\"0\",\"serial_number\":\"0\"},\"content\":{\"from\":\"127.0.0.1:6001\",\"to\":\"127.0.0.1:6002\",\"verb\":\"DoService\",\"param\":{\"service_name\":\"Init\",\"service_param\":{\"app_name\":\"robotterminalapp\",\"strategy_name\":\"robotterminalapp\"}}}}";

    // while (true) 
    // {
    //     getline(std::cin, message, '\n'); 
    //                 // 以换行为结束，允许行内有空格
    //     std::cout << message << std::endl;
    // }

    if(ret) {
        std::cout << "==============通讯建立成功==============" <<std::endl;
        std::cout << "==============准备发送数据:" <<std::endl;
        send(message);
    } else {
        std::cout << "通讯建立失败。。。" <<std::endl;
    }

    getchar();

    return 0;
}