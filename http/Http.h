//
// Created by wyatt on 2022/4/2.
//

#ifndef MEMORYTOOLS_HTTP_H
#define MEMORYTOOLS_HTTP_H

#include "hv/HttpServer.h"
#include "../MemoryTools/MemorySearchTools.h"
#include "../MemoryTools/FileManager.h"
#include <boost/lexical_cast.hpp>

class HttpServer {

private:
    static std::string ToHex(long long x) {
        stringstream ss;
        ss << hex << x;
        std::string ans;
        ss >> ans;
        return ans;
    }

    template<class T>
    static T ToDec(const std::string& str)
    {
        stringstream ss;
        ss << hex <<  str;
        T a;
        ss >> a;
        return a;
    }

    vector<MemPage> pages;
    shared_ptr<MemoryTools> memoryTools;
    int cnt = 0;

public:

    int a = 98765;


    template<class T>
    static void show(const Address & address,const T& newValue)
    {
        cout << address << " " << newValue << endl;
    }

    HttpServer() {
        HttpService router;
        router.GET("/ping", [](HttpRequest *req, HttpResponse *resp) {
            return resp->String("pong");
        });

        router.GET("/data", [](HttpRequest *req, HttpResponse *resp) {
            static char data[] = "0123456789";
            return resp->Data(data, 10);
        });

        router.GET("/paths", [&router](HttpRequest *req, HttpResponse *resp) {
            return resp->Json(router.Paths());
        });

        router.GET("/getAllPid", [](HttpRequest *req, HttpResponse *resp) {
            auto allPid = MemoryTools::getAllPID();
            for (auto &v: allPid) {
                resp->json[v.first] = hv::Json(v.second);
            }
            return 200;
        });


        router.GET("/getPages", [this](HttpRequest *req, HttpResponse *resp) {
            int pid = boost::lexical_cast<int>(req->GetParam("pid", "-1"));
            MemoryTools::getMemPage(pid, pages);
            if (pages.size() == 0)return 400;
            vector<std::map<string, string>> res;
            for (auto &i: pages) {
                std::map<string, string> page;
                page["start"] = ToHex(i.start);
                page["end"] = ToHex(i.end);
                page["flags"] = i.flags;
                page["name"] = i.name;
                res.push_back(page);
                cout << i << endl;
            }
            resp->json = hv::Json(res);
            return 200;
        });


        router.GET("/init", [this](HttpRequest *req, HttpResponse *resp) {
            int pid = boost::lexical_cast<int>(req->GetParam("pid", "-1"));
            memoryTools.reset(new MemoryTools(pid));
            cnt = 0;
            return 200;
        });


        router.GET("/first", [this](HttpRequest *req, HttpResponse *resp) {
            pages.clear();

            MemoryTools::getMemPage(getpid(), pages);
            memoryTools.reset(new MemoryTools(getpid()));

            std::string type = (req->GetParam("type", "int"));
#define XX(T)           if (type == #T) {\
                size_t size = memoryTools->FirstSearch<T>(pages);\
                resp->json["size"] = hv::Json(size);\
            }
            XX(char)
            XX(short)
            XX(int)
            XX(long)
            XX(long long)
            XX(float)
            XX(double)
#undef XX
            cnt = 1;
            return 200;
        });

        router.GET("/second", [this](HttpRequest *req, HttpResponse *resp) {


            std::string type = (req->GetParam("type", "int"));
            std::string value = (req->GetParam("value", "null"));
            std::string func = (req->GetParam("func", "isEqual"));

            size_t size = 0;

#define XX(T, F)           if(type == #T)\
            {\
                if (value == "null") {  \
                    if(func == #F){     \
                    size = memoryTools->SecondSearch<T>(&MemoryTools::F<T>);\
                    }                    \
                }else{\
                    T v = boost::lexical_cast<T>(value);                    \
                    if(func == #F){     \
                    size = memoryTools->SecondSearch<T>(&MemoryTools::F<T>,&v);\
                    }                 \
            }}


#define XXX(F)  XX(char,F)\
            XX(short,F)\
            XX(int,F)\
            XX(long,F)\
            XX(long long,F)\
            XX(float,F)\
            XX(double,F)

            XXX(isNotEqual)
            XXX(isEqual)
            XXX(isBigger)
            XXX(isSmaller)

#undef XXX

#undef XX

            resp->json["size"] = hv::Json(size);
            cnt = 2;
            cout << &a << endl;
            return 200;

        });


