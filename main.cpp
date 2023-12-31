#include <iostream>
#include "src/http_server.hpp"
#include "struct_json/json_reader.h"
#include "struct_json/json_writer.h"
#include "cinatra/coro_http_server.hpp"

struct Message {
    int code;
    std::string_view msg;
};
REFLECTION(Message, code, msg);

struct LogAspect {
    bool Before(coral::Request& req, coral::Response& rsp) {
        coral::log.Info("new connection!");
    }
};

int main() {
    coral::log.Info("start server");
//    coral::TcpServer server(std::thread::hardware_concurrency(), 5134);
//    server.run();
    coral::Router& r = coral::Router::instance();

    using namespace coral;
    r.GET("/", [](Request& req, Response& rsp){
        Message m{.code = 200, .msg = "hello"};
        std::string dump;
        struct_json::to_json(m, dump);

        rsp.json(dump);
    }, LogAspect{});

    HTTPServer server(2, 5132);
    server.run();
//    using namespace cinatra;
//    cinatra::coro_http_server server(std::thread::hardware_concurrency(), 5134);
//    server.set_http_handler<GET>("/", [](coro_http_request& req, coro_http_response& rsp){
//        rsp.set_status_and_content(status_type::ok, "hello world");
//    });
//    server.sync_start();
    return 0;
}

//#include "http_client.hpp"
//#include "ylt/coro_http/coro_http_client.hpp"
//
//async_simple::coro::Lazy<void>client_test() {
//    coral::HTTPClient cli;
//    coral::Response rsp = co_await cli.GET("http://www.baidu.com");
//}
//
//int main() {
//    coral::HTTPClient client;
//    async_simple::coro::syncAwait(client_test());
//    return 0;
//}