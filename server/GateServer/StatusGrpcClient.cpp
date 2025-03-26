#include "StatusGrpcClient.h"



message::GetChatServerRsp StatusGrpcClient::GetChatServer(int uid)
{

	grpc::ClientContext context;
	message::GetChatServerReq request;
	message::GetChatServerRsp reply;

	request.set_uid(uid);

	auto stub = m_pool->getConnection();
	grpc::Status status = stub->GetChatServer(&context, request, &reply);

	Defer defer([&stub, this]() {
		m_pool->returnConnection(std::move(stub));
		});

	if (status.ok()) {
		return reply;
	}
	else {
		reply.set_error(ErrorCodes::RPCFailed);
		return reply;
	}
}


//StatusGrpcClient的构造函数,读取配置并初始化grpc连接池

StatusGrpcClient::StatusGrpcClient() {

	auto configMgr = ConfigMgr::Inst();
	std::string host = configMgr["StatusServer"]["Host"];
	std::string port = configMgr["StatusServer"]["Port"];
	m_pool.reset(new StatusConPool(5, host, port));
}