#include <iostream>
#include "src/http_server.hpp"
#include "struct_json/json_reader.h"
#include "struct_json/json_writer.h"
#include "cinatra/coro_http_server.hpp"

//struct Message {
//    int code;
//    std::string_view msg;
//};
//REFLECTION(Message, code, msg);
//
//int main() {
//    coral::log.Info("start server");
////    coral::TcpServer server(std::thread::hardware_concurrency(), 5134);
////    server.run();
//    coral::Router& r = coral::Router::instance();
//
//    using namespace coral;
//    r.GET("/", [](Request& req, Response& rsp){
//        Message m{.code = 200, .msg = "hello"};
//        std::string dump;
//        struct_json::to_json(m, dump);
//
//        rsp.json(dump);
//          rsp.setPath(".txt");
//          rsp.write("hello world");
//    });
//
//    HTTPServer server(2, 5134);
//    server.run();
////    using namespace cinatra;
////    cinatra::coro_http_server server(std::thread::hardware_concurrency(), 5134);
////    server.set_http_handler<GET>("/", [](coro_http_request& req, coro_http_response& rsp){
////        rsp.set_status_and_content(status_type::ok, "hello world");
////    });
////    server.sync_start();
//    return 0;
//}

#include "http_client.hpp"
#include "ylt/coro_http/coro_http_client.hpp"

async_simple::coro::Lazy<void>client_test() {
    coral::HTTPClient cli;
    coral::Response rsp = co_await cli.GET("http://www.baidu.com");
    coral::log.Info(std::to_string(rsp.getCode()));
    coral::log.Info(rsp.getHeader("Content-Type"));
}

async_simple::coro::Lazy<void> test_async_client() {
    std::string uri = "http://www.baidu.com";

    {
        coro_http::coro_http_client client{};
        auto data = co_await client.async_get(uri);
        std::cout << data.status << std::endl;

        data = co_await client.async_get(uri);
        std::cout << data.status << std::endl;

        data = co_await client.async_post(uri, "hello", coro_http::req_content_type::string);
        std::cout << data.status << std::endl;
    }
}

int main() {
    coral::HTTPClient client;
    async_simple::coro::syncAwait(client_test());
    return 0;
}