#pragma once
#include"const.h"


class HttpConnection;

typedef std::function<void(std::shared_ptr<HttpConnection>)>HttpHandler;

class LogicSystem : public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;

public:
	~LogicSystem() {}; 
	bool HandleGet(std::string, std::shared_ptr<HttpConnection>);
	bool HandlePost(std::string, std::shared_ptr<HttpConnection>);

	void ReqGet(std::string, HttpHandler hanler);
	void ReqPost(std::string url, HttpHandler handler);


private:
	LogicSystem();
	std::map<std::string, HttpHandler> m_post_handlers;
	std::map<std::string, HttpHandler> m_get_handlers;
};