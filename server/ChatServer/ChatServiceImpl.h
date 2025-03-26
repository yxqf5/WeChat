#pragma once
#include <grpc/grpc.h>
#include "message.grpc.pb.h"
#include "message.pb.h"

#include <mutex>
#include "data.h"
#include "const.h"

using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::RplyFriendReq;
using message::RplyFriendRsp;

using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using message::TextChatData;

using message::SendChatMsgReq;
using message::SendChatMsgRsp;

using message::LoginRsp;
using message::LoginReq;

using message::ChatService;
using message::GetChatServerRsp;//???


//using grpc::Status;
//using grpc::Channel;
//using grpc::ClientContext;

using grpc::Status;
using grpc::Server;
using grpc::ServerContext;
using grpc::ServerBuilder;

class ChatServiceImpl final : public ChatService::Service
{
public:
	ChatServiceImpl();

    virtual ::grpc::Status NotifyAddFriend(::grpc::ServerContext* context, const ::message::AddFriendReq* request, ::message::AddFriendRsp* response);
    virtual ::grpc::Status RplyAddFriend(::grpc::ServerContext* context, const ::message::RplyFriendReq* request, ::message::RplyFriendRsp* response);
    virtual ::grpc::Status SendChatMsg(::grpc::ServerContext* context, const ::message::SendChatMsgReq* request, ::message::SendChatMsgRsp* response);
    virtual ::grpc::Status NotifyAuthFriend(::grpc::ServerContext* context, const ::message::AuthFriendReq* request, ::message::AuthFriendRsp* response);
    virtual ::grpc::Status NotifyTextChatMsg(::grpc::ServerContext* context, const ::message::TextChatMsgReq* request, ::message::TextChatMsgRsp* response);

    bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);



};

