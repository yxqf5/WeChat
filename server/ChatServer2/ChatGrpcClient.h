#pragma once


#include <grpc/grpc.h>
#include "message.grpc.pb.h"
#include "message.pb.h"

#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>
#include "const.h"
#include "Singleton.h"
#include "data.h"

using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::RplyFriendReq;
using message::RplyFriendRsp;

using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using message::TextChatData;

using message::SendChatMsgReq;
using message::SendChatMsgRsp;

using message::LoginRsp;
using message::LoginReq;

using message::ChatService;
using message::GetChatServerRsp;//???


using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;



class ChatConPool
{
public:
	ChatConPool(size_t poolsize, std::string host, std::string port) : poolSize_(poolsize), host_(host)
		, port_(port), b_stop_(false)
	{
		
		for (size_t i = 0; i < poolSize_; i++) {

			std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(host + ":" + port,
				grpc::InsecureChannelCredentials() );

			connection_.push(ChatService::NewStub(channel));
		}
	}

	~ChatConPool() {

		b_stop_ = true;
		Stop();
	}

	std::unique_ptr<ChatService::Stub> GetConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		
		cond_.wait(lock, [this]() {
			
			if (b_stop_) {
				return true;
			}
			return !connection_.empty();
			});

		if (b_stop_) {
			return nullptr;
		}

		auto conn = std::move(connection_.front());
		connection_.pop();
		return conn;
	}

	void returnConnection(std::unique_ptr<ChatService::Stub> conn){
		std::lock_guard<std::mutex> lock(mutex_);
		connection_.push(std::move(conn));
	}


	void Stop() {
		if (b_stop_) {
			while (!connection_.empty()) {
				connection_.front().reset();
				connection_.pop();
			}

			cond_.notify_all();
			return;
		}

	}


private:
	std::atomic<int> b_stop_;

	size_t poolSize_;
	std::string host_;
	std::string port_;

	std::queue<std::unique_ptr<ChatService::Stub> > connection_;
	std::mutex mutex_;
	std::condition_variable cond_;
	
};


class ChatGrpcClient :public Singleton<ChatGrpcClient>
{
	friend class Singleton<ChatGrpcClient>;
public:
	~ChatGrpcClient() {

	}

	AddFriendRsp NotifyAddFriend(std::string server_ip, const AddFriendReq& req);
	AuthFriendRsp NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req);
	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
	TextChatMsgRsp NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue);


private:

	ChatGrpcClient();
	std::unordered_map<std::string, std::unique_ptr<ChatConPool>> pools_;

};

