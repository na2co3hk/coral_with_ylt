//
// Created by Manjusaka on 2023/10/18.
//

#ifndef CORAL_WITH_YLT_HTTP_SERVER_HPP
#define CORAL_WITH_YLT_HTTP_SERVER_HPP

#include <memory>

#include "tcp_server.hpp"
#include "common.hpp"
#include "http_router.hpp"

namespace coral {

class HTTPServer : noncopyable {
public:
    HTTPServer(size_t thread_num, unsigned short port):
       tcp_server_(std::make_unique<TcpServer>(thread_num, port)),
       port_(port)
    {
        tcp_server_->set_msg_callback(std::bind(&HTTPServer::message_handler, this, std::placeholders::_1));
    }

    auto& get_port() {
        return port_;
    }

    std::errc run() {
        return tcp_server_->run();
    }

private:

    void message_handler(Buffer& buffer) {
        Request req(buffer);
        Response rsp(buffer, req.getErr(), req.isKeepAlive());

        Router& router = Router::instance();
        router.handleHTTPRequest(req, rsp);
        rsp.addStateLine();
        rsp.addHeader();
        rsp.addBody();
    }

    unsigned short port_;
    std::unique_ptr<TcpServer> tcp_server_;
};

} //namespace coral

#endif //CORAL_WITH_YLT_HTTP_SERVER_HPP
