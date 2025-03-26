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
	Error_Json = 1001,  //Json解析错误
	RPCFailed = 1002,  //RPC请求错误
	VarifyExpired = 1003, //验证码过期
	VarifyCodeErr = 1004, //验证码错误
	UserExist = 1005,       //用户已经存在
	PasswdErr = 1006,    //密码错误
	EmailNotMatch = 1007,  //邮箱不匹配
	PasswdUpFailed = 1008,  //更新密码失败
	PasswdInvalid = 1009,   //密码更新失败
	TokenInvalid = 1010,   //Token失效
	UidInvalid = 1011,  //uid无效
};




// Defer类
class Defer {
public:
	// 接受一个lambda表达式或者函数指针
	Defer(std::function<void()> func) : func_(func) {}

	// 析构函数中执行传入的函数
	~Defer() {
		func_();
	}

private:
	std::function<void()> func_;
};


class ConfigMgr;
extern ConfigMgr g_config_mgr;

#define CODEPREFIX  "code_"
