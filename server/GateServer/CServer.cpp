
#include "CServer.h"
#include"HttpConnection.h"
#include"AsioIOContextPool.h"

CServer::CServer(boost::asio::io_context & ioc, unsigned short& port) :m_ioc(ioc),
				 m_acceptor(ioc,tcp::endpoint(tcp::v4(),port))//v4()��ʾ0.0.0.0
{

	std::cout << "CServer is runing on " << port << std::endl;


}

void CServer::Start() {
	auto self = shared_from_this();

	auto& io_context = AsioIOContextPool::GetInstance()->GetIOContext();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);


	m_acceptor.async_accept(new_con->GetSocket(), [self, new_con](beast::error_code ec) {
		try {
			//�������Ϊ��,�ͷ������������,����ȥ������������;
			if (ec) {
				self->Start();
				return;
			}

			//����һ��������,�������ӽ���httpconnection�����
			//std::make_shared<HttpConnection>(std::move(self->m_socket))->Start();
			// 
			//HttpConnection(std::move(m_socket));

			new_con->Start();

			//��������
			self->Start();
		}
		catch (std::exception &exp) {
			std::cout << exp.what() << std::endl;
			self->Start();

		}

		});

}