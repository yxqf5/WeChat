#include "VarifyGrpcClient.h"

RPCConPool::RPCConPool(size_t poolsize, std::string host, std::string port) : m_pool_size(poolsize), m_host(host), m_port(port)
{

	for (size_t i = 0; i < poolsize; i++)
	{
		std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());

		//auto stub = VarifyService::NewStub(channel);//�����Ͳ���;��Ϊ����һ����ֵ,����ֱ��push��ȥ
		//m_connections.push(stub);    //m_connections.push(std::move(stub));  
		m_connections.push(VarifyService::NewStub(channel));//������һ����ʱ����,��һ����ֵ,���Կ���ֱ��push��ȥ

	}



}

RPCConPool::~RPCConPool()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	Close();

	while (!m_connections.empty()) {
		m_connections.pop();
	}
}

std::unique_ptr<VarifyService::Stub> RPCConPool::GetConnection()
{
	std::unique_lock<std::mutex> lock(m_mutex);//������unique_lock����ΪҪ�õ�wait,wait��Ҫunique_lock
	
	m_cond.wait(lock, [this] {//�����lambda���ʽ��һ������,�������true,��ô�ͼ���ִ��,�������false,��ô�͵ȴ�

		if (m_b_stop) {
			return true;
		}

		return !m_connections.empty();
		});

	if (m_b_stop) {
		return nullptr;
	}

	auto conn = std::move(m_connections.front());
	m_connections.pop();
	return conn;
}

void RPCConPool::ReturnConnection(std::unique_ptr<VarifyService::Stub> context)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_b_stop)//��Ҫ����,��Ϊ����������ܻᱻ����̵߳���
	{
		return;//����Ѿ��ر���,��ô��ֱ�ӷ���
	}
	m_connections.push(std::move(context));
	m_cond.notify_one();//����һ���ȴ����߳�,���û�еȴ����߳�,��ô�Ͳ������κβ���
	return;
}

void RPCConPool::Close()
{
	m_b_stop = true;

	m_cond.notify_all();

}
