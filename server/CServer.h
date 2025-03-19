#pragma once
#include"const.h"

class HttpConnection;

class CServer : public std::enable_shared_from_this<CServer>
{
	std::map<std::string, std::map<std::string, std::string>> m;
public:
	CServer(boost::asio::io_context& ioc, unsigned short& port);

	void Start();
private:


	tcp::acceptor m_acceptor;
	net::io_context& m_ioc;


};