        router.GET("/third", [this](HttpRequest *req, HttpResponse *resp) {


            int count = boost::lexical_cast<int>(req->GetParam("count", to_string(cnt)));
            std::string type = (req->GetParam("type", "int"));
            std::string value = (req->GetParam("value", "null"));
            std::string func = (req->GetParam("func", "isEqual"));
            size_t size = 0;

#define XX(T, F)           if(type == #T)\
            {\
                if (value == "null") {  \
                    if(func == #F){     \
                    size = memoryTools->ThirdSearch<T>(&MemoryTools::F<T>,count);\
                    }                    \
                }else{\
                    T v = boost::lexical_cast<T>(value);                    \
                    if(func == #F){     \
                    size = memoryTools->ThirdSearch<T>(&MemoryTools::F<T>,count,&v);\
                    }                 \
            }}


#define XXX(F)  XX(char,F)\
            XX(short,F)\
            XX(int,F)\
            XX(long,F)\
            XX(long long,F)\
            XX(float,F)\
            XX(double,F)

            XXX(isNotEqual)
            XXX(isEqual)
            XXX(isBigger)
            XXX(isSmaller)

#undef XXX

#undef XX

            cout << size << endl;
//            resp->json.push_back({{"size", size}});
            resp->json["size"] = hv::Json(size);
//            resp->json = hv::Json();
            cnt++;
            return 200;
        });



        router.GET("/dump", [this](HttpRequest *req, HttpResponse *resp) {


            int count = boost::lexical_cast<int>(req->GetParam("count", to_string(cnt)));
            int skip = boost::lexical_cast<int>(req->GetParam("skip", "1000")); // 默认1000条


            a++;


            std::vector<map<std::string,std::string> > vec;

            memoryTools->dump<int>(count, [this, &vec, &skip](const Address & address,const int& oldValue){
                if(vec.size() > skip)return;
                map<std::string,std::string> mp;
                mp["address"] = ToHex(address);
                mp["oldValue"] = to_string(oldValue);
                vec.emplace_back(mp);
                }, false);
            int c = 0;
            memoryTools->dump<int>(count, [this, &vec, &c, &skip](const Address & address,const int& newValue){
                if(c > skip)return;
                vec[c++]["newValue"] = to_string(newValue);
            }, true);
            resp->json = hv::Json(vec);
            return 200;
        });


        router.GET("/read", [this](HttpRequest *req, HttpResponse *resp) {
            cout << &a << endl;
            int pid = boost::lexical_cast<int>(req->GetParam("pid", to_string(getpid())));
            auto address = ToDec<Address>(req->GetParam("address", ToHex((long)&a)));
            std::string type = req->GetParam("type", "int");
            auto len = boost::lexical_cast<size_t>(req->GetParam("size", "1"));

#define XX(T)            if(type == #T)\
            {\
                size_t size = len * sizeof(T);\
                vector<T> buff(len);\
                MemoryTools::preadv(pid,address,buff.data(),size);\
                resp->json = hv::Json(buff);\
            }

            XX(char)
            XX(short)
            XX(int)
            XX(long)
            XX(long long)
            XX(float)
            XX(double)
#undef XX
            return 200;
        });

        router.GET("/write", [this](HttpRequest *req, HttpResponse *resp) {
            cout << &a << endl;
            int pid = boost::lexical_cast<int>(req->GetParam("pid", to_string(getpid())));
            auto address = ToDec<Address>(req->GetParam("address", ToHex((long)&a)));
            std::string type = req->GetParam("type", "int");
//            auto len = boost::lexical_cast<size_t>(req->GetParam("size", "1"));
            std::string value = req->GetParam("value", "100");
#define XX(T)            if(type == #T)\
            {                          \
                T v; \
                if(type=="char"){      \
                v =  boost::lexical_cast<int>(value);                      \
                }else{                 \
                v = boost::lexical_cast<T>(value);                       \
                }\
                MemoryTools::pwritev(  \
                pid,address,&v,1);\
                resp->json = hv::Json(v);\
            }

            XX(char)
            XX(short)
            XX(int)
            XX(long)
            XX(long long)
            XX(float)
            XX(double)
#undef XX

            cout << a << endl;
            return 200;
        });

        router.POST("/echo", [](const HttpContextPtr &ctx) {
            return ctx->send(ctx->body(), ctx->type());
        });

        http_server_t server;
        server.port = 8080;
        server.service = &router;
        http_server_run(&server);

    }
};


#endif //MEMORYTOOLS_HTTP_H
