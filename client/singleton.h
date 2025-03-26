#ifndef SINGLETON_H
#define SINGLETON_H

#include"global.h"


// using namespace std;

template <typename T>
class Singleton{
protected:
    Singleton() =default;
    Singleton(const Singleton<T>& st) = delete;
    Singleton& operator = (const Singleton<T>& st) = delete;
    //自动回收单例
    static std::shared_ptr<T> _instance;

public:
    static std::shared_ptr<T> GetInstance(){
        static std::once_flag s_flag;
        std::call_once(s_flag,[&](){
            //为什么不使用make_shared()? 答案:make_shared不能调用受到保护的构造函数,但是new是在类的内部/派生类的内部调用的,可以访问protected的构造函数
            // _instance = std::make_shared(T);
            _instance = std::shared_ptr<T>(new T);
            return _instance;//bug,之前只在这里返回了实例,导致后面拿不到httpmgr的实例了,出现报错;//这个函数只会进来一次,所以需要给返回值放外面;
        });
         return _instance;
    }
    void PrintAddress(){
        std::cout<<_instance.get()<<std::endl;
    }

    ~Singleton(){
        std::cout<<"the destruct of Singleton "<<std::endl;
    }
};


template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;


#endif // SINGLETON_H
