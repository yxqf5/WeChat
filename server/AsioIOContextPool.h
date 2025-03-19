#pragma once

#include<vector>
#include<boost/asio.hpp>
#include"Singleton.h"



class AsioIOContextPool : public Singleton<AsioIOContextPool>
{
	friend class Singleton<AsioIOContextPool>;

public:
	using IOContext = boost::asio::io_context;
	using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;//新版boost弃用了io_context::work, 使用executor_work_guard代替
	using WorkPtr = std::unique_ptr<Work>;

	~AsioIOContextPool();
	AsioIOContextPool(AsioIOContextPool&) = delete;
	AsioIOContextPool& operator = (AsioIOContextPool&) = delete;


	IOContext& GetIOContext();

	void Stop();

private:
	AsioIOContextPool(std::size_t size = 2);//默认2个io_context

	std::vector<IOContext> m_io_contexts;
	std::vector<WorkPtr> m_works;
	std::vector<std::thread> m_threads;
	std::size_t m_next_io_context = 0;
};

