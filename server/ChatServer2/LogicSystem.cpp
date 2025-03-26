﻿
#include "LogicSystem.h"
#include "StatusGrpcClient.h"
#include "MysqlMgr.h"
#include "const.h"
#include "RedisMgr.h"
#include "UserMgr.h"
#include "ChatGrpcClient.h"
//#include "data.h"

using namespace std;

LogicSystem::LogicSystem() :_b_stop(false) {
	RegisterCallBacks();
	_worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem() {
	_b_stop = true;
	_consume.notify_one();
	_worker_thread.join();
}

void LogicSystem::PostMsgToQue(shared_ptr < LogicNode> msg) {
	std::unique_lock<std::mutex> unique_lk(_mutex);
	_msg_que.push(msg);
	
	if (_msg_que.size() == 1) {
		unique_lk.unlock();
		_consume.notify_one();
	}
}

void LogicSystem::DealMsg() {
	for (;;) {
		std::unique_lock<std::mutex> unique_lk(_mutex);
		
		while (_msg_que.empty() && !_b_stop) {
			_consume.wait(unique_lk);
		}

		
		if (_b_stop) {
			while (!_msg_que.empty()) {
				auto msg_node = _msg_que.front();
				cout << "recv_msg id  is " << msg_node->_recvnode->_msg_id << endl;
				auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
				if (call_back_iter == _fun_callbacks.end()) {
					_msg_que.pop();
					continue;
				}
				call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
					std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
				_msg_que.pop();
			}
			break;
		}

		
		auto msg_node = _msg_que.front();
		cout << "recv_msg id  is " << msg_node->_recvnode->_msg_id << endl;
		auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
		if (call_back_iter == _fun_callbacks.end()) {
			_msg_que.pop();
			std::cout << "msg id [" << msg_node->_recvnode->_msg_id << "] handler not found" << std::endl;
			continue;
		}
		call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
			std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
		_msg_que.pop();
	}
}

void LogicSystem::RegisterCallBacks() {
	_fun_callbacks[MSG_CHAT_LOGIN] = std::bind(&LogicSystem::LoginHandler, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

	//_fun_callbacks[ID_SEARCH_USER_REQ] = std::bind(&LogicSystem::SearchInfo, this,
	//	placeholders::_1, placeholders::_2, placeholders::_3);

	_fun_callbacks[ID_ADD_FRIEND_REQ] = std::bind(&LogicSystem::AddFriendApply, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

	_fun_callbacks[ID_AUTH_FRIEND_REQ] = std::bind(&LogicSystem::AuthFriendApply, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

	_fun_callbacks[ID_TEXT_CHAT_MSG_REQ] = std::bind(&LogicSystem::DealChatTextMsg, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

}

bool LogicSystem::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
	//���Ȳ�redis�в�ѯ�û���Ϣ
	std::string info_str = "";
	bool b_base = RedisMgr::GetInstance()->Get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		userinfo->uid = root["uid"].asInt();
		userinfo->name = root["name"].asString();
		userinfo->pwd = root["pwd"].asString();
		userinfo->email = root["email"].asString();
		userinfo->nick = root["nick"].asString();
		userinfo->desc = root["desc"].asString();
		userinfo->sex = root["sex"].asInt();
		userinfo->icon = root["icon"].asString();
		std::cout << "user login uid is  " << userinfo->uid << " name  is "
			<< userinfo->name << " pwd is " << userinfo->pwd << " email is " << userinfo->email << endl;
	}
	else {

		std::shared_ptr<UserInfo> user_info = nullptr;
		user_info = MysqlMgr::GetInstance()->GetUser(uid);
		if (user_info == nullptr) {
			return false;
		}

		userinfo = user_info;

		Json::Value redis_root;
		redis_root["uid"] = uid;
		redis_root["pwd"] = userinfo->pwd;
		redis_root["name"] = userinfo->name;
		redis_root["email"] = userinfo->email;
		redis_root["nick"] = userinfo->nick;
		redis_root["desc"] = userinfo->desc;
		redis_root["sex"] = userinfo->sex;
		redis_root["icon"] = userinfo->icon;
		RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());
	}

	return true;
}

void LogicSystem::LoginHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();
	std::cout << "user login uid is  " << uid << " user token  is "
		<< root["token"].asString() << endl;


	Json::Value  rtvalue;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, MSG_CHAT_LOGIN_RSP);
		});

	//从redis获取用户token是否正确
	int uid_str =uid;
	std::string token_key = USERTOKENPREFIX + std::to_string(uid_str);
	std::string token_value = "";
	bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
	if (!success) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	if (token_value != token) {
		rtvalue["error"] = ErrorCodes::TokenInvalid;
		return;
	}

	rtvalue["error"] = ErrorCodes::Success;

	std::string base_key = USER_BASE_INFO + std::to_string(uid_str);
	auto user_info = std::make_shared<UserInfo>();
	bool b_base = GetBaseInfo(base_key, uid, user_info);
	if (!b_base) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
	rtvalue["uid"] = uid;
	rtvalue["pwd"] = user_info->pwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;

	//从数据库获取申请列表

	//获取好友列表

	auto& config = ConfigMgr::Inst();
	
	auto server_name = config["SelfServer"]["Name"];
	
	//将登录数量增加
	auto rd_res = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server_name);
	int count = 0;
	if (!rd_res.empty()) {
		count = std::stoi(rd_res);
	}

	count++;

	auto count_str = std::to_string(count);
	RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, count_str);

	//session绑定用户uid
	session->SetUserId(uid);

	//为用户设置登录ip server的名字
	std::string  ipkey = USERIPPREFIX + std::to_string(uid_str);
	RedisMgr::GetInstance()->Set(ipkey, server_name);

	//uid和session绑定管理,方便以后踢人操作
	UserMgr::GetInstance()->SetUserSession(uid, session);

	return;
}


