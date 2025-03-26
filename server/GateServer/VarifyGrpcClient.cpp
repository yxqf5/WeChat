#include "VarifyGrpcClient.h"

RPCConPool::RPCConPool(size_t poolsize, std::string host, std::string port) : m_pool_size(poolsize), m_host(host), m_port(port)
{

	for (size_t i = 0; i < poolsize; i++)
	{
		std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());

		//auto stub = VarifyService::NewStub(channel);//这样就不行;因为这是一个左值,不能直接push进去
		//m_connections.push(stub);    //m_connections.push(std::move(stub));  
		m_connections.push(VarifyService::NewStub(channel));//这里是一个临时对象,是一个右值,所以可以直接push进去

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
	std::unique_lock<std::mutex> lock(m_mutex);//这里用unique_lock是因为要用到wait,wait需要unique_lock
	
	m_cond.wait(lock, [this] {//这里的lambda表达式是一个条件,如果返回true,那么就继续执行,如果返回false,那么就等待

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
	if (m_b_stop)//需要上锁,因为这个函数可能会被多个线程调用
	{
		return;//如果已经关闭了,那么就直接返回
	}
	m_connections.push(std::move(context));
	m_cond.notify_one();//唤醒一个等待的线程,如果没有等待的线程,那么就不会有任何操作
	return;
}

void RPCConPool::Close()
{
	m_b_stop = true;

	m_cond.notify_all();

}
