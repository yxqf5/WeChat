#include "httpmgr.h"



HttpMgr::HttpMgr() {

    connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slot_http_finish);

}


HttpMgr::~HttpMgr(){


}


void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod)
{
    QByteArray data = QJsonDocument(json).toJson();
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"applcation/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,QByteArray::number(data.length()));
    auto self = shared_from_this();//将HttpMgr对象传递给lamda表达式,以保证httpMgr对象的生命周期可以直到lamda表达式结后,智能指针自动回收;
    QNetworkReply * reply = _manager.post(request, data);//这一步是异步的,为了防止httpMgr对象被销毁后,异步会调用这个对象;所以上一步获取当前对象的智能指针，保证在异步操作过程中，当前对象不会被意外销毁。
    //reply是发送post的返回的 QNetworkReply 指针，用来表示异步的响应对象。,需要自己delete
    QObject::connect(reply, &QNetworkReply::finished, [self, reply, req_id, mod](){
         //处理错误情况
        if(reply->error() != QNetworkReply::NoError){
             qDebug()<<reply->errorString();
            //发送信号通知完成
             emit self->sig_http_finish(req_id,"", ErrorCodes::ERR_NETWORK, mod);
             reply->deleteLater();//这里是reply可能还会被用于底层的事件循环,所以就是告诉qt,让它在不使用reply后帮我回收reply;
            return;
         }

         // noError

         QString res = reply->readAll();
         //
         emit self->sig_http_finish(req_id, res, ErrorCodes::SUCCESS, mod);
         reply->deleteLater();
         return;
    });

}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    if(mod == Modules::REGISTERMOD){
        //发送信号通知指定模块,http的响应结束了
        emit sig_reg_mod_finish(id, res, err);

    }

    if(mod == Modules::RESETMOD){
        emit sig_reg_mod_finish(id, res, err);
    }

    if(mod == Modules::LOGINMOD){
        emit sig_login_mod_finish(id, res, err);
    }
}