void LogicSystem::AddFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto my_uid = root["myuid"].asInt();
	auto toName = root["toname"].asString();
	auto toemail = root["toemail"].asString();

	//通过姓名查找到要添加用户的所有信息
	//,被添加者
   std::shared_ptr<UserInfo> applyUserInfo = MysqlMgr::GetInstance()->GetUser(toName);

   // 添加者
   std::shared_ptr<UserInfo> theapply_UserInfo =  MysqlMgr::GetInstance()->GetUser(my_uid);
	auto to_uid = applyUserInfo->uid;

	std::cout << "user login uid is  " << my_uid << " applyname  is "
		<< toName << " applyEmail is " << toemail << " touid is " << to_uid << endl;

	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;

	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		//给发送请求的客户端回包,通知服务端接收到你的请求了
		session->Send(return_str, ID_ADD_FRIEND_RSP);
		});


	//先更新数据库
	MysqlMgr::GetInstance()->AddFriendApply(my_uid, to_uid);

	//查询redis 查找touid对应的server ip
	auto to_str = std::to_string(to_uid);
	auto to_ip_key = USERIPPREFIX + to_str;
	std::string to_ip_value = "";
	bool b_ip = RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
	if (!b_ip) {
		return;
	}

	auto& cfg = ConfigMgr::Inst();
	auto self_name = cfg["SelfServer"]["Name"];

	//直接通知对方有申请消息
	if (to_ip_value == self_name) {
		auto session = UserMgr::GetInstance()->GetSession(to_uid);

		//session不为空,代表对方在线,然后发送通知给对方
		if (session) {
			//在内存中则直接发送通知对方
			Json::Value  notify;
			notify["error"] = ErrorCodes::Success;
			notify["applyuid"] = my_uid;
			notify["name"] = theapply_UserInfo->name;
			notify["desc"] = "";
			std::string return_str = notify.toStyledString();
			session->Send(return_str, ID_NOTIFY_ADD_FRIEND_REQ);
		}

		return;
	}

	std::string base_key = USER_BASE_INFO + std::to_string(my_uid);
	auto apply_info = std::make_shared<UserInfo>();
	bool b_info = GetBaseInfo(base_key, my_uid, apply_info);

	AddFriendReq add_req;
	add_req.set_applyuid(my_uid);
	add_req.set_touid(to_uid);
	add_req.set_name(theapply_UserInfo->name);
	add_req.set_desc("");
	if (b_info) {
		add_req.set_icon(apply_info->icon);
		add_req.set_sex(apply_info->sex);
		add_req.set_nick(apply_info->nick);
	}

	//发送通知
	ChatGrpcClient::GetInstance()->NotifyAddFriend(to_ip_value, add_req);
}


