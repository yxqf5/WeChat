#include "AsioIOContextPool.h"
#include<iostream>



AsioIOContextPool::~AsioIOContextPool()
{
	Stop();
	std::cout << "AsioIOContextPool destruct! " << std::endl;
}

void AsioIOContextPool::Stop()
{
	for (auto& work : m_works)
	{
		//work->get_executor().on_work_finished();
		work->get_executor().context().stop();
		work.reset();
	}

	for (auto& t : m_threads)
	{
		t.join();
	}

}

boost::asio::io_context& AsioIOContextPool::GetIOContext()
{
	auto& service = m_io_contexts[m_next_io_context++];

	m_next_io_context = m_next_io_context % m_io_contexts.size();

	return service;


	// TODO: 在此处插入 return 语句
}





AsioIOContextPool::AsioIOContextPool(std::size_t size) :m_io_contexts(size),
m_works(size), m_next_io_context(0)
{

	for (int i = 0; i < size; i++) {
		m_works[i] = std::make_unique<Work>(
			boost::asio::make_work_guard(m_io_contexts[i])
		);
	}



	for (int i = 0; i < m_io_contexts.size(); i++) {

		m_threads.emplace_back(
			[this, i]() {
				m_io_contexts[i].run();
			}
		);

	}

}