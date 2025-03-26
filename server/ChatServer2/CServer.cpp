
#include "CServer.h"


#include"AsioIOContextPool.h"
#include "UserMgr.h"


CServer::CServer(boost::asio::io_context& io_context, int port) :m_io_context(io_context), m_port(port) ,
				 m_accepter(io_context,boost::asio::ip::tcp::endpoint(tcp::v4(),port))
{
	std::cout << "Server start success, listen on port : " << m_port << std::endl;

	StartAccept();
}




CServer::~CServer() {

}

void CServer::ClearSession(std::string session_id) {

	if (m_sessions.find(session_id) != m_sessions.end()) {



		//清理掉session和id的绑定
		UserMgr::GetInstance()->RmvUserSession(m_sessions[session_id]->GetUserId());

	}


	{
		std::lock_guard<mutex>lock(m_mutex);
		m_sessions.erase(session_id);
	}
}


void CServer::HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error) {
	if (error) {

		std::cout << "session accept failed, error is " << error.what() << std::endl;
	}
	else {
		new_session->Start();
		std::lock_guard<mutex> lock(m_mutex);
		m_sessions.insert(make_pair(new_session->GetSessionId(), new_session));
	}
	
	StartAccept();
}

void CServer::StartAccept() {
	//auto self = shared_from_this();
	auto& io_context = AsioIOContextPool::GetInstance()->GetIOContext();
	std::shared_ptr<CSession> new_session = std::make_shared<CSession>(io_context, this);
	//m_accepter.async_accept(new_session->GetSocket(), [self,&new_session](const boost::system::error_code& ec){
	//	
	//	self->HandleAccept(new_session,ec);

	//	});

	m_accepter.async_accept(new_session->GetSocket(), std::bind(&CServer::HandleAccept, this, new_session, placeholders::_1));

}
