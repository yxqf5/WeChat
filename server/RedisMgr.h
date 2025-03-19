#pragma once

#include"const.h"


class RedisConPool;

class RedisMgr : public Singleton<RedisMgr>, public std::enable_shared_from_this<RedisMgr>
{
	friend class Singleton<RedisMgr>;
public:
	~RedisMgr();
	/* bool Connect(const std::string& host, int port);*/
	bool Get(const std::string& key, std::string& value);
	bool Set(const std::string& key, const std::string& value);
	//bool Auth(const std::string& password);
	bool LPush(const std::string& key, const std::string& value);
	bool LPop(const std::string& key, std::string& value);
	bool RPush(const std::string& key, const std::string& value);
	bool RPop(const std::string& key, std::string& value);
	bool HSet(const std::string& key, const std::string& hkey, const std::string& value);
	bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
	std::string HGet(const std::string& key, const std::string& hkey);
	bool Del(const std::string& key);
	bool ExistsKey(const std::string& key);
	void Close();
private:
	RedisMgr();


	std::unique_ptr<RedisConPool> m_pool;

};


class RedisConPool {
public:
	RedisConPool(size_t poolsize, const char* host, int port, const char* passwd) :m_poolsize(poolsize), m_host(host), m_port(port) {

		for (size_t i = 0; i < poolsize; i++) {

			auto* context = redisConnect(host, port);
			if (context == NULL || context->err != 0) {
				if (context != nullptr) {
					redisFree(context);
				}
				continue;
			}

			auto reply = (redisReply*)redisCommand(context, "AUTH %s", passwd);
			if (reply->type == REDIS_REPLY_ERROR) {
				std::cout << "认证失败" << std::endl;

				redisFree(context);
				continue;
			}
			freeReplyObject(reply);


			std::cout << "认证成功" << std::endl;
			m_connections.push(context);
		}
	}

	~RedisConPool() {
		Stop();
		while (!m_connections.empty())
		{
			auto temp = m_connections.front();
			m_connections.pop();
			redisFree(temp);
		}
	}


	redisContext* GetConnection() {
		std::unique_lock<std::mutex> lock(m_mutex);

		m_cond.wait(lock, [this]() {

			if (m_b_stop) {
				return true;
			}

			return !m_connections.empty();
			});

		if (m_b_stop) {
			return nullptr;
		}

		auto temp = m_connections.front();
		m_connections.pop();
		return temp;
	}

	void ReturnConnection(redisContext* redisConn) {
		std::lock_guard<std::mutex> lock(m_mutex);

		m_connections.push(redisConn);
		m_cond.notify_one();

	}






	void Stop() {
		m_b_stop = true;
		m_cond.notify_all();
	}






private:

	std::atomic<bool> m_b_stop;

	size_t m_poolsize;
	const char* m_host;
	int m_port;

	std::queue<redisContext*> m_connections;
	std::mutex m_mutex;
	std::condition_variable m_cond;

};