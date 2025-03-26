
#include "CServer.h"
#include"HttpConnection.h"
#include"AsioIOContextPool.h"

CServer::CServer(boost::asio::io_context & ioc, unsigned short& port) :m_ioc(ioc),
				 m_acceptor(ioc,tcp::endpoint(tcp::v4(),port))//v4()表示0.0.0.0
{

	std::cout << "CServer is runing on " << port << std::endl;


}

void CServer::Start() {
	auto self = shared_from_this();

	auto& io_context = AsioIOContextPool::GetInstance()->GetIOContext();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);


	m_acceptor.async_accept(new_con->GetSocket(), [self, new_con](beast::error_code ec) {
		try {
			//如果出错为真,就放弃掉这个链接,继续去监听其他连接;
			if (ec) {
				self->Start();
				return;
			}

			//创建一个新连接,并把连接交给httpconnection类管理
			//std::make_shared<HttpConnection>(std::move(self->m_socket))->Start();
			// 
			//HttpConnection(std::move(m_socket));

			new_con->Start();

			//继续监听
			self->Start();
		}
		catch (std::exception &exp) {
			std::cout << exp.what() << std::endl;
			self->Start();

		}

		});

}