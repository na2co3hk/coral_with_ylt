//
// Created by Manjusaka on 2023/10/18.
//

#ifndef CORAL_WITH_YLT_TCP_CONNECTION_HPP
#define CORAL_WITH_YLT_TCP_CONNECTION_HPP

#include <memory>
#include <asio.hpp>
#include <async_simple/Executor.h>
#include <async_simple/coro/Lazy.h>
#include <ylt/coro_io/coro_io.hpp>

#include "io_buffer.hpp"
#include "crlog.hpp"

namespace coral {

class TcpConnection : public std::enable_shared_from_this<coral::TcpConnection> {
public:
    template<typename Executor>
    TcpConnection(Executor* executor, asio::ip::tcp::socket socket):
       executor_(executor),
       socket_(std::move(socket)) {};

    async_simple::coro::Lazy<void>session() {
        for (;;) {
            Buffer buffer(1024);
            auto [rec, recv_len] = co_await coro_io::async_read_some(socket_, asio::buffer(buffer.beginWrite(), buffer.writableBytes()));
            buffer.moveWriter(recv_len);

//            if (rec) {
//                //TODO: error log
//                coral::log.Error(rec.message());
//                close();
//                break;
//            }

            if (msg_cb_) {
                msg_cb_(buffer);
            }

            auto [wec, send_len] = co_await coro_io::async_write(socket_, asio::buffer(buffer.peek(), buffer.readableBytes()));
            buffer.moveReader(send_len);
//            if (wec) {
//                //TODO: error log
//                coral::log.Error("write error");
//                close();
//                break;
//            }

            if(!keep_alive_) {
                close();
            }

            buffer.retrieveAll();
        }

    }

    void set_quit_callback(std::function<void(const uint64_t &conn_id)> callback,
                           uint64_t conn_id)
    {
        quit_cb_ = std::move(callback);
        conn_id_ = conn_id;
    }

    void set_msg_callback(std::function<void(Buffer& )>callback) {
        msg_cb_ = std::move(callback);
    }

    auto &get_executor() { return *executor_; }

private:

    void close() {
        if (has_closed_) {
            return;
        }

        asio::dispatch(socket_.get_executor(), [this, self = shared_from_this()] {
           std::error_code ec;
           socket_.shutdown(asio::socket_base::shutdown_both, ec); //shutdown both read and write
           socket_.close(ec);
            if (quit_cb_) {
                quit_cb_(conn_id_);
            }
            has_closed_ = true;
        });
    }

    std::atomic<bool> keep_alive_{false};
    std::atomic<bool> has_closed_{false};
    uint64_t conn_id_;
    async_simple::Executor* executor_;
    asio::ip::tcp::socket socket_;
    std::vector<asio::const_buffer> buffers_;

    std::function<void(Buffer& )>msg_cb_ = nullptr;
    std::function<void(const uint64_t &conn_id)>quit_cb_ = nullptr;
};

} //namespace coral

#endif //CORAL_WITH_YLT_TCP_CONNECTION_HPP
