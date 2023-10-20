//
// Created by Manjusaka on 2023/10/18.
//

#ifndef CORAL_WITH_YLT_COMMON_HPP
#define CORAL_WITH_YLT_COMMON_HPP

#include <string>
#include <vector>
#include <type_traits>
#include <functional>

namespace coral {

#define AUTO_GET_SET(x, y)                                     \
auto get##y() const {                                          \
     return x;                                                 \
}                                                              \
template<typename T>                                           \
void set##y(const T& t){                                       \
     x = t;                                                    \
}

class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

inline std::vector<std::string> split(const std::string& s, const std::string& c) {
    size_t pos1, pos2;
    std::vector<std::string>v;
    pos2 = s.find(c); //可以根据KMP手写
    pos1 = 0;
    while (std::string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2 - pos1));
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length())v.push_back(s.substr(pos1));
    return v;
}

#define HAS_MEMBER(member)                                                                                    \
template <typename T, typename... Args>                                                                       \
struct has_member_##member                                                                                    \
{                                                                                                             \
private:                                                                                                      \
    template <typename U>                                                                                     \
    static auto Check(int) -> decltype(std::declval<U>().member(std::declval<Args>()...), std::true_type());  \
    template <typename U>                                                                                     \
    static std::false_type Check(...);                                                                        \
public:                                                                                                       \
    enum {value = std::is_same<decltype(Check<T>(0)), std::true_type>::value};                                \
};                                                                                                            \


HAS_MEMBER(Before)
HAS_MEMBER(After)

template <typename Func, typename... Args>
struct Aspect
{
    Aspect(Func&& f) : m_func(std::forward<Func>(f))
    {

    }

    template <typename T>
    typename std::enable_if<has_member_Before<T, Args...>::value&& has_member_After<T, Args...>::value>::type
    Invoke(Args&&... args, T&& aspect)
    {
        bool next = aspect.Before(std::forward<Args>(args)...);
        if (!next) {
            return;
        }
        m_func(std::forward<Args>(args)...);
        aspect.After(std::forward<Args>(args)...);
    }

    template <typename T>
    typename std::enable_if<has_member_Before<T, Args...>::value && !has_member_After<T, Args...>::value>::type
    Invoke(Args&&... args, T&& aspect)
    {
        bool next = aspect.Before(std::forward<Args>(args)...);
        if (!next) {
            return;
        }
        m_func(std::forward<Args>(args)...);
    }

    template <typename T>
    typename std::enable_if<!has_member_Before<T, Args...>::value&& has_member_After<T, Args...>::value>::type
    Invoke(Args&&... args, T&& aspect)
    {
        m_func(std::forward<Args>(args)...);
        aspect.After(std::forward<Args>(args)...);
    }

    template <typename Head, typename... Tail>
    typename std::enable_if<has_member_Before<Head, Args...>::value&& has_member_After<Head, Args...>::value>::type
    Invoke(Args&&... args, Head&& headAspect, Tail&&... tailAspect)
    {
        bool next = headAspect.Before(std::forward<Args>(args)...);
        if (!next) {
            return;
        }
        Invoke(std::forward<Args>(args)..., std::forward<Tail>(tailAspect)...);
        headAspect.After(std::forward<Args>(args)...);
    }

    template <typename Head, typename... Tail>
    typename std::enable_if<has_member_Before<Head, Args...>::value && !has_member_After<Head, Args...>::value>::type
    Invoke(Args&&... args, Head&& headAspect, Tail&&... tailAspect)
    {
        bool next = headAspect.Before(std::forward<Args>(args)...);
        if (!next) {
            return;
        }
        Invoke(std::forward<Args>(args)..., std::forward<Tail>(tailAspect)...);
    }

    template <typename Head, typename... Tail>
    typename std::enable_if<!has_member_Before<Head, Args...>::value&& has_member_After<Head, Args...>::value>::type
    Invoke(Args&&... args, Head&& headAspect, Tail&&... tailAspect)
    {
        Invoke(std::forward<Args>(args)..., std::forward<Tail>(tailAspect)...);
        headAspect.After(std::forward<Args>(args)...);
    }

private:
    Func m_func;
};

//Íâ²¿½Ó¿Ú
template<typename... AP, typename... Args, typename Func>
void Invoke(Func&& f, Args&&... args)
{
    Aspect<Func, Args...> asp(std::forward<Func>(f));
    asp.Invoke(std::forward<Args>(args)..., AP()...);
}

} //namespace coral

#endif //CORAL_WITH_YLT_COMMON_HPP
