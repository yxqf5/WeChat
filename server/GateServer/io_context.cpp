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

	//�����work��һ��ռλ�������û�����ռλ����io_context����û�������ʱ���Զ��˳�
	//auto work = boost::asio::require(the_io_context.get_executor(), boost::asio::execution::outstanding_work.tracked);
	//boost::asio::any_io_executor work = asio::require(the_io_context.get_executor(), asio::execution::outstanding_work.tracked);
	//work = asio::any_io_executor();//���� run() �˳�, ���ǲ��������˳�, ��ȴ��������, Ȼ���˳�;


	std::cout << "Do you reckon this line displays?111111111" << std::endl;

	//asio::io_context::work work(the_io_context);//Ϊʲô����ᱨ��??

	for (int x = 0; x < 42; ++x)
	{
		the_io_context.poll();//�����poll()��ͬ���ģ���������ֱ�������������, ���ǲ����˳�, ��ȴ��������, Ȼ���˳�;
		std::cout << "Counter: " << x << std::endl;
	}

	std::cout << "Do you reckon this line displays?2222222222222" << std::endl;

	//std::shared_ptr<asio::io_context::

	the_io_context.run();
	std::cout << "Do you reckon this line displays?" << std::endl;

	return 0;
}