void LogicSystem::AuthFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {

	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);

	auto uid = root["fromuid"].asInt();
	auto touid = root["touid"].asInt();
	auto back_name = root["back"].asString();
	std::cout << "from " << uid << " auth friend to " << touid << std::endl;

	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	auto user_info = std::make_shared<UserInfo>();

	std::string base_key = USER_BASE_INFO + std::to_string(touid);
	bool b_info = GetBaseInfo(base_key, touid, user_info);
	if (b_info) {
		rtvalue["name"] = user_info->name;
		rtvalue["nick"] = user_info->nick;
		rtvalue["icon"] = user_info->icon;
		rtvalue["sex"] = user_info->sex;
		rtvalue["uid"] = touid;
	}
	else {
		rtvalue["error"] = ErrorCodes::UidInvalid;
	}


	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, ID_AUTH_FRIEND_RSP);
		});

	//先更新数据库
	MysqlMgr::GetInstance()->AuthFriendApply(uid, touid);

	//更新数据库添加好友
	MysqlMgr::GetInstance()->AddFriend(uid, touid, back_name);

	//从redis查找到touid对应的server_ip
	auto to_str = std::to_string(touid);
	auto to_ip_key = USERIPPREFIX + to_str;
	std::string to_ip_value = "";
	bool b_ip = RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
	if (!b_ip) {
		return;
	}

	auto& cfg = ConfigMgr::Inst();
	auto self_name = cfg["SelfServer"]["Name"];

	if (to_ip_value == self_name) {
		auto session = UserMgr::GetInstance()->GetSession(touid);
		if (session) {
		
			Json::Value  notify;
			notify["error"] = ErrorCodes::Success;
			notify["fromuid"] = uid;
			notify["touid"] = touid;
			std::string base_key = USER_BASE_INFO + std::to_string(uid);
			auto user_info = std::make_shared<UserInfo>();
			bool b_info = GetBaseInfo(base_key, uid, user_info);
			if (b_info) {
				notify["name"] = user_info->name;
				notify["nick"] = user_info->nick;
				notify["icon"] = user_info->icon;
				notify["sex"] = user_info->sex;
			}
			else {
				notify["error"] = ErrorCodes::UidInvalid;
			}


			std::string return_str = notify.toStyledString();
			session->Send(return_str, ID_NOTIFY_AUTH_FRIEND_REQ);
		}

		return;
	}


	AuthFriendReq auth_req;
	auth_req.set_fromuid(uid);
	auth_req.set_touid(touid);

	
	ChatGrpcClient::GetInstance()->NotifyAuthFriend(to_ip_value, auth_req);
}


void LogicSystem::DealChatTextMsg(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);

	auto uid = root["fromuid"].asInt();
	auto touid = root["touid"].asInt();

	const Json::Value  arrays = root["text_array"];

	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["text_array"] = arrays;
	rtvalue["fromuid"] = uid;
	rtvalue["touid"] = touid;

	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, ID_TEXT_CHAT_MSG_RSP);
		});


	//查询redis 查找touid对应的server ip
	auto to_str = std::to_string(touid);
	auto to_ip_key = USERIPPREFIX + to_str;
	std::string to_ip_value = "";
	bool b_ip = RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
	if (!b_ip) {
		return;
	}

	auto& cfg = ConfigMgr::Inst();
	auto self_name = cfg["SelfServer"]["Name"];
	//直接通知对方有认证通过消息
	if (to_ip_value == self_name) {
		auto session = UserMgr::GetInstance()->GetSession(touid);
		if (session) {
			//在内存中则直接发送通知对方
			std::string return_str = rtvalue.toStyledString();
			session->Send(return_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);
		}

		return;
	}


	TextChatMsgReq text_msg_req;
	text_msg_req.set_fromuid(uid);
	text_msg_req.set_touid(touid);
	for (const auto& txt_obj : arrays) {
		auto content = txt_obj["content"].asString();
		auto msgid = txt_obj["msgid"].asString();
		std::cout << "content is " << content << std::endl;
		std::cout << "msgid is " << msgid << std::endl;
		auto* text_msg = text_msg_req.add_textmsgs();
		text_msg->set_msgid(msgid);
		text_msg->set_msgcontent(content);
	}


	//发送通知 todo...
	ChatGrpcClient::GetInstance()->NotifyTextChatMsg(to_ip_value, text_msg_req, rtvalue);
}


//void LogicSystem::SearchInfo(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
//{
//	Json::Reader reader;
//	Json::Value root;
//	reader.parse(msg_data, root);
//	auto uid_str = root["uid"].asString();
//	std::cout << "user SearchInfo uid is  " << uid_str << endl;
//
//	Json::Value  rtvalue;
//
//	Defer defer([this, &rtvalue, session]() {
//		std::string return_str = rtvalue.toStyledString();
//		session->Send(return_str, ID_SEARCH_USER_RSP);
//		});
//
//	bool b_digit = isPureDigit(uid_str);
//	if (b_digit) {
//		GetUserByUid(uid_str, rtvalue);
//	}
//	else {
//		GetUserByName(uid_str, rtvalue);
//	}
//	return;
//}
//
//void LogicSystem::AddFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
//{
//	Json::Reader reader;
//	Json::Value root;
//	reader.parse(msg_data, root);
//	auto uid = root["uid"].asInt();
//	auto applyname = root["applyname"].asString();
//	auto bakname = root["bakname"].asString();
//	auto touid = root["touid"].asInt();
//	std::cout << "user login uid is  " << uid << " applyname  is "
//		<< applyname << " bakname is " << bakname << " touid is " << touid << endl;
//
//	Json::Value  rtvalue;
//	rtvalue["error"] = ErrorCodes::Success;
//	Defer defer([this, &rtvalue, session]() {
//		std::string return_str = rtvalue.toStyledString();
//		session->Send(return_str, ID_ADD_FRIEND_RSP);
//		});
//
//	//�ȸ������ݿ�
//	MysqlMgr::GetInstance()->AddFriendApply(uid, touid);
//
//	//��ѯredis ����touid��Ӧ��server ip
//	auto to_str = std::to_string(touid);
//	auto to_ip_key = USERIPPREFIX + to_str;
//	std::string to_ip_value = "";
//	bool b_ip = RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
//	if (!b_ip) {
//		return;
//	}
//
//
//	auto& cfg = ConfigMgr::Inst();
//	auto self_name = cfg["SelfServer"]["Name"];
//
//
//	std::string base_key = USER_BASE_INFO + std::to_string(uid);
//	auto apply_info = std::make_shared<UserInfo>();
//	bool b_info = GetBaseInfo(base_key, uid, apply_info);
//
//	//ֱ��֪ͨ�Է���������Ϣ
//	if (to_ip_value == self_name) {
//		auto session = UserMgr::GetInstance()->GetSession(touid);
//		if (session) {
//			//���ڴ�����ֱ�ӷ���֪ͨ�Է�
//			Json::Value  notify;
//			notify["error"] = ErrorCodes::Success;
//			notify["applyuid"] = uid;
//			notify["name"] = applyname;
//			notify["desc"] = "";
//			if (b_info) {
//				notify["icon"] = apply_info->icon;
//				notify["sex"] = apply_info->sex;
//				notify["nick"] = apply_info->nick;
//			}
//			std::string return_str = notify.toStyledString();
//			session->Send(return_str, ID_NOTIFY_ADD_FRIEND_REQ);
//		}
//
//		return;
//	}
//
//
//	AddFriendReq add_req;
//	add_req.set_applyuid(uid);
//	add_req.set_touid(touid);
//	add_req.set_name(applyname);
//	add_req.set_desc("");
//	if (b_info) {
//		add_req.set_icon(apply_info->icon);
//		add_req.set_sex(apply_info->sex);
//		add_req.set_nick(apply_info->nick);
//	}
//
//	//����֪ͨ
//	ChatGrpcClient::GetInstance()->NotifyAddFriend(to_ip_value, add_req);
//
//}
//
//void LogicSystem::AuthFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
//
//	Json::Reader reader;
//	Json::Value root;
//	reader.parse(msg_data, root);
//
//	auto uid = root["fromuid"].asInt();
//	auto touid = root["touid"].asInt();
//	auto back_name = root["back"].asString();
//	std::cout << "from " << uid << " auth friend to " << touid << std::endl;
//
//	Json::Value  rtvalue;
//	rtvalue["error"] = ErrorCodes::Success;
//	auto user_info = std::make_shared<UserInfo>();
//
//	std::string base_key = USER_BASE_INFO + std::to_string(touid);
//	bool b_info = GetBaseInfo(base_key, touid, user_info);
//	if (b_info) {
//		rtvalue["name"] = user_info->name;
//		rtvalue["nick"] = user_info->nick;
//		rtvalue["icon"] = user_info->icon;
//		rtvalue["sex"] = user_info->sex;
//		rtvalue["uid"] = touid;
//	}
//	else {
//		rtvalue["error"] = ErrorCodes::UidInvalid;
//	}
//
//
//	Defer defer([this, &rtvalue, session]() {
//		std::string return_str = rtvalue.toStyledString();
//		session->Send(return_str, ID_AUTH_FRIEND_RSP);
//		});
//
//	//�ȸ������ݿ�
//	MysqlMgr::GetInstance()->AuthFriendApply(uid, touid);
//
//	//�������ݿ����Ӻ���
//	MysqlMgr::GetInstance()->AddFriend(uid, touid, back_name);
//
//	//��ѯredis ����touid��Ӧ��server ip
//	auto to_str = std::to_string(touid);
//	auto to_ip_key = USERIPPREFIX + to_str;
//	std::string to_ip_value = "";
//	bool b_ip = RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
//	if (!b_ip) {
//		return;
//	}
//
//	auto& cfg = ConfigMgr::Inst();
//	auto self_name = cfg["SelfServer"]["Name"];
//	//ֱ��֪ͨ�Է�����֤ͨ����Ϣ
//	if (to_ip_value == self_name) {
//		auto session = UserMgr::GetInstance()->GetSession(touid);
//		if (session) {
//			//���ڴ�����ֱ�ӷ���֪ͨ�Է�
//			Json::Value  notify;
//			notify["error"] = ErrorCodes::Success;
//			notify["fromuid"] = uid;
//			notify["touid"] = touid;
//			std::string base_key = USER_BASE_INFO + std::to_string(uid);
//			auto user_info = std::make_shared<UserInfo>();
//			bool b_info = GetBaseInfo(base_key, uid, user_info);
//			if (b_info) {
//				notify["name"] = user_info->name;
//				notify["nick"] = user_info->nick;
//				notify["icon"] = user_info->icon;
//				notify["sex"] = user_info->sex;
//			}
//			else {
//				notify["error"] = ErrorCodes::UidInvalid;
//			}
//
//
//			std::string return_str = notify.toStyledString();
//			session->Send(return_str, ID_NOTIFY_AUTH_FRIEND_REQ);
//		}
//
//		return;
//	}
//
//
//	AuthFriendReq auth_req;
//	auth_req.set_fromuid(uid);
//	auth_req.set_touid(touid);
//
//	//����֪ͨ
//	ChatGrpcClient::GetInstance()->NotifyAuthFriend(to_ip_value, auth_req);
//}
//
//void LogicSystem::DealChatTextMsg(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
//	Json::Reader reader;
//	Json::Value root;
//	reader.parse(msg_data, root);
//
//	auto uid = root["fromuid"].asInt();
//	auto touid = root["touid"].asInt();
//
//	const Json::Value  arrays = root["text_array"];
//
//	Json::Value  rtvalue;
//	rtvalue["error"] = ErrorCodes::Success;
//	rtvalue["text_array"] = arrays;
//	rtvalue["fromuid"] = uid;
//	rtvalue["touid"] = touid;
//
//	Defer defer([this, &rtvalue, session]() {
//		std::string return_str = rtvalue.toStyledString();
//		session->Send(return_str, ID_TEXT_CHAT_MSG_RSP);
//		});
//
//
//	//��ѯredis ����touid��Ӧ��server ip
//	auto to_str = std::to_string(touid);
//	auto to_ip_key = USERIPPREFIX + to_str;
//	std::string to_ip_value = "";
//	bool b_ip = RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
//	if (!b_ip) {
//		return;
//	}
//
//	auto& cfg = ConfigMgr::Inst();
//	auto self_name = cfg["SelfServer"]["Name"];
//	//ֱ��֪ͨ�Է�����֤ͨ����Ϣ
//	if (to_ip_value == self_name) {
//		auto session = UserMgr::GetInstance()->GetSession(touid);
//		if (session) {
//			//���ڴ�����ֱ�ӷ���֪ͨ�Է�
//			std::string return_str = rtvalue.toStyledString();
//			session->Send(return_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);
//		}
//
//		return;
//	}
//
//
//	TextChatMsgReq text_msg_req;
//	text_msg_req.set_fromuid(uid);
//	text_msg_req.set_touid(touid);
//	for (const auto& txt_obj : arrays) {
//		auto content = txt_obj["content"].asString();
//		auto msgid = txt_obj["msgid"].asString();
//		std::cout << "content is " << content << std::endl;
//		std::cout << "msgid is " << msgid << std::endl;
//		auto* text_msg = text_msg_req.add_textmsgs();
//		text_msg->set_msgid(msgid);
//		text_msg->set_msgcontent(content);
//	}
//
//
//	//����֪ͨ todo...
//	ChatGrpcClient::GetInstance()->NotifyTextChatMsg(to_ip_value, text_msg_req, rtvalue);
//}
//
//
//
//bool LogicSystem::isPureDigit(const std::string& str)
//{
//	for (char c : str) {
//		if (!std::isdigit(c)) {
//			return false;
//		}
//	}
//	return true;
//}

