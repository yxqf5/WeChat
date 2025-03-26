#ifndef HTTPMGR_H
#define HTTPMGR_H
#include"singleton.h"
#include<QString>
#include<QUrl>
#include<QObject>
#include<QNetworkAccessManager>
#include<QJsonObject>//生成JSON
#include<QJsonDocument>//解析JSON

//CRTP奇特递归模板模式
class HttpMgr : public QObject , public Singleton<HttpMgr>,
                public std::enable_shared_from_this<HttpMgr> //CRTP---奇特递归模板模式   //enable_shared_from_this用于实现shared_from_this的功能
{
    Q_OBJECT //使用信号和槽需要定义这样一个宏
public:

    ~HttpMgr();//这里是因为httpMgr是Singleton的模板实例类型,在_instance析构的时候,智能指针会调用T(HttpMgr)类型的析构函数,也就是这个函数,所以必须是公有函数;

    void PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod);

private:
    friend class Singleton<HttpMgr>;//这里,在基类构造的时候,需要调用T(HttpMgr)的构造函数,所以为了让基类可以调用子类的构造函数,需要给基类声明为子类的友元类;  笑死,父亲是儿子的朋友
    HttpMgr();
    QNetworkAccessManager _manager;



private slots:
    void slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);//槽函数的参数数量必须<=信号的参数数量

signals:
    void sig_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
    void sig_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
    void sig_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
    void sig_login_mod_finish(ReqId id, QString res, ErrorCodes err);
};

#endif // HTTPMGR_H
