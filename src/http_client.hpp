//
// Created by Manjusaka on 2023/10/23.
//

#ifndef CORAL_WITH_YLT_HTTP_CLIENT_HPP
#define CORAL_WITH_YLT_HTTP_CLIENT_HPP

#include <memory>

#include "tcp_client.hpp"
#include "common.hpp"
#include "http_response.hpp"
#include "http_request.hpp"

namespace coral {

class HTTPClient : noncopyable {
public:
    using RequestBuilder = std::function<void(coral::Request&)>;
    using AsyncRequestBuilder = std::function<async_simple::coro::Lazy<void>(coral::Request&)>;

    struct url{
        std::string_view schema;
        std::string_view host;
        std::string_view port;
        std::string_view path;
    };

    HTTPClient(asio::io_context::executor_type executor):
       tcp_client_(std::make_unique<TcpClient>(executor))
    {
        tcp_client_->set_url_parser(std::bind(&HTTPClient::url_parser, this, std::placeholders::_1));
    }

    HTTPClient(coro_io::ExecutorWrapper<> *executor = coro_io::get_global_executor()):
       tcp_client_(std::make_unique<TcpClient>(executor))
   {
       tcp_client_->set_url_parser(std::bind(&HTTPClient::url_parser, this, std::placeholders::_1));
   }

    std::pair<std::string, std::string> url_parser(const std::string& url) {

        size_t pos1 = url.find_first_of("://");
        std::string schema_ = url.substr(0, pos1);

        size_t pos2 = url.find_last_of(":");
        std::string host_ = url.substr(pos1 + 3, pos2 - pos1 - 3);

        std::string sub = url.substr(pos2);
        size_t pos3 = sub.find_first_of("/");
        std::string port_ = sub.substr(1, pos3 - 1);
        std::string path_ = sub.substr(pos3);
        url_.schema = schema_;
        url_.host = host_;
        url_.port = port_;
        url_.path = path_;

        return {host_, port_};
    }

    //@RequestBuilder
    async_simple::coro::Lazy<coral::Response> GET(const std::string& url, RequestBuilder func = RequestBuilder()){
        auto ec = co_await tcp_client_->connect(url);
        if (ec) {
            coral::log.Error("fail to connect to server");
        }

        coral::Request req;
        req.setMethod("GET");
        req.setPath(url);
        req.setVersion("1.1");

        req.setHeader("Connection", "keep-alive");
        std::string& host_ = std::string(url_.host).append(":").append(url_.port);
        req.setHeader("Host", host_);

        if (func) {
            func(req);
        }

        auto [wec, send_len] = co_await tcp_client_->write(req.serialize());
        if (wec) {
            coral::log.Error("fail to send GET request");
        }

        auto [rec, recv_len] = co_await tcp_client_->read();
        Response rsp(tcp_client_->get_buffer(), true);
        rsp.deserialize();
        co_return rsp;
    }

    // you must implement RequestBuilder function to send POST request
    async_simple::coro::Lazy<coral::Response> POST(const std::string& url, RequestBuilder func){
        auto ec = co_await tcp_client_->connect(url);
        if (ec) {
            coral::log.Error("fail to connect to server");
        }

        coral::Request req;
        req.setMethod("POST");
        req.setPath(url);
        req.setVersion("1.1");

        req.setHeader("Connection", "keep-alive");
        std::string& host_ = std::string(url_.host).append(":").append(url_.port);
        req.setHeader("Host", host_);

        func(req);

        auto [wec, send_len] = co_await tcp_client_->write(req.serialize());
        if (wec) {
            coral::log.Error("fail to send POST request");
        }

        auto [rec, recv_len] = co_await tcp_client_->read();
        Response rsp(tcp_client_->get_buffer(), true);
        rsp.deserialize();
        co_return rsp;
    }

private:
    url url_;
    std::unique_ptr<TcpClient>tcp_client_;
};

} //namespace coral

#endif //CORAL_WITH_YLT_HTTP_CLIENT_HPP
