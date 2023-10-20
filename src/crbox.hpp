//
// Created by Manjusaka on 2023/10/20.
//

#ifndef CORAL_WITH_YLT_CRBOX_HPP
#define CORAL_WITH_YLT_CRBOX_HPP

#include <unordered_map>
#include "common.hpp"

#include "crany.hpp"

namespace coral {

//any box
    class Box : noncopyable {
    public:

        static Box& instance() {
            static Box instance;
            return instance;
        }

        template<typename T>
        void append(const std::string_view token, T&& object) {
            box_[token] = std::forward<T>(object);
        }

        void pop(const std::string_view token) {
            if (box_.find(token) != box_.end()){
                box_.erase(token);
            }
        }

        template<typename T>
        T& get(const std::string_view token) {
            if (box_.find(token) != box_.end()){
                return box_[token].cast<T>();
            }
        }

    private:
        std::unordered_map<std::string_view, var>box_;
    };

} //namespace coral

#endif //CORAL_WITH_YLT_CRBOX_HPP
