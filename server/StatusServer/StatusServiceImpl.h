#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include<map>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;

using message::LoginReq;
using message::LoginRsp;


class  ChatServer {
public:
	ChatServer() :host(""), port(""), name(""), con_count(0) {}
	ChatServer(const ChatServer& cs) :host(cs.host), port(cs.port), name(cs.name), con_count(cs.con_count) {}
	ChatServer& operator=(const ChatServer& cs) {
		if (&cs == this) {
			return *this;
		}

		host = cs.host;
		name = cs.name;
		port = cs.port;
		con_count = cs.con_count;
		return *this;
	}
	std::string host;
	std::string port;
	std::string name;
	int con_count;
};

class StatusServiceImpl final : public StatusService::Service
{
public:
    StatusServiceImpl();
    Status GetChatServer(ServerContext* context, const GetChatServerReq* request,
        GetChatServerRsp* reply) override;

    Status Login(ServerContext* context, const LoginReq* request, LoginRsp* reply) override;


    //负载均衡,用来返回负载最小的一个服务器
    ChatServer getChatServer();

    std::unordered_map<std::string, ChatServer> m_servers;
    std::mutex m_server_mutex;


    //将uid和token插入缓存中(后期干的事情,前期先放map内存中)
    void insertToken(int uid, std::string token);

    //std::mutex m_token_mutex;
    //std::map<std::string, std::string>m_tokens;

    std::vector<ChatServer> _servers;
    int _server_index;
};

