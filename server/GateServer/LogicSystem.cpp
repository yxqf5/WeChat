#include "LogicSystem.h"
#include "HttpConnection.h"
#include"VarifyGrpcClient.h"
#include"RedisMgr.h"
#include"MysqlMgr.h"
#include"StatusGrpcClient.h"

LogicSystem::LogicSystem()
{
	ReqGet("/get_test", [](std::shared_ptr<HttpConnection>connection) {
		beast::ostream(connection->m_response.body())<< "receive get_test req\r\n";
		
		for (auto& elem : connection->_get_params)
		{
			beast::ostream(connection->m_response.body())<< "pararm: " << elem.first << "  value: " << elem.second << std::endl;

		}
		});

	ReqPost("/get_varrfycode", [](std::shared_ptr<HttpConnection>connection) {
		auto body_str = boost::beast::buffers_to_string(connection->m_request.body().data());
		std::cout << "the post body is " << body_str << std::endl;
		connection->m_response.set(http::field::content_type,"text/json");

		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success)
		{

			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body())<< jsonstr;

			return true;
		}

		if (!src_root.isMember("email")) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}

		auto email = src_root["email"].asString();


		GetVarifyRsp rsp = VarifyGrpcClient::GetInstance()->GetVarifyCode(email);
		std::cout << "the email is " << email << std::endl;
		root["error"] = rsp.error();
		root["email"] = src_root["email"];

		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->m_response.body()) << jsonstr;
		return true;
		});
	

	ReqPost("/user_register", [](std::shared_ptr<HttpConnection>connection) {
		auto body_str = boost::beast::buffers_to_string(connection->m_request.body().data());
		std::cout << "receive body is "<< body_str << std::endl;
		connection->m_response.set(http::field::content_type,"text/json");

		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}


		auto email = src_root["email"].asString();
		auto name = src_root["user"].asString();
		auto pwd = src_root["passwd"].asString();
		auto confirm = src_root["confirm"].asString();


		//先查找redis中email对应的验证码是否合理
		std::string varify_code;
		bool b_get_varify = RedisMgr::GetInstance()->Get("code_"+src_root["email"].asString(), varify_code);
		if (!b_get_varify) {
			std::cout << " get varify code expired" << std::endl;
			root["error"] = ErrorCodes::VarifyExpired;
			beast::ostream(connection->m_response.body())<<root.toStyledString();
			return true;
		}

		if (varify_code != src_root["varifycode"].asString()) {
			std::cout << " varify code error" << std::endl;
			root["error"] = ErrorCodes::VarifyCodeErr;
			beast::ostream(connection->m_response.body()) << root.toStyledString();
			return true;
		}


		bool b_user_exist = RedisMgr::GetInstance()->ExistsKey(src_root["user"].asString());
		if (b_user_exist) {
			std::cout << " user exist" << std::endl;
			root["error"] = ErrorCodes::UserExist;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}


		//返回100代表数据库插入成功
		//查找数据库判断用户是否存在
		int uid = MysqlMgr::GetInstance()->RegUser(name,email,pwd,"");
		if (uid == 0 || uid == -1) {
			std::cout << "user or name exist " << std::endl;
			root["error"] = ErrorCodes::UserExist;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}



		root["error"] = 0;
		root["email"] = src_root["email"];
		root["user"] = src_root["user"];
		root["passwd"] = src_root["passwd"].asString();
		root["confirm"] = src_root["confirm"].asString();
		root["varifycode"] = src_root["varifycode"];

		std::string jsonstr = root.toStyledString();

		beast::ostream(connection->m_response.body()) << jsonstr;
		return true;
		});

	//重置回调逻辑
	ReqPost("/reset_pwd", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->m_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->m_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}

		auto email = src_root["email"].asString();
		auto name = src_root["user"].asString();
		auto pwd = src_root["passwd"].asString();

		//先查找redis中email对应的验证码是否合理
		std::string  varify_code;
		bool b_get_varify = RedisMgr::GetInstance()->Get(CODEPREFIX + src_root["email"].asString(), varify_code);
		if (!b_get_varify) {
			std::cout << " get varify code expired" << std::endl;
			root["error"] = ErrorCodes::VarifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}

		if (varify_code != src_root["varifycode"].asString()) {
			std::cout << " varify code error" << std::endl;
			root["error"] = ErrorCodes::VarifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}
		//查询数据库判断用户名和邮箱是否匹配
		bool email_valid = MysqlMgr::GetInstance()->CheckEmail(name, email);
		if (!email_valid) {
			std::cout << " user email not match" << std::endl;
			root["error"] = ErrorCodes::EmailNotMatch;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}

		//更新密码为最新密码
		bool b_up = MysqlMgr::GetInstance()->UpdatePwd(name, pwd);
		if (!b_up) {
			std::cout << " update pwd failed" << std::endl;
			root["error"] = ErrorCodes::PasswdUpFailed;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}

		std::cout << "succeed to update password" << pwd << std::endl;
		root["error"] = 0;
		root["email"] = email;
		root["user"] = name;
		root["passwd"] = pwd;
		root["varifycode"] = src_root["varifycode"].asString();
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->m_response.body()) << jsonstr;
		return true;
		});

	//登录回调逻辑
	ReqPost("/user_login", [](std::shared_ptr<HttpConnection>connection) {

		auto body_str = boost::beast::buffers_to_string(connection->m_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->m_response.set(http::field::content_type, "text/json");

		Json::Value root; 
		Json::Reader reader;
		Json::Value src_root;

		bool parse_success = reader.parse(body_str, src_root);
		if(!parse_success){
			std::cout << "Failed to parse JSON data" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			beast::ostream(connection->m_response.body())<<root.toStyledString();
			return true;
		}

		std::string user = src_root["user"].asString();
		std::string passwd = src_root["passwd"].asString();
		UserInfo userInfo;

		bool pwd_valid = MysqlMgr::GetInstance()->CheckPwd(user, passwd, userInfo);
		if (!pwd_valid) {
			std::cout << " user pwd not match" << std::endl;
			root["error"] = ErrorCodes::PasswdInvalid;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}

		//查询StatusServer找到合适的连接
		auto reply = StatusGrpcClient::GetInstance()->GetChatServer(userInfo.uid);
		if (reply.error()) {
			std::cout << " grpc get chat server failed, error is " << reply.error() << std::endl;
			root["error"] = ErrorCodes::RPCFailed;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}


		std::cout << "succeed to load userinfo uid is " << userInfo.uid << std::endl;
		root["error"] = 0;
		root["user"] = userInfo.name;
		root["uid"] = userInfo.uid;
		root["token"] = reply.token();
		root["host"] = reply.host();
		root["port"] = reply.port();
		std::cout <<"the reply" << reply.host() << ":" << reply.port();

		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->m_response.body()) << jsonstr;
		return true;


		});

}


void LogicSystem::ReqGet(std::string url, HttpHandler handler)
{
	m_get_handlers.insert(make_pair(url, handler));

}

void LogicSystem::ReqPost(std::string url, HttpHandler handler)
{
	m_post_handlers.insert(make_pair(url, handler));

}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con) 
{
	if (m_get_handlers.find(path) == m_get_handlers.end())
	{
		return false;
	}

	m_get_handlers[path](con);
	return true;

}

bool LogicSystem::HandlePost(std::string path , std::shared_ptr<HttpConnection> con)
{
	if (m_post_handlers.find(path) == m_post_handlers.end()) {
		return false;
	}

	m_post_handlers[path](con);
	return true;
}

