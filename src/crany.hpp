//
// Created by Manjusaka on 2023/10/20.
//

#ifndef CORAL_WITH_YLT_CRANY_HPP
#define CORAL_WITH_YLT_CRANY_HPP

#include<iostream>
#include<string>
#include<unordered_map>
#include<memory>
#include<functional>
#include<memory>
#include<typeindex>

struct Any {

    Any(void) : tpIndex_(std::type_index(typeid(void))) {}
    Any(Any& that) :
            ptr_(that.Clone()),
            tpIndex_(that.tpIndex_) {}
    Any(Any&& that) :
            ptr_(std::move(that.ptr_)),
            tpIndex_(that.tpIndex_) {}

    template<typename U, class = typename std::enable_if<
            !std::is_same_v<typename std::decay<U>::type, Any>, U>::type> Any(U&& value) :
            ptr_(new AnyDerived<typename std::decay<U>::type>(std::forward<U>(value))),
            tpIndex_(std::type_index(typeid(typename std::decay<U>::type))) {}

    bool isNull() const {
        return !bool(ptr_);
    }

    template<class U>
    bool is() const {
        return tpIndex_ == std::type_index(typeid(U));
    }

    //转换为实际类型
    template<class U>
    auto& cast() {
        if (!is<U>()) {
            throw std::invalid_argument("type error");
        }

        AnyDerived<U>* Derived = dynamic_cast<AnyDerived<U>*>(ptr_.get());
        return Derived->value_;
    }

    Any& operator=(const Any& a) {
        if (ptr_ == a.ptr_) {
            return *this;
        }
        ptr_ = a.Clone();
        tpIndex_ = a.tpIndex_;
        return *this;
    }

private:
    struct AnyBase;
    using AnyBasePtr = std::unique_ptr<AnyBase>;

    struct AnyBase {
        virtual ~AnyBase() {}
        virtual AnyBasePtr Clone() const = 0;
    };

    template<typename T>
    struct AnyDerived : AnyBase {

        template<typename U>
        AnyDerived(U&& value) :
                value_(value) {}

        AnyBasePtr Clone() const override {
            return AnyBasePtr(new AnyDerived<T>(value_));
        }

        T& ReturnValue() const {
            return value_;
        }

        T value_;
    };

    AnyBasePtr Clone() const {
        if (ptr_ != nullptr) {
            return ptr_->Clone();
        }
        return nullptr;
    }

    AnyBasePtr ptr_;
    std::type_index tpIndex_;

};

using var = Any;

#endif //CORAL_WITH_YLT_CRANY_HPP
