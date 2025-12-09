// SPDX-FileCopyrightText: 2008 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <exception>
#include <string>
#include <memory>
#include <utility>
#include <type_traits>

/**
 * \def MDDS_ASCII(literal)
 *
 * Expands a \a literal string into two arguments: the first one is the
 * literal string itself, and the second one is the length of that string.
 *
 * Note that this macro only works with literal strings defined inline; it
 * does not work with pointer values that point to strings defined
 * elsewhere.
 */
#define MDDS_ASCII(literal) literal, sizeof(literal) - 1

/**
 * \def MDDS_N_ELEMENTS(name)
 *
 * Calculates the length of \a name array provided that the array definition
 * is given in the same compilation unit.
 *
 * @deprecated Please use \c std::size instead.
 */
#define MDDS_N_ELEMENTS(name) sizeof(name) / sizeof(name[0])

#ifdef __GNUC__
#define MDDS_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define MDDS_DEPRECATED __declspec(deprecated)
#else
#define MDDS_DEPRECATED
#endif

#ifndef MDDS_LOOP_UNROLLING
#define MDDS_LOOP_UNROLLING 1
#endif

#ifndef MDDS_USE_OPENMP
#define MDDS_USE_OPENMP 0
#endif

#if defined(__AVX__) || defined(__AVX2__)
#ifndef __SSE2__
#define __SSE2__ 1
#endif
#endif

namespace mdds {

class general_error : public ::std::exception
{
public:
    general_error(const ::std::string& msg) : m_msg(msg)
    {}
    virtual ~general_error() noexcept
    {}

    virtual const char* what() const noexcept
    {
        return m_msg.c_str();
    }

private:
    ::std::string m_msg;
};

class invalid_arg_error : public general_error
{
public:
    invalid_arg_error(const ::std::string& msg) : general_error(msg)
    {}
};

class size_error : public general_error
{
public:
    size_error(const std::string& msg) : general_error(msg)
    {}
};

class type_error : public general_error
{
public:
    type_error(const std::string& msg) : general_error(msg)
    {}
};

class integrity_error : public general_error
{
public:
    integrity_error(const std::string& msg) : general_error(msg)
    {}
};

namespace detail {

template<typename T>
class has_value_type
{
    using y_type = char;
    using n_type = long;

    template<typename U>
    static y_type test(typename U::value_type);
    template<typename U>
    static n_type test(...);

public:
    static constexpr bool value = sizeof(test<T>(0)) == sizeof(y_type);
};

template<typename T, typename IsConst>
struct const_or_not;

template<typename T>
struct const_or_not<T, std::true_type>
{
    using type = typename std::add_const<T>::type;
};

template<typename T>
struct const_or_not<T, std::false_type>
{
    using type = T;
};

template<typename T, bool Const>
using const_t = typename const_or_not<T, std::bool_constant<Const>>::type;

template<typename T, typename Mutable>
struct mutable_or_not;

template<typename T>
struct mutable_or_not<T, std::true_type>
{
    using type = T;
};

template<typename T>
struct mutable_or_not<T, std::false_type>
{
    using type = typename std::add_const<T>::type;
};

template<typename T, bool Mutable>
using mutable_t = typename mutable_or_not<T, std::bool_constant<Mutable>>::type;

template<typename T, typename IsConst>
struct get_iterator_type;

template<typename T>
struct get_iterator_type<T, std::true_type>
{
    using type = typename T::const_iterator;
};

template<typename T>
struct get_iterator_type<T, std::false_type>
{
    using type = typename T::iterator;
};

template<int T>
constexpr bool invalid_static_int()
{
    return false;
}

template<typename T, typename = void>
struct is_complete : std::false_type
{
};

template<typename T>
struct is_complete<T, std::void_t<decltype(sizeof(T) != 0)>> : std::true_type
{
};

} // namespace detail

} // namespace mdds
