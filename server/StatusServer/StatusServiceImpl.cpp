#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "const.h"
#include "RedisMgr.h"

std::string generate_unique_string() {
    // 创建UUID对象
    boost::uuids::uuid uuid = boost::uuids::random_generator()();

    // 将UUID转换为字符串
    std::string unique_string = to_string(uuid);

    return unique_string;
}

Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
{
    std::string prefix("llfc status server has received :  ");
    const auto& server = getChatServer();
    reply->set_host(server.host);
    reply->set_port(server.port);
    std::cout << "the reply" << server.host << ":" << server.port;

    reply->set_error(ErrorCodes::Success);
    reply->set_token(generate_unique_string());
    insertToken(request->uid(), reply->token());
    return Status::OK;
}

Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply)
{
    //auto uid = request->uid();
    //auto token = request->token();
    //std::lock_guard<std::mutex> guard(m_token_mutex);
    //auto iter = m_tokens.find(uid);
    //if (iter == m_tokens.end()) {
    //    reply->set_error(ErrorCodes::UidInvalid);
    //    return Status::OK;
    //}
    //if (iter->second != token) {
    //    reply->set_error(ErrorCodes::TokenInvalid);
    //    return Status::OK;
    //}
    //reply->set_error(ErrorCodes::Success);
    //reply->set_uid(uid);
    //reply->set_token(token);
    return Status::OK;
}

ChatServer StatusServiceImpl::getChatServer()
{
    std::lock_guard<std::mutex> guard(m_server_mutex);
    auto minServer = m_servers.begin()->second;
    auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, minServer.name);

    if (count_str.empty()) {
        minServer.con_count = INT_MAX;
    }
    else
    {
        minServer.con_count = std::stoi(count_str);
    }


    for ( auto& server : m_servers) {

        if (server.second.name == minServer.name) {
            continue;
       }

       auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server.second.name);

       if (count_str.empty()) {
           server.second.con_count = INT_MAX;
       }
       else {
           server.second.con_count = std::stoi(count_str);
       }

       if (server.second.con_count < minServer.con_count) {
           minServer = server.second;
       }
    }

    return minServer;
}




StatusServiceImpl::StatusServiceImpl() :_server_index(0)
{
    auto& config = ConfigMgr::Inst();
    auto server_list = config["chatServers"]["Name"];

    std::vector<std::string> words;
    std::stringstream ss(server_list);
    std::string word;

    while(std::getline(ss,word, ',')) {
        words.push_back(word);
    }

    for (auto& word : words) {
        if (config[word]["Name"].empty()) {
            continue;
        }

        ChatServer server;
        server.port = config[word]["Port"];
        server.host = config[word]["Host"];
        server.name = config[word]["Name"];
        m_servers[server.name] = server;

    }

}


void StatusServiceImpl::insertToken(int uid, std::string token)
{

    //m_tokens[uid] = token;
    std::string token_key = USERTOKENPREFIX + std::to_string(uid);
    RedisMgr::GetInstance()->Set(token_key, token);
}