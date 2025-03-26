#include "ChatGrpcClient.h"


ChatGrpcClient::ChatGrpcClient() {

    auto& config = ConfigMgr::Inst();
    auto server_list = config["PeerServer"]["Servers"];

    std::stringstream ss(server_list);
    std::vector<std::string> words;

    std::string word;
    while(std::getline(ss, word, ',')) {
        words.push_back(word);
    }

    for (auto& word : words) {
        if (config[word]["Name"].empty()) {
            continue;
        }

        pools_[config[word]["Name"]] = std::make_unique<ChatConPool>(5, config[word]["Host"], config[word]["Port"]);
    }




}




AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendReq& req)
{
    AddFriendRsp rsp;
    Defer defer([&rsp, &req]() {
        rsp.set_error(ErrorCodes::Success);
        rsp.set_applyuid(req.applyuid());
        rsp.set_touid(req.touid());
        });

    auto find_iter = pools_.find(server_ip);
    if (find_iter == pools_.end()) {
        return rsp;
    }

    auto& pool = find_iter->second;
    ClientContext context;
    auto stub = pool->GetConnection();
    Status status = stub->NotifyAddFriend(&context, req, &rsp);
    Defer defercon([&stub, this, &pool]() {
        pool->returnConnection(std::move(stub));
        });

    if (!status.ok()) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    return rsp;

}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req)
{
    AuthFriendRsp rsp;
    rsp.set_error(ErrorCodes::Success);

    Defer defer([&rsp, &req]() {
        rsp.set_fromuid(req.fromuid());
        rsp.set_touid(req.touid());
        });

    auto find_iter = pools_.find(server_ip);
    if (find_iter == pools_.end()) {
        return rsp;
    }

    auto& pool = find_iter->second;
    ClientContext context;
    auto stub = pool->GetConnection();
    Status status = stub->NotifyAuthFriend(&context, req, &rsp);
    Defer defercon([&stub, this, &pool]() {
        pool->returnConnection(std::move(stub));
        });

    if (!status.ok()) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    return rsp;
}

bool ChatGrpcClient::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
    return false;
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip,
    const TextChatMsgReq& req, const Json::Value& rtvalue) {

    TextChatMsgRsp rsp;
    rsp.set_error(ErrorCodes::Success);

    Defer defer([&rsp, &req]() {
        rsp.set_fromuid(req.fromuid());
        rsp.set_touid(req.touid());
        for (const auto& text_data : req.textmsgs()) {
            TextChatData* new_msg = rsp.add_textmsgs();
            new_msg->set_msgid(text_data.msgid());
            new_msg->set_msgcontent(text_data.msgcontent());
        }

        });

    auto find_iter = pools_.find(server_ip);
    if (find_iter == pools_.end()) {
        return rsp;
    }

    auto& pool = find_iter->second;
    ClientContext context;
    auto stub = pool->GetConnection();
    Status status = stub->NotifyTextChatMsg(&context, req, &rsp);
    Defer defercon([&stub, this, &pool]() {
        pool->returnConnection(std::move(stub));
        });

    if (!status.ok()) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    return rsp;
}
