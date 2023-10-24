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

        size_t pos1, pos2, pos3;
        std::string schema_, host_, port_, path_, sub;
        pos1 = url.find_first_of("://");
        schema_ = url.substr(0, pos1);

        pos2 = url.find_last_of(":");
        if (pos2 == 4 or pos2 == 5) { //resolve the domain name
            tcp_client_->is_domain = true;
            pos2 = url.substr(pos1 + 3).find_first_of("/");
            if (pos2 == std::string::npos) {
                pos2 = url.size();
            }
            std::string domain = url.substr(pos1 + 3, pos2);

//            coro_io::ExecutorWrapper<> *executor = coro_io::get_global_executor();
//            asio::ip::tcp::resolver resolver(executor->get_asio_executor());
//            port_ = schema_.size() == 4 ? "80" : "443";
//            asio::ip::tcp::resolver::query query(domain, port_);
//            auto endpoints = resolver.resolve(query); //sync call, we don't use DNS very often
//            for(auto& endpoint : endpoints) {
//                host_ = endpoint.host_name();
//            }
//            path_ = pos2 == url.size() ? "/" : sub.substr(pos1 + pos2 + 3);
            port_ = schema_.size() == 4 ? "80" : "443";
            host_ = domain;
            path_ = pos2 == url.size() ? "/" : url.substr(pos1 + pos2 + 3);
        }
        else {
            host_ = url.substr(pos1 + 3, pos2 - pos1 - 3);
            sub = url.substr(pos2);
            pos3 = sub.find_first_of("/");
            if (pos3 == std::string::npos) {
                pos3 = sub.size();
            }
            port_ = sub.substr(1, pos3 - 1);
            path_ = pos3 == sub.size() ? "/" : sub.substr(pos3);
        }

        tcp_client_->setSchema(schema_);
        tcp_client_->setHost(host_);
        tcp_client_->setPort(port_);
        tcp_client_->setPath(path_);

        return {host_, port_};
    }

    //@RequestBuilder
    async_simple::coro::Lazy<coral::Response> GET(const std::string& url, RequestBuilder func = RequestBuilder()){
        auto ec = co_await tcp_client_->connect(url);
        if (ec) {
            coral::log.Error("fail to connect to server");
            coral::log.Error(ec.message());
        }

        coral::Request req;
        req.setMethod("GET");
        req.setPath(tcp_client_->getPath());
        req.setVersion("1.1");

        req.setHeader("Connection", "keep-alive");
        std::string host_ = tcp_client_->getHost();
        if (!tcp_client_->is_domain) {
            host_ += ":";
            host_ += tcp_client_->getPort();
        }
        req.setHeader("Host", host_);
        req.setHeader("Accept-Encoding", "gzip, deflate, br");
        req.setHeader("Accept", "*/*");

        if (func) {
            func(req);
        }

        auto [wec, send_len] = co_await tcp_client_->write(req.serialize());
        if (wec) {
            coral::log.Error("fail to send GET request");
            coral::log.Error(wec.message());
        }

        Buffer buffer(65536);
        auto [rec, recv_len] = co_await tcp_client_->read(buffer);
        if (rec) {
            coral::log.Error("fail to recv GET response");
            coral::log.Error(rec.message());
        }

        Response rsp(buffer, true);
        rsp.deserialize();
        co_return rsp;
    }

    // you must implement RequestBuilder function to send POST request


private:
    std::unique_ptr<TcpClient>tcp_client_;
};

} //namespace coral

#endif //CORAL_WITH_YLT_HTTP_CLIENT_HPP