//void LogicSystem::GetUserByUid(std::string uid_str, Json::Value& rtvalue)
//{
//	rtvalue["error"] = ErrorCodes::Success;
//
//	std::string base_key = USER_BASE_INFO + uid_str;
//
//	//���Ȳ�redis�в�ѯ�û���Ϣ
//	std::string info_str = "";
//	bool b_base = RedisMgr::GetInstance()->Get(base_key, info_str);
//	if (b_base) {
//		Json::Reader reader;
//		Json::Value root;
//		reader.parse(info_str, root);
//		auto uid = root["uid"].asInt();
//		auto name = root["name"].asString();
//		auto pwd = root["pwd"].asString();
//		auto email = root["email"].asString();
//		auto nick = root["nick"].asString();
//		auto desc = root["desc"].asString();
//		auto sex = root["sex"].asInt();
//		auto icon = root["icon"].asString();
//		std::cout << "user  uid is  " << uid << " name  is "
//			<< name << " pwd is " << pwd << " email is " << email << " icon is " << icon << endl;
//
//		rtvalue["uid"] = uid;
//		rtvalue["pwd"] = pwd;
//		rtvalue["name"] = name;
//		rtvalue["email"] = email;
//		rtvalue["nick"] = nick;
//		rtvalue["desc"] = desc;
//		rtvalue["sex"] = sex;
//		rtvalue["icon"] = icon;
//		return;
//	}
//
//	auto uid = std::stoi(uid_str);
//	//redis��û�����ѯmysql
//	//��ѯ���ݿ�
//	std::shared_ptr<UserInfo> user_info = nullptr;
//	user_info = MysqlMgr::GetInstance()->GetUser(uid);
//	if (user_info == nullptr) {
//		rtvalue["error"] = ErrorCodes::UidInvalid;
//		return;
//	}
//
//	//�����ݿ�����д��redis����
//	Json::Value redis_root;
//	redis_root["uid"] = user_info->uid;
//	redis_root["pwd"] = user_info->pwd;
//	redis_root["name"] = user_info->name;
//	redis_root["email"] = user_info->email;
//	redis_root["nick"] = user_info->nick;
//	redis_root["desc"] = user_info->desc;
//	redis_root["sex"] = user_info->sex;
//	redis_root["icon"] = user_info->icon;
//
//	RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());
//
//	//��������
//	rtvalue["uid"] = user_info->uid;
//	rtvalue["pwd"] = user_info->pwd;
//	rtvalue["name"] = user_info->name;
//	rtvalue["email"] = user_info->email;
//	rtvalue["nick"] = user_info->nick;
//	rtvalue["desc"] = user_info->desc;
//	rtvalue["sex"] = user_info->sex;
//	rtvalue["icon"] = user_info->icon;
//}
//
//void LogicSystem::GetUserByName(std::string name, Json::Value& rtvalue)
//{
//	rtvalue["error"] = ErrorCodes::Success;
//
//	std::string base_key = NAME_INFO + name;
//
//	//���Ȳ�redis�в�ѯ�û���Ϣ
//	std::string info_str = "";
//	bool b_base = RedisMgr::GetInstance()->Get(base_key, info_str);
//	if (b_base) {
//		Json::Reader reader;
//		Json::Value root;
//		reader.parse(info_str, root);
//		auto uid = root["uid"].asInt();
//		auto name = root["name"].asString();
//		auto pwd = root["pwd"].asString();
//		auto email = root["email"].asString();
//		auto nick = root["nick"].asString();
//		auto desc = root["desc"].asString();
//		auto sex = root["sex"].asInt();
//		std::cout << "user  uid is  " << uid << " name  is "
//			<< name << " pwd is " << pwd << " email is " << email << endl;
//
//		rtvalue["uid"] = uid;
//		rtvalue["pwd"] = pwd;
//		rtvalue["name"] = name;
//		rtvalue["email"] = email;
//		rtvalue["nick"] = nick;
//		rtvalue["desc"] = desc;
//		rtvalue["sex"] = sex;
//		return;
//	}
//
//	//redis��û�����ѯmysql
//	//��ѯ���ݿ�
//	std::shared_ptr<UserInfo> user_info = nullptr;
//	user_info = MysqlMgr::GetInstance()->GetUser(name);
//	if (user_info == nullptr) {
//		rtvalue["error"] = ErrorCodes::UidInvalid;
//		return;
//	}
//
//	//�����ݿ�����д��redis����
//	Json::Value redis_root;
//	redis_root["uid"] = user_info->uid;
//	redis_root["pwd"] = user_info->pwd;
//	redis_root["name"] = user_info->name;
//	redis_root["email"] = user_info->email;
//	redis_root["nick"] = user_info->nick;
//	redis_root["desc"] = user_info->desc;
//	redis_root["sex"] = user_info->sex;
//
//	RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());
//
//	//��������
//	rtvalue["uid"] = user_info->uid;
//	rtvalue["pwd"] = user_info->pwd;
//	rtvalue["name"] = user_info->name;
//	rtvalue["email"] = user_info->email;
//	rtvalue["nick"] = user_info->nick;
//	rtvalue["desc"] = user_info->desc;
//	rtvalue["sex"] = user_info->sex;
//}
//
//bool LogicSystem::GetBaseInfo(std::string base_key, std::string uid, std::shared_ptr<UserInfo>& userinfo)
//{
//	
//	std::string info_str = "";
//	bool b_base = RedisMgr::GetInstance()->Get(base_key, info_str);
//	if (b_base) {
//		Json::Reader reader;
//		Json::Value root;
//		reader.parse(info_str, root);
//		userinfo->uid = root["uid"].asInt();
//		userinfo->name = root["name"].asString();
//		userinfo->pwd = root["pwd"].asString();
//		userinfo->email = root["email"].asString();
//		//userinfo->nick = root["nick"].asString();
//		//userinfo->desc = root["desc"].asString();
//		//userinfo->sex = root["sex"].asInt();
//		//userinfo->icon = root["icon"].asString();
//		std::cout << "user login uid is  " << userinfo->uid << " name  is "
//			<< userinfo->name << " pwd is " << userinfo->pwd << " email is " << userinfo->email << endl;
//	}
//	else {
//
//
//		std::shared_ptr<UserInfo> user_info = nullptr;
//		user_info = MysqlMgr::GetInstance()->GetUser(uid);
//		if (user_info == nullptr) {
//			return false;
//		}
//
//		userinfo = user_info;
//
//		//�����ݿ�����д��redis����
//		Json::Value redis_root;
//		redis_root["uid"] = uid;
//		redis_root["pwd"] = userinfo->pwd;
//		redis_root["name"] = userinfo->name;
//		redis_root["email"] = userinfo->email;
//		//redis_root["nick"] = userinfo->nick;
//		//redis_root["desc"] = userinfo->desc;
//		//redis_root["sex"] = userinfo->sex;
//		//redis_root["icon"] = userinfo->icon;
//		RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());
//	}
//
//	return true;
//}
//
//bool LogicSystem::GetFriendApplyInfo(int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list) {
//	//��mysql��ȡ���������б�
//	return MysqlMgr::GetInstance()->GetApplyList(to_uid, list, 0, 10);
//}
//
//bool LogicSystem::GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_list) {
//	//��mysql��ȡ�����б�
//	return MysqlMgr::GetInstance()->GetFriendList(self_id, user_list);
//}
