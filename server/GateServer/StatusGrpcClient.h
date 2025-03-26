#pragma once

#include"const.h"
#include"Singleton.h"
#include"ConfigMgr.h"


class StatusConPool {
public:
    StatusConPool(size_t poolSize, std::string host, std::string port)
        : poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {
        for (size_t i = 0; i < poolSize_; ++i) {

            std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(host + ":" + port,
                grpc::InsecureChannelCredentials());

            connections_.push(message::StatusService::NewStub(channel));
        }
    }

    ~StatusConPool() {
        std::lock_guard<std::mutex> lock(mutex_);
        Close();
        while (!connections_.empty()) {
            connections_.pop();
        }
    }

    std::unique_ptr<message::StatusService::Stub> getConnection() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] {
            if (b_stop_) {
                return true;
            }
            return !connections_.empty();
            });
        //如果停止则直接返回空指针
        if (b_stop_) {
            return  nullptr;
        }
        auto context = std::move(connections_.front());
        connections_.pop();
        return context;
    }

    void returnConnection(std::unique_ptr<message::StatusService::Stub> context) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (b_stop_) {
            return;
        }
        connections_.push(std::move(context));
        cond_.notify_one();
    }

    void Close() {
        b_stop_ = true;
        cond_.notify_all();
    }

private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    std::string host_;
    std::string port_;
    std::queue<std::unique_ptr<message::StatusService::Stub>> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};


class StatusGrpcClient : public Singleton<StatusGrpcClient>
{
	friend class Singleton<StatusGrpcClient>;

public:
    ~StatusGrpcClient() {

    }

	message::GetChatServerRsp GetChatServer(int uid);
	

private:
	StatusGrpcClient();

	std::unique_ptr<StatusConPool> m_pool;


};

