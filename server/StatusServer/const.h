#pragma once

#include<boost/beast/http.hpp>
#include<boost/beast.hpp>
#include<boost/asio.hpp>

#include<iostream>
#include<memory>

#include"Singleton.h"
#include<functional>
#include<map>
#include<unordered_map>

#include<json/json.h>
#include<json/value.h>
#include<json/reader.h>

#include<boost/filesystem.hpp>
#include<boost/property_tree/ptree.hpp>
#include<boost/property_tree/ini_parser.hpp>

#include<grpcpp/grpcpp.h>
#include"message.grpc.pb.h"

//RPCconPool
#include<atomic>
#include<queue>
#include<condition_variable>

#include<hiredis.h>
//#include<Win32_Interop>
#include<assert.h>
#include"ConfigMgr.h"


//mysqlPool
//#include<mysql/jdbc.h>

#include <iostream>
#include <functional>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>



#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>


namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;




enum ErrorCodes {
	Success = 0,
	Error_Json = 1001,  //Json��������
	RPCFailed = 1002,  //RPC�������
	VarifyExpired = 1003, //��֤�����
	VarifyCodeErr = 1004, //��֤�����
	UserExist = 1005,       //�û��Ѿ�����
	PasswdErr = 1006,    //�������
	EmailNotMatch = 1007,  //���䲻ƥ��
	PasswdUpFailed = 1008,  //��������ʧ��
	PasswdInvalid = 1009,   //�������ʧ��
	TokenInvalid = 1010,   //TokenʧЧ
	UidInvalid = 1011,  //uid��Ч
};




// Defer��
class Defer {
public:
	// ����һ��lambda���ʽ���ߺ���ָ��
	Defer(std::function<void()> func) : func_(func) {}

	// ����������ִ�д���ĺ���
	~Defer() {
		func_();
	}

private:
	std::function<void()> func_;
};


class ConfigMgr;
extern ConfigMgr g_config_mgr;

#define CODEPREFIX  "code_"
#define USERTOKENPREFIX  "utoken_"

#define USERIPPREFIX  "uip_"
#define USERTOKENPREFIX  "utoken_"
#define IPCOUNTPREFIX  "ipcount_"
#define USER_BASE_INFO "ubaseinfo_"
#define LOGIN_COUNT  "logincount"
#define NAME_INFO  "nameinfo_"