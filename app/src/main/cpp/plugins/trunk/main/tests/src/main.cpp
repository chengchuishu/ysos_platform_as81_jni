#include <iostream>
#include <boost/locale.hpp>
#include "../../public/include/httpclient/httpclient.h"
#include "../../../../../core/trunk/main/public/include/core_help_package/utility.h"

#include <codecvt>
#include <string>

#include <iostream>
#include <clocale>
#include <cstdlib>

#include <json/json.h>

using namespace ysos;

std::string AsciiToUtf8(const std::string &data);
std::string Utf8ToAscii(const std::string &data);



std::string AsciiToUtf8(const std::string &data) {
#ifdef WIN32
  //先把 ascii 转为 unicode
  std::wstring wstr = AcsiiToUnicode(data);
  //最后把 unicode 转为 utf8
  return UnicodeToUtf8(wstr);
#else
  std::string const &to_encoding("UTF-8");
  std::string const &from_encoding("GBK");
  boost::locale::conv::method_type how = boost::locale::conv::default_method;

  return boost::locale::conv::between(data.c_str(), to_encoding, from_encoding, how);
#endif
}

std::string Utf8ToAscii(const std::string &data) {
#ifdef WIN32
  //先把 utf8 转为 unicode
  std::wstring wstr = Utf8ToUnicode(data);
  //最后把 unicode 转为 ascii
  return UnicodeToAcsii(wstr);
#else
  std::string const &to_encoding("GBK");
  std::string const &from_encoding("UTF-8");
  boost::locale::conv::method_type how = boost::locale::conv::default_method;

  return boost::locale::conv::between(data.c_str(), to_encoding, from_encoding, how);
#endif
}

std::wstring s2ws(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
 
    return converterX.from_bytes(str);
}
 
std::string ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
 
    return converterX.to_bytes(wstr);
}



void test_nlp() {
	HttpClientInterface *http_client = new HttpClient();
	http_client->SetHttpUrl("http://47.100.169.253:8090/min/ask");
	http_client->SetHttpHeader("Content-Type","application/json;charset=UTF-8");
  //http_client->SetHttpHeader("Content-Type","application/json");
	http_client->SetHttpHeader("session_id","");
	http_client->SetHttpHeader("mall_code","8000000001");
	http_client->SetHttpHeader("machine_code","HX-00066");
	http_client->SetHttpHeader("floor",std::to_string(2));
  //---------------------------------------------------------------------POST
  /*
  /// 设置Content值
  std::string temp = "{\"business\":{\"is_wake\":0,\"location\":\"上海\",\"nav_switch\":1},\"sentence\":\"你好\"}"; 
  //temp = AsciiToUtf8(temp);
  std::string content_post(temp);
  */
   Json::FastWriter json_writer_;
   Json::Value business_set;
    business_set["nav_switch"] = 1;
    business_set["is_wake"] = 0;
    business_set["location"] = "上海";

    //Json::Value req_head;
    //req_head["Content-Type"] = "application/json";
    //req_head["session_id"] = "";
    //req_head["mall_code"] = "8000000001";
    //req_head["machine_code"] = "HX-00066";
    //req_head["floor"] = 2;

    Json::Value req_body;
    req_body["sentence"] = "你好啊";
    req_body["business"] = business_set;
    
    //Json::Value json_req;
    //json_req["REQ_HEAD"] = req_head;
    //json_req["REQ_BODY"] = req_body;
    std::string content_post = json_writer_.write ( req_body );
    //req_data = "REQ_MESSAGE=" + req_data;
    std::cout << "req_data = " << content_post <<"\n";
    content_post = GetUtility()->ReplaceAllDistinct ( content_post, "\\r\\n", "" );

  http_client->SetHttpContent(content_post.c_str(), content_post.length());

  /// 发送请求
  std::string response_post;
  int ret22 = http_client->Request(response_post, HTTP_POST);
  if (ret22 == 0) {
	 std::cout << "http response: " << response_post << std::endl;
	 std::cout << "=========================================================" << std::endl;
   response_post = boost::locale::conv::from_utf<char>(response_post.c_str(), "gbk");
   std::cout << "http response: " << response_post << std::endl;
  }
  else {
    std::cout << "http request failed: " << ret22 << std::endl;
  }
  if (NULL != http_client) {
    delete http_client;
    http_client = NULL;
  }
}

int main(void) {
  test_nlp();
  return 0;
}