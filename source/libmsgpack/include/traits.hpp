/*************************************************************************
    > File Name: has_member.hpp
    > Author: hsz
    > Brief:
    > Created Time: Thu 04 Jul 2024 04:00:14 PM CST
 ************************************************************************/

#ifndef __EULAR_UTILS_HAS_MEMBER_H__
#define __EULAR_UTILS_HAS_MEMBER_H__

#include <string>
#include <type_traits>
#include <utility>
#include <typeinfo>

#if defined(__linux__) || defined(__linux)
#define OS_LINUX 1
#include <cxxabi.h>
#endif

#define HAS_MEMBER(NameSpace, Func)                                                                         \
namespace NameSpace {                                                                                       \
template<typename T, typename R, typename... Args>                                                          \
struct has_member_##Func                                                                                    \
{                                                                                                           \
private:                                                                                                    \
    template<typename U>                                                                                    \
    static auto Check(int) -> decltype(std::declval<U>().Func(std::declval<Args>()...), std::true_type());  \
                                                                                                            \
    template<typename U>                                                                                    \
    static std::false_type Check(...);                                                                      \
                                                                                                            \
    template<typename U>                                                                                    \
    static auto GetReturnType(int) -> decltype(std::declval<U>().Func(std::declval<Args>()...)) {}          \
                                                                                                            \
    template<typename U>                                                                                    \
    static void GetReturnType(...) {}                                                                       \
                                                                                                            \
public:                                                                                                     \
    static constexpr auto value = decltype(Check<T>(0))::value &&                                           \
                                  std::is_same<R, decltype(GetReturnType<T>(0))>::value;                    \
};                                                                                                          \
}

#define HAS_MEMBER_CONST(NameSpace, Func)                                                                   \
namespace NameSpace {                                                                                       \
template<typename T, typename R, typename... Args>                                                          \
struct has_member_##Func                                                                                    \
{                                                                                                           \
private:                                                                                                    \
    template<typename U>                                                                                    \
    static auto Check(int) -> decltype(std::declval<const U>().Func(std::declval<Args>()...), std::true_type());  \
                                                                                                            \
    template<typename U>                                                                                    \
    static std::false_type Check(...);                                                                      \
                                                                                                            \
    template<typename U>                                                                                    \
    static auto GetReturnType(int) -> decltype(std::declval<U>().Func(std::declval<Args>()...)) {}          \
                                                                                                            \
    template<typename U>                                                                                    \
    static void GetReturnType(...) {}                                                                       \
                                                                                                            \
public:                                                                                                     \
    static constexpr auto value = decltype(Check<T>(0))::value &&                                           \
                                  std::is_same<R, decltype(GetReturnType<T>(0))>::value;                    \
};                                                                                                          \
}

namespace traits {
static inline std::string _demangle(const char* name)
{
#ifdef OS_LINUX
    int32_t status = 0;
    char *buf = abi::__cxa_demangle(name, NULL, NULL, &status);
    if (status == 0 && buf) {
        std::string s(buf);
        free(buf);
        return s;
    }
#endif

    return std::string(name);
}

template <typename T> struct ClassNameHelper { static std::string Name; };
template <typename T> std::string ClassNameHelper<T>::Name = traits::_demangle(typeid(T).name());

template <typename T>
using RemoveConstRef = typename std::remove_const<typename std::remove_reference<T>::type>::type;

} // namespace traits

#endif // __EULAR_UTILS_HAS_MEMBER_H__
