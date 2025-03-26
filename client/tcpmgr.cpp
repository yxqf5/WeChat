#include "tcpmgr.h"
#include <QAbstractSocket>
#include<QJsonDocument>
#include<usermgr.h>

TcpMgr::TcpMgr():_host(""),_port(0),_b_recv_pending(false),_message_id(0),_message_len(0)
{


    QObject::connect(&_socket , &QTcpSocket::connected, [&](){
        qDebug()<<"socket connect success";

        emit sig_con_success(true);

    });

    connect(&_socket, &QTcpSocket::readyRead, [&](){

        _buffer.append(_socket.readAll());
        QDataStream stream(&_buffer, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_6_7);

        forever
        {
            if(!_b_recv_pending){

                if(_buffer.size() < static_cast<int>(sizeof(quint16)*2)){
                    return;
                }

                stream >> _message_id >> _message_len;
                _buffer = _buffer.mid(sizeof(quint16)*2);

            }

            if(_buffer.size() < _message_len){
                _b_recv_pending = true;
                return;
            }

            QByteArray messageBody = _buffer.mid(0, _message_len);
            _buffer = _buffer.mid(_message_len);
            qDebug() << "receive body msg is " << messageBody ;

            handleMsg(ReqId(_message_id), _message_len, messageBody);

        }
    });


    //5.15 之后版本
    QObject::connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError){
        Q_UNUSED(socketError);
        qDebug()<<"socket Error"<<_socket.errorString();

    });

    QObject::connect(&_socket, &QTcpSocket::disconnected, [&](){
        qDebug()<<"socket disconnect";
    });


    connect(this, &TcpMgr::sig_send_data, this, &TcpMgr::slot_send_data);


    initHandlers();



}

void TcpMgr::initHandlers()
{

    //登录请求回包解析
    m_handlers.insert(ReqId::ID_CHAT_LOGIN_RSP, [this](ReqId id, int len, QByteArray data){
     Q_UNUSED(len);
     qDebug()<< "handle id is "<< id ;
     // 将QByteArray转换为QJsonDocument
     QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
     // 检查转换是否成功
     if(jsonDoc.isNull()){
         qDebug() << "Failed to create QJsonDocument.";
         return;
     }
     QJsonObject jsonObj = jsonDoc.object();
     qDebug()<< "data jsonobj is " << jsonObj ;
     if(!jsonObj.contains("error")){
         int err = ErrorCodes::ERR_JSON;
         qDebug() << "Login Failed, err is Json Parse Err" << err ;
         emit sig_login_failed(err);
         return;
     }
     int err = jsonObj["error"].toInt();
     if(err != ErrorCodes::SUCCESS){
         qDebug() << "Login Failed, err is " << err ;
         emit sig_login_failed(err);
         return;
     }

     auto uid = jsonObj["uid"].toInt();
     auto name = jsonObj["name"].toString();
     auto nick = jsonObj["nick"].toString();
     auto icon = jsonObj["icon"].toString();
     auto sex = jsonObj["sex"].toInt();
     auto user_info = std::make_shared<UserInfo>(uid, name, nick, icon, sex,"");



     UserMgr::GetInstance()->SetUserInfo(user_info);
     UserMgr::GetInstance()->SetToken(jsonObj["token"].toString());
     // if(jsonObj.contains("apply_list")){
     //     UserMgr::GetInstance()->AppendApplyList(jsonObj["apply_list"].toArray());
     // }

     UserMgr::GetInstance()->SetName(jsonObj["name"].toString());
     UserMgr::GetInstance()->SetUid(jsonObj["uid"].toInt());
     UserMgr::GetInstance()->SetToken(jsonObj["token"].toString());




     //添加好友列表
     // if (jsonObj.contains("friend_list")) {
     //     UserMgr::GetInstance()->AppendFriendList(jsonObj["friend_list"].toArray());
     // }
        //发送什么int ?


     emit sig_switch_chatdlg();
    });

    m_handlers.insert(ID_ADD_FRIEND_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Add Friend Failed, err is Json Parse Err" << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Add Friend Failed, err is " << err;
            return;
        }

        qDebug() << "Add Friend Success " ;
    });


    //收到来自chatserver的通知请求req
    m_handlers.insert(ID_NOTIFY_ADD_FRIEND_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "ID_NOTIFY_ADD_FRIEND_REQ Failed, err is Json Parse Err" << err;

            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "ID_NOTIFY_ADD_FRIEND_REQ Failed, err is " << err;
            return;
        }

        int from_uid = jsonObj["applyuid"].toInt();
        QString name = jsonObj["name"].toString();
        QString desc = jsonObj["desc"].toString();
        QString icon = jsonObj["icon"].toString();
        QString nick = jsonObj["nick"].toString();
        int sex = jsonObj["sex"].toInt();

        auto apply_info = std::make_shared<AddFriendApply>(
            from_uid, name, desc,
            icon, nick, sex);

        emit sig_friend_apply(apply_info);
    });



    //发送接收好友申请
    m_handlers.insert(ID_NOTIFY_AUTH_FRIEND_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Auth Friend Failed, err is " << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Auth Friend Failed, err is " << err;
            return;
        }

        int from_uid = jsonObj["fromuid"].toInt();
        QString name = jsonObj["name"].toString();
        QString nick = jsonObj["nick"].toString();
        QString icon = jsonObj["icon"].toString();
        int sex = jsonObj["sex"].toInt();

        auto auth_info = std::make_shared<AuthInfo>(from_uid,name,
                                                    nick, icon, sex);

        emit sig_add_auth_friend(auth_info);
    });

    m_handlers.insert(ID_ADD_FRIEND_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Add Friend Failed, err is Json Parse Err" << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Add Friend Failed, err is " << err;
            return;
        }

        qDebug() << "Add Friend Success " ;
    });

    m_handlers.insert(ID_AUTH_FRIEND_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Auth Friend Failed, err is Json Parse Err" << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Auth Friend Failed, err is " << err;
            return;
        }

        auto name = jsonObj["name"].toString();
        auto nick = jsonObj["nick"].toString();
        auto icon = jsonObj["icon"].toString();
        auto sex = jsonObj["sex"].toInt();
        auto uid = jsonObj["uid"].toInt();
        auto rsp = std::make_shared<AuthRsp>(uid, name, nick, icon, sex);
        emit sig_auth_rsp(rsp);

        qDebug() << "Auth Friend Success " ;
    });

    m_handlers.insert(ID_NOTIFY_AUTH_FRIEND_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Auth Friend Failed, err is " << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Auth Friend Failed, err is " << err;
            return;
        }

        int from_uid = jsonObj["fromuid"].toInt();
        QString name = jsonObj["name"].toString();
        QString nick = jsonObj["nick"].toString();
        QString icon = jsonObj["icon"].toString();
        int sex = jsonObj["sex"].toInt();

        auto auth_info = std::make_shared<AuthInfo>(from_uid,name,
                                                    nick, icon, sex);

        emit sig_add_auth_friend(auth_info);
    });

    m_handlers.insert(ID_TEXT_CHAT_MSG_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Chat Msg Rsp Failed, err is Json Parse Err" << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Chat Msg Rsp Failed, err is " << err;
            return;
        }

        qDebug() << "Receive Text Chat Rsp Success " ;
        //ui设置送达等标记 todo...
    });


    m_handlers.insert(ID_NOTIFY_TEXT_CHAT_MSG_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Notify Chat Msg Failed, err is Json Parse Err" << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Notify Chat Msg Failed, err is " << err;
            return;
        }

        qDebug() << "Receive Text Chat Notify Success " ;
        auto msg_ptr = std::make_shared<TextChatMsg>(jsonObj["fromuid"].toInt(),
                                                     jsonObj["touid"].toInt(),jsonObj["text_array"].toArray());
        emit sig_text_chat_msg(msg_ptr);
    });

}

