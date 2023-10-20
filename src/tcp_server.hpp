//
// Created by Manjusaka on 2023/10/18.
//

#ifndef CORAL_WITH_YLT_TCP_SERVER_HPP
#define CORAL_WITH_YLT_TCP_SERVER_HPP

#include <asio.hpp>
#include <ylt/coro_io/io_context_pool.hpp>
#include <ylt/coro_io/coro_io.hpp>
#include <async_simple/Future.h>
#include <async_simple/Try.h>
#include <mutex>
#include <unordered_map>

#include "tcp_connection.hpp"

namespace coral {

class TcpServer {
public:
    TcpServer(size_t thread_num,
              unsigned short port):
            pool_(thread_num),
            port_(port),
            acceptor_(pool_.get_executor()->get_asio_executor()) {}

    ~TcpServer() {}

    std::errc run() {
        auto ret = async_run();
        ret.wait();
        return ret.value();
    }

    async_simple::Future<std::errc> async_run() {
        auto ec = listen();
        async_simple::Promise<std::errc> promise;
        auto future = promise.getFuture();

        //no error
        if (ec == std::errc{}) {
            thd_ = std::thread([this]{
               pool_.run();
            });
        }

        this->accept().start([p = std::move(promise)](async_simple::Try<std::errc>&& res) mutable {
            if (res.hasError()) {
                p.setValue(std::errc::io_error);
            }
            else {
                p.setValue(res.value());
            }
        });

        return std::move(future);
    }

    void set_msg_callback(std::function<void(Buffer& )>callback) {
        msg_cb_ = std::move(callback);
    }

private:

    std::errc listen() {
        using asio::ip::tcp;
        auto endpoint = tcp::endpoint(tcp::v4(), port_);
        acceptor_.open(endpoint.protocol());
        asio::error_code ec;
        acceptor_.bind(endpoint, ec);
        if (ec) {
            //TODO: error log
            coral::log.Error("bind error");
            return std::errc::address_in_use;
        }
        acceptor_.set_option(tcp::acceptor::reuse_address(true));
        acceptor_.listen();
        auto end_point = acceptor_.local_endpoint(ec);
        if (ec) {
            //TODO: error log
            coral::log.Error("listen error");
            return std::errc::address_in_use;
        }
        port_ = end_point.port();

        return {};
    }

    async_simple::coro::Lazy<std::errc> accept() {
        for(;;) {
            auto executor = pool_.get_executor();
            asio::ip::tcp::socket sock(executor->get_asio_executor());
            auto ec = co_await coro_io::async_accept(acceptor_, sock);
            if (ec) {
                //TODO: error log
                coral::log.Error("accept error");
                continue;
            }

            //handle message
            uint64_t conn_id = ++conn_id_;
            auto conn = std::make_shared<TcpConnection>(executor, std::move(sock));

            conn->set_msg_callback(msg_cb_);

            conn->set_quit_callback(
                [this](const uint64_t &id) {
                    std::scoped_lock lock(conn_mtx_);
                    if (!connections_.empty())
                        connections_.erase(id);
                },
                conn_id);

            {
                std::scoped_lock lock(conn_mtx_);
                connections_.emplace(conn_id, conn);
            }

            start_session(conn).via(&conn->get_executor()).detach(); //alloc the executor to the session
        }
    }

    async_simple::coro::Lazy<void> start_session(std::shared_ptr<TcpConnection> conn) noexcept {
        co_await conn->session();
    }

    uint64_t conn_id_ = 0;
    std::thread thd_;
    coro_io::io_context_pool pool_;
    uint16_t port_;
    asio::ip::tcp::acceptor acceptor_;
    std::mutex conn_mtx_;
    std::unordered_map<uint64_t, std::shared_ptr<TcpConnection>>connections_;

    std::function<void(Buffer& )>msg_cb_ = nullptr;
};

} //namespace coral

#endif //CORAL_WITH_YLT_TCP_SERVER_HPP
