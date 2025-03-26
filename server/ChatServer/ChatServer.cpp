#include "LogicSystem.h"
#include <csignal>
#include <thread>
#include <mutex>
#include "AsioIOContextPool.h"
#include "CServer.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "const.h"
#include "ChatServiceImpl.h"
#include "ChatGrpcClient.h"

bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

int main()
{
    try {
        auto& config = ConfigMgr::Inst();
        auto server_name = config["SelfServer"]["Name"];

        auto pool = AsioIOContextPool::GetInstance();

        //设置本服务器的连接数量为0,并添加到redis中记录
        RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, "0");

        ChatGrpcClient::GetInstance();


        std::string server_address(config["SelfServer"]["Host"] + ":" + config["SelfServer"]["RPCPort"]);
        ChatServiceImpl service;
        grpc::ServerBuilder builder;

        //添加rpc监听的端口
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << "the rpc service is runing on: " << server_address << std::endl;

        //单独启动一个线程处理grpc服务
        std::thread grpc_server_thread([&server]() {
            server->Wait();
            });


        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context,pool](auto,auto) {
            io_context.stop();
            pool->Stop();
            
            });

        auto port_str = config["SelfServer"]["Port"];
        CServer s(io_context,atoi(port_str.c_str()));
        io_context.run();


        //处理grpc的结束服务
        RedisMgr::GetInstance()->HDel(LOGIN_COUNT, server_name);
        RedisMgr::GetInstance()->Close();
        grpc_server_thread.join();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

}