void TcpMgr::handleMsg(ReqId id, int len, QByteArray data)
{
    auto find_iter = m_handlers.find(id);
    if(find_iter == m_handlers.end()){
        std::cout<<" not found id ["<<id<<"] handler"<<std::endl;
    }

    find_iter.value()(id, len, data);
}

void TcpMgr::slot_tcp_connect(ServerInfo si)
{
    qDebug()<< "receive tcp connect signal";
    // 尝试连接到服务器
    qDebug() << "Connecting to server...";
    qDebug() <<"Host: "<< si.Host <<" Port: "<<si.Port <<" Token: "<< si.Token <<" Uid: "<<si.Uid;
    _host = si.Host;
    _port = static_cast<uint16_t>(si.Port.toUInt());
    _socket.connectToHost(_host, _port);
}



void TcpMgr::slot_send_data(ReqId reqId, QString data)
{
    uint16_t id = reqId;

    qDebug()<<"socket send login data is :" <<data;
    // 将字符串转换为UTF-8编码的字节数组
    QByteArray dataBytes = data.toUtf8();

    // 计算长度（使用网络字节序转换）
    quint16 len = static_cast<quint16>(data.size());

    // 创建一个QByteArray用于存储要发送的所有数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    // 设置数据流使用网络字节序,大端模式,高字节低地址,低字节高地址
    out.setByteOrder(QDataStream::BigEndian);

    // 写入ID和长度
    out << id << len;

    // 添加字符串数据
    block.append((char*)dataBytes.data());

    // 发送数据
    _socket.write(block);


    // quint16 id1 = id;

    // quint16 len1 = static_cast<quint16>(data.size());

    // QByteArray databody = data.toUtf8();

    // QByteArray block1;
    // QDataStream stream(&block1, QIODevice::WriteOnly);

    // stream.setByteOrder(QDataStream::BigEndian);

    // stream << id1 <<len1;

    // block1.append((char*)databody.data());

    // _socket.write(block1);


}


// void TcpMgr::slot_send_data(ReqId reqId, QByteArray dataBytes)
// {
//     uint16_t id = reqId;

//     // 计算长度（使用网络字节序转换）
//     quint16 len = static_cast<quint16>(dataBytes.length());

//     // 创建一个QByteArray用于存储要发送的所有数据
//     QByteArray block;
//     QDataStream out(&block, QIODevice::WriteOnly);

//     // 设置数据流使用网络字节序
//     out.setByteOrder(QDataStream::BigEndian);

//     // 写入ID和长度
//     out << id << len;

//     // 添加字符串数据
//     block.append(dataBytes);

//     // 发送数据
//     _socket.write(block);
//     qDebug() << "tcp mgr send byte data is " << block ;
// }
