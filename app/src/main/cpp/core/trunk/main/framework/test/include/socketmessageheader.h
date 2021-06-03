/**   //NOLINT
  *@file tdsocketmessageheader.h
  *@brief Definition of
  *@version 0.1
  *@author Livisen.WAN
  *@date Created on: 2018:4:24   15:44
  *@copyright Copyright(c)
  * http://www.fubaorobot.com/
  *@howto Usage:
  *@todo
  */

#pragma once

/// stl headers
#include <cstdint>
#include <string>
#include <vector>


/*
协议头数据格式，占16个字节，以下是每个字节的说明，从0开始。
0~3 总消息长度，占4字节，包含消息头和消息体的长度。整数型数据。整数内存布局为小端模式（高地址存放最高有效位）, 以下同。
4~7消息体长度，占4字节。整数型数据。
8~9 消息数据字符集，占2字节。短整型数据，当前只支持 0 UTF8, 1 GBK
10~11 版本号，占2字节。短整型数据，当前版本号是0。
12~15 预留，占4字节。当前默认填充0。

*/
/*
struct SocketMessageHeader{
  uint32_t message_all_len;
  uint32_t message_body_len;
  uint16_t message_character_set;
  uint16_t message_version;
  uint32_t undefine;
  SocketMessageHeader() {
    message_all_len =0;
    message_body_len =0;
    message_character_set =0;
    message_version =0;
    undefine =0;
  }
};*/

/*
0~3总长度，占4字节。总长度为协议头和消息体长度的总和。
4~7当前消息索引, 占4字节。当前索引号，从0开始。
8~11消息体长度，占4字节。消息体长度。
12~15版本号，占4字节。当前版本号是0。
*/
struct SocketMessageHeader{
    uint32_t message_count;
    uint32_t message_index;
    uint32_t message_body_len;
    uint32_t message_version;
    SocketMessageHeader() {
        message_count =0;
        message_index =0;
        message_body_len =0;
        message_version =0;
    }
};

int DecodeSocketHeader(const unsigned char* data, const int data_length, SocketMessageHeader* header, std::string& error_mesages) {
    assert(data && data_length >= sizeof(SocketMessageHeader));
    if (NULL == data || data_length < sizeof(SocketMessageHeader)) {
        error_mesages = "data is null or data length is not fill headeer length";
        return -1;
    }
    /*SocketMessageHeader* pheader = reinterpret_cast<SocketMessageHeader*>((unsigned char*)data);
    assert(pheader->message_all_len == (pheader->message_body_len + sizeof(SocketMessageHeader)));
    if (!(pheader->message_all_len == (pheader->message_body_len + sizeof(SocketMessageHeader)))) {
      error_mesages = "error, decodee socket rquest header error";
      return -1;
    }*/
    SocketMessageHeader* pheader = reinterpret_cast<SocketMessageHeader*>((unsigned char*)data);

    //memcpy_s(header, sizeof(SocketMessageHeader), pheader, sizeof(SocketMessageHeader));
    memcpy(header, pheader, sizeof(SocketMessageHeader)); // need add fro linux
    return 0;
}

int DecodeSocketMessage(const unsigned char* data, const int data_length, std::vector<unsigned char>& vec_body_msg, std::string& error_mesages) {
    assert(data && data_length >= sizeof(SocketMessageHeader));
    if (NULL == data) {
        error_mesages = "data is nul";
        return -1;
    }
    if (data_length < sizeof(SocketMessageHeader)) {
        error_mesages = "data length is less than socket message header";
        return -1;
    }
    SocketMessageHeader* pheader = reinterpret_cast<SocketMessageHeader*>((unsigned char*)data);

    /*assert(pheader->message_all_len == (pheader->message_body_len + sizeof(SocketMessageHeader)));
    if (!(pheader->message_all_len == (pheader->message_body_len + sizeof(SocketMessageHeader)))) {
      error_mesages = "error, decode message header error";
      return -1;
    }*/

    vec_body_msg.reserve(pheader->message_body_len);
    const unsigned char* pbody_msg = &data[sizeof(SocketMessageHeader)];
    vec_body_msg.insert(vec_body_msg.end(), pbody_msg, pbody_msg + pheader->message_body_len);
    return 0;
}



int EncodeSocketMessage(const unsigned char* data, const int data_length, std::vector<unsigned char>& vec_msg_all) {
    SocketMessageHeader msg_header;
    msg_header.message_body_len = data_length;
    //msg_header.message_len = msg_header.message_body_len + sizeof(TDSocketMessageHeader);
    msg_header.message_count =1;
    msg_header.message_index =0;

    //std::vector<unsigned char> vec_send_msg;
    vec_msg_all.reserve(msg_header.message_body_len + sizeof(SocketMessageHeader) );

    unsigned char* pheader_char = reinterpret_cast<unsigned char*>(&msg_header);
    vec_msg_all.insert(vec_msg_all.end(),  pheader_char, pheader_char + sizeof(SocketMessageHeader));
    vec_msg_all.insert(vec_msg_all.end(), data , data + data_length);
    //vec_msg_all.push_back('\0');

    return 0;
}