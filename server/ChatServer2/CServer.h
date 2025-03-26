#pragma once
#include"const.h"
#include"CSession.h"
#include<memory.h>
#include<map>
#include<mutex>


class CServer : public std::enable_shared_from_this<CServer>
{
public:

	CServer(boost::asio::io_context&, int);
	~CServer();
	void ClearSession(std::string);


private:
	void HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error);
	void StartAccept();
	boost::asio::io_context& m_io_context;
	short m_port;

	boost::asio::ip::tcp::acceptor m_accepter;
	std::map<std::string, shared_ptr<CSession>> m_sessions;
	std::mutex m_mutex;

};