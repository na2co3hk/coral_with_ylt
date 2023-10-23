//
// Created by Manjusaka on 2023/10/23.
//

#ifndef CORAL_WITH_YLT_TCP_CLIENT_HPP
#define CORAL_WITH_YLT_TCP_CLIENT_HPP

#include <memory>
#include <asio.hpp>
#include <ylt/coro_io/coro_io.hpp>
#include <functional>

#include "io_buffer.hpp"

namespace coral {

class TcpClient {
public:
    using UrlParser = std::function<std::pair<std::string, std::string>(std::string_view url)>;

    TcpClient(asio::io_context::executor_type executor):
        socket_(std::make_shared<asio::ip::tcp::socket>(executor)),
        executor_(executor) {}

    TcpClient(coro_io::ExecutorWrapper<> *executor = coro_io::get_global_executor()):
        TcpClient(executor->get_asio_executor()) {}

    void set_url_parser(UrlParser parser) {
        parser_ = parser;
    }

    async_simple::coro::Lazy<std::error_code>connect(std::string_view url) {
        auto [host, port] = parser_(url);
        std::error_code ec = co_await coro_io::async_connect(&executor_, *socket_, host, port);
        co_return ec;
    }

    async_simple::coro::Lazy<std::error_code>connect(const std::string& host, const std::string port) {
        std::error_code ec = co_await coro_io::async_connect(&executor_, *socket_, host, port);
        co_return ec;
    }

    async_simple::coro::Lazy<std::pair<std::error_code, size_t>>read() {
        auto [ec, recv_len] = co_await coro_io::async_read(*socket_, asio::buffer(buffer_.beginWrite(), buffer_.writableBytes()));
        buffer_.moveWriter(recv_len);
        co_return std::make_pair(ec, recv_len);
    }

    async_simple::coro::Lazy<std::pair<std::error_code, size_t>>write(const char* data, size_t len) {
        co_return co_await coro_io::async_write(*socket_, asio::buffer(data, len));
    }

    async_simple::coro::Lazy<std::pair<std::error_code, size_t>>write(const std::string& msg) {
        co_return co_await coro_io::async_write(*socket_, asio::buffer(msg.data(), msg.size()));
    }

    coral::Buffer& get_buffer() {
        return buffer_;
    }

private:

    coro_io::ExecutorWrapper<> executor_;
    std::shared_ptr<asio::ip::tcp::socket> socket_;

    coral::Buffer buffer_{1024};
    UrlParser parser_;
};

} //namespace coral

#endif //CORAL_WITH_YLT_TCP_CLIENT_HPP
