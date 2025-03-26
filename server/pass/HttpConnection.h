#pragma once

#include"const.h"

class HttpConnection :public std::enable_shared_from_this<HttpConnection>
{

public:
	friend class LogicSystem;
	HttpConnection(boost::asio::io_context& io_context);

	void Start();

	tcp::socket& GetSocket() { return m_socket; }

private:
	//friend class LogicSystem;

	void CheckDeadline();
	void WriteResponse();
	void HandleReq();
	void PreParseGetParam();

	tcp::socket m_socket;

	beast::flat_buffer m_buffer{8192};
	http::request<http::dynamic_body> m_request;
	http::response<http::dynamic_body> m_response;

	net::steady_timer deadline_{
		m_socket.get_executor(),std::chrono::seconds(60)
	};

	std::string _get_url;
	std::unordered_map<std::string, std::string>_get_params;

};

