#pragma once

#include<grpcpp/grpcpp.h>

#include"const.h"
#include"Singleton.h"
#include"message.grpc.pb.h"
#include"ConfigMgr.h"


using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

class RPCConPool {

public:
	RPCConPool(size_t poolsize, std::string host, std::string port);
	~RPCConPool();

	std::unique_ptr < VarifyService::Stub> GetConnection();
	void ReturnConnection(std::unique_ptr<VarifyService::Stub> context);


	void Close();

private:
	std::atomic<bool> m_b_stop;
	size_t m_pool_size;
	std::string m_host;
	std::string m_port;
	std::queue<std::unique_ptr<VarifyService::Stub>> m_connections;
	std::mutex m_mutex;
	std::condition_variable m_cond;
};

class VarifyGrpcClient : public Singleton<VarifyGrpcClient>
{
	friend class Singleton<VarifyGrpcClient>;

public:

	GetVarifyRsp GetVarifyCode(std::string email) {

		ClientContext context;
		GetVarifyRsp reply;
		GetVarifyReq request;

		request.set_email(email);
		auto stub = m_pool->GetConnection();
		Status status = stub->GetVarifyCode(&context, request, &reply);

		if (status.ok()) {
			m_pool->ReturnConnection(std::move(stub));
			return reply;
		}
		else {
			m_pool->ReturnConnection(std::move(stub));
			reply.set_error(ErrorCodes::RPCFailed);
			
			return reply;
		}

	}

private:


	VarifyGrpcClient() {
		/*std::shared_ptr<Channel> channel = grpc::CreateChannel("127.0.0.1:50051",grpc::InsecureChannelCredentials());
		stub_ = VarifyService::NewStub(channel);*/

		auto& config = ConfigMgr::Inst();
		std::string host = config["VarifyServer"]["Host"];
		std::string port = config["VarifyServer"]["Port"];
		m_pool.reset(new RPCConPool(5, host, port));
	}

	

	std::unique_ptr<RPCConPool> m_pool;
};






