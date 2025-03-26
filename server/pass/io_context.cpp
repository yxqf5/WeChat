#include<boost/asio.hpp>
#include<iostream>


void my_task() {
	std::cout << "my task finish! " << std::endl;
}


namespace asio = boost::asio;
//using Work = boost::asio::io_context::work;


int main2() {

	boost::asio::io_context the_io_context;

	boost::asio::post(the_io_context, my_task);

	boost::asio::post(the_io_context, []() {
		std::cout << "the lambda finish! " << std::endl;
		});

	//这里的work是一个占位符，如果没有这个占位符，io_context会在没有任务的时候自动退出
	//auto work = boost::asio::require(the_io_context.get_executor(), boost::asio::execution::outstanding_work.tracked);
	//boost::asio::any_io_executor work = asio::require(the_io_context.get_executor(), asio::execution::outstanding_work.tracked);
	//work = asio::any_io_executor();//允许 run() 退出, 但是不会立即退出, 会等待任务完成, 然后退出;


	std::cout << "Do you reckon this line displays?111111111" << std::endl;

	//asio::io_context::work work(the_io_context);//为什么这里会报错??

	for (int x = 0; x < 42; ++x)
	{
		the_io_context.poll();//这里的poll()是同步的，会阻塞，直到所有任务完成, 但是不会退出, 会等待任务完成, 然后退出;
		std::cout << "Counter: " << x << std::endl;
	}

	std::cout << "Do you reckon this line displays?2222222222222" << std::endl;

	//std::shared_ptr<asio::io_context::

	the_io_context.run();
	std::cout << "Do you reckon this line displays?" << std::endl;

	return 0;
}