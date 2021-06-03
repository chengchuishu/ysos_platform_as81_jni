/****************************************************************************
#include <io.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <memory>
#include <functional>
#include "../httpclient/include/httpclient.h"
#include "../httpdownload/include/httpdownload.h"
#include "../ysoslog/include/ysoslog.h"


using namespace ysos;

std::string AsciiToUtf8(const std::string &data);
std::string Utf8ToAscii(const std::string &data);
std::wstring Utf8ToUnicode(const std::string& data);
std::string UnicodeToUtf8(const std::wstring& data);
std::string UnicodeToAcsii(const std::wstring& data);
std::wstring AcsiiToUnicode(const std::string& data);

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

std::wstring Utf8ToUnicode(const std::string& data) {
#ifdef WIN32
  int widesize = ::MultiByteToWideChar(CP_UTF8, 0, data.c_str(),data.size(), NULL, 0);
  if (widesize == ERROR_NO_UNICODE_TRANSLATION) {
    return std::wstring();
  }
  if (widesize == 0) {
    return std::wstring();
  }
  std::vector<wchar_t> resultstring(widesize+1);
  int convresult = ::MultiByteToWideChar(CP_UTF8, 0, data.c_str(),data.size(), &resultstring[0], widesize);
  if (convresult != widesize) {
    return std::wstring();
  }
  return std::wstring(&resultstring[0]);
#else
  return "";
#endif
}

std::string UnicodeToUtf8(const std::wstring& data) {
#ifdef WIN32
  int utf8size = ::WideCharToMultiByte(CP_UTF8, 0,data.c_str(),data.size(), NULL, 0, NULL, NULL);
  if (utf8size == 0) {
    return std::string("");
  }
  std::vector<char> resultstring(utf8size+1+3);
  int convresult = ::WideCharToMultiByte(CP_UTF8, 0,data.c_str(),data.size(), &resultstring[3], utf8size, NULL, NULL);
  if (convresult != utf8size) {
    return std::string("");
  }
  resultstring[0]=(char)0xEF;
  resultstring[1]=(char)0xBB;
  resultstring[2]=(char)0xBF;
  return std::string(&resultstring[0]);
#else
  return "";
#endif
}

std::string UnicodeToAcsii(const std::wstring& data) {
#ifdef WIN32
  int asciisize = ::WideCharToMultiByte(CP_OEMCP,0,data.c_str(),data.size(), NULL, 0, NULL, NULL);
  if (asciisize == ERROR_NO_UNICODE_TRANSLATION) {
    return std::string("");
  }
  if (asciisize == 0) {
    return std::string("");
  }
  std::vector<char> resultstring(asciisize+1);
  int convresult =::WideCharToMultiByte(CP_OEMCP,0,data.c_str(),data.size(),&resultstring[0], asciisize, NULL, NULL);
  if (convresult != asciisize) {
    return std::string("");
  }
  resultstring[convresult] = '\0';
  return std::string(&resultstring[0]);
#else
  return "";
#endif
}

std::wstring AcsiiToUnicode(const std::string& data) {
#ifdef WIN32
  int widesize = MultiByteToWideChar(CP_ACP,0,data.c_str(),data.size(), NULL, 0);
  if (widesize == ERROR_NO_UNICODE_TRANSLATION)  {
    return std::wstring();
  }
  if (widesize == 0) {
    return std::wstring();
  }
  std::vector<wchar_t> resultstring(widesize+1);
  int convresult = MultiByteToWideChar(CP_ACP, 0,data.c_str(),data.size(), &resultstring[0], widesize);
  if (convresult != widesize) {
    return std::wstring();
  }
  return std::wstring(&resultstring[0]);
#else
  return "";
#endif
}

//------------------------------------------------------------------------------------------------------zgl
void test_proxy() {
	HttpClientInterface *http_client = new HttpClient();
	http_client->SetHttpUrl("http://42.159.195.161:8088/tdcctp/");

  //---------------------------------------------------------------------------------------代理
  std::string proxy_server_ip = "192.168.4.101";  //10.100.18.31
  std::string proxy_server_port = "808";          //4444
  std::string proxy_server_ip_port;
  if ((!proxy_server_ip.empty()) && (!proxy_server_port.empty()))
    proxy_server_ip_port = proxy_server_ip + ":" + proxy_server_port;

  std::string proxy_server_user = "zgl";     //user1  zgl
  std::string proxy_server_password = "123"; //pwd    123
  std::string proxy_server_user_password;
  if ((!proxy_server_user.empty()) && (!proxy_server_password.empty()))
    proxy_server_user_password = proxy_server_user + ":" + proxy_server_password;

  if (!proxy_server_ip_port.empty())
    http_client->SetProxyInfo(proxy_server_ip_port, proxy_server_user_password);


	//---------------------------------------------------------------------GET
	/// 设置Content值
	std::string content_get2("");  //REQ_MESSAGE={\"REQ_BODY\":{\"orgId\":\"000000011\",\"tagNo\":\"\",\"termId\":\"00000148\"},\"REQ_HEAD\":{\"caller\":\"tdupdate_module\"}}
	http_client->SetHttpContent(content_get2.c_str(), content_get2.length());

	/// 发送请求
	std::string response_get2;
	int ret21 = http_client->Request(response_get2, HTTP_GET);
	if (ret21 == 0)
		std::cout << "http response: " << Utf8ToAscii(response_get2) << std::endl;
	else
		std::cout << "http request failed: " << ret21 << std::endl;

  //---------------------------------------------------------------------POST
  /// 设置Content值
  std::string content_post("");  //REQ_MESSAGE={\"REQ_BODY\":{\"orgId\":\"000000011\",\"tagNo\":\"\",\"termId\":\"00000148\"},\"REQ_HEAD\":{\"caller\":\"tdupdate_module\"}}
  http_client->SetHttpContent(content_post.c_str(), content_post.length());

  /// 发送请求
  std::string response_post;
  int ret22 = http_client->Request(response_post, HTTP_POST);
  if (ret22 == 0)
    std::cout << "http response: " << Utf8ToAscii(response_post) << std::endl;
  else
    std::cout << "http request failed: " << ret22 << std::endl;

  if (NULL != http_client) {
    delete http_client;
    http_client = NULL;
  }
}

//------------------------------------------------------------------------------------------------------zgl
void test_upload_file_stream_data() {
  HttpClientInterface *http_client = new HttpClient();
  //http_client->SetHttpUrl("http://192.168.1.79:9001/transfer_file?TargetPathFile=D:/test2.png");  //127.0.0.1 FileType=png
  //http_client->SetHttpUrl("http://10.7.5.88:8080/gs-robot/cmd/sync_camera_image?fileType=png");

  FILE *f = fopen("D:\\png\\test2.png", "rb");
  int file_length = _filelength(_fileno(f));
  char *buffer = (char *)malloc(file_length);
  fread(buffer, file_length, 1, f);
  fclose(f);

  std::string s_buffer;
  s_buffer.assign(buffer, file_length);
  free(buffer);
  
  http_client->SetHttpContent(s_buffer.c_str(), s_buffer.length());
  http_client->SetTimeOut(300, 300);  //TimeOut

  /// 发送请求
  std::string response_post;
  int ret22 = http_client->Request(response_post, HTTP_POST);
  if (ret22 == 0)
    std::cout << "http response: " << Utf8ToAscii(response_post) << std::endl;
  else
    std::cout << "http request failed: " << ret22 << std::endl;

  if (NULL != http_client) {
    delete http_client;
    http_client = NULL;
  }
}


#if 1
//------------------------------------------------------------------------------------------------------zgl
int main(void) {
  //--------------------------------zgl
  //test_proxy();
  test_upload_file_stream_data();

  return 0;
}
#endif


// #if 0
// int main(void) {
//   HttpClientInterface *http_client = new HttpClient();
//
//   /// 设置url
//   //http_client->SetHttpUrl("http://yuyi.tangdi.net:8080/CSRBroker/queryAction");
//   http_client->SetHttpUrl("http://jqrcs.tangdi.net:8088/tdcctp/AV0001.json");
//   //http_client->SetHttpUrl("http://jqrcs.tangdi.net:8088/tdcctp/AV0002.json");
//   /// 设置Header值
//   /*http_client->SetHtttpHeader("protocolId", "5");
//   http_client->SetHtttpHeader("robotHashCode", "tdrobot2");
//   http_client->SetHtttpHeader("platformConnType", "2");
//   http_client->SetHtttpHeader("userId", "100031");
//   http_client->SetHtttpHeader("talkerId", "100011");
//   http_client->SetHtttpHeader("receiverId", "100111");
//   http_client->SetHtttpHeader("appKey", "ac5d5452");
//   http_client->SetHtttpHeader("type", "text");
//   http_client->SetHtttpHeader("msgID", "1111");
//   http_client->SetHtttpHeader("isQuestionQuery", "0");
//   http_client->SetHtttpHeader("query", "0");*/
//   /// 设置Content值
//   //std::string content("appKey=ac5d5452&isQuestionQuery=0&msgID=1111&platformConnType=2&protocolId=5&query=浣犲ソ&receiverId=600000000&robotHashCode=tdrobot2&talkerId=600000000&type=text&userId=600000000");
//   std::string content("REQ_MESSAGE={\"REQ_BODY\":{\"orgId\":\"000000011\",\"tagNo\":\"\",\"termId\":\"00000148\"},\"REQ_HEAD\":{\"caller\":\"tdupdate_module\"}}");
//   //std::string content("REQ_MESSAGE={\"REQ_BODY\":{\"orgId\":\"000000011\",\"termId\":\"00000148\"},\"REQ_HEAD\":{\"caller\":\"tdupdate_module\"}}");
//   http_client->SetHttpContent(content.c_str(), content.length());
//
//   do {
//     /// 发送请求
//     std::string response;
//     int ret = http_client->Request(response);
//     if (ret != 0) {
//       std::cout << "http request failed: " << ret << std::endl;
//     } else {
//       std::cout << "http response: " << Utf8ToAscii(response) << std::endl;
//     }
//
//     // ::Sleep(1000);
//   } while (true);
//
//   delete http_client;
//
//   return 0;
// }
//
// #else
//
// void ProgressCallback(
//   const std::string& remote_file_path/*文件远程路径*/,
//   const std::string& local_file_path/*文件本地路径*/,
//   const double& total_time/*已下载花费的总时间，单位为秒*/,
//   const double& progress_rate/*已下载文件百分比，单位为%*/,
//   const double& required_time/*下载所需时间，单位为秒*/,
//   const double& average_download_speed/*平均下载速度，单位为千字节/秒*/,
//   const double& total_file_size/*文件总大小，单位为千字节*/,
//   const double& download_file_size/*已下载文件大小，单位为千字节*/) {
//
//   std::cout << "==============================================" << std::endl;
//   std::cout << "==============================================" << std::endl;
//   std::cout << "==============================================" << std::endl;
//   std::cout << "已下载花费的总时间: " << total_time << " (秒) " <<std::endl;
//   std::cout << "已下载文件百分比: " << progress_rate << " (%) " <<std::endl;
//   std::cout << "下载所需时间: " << required_time << " (秒) " <<std::endl;
//   std::cout << "平均下载速度: " << average_download_speed << " (千字节/秒)" <<std::endl;
//   std::cout << "文件总大小: " << total_file_size << " (千字节)" <<std::endl;
//   std::cout << "已下载文件大小: " << download_file_size << " (千字节) " <<std::endl;
//   std::cout << "文件远程路径: " << remote_file_path << std::endl;
//   std::cout << "文件本地路径: " << local_file_path << std::endl;
//   std::cout << std::endl;
//   std::cout << std::endl;
//   std::cout << std::endl;
//
//   return;
// }
//
// int main(void) {
//   int result = 0;
//
//   HTTPDownloadInterface* http_download = NULL;
//   do {
//     http_download = new HTTPDownload();
//     if (NULL == http_download) {
//       break;
//     }
//
//     if (0 != http_download->Init(std::bind(ProgressCallback,
//                                            std::placeholders::_1,
//                                            std::placeholders::_2,
//                                            std::placeholders::_3,
//                                            std::placeholders::_4,
//                                            std::placeholders::_5,
//                                            std::placeholders::_6,
//                                            std::placeholders::_7,
//                                            std::placeholders::_8))) {
//       break;
//     }
//
//     std::string remote_server_file_path = "http://jqrcs.tangdi.net:8080/appVersionFile/1101/710000392.exe";
//     std::string local_file_path = "C:\\YSOSUpdate\\710000392.exe";
//     std::string md5 = "e20c7989978374f37534563948125dcb";
//     if (YSOS_HTTP_DOWNLOAD_ERROR_SUCCEED != http_download->StartDownload(remote_server_file_path, local_file_path, md5)) {
//       break;
//     }
//
//     std::string local_file_md5;
//     if (YSOS_HTTP_DOWNLOAD_ERROR_SUCCEED != HTTPDownload::CalcMD5(local_file_path, local_file_md5)) {
//       break;
//     }
//
//     if (YSOS_HTTP_DOWNLOAD_ERROR_SUCCEED != http_download->StopDownload()) {
//       break;
//     }
//   } while (0);
//
//   if (NULL != http_download) {
//     if (YSOS_HTTP_DOWNLOAD_ERROR_SUCCEED != http_download->Uninit()) {
//       std::cout << "The function[http_download->Uninit()] is failed" << std::endl;
//     }
//
//     delete http_download;
//     http_download = NULL;
//   }
//
//   return result;
// }
// #endif

/*
/// for ysoslog
log4cplus::Logger logger_;

void main(void) {

  if (true != ysos::log::InitLoggerExt("./ItmDevCtlCtrl.log", "ItmDevCtlCtrl")) {
    return;
  }

  /// 判断logger是否可用
  logger_ = ysos::log::GetLogger("ItmDevCtlCtrl");
  if (true != logger_.valid()) {
    return;
  }

  YSOS_LOG_DEBUG("####################################log test start#########################################");
  while (1) {
    YSOS_LOG_DEBUG("log test");
    YSOS_LOG_DEBUG("log test[" << 12345 << "]");
    Sleep(10);
  }

  return;
}
*/