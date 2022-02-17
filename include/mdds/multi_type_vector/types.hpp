/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2021 Kohei Yoshida
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************/

#ifndef INCLUDED_MDDS_MULTI_TYPE_VECTOR_TYPES_2_HPP
#define INCLUDED_MDDS_MULTI_TYPE_VECTOR_TYPES_2_HPP

#include "../global.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <cstdint>

#ifdef MDDS_MULTI_TYPE_VECTOR_USE_DEQUE
#include <deque>
#else
#include <vector>
#endif

#if defined(MDDS_UNIT_TEST) || defined(MDDS_MULTI_TYPE_VECTOR_DEBUG)
#include <iostream>
#include <sstream>
using std::cerr;
using std::cout;
using std::endl;
#endif

namespace mdds { namespace mtv {

using element_t = int;

constexpr element_t element_type_empty = -1;

constexpr element_t element_type_boolean = 0;
constexpr element_t element_type_int8 = 1;
constexpr element_t element_type_uint8 = 2;
constexpr element_t element_type_int16 = 3;
constexpr element_t element_type_uint16 = 4;
constexpr element_t element_type_int32 = 5;
constexpr element_t element_type_uint32 = 6;
constexpr element_t element_type_int64 = 7;
constexpr element_t element_type_uint64 = 8;
constexpr element_t element_type_float = 9;
constexpr element_t element_type_double = 10;
constexpr element_t element_type_string = 11;

constexpr element_t element_type_user_start = 50;

/**
 * Loop-unrolling factor with optional SIMD feature.
 *
 * In each enumerator value, the first byte contains the loop-unrolling factor
 * (either 0, 4, 8, 16 or 32), while the second byte stores SIMD flags.
 */
enum class lu_factor_t : int
{
    none = 0,
    lu4 = 4,
    lu8 = 8,
    lu16 = 16,
    lu32 = 32,
    sse2_x64 = 1 << 8,
    sse2_x64_lu4 = 1 << 8 | 4,
    sse2_x64_lu8 = 1 << 8 | 8,
    sse2_x64_lu16 = 1 << 8 | 16,
    avx2_x64 = 2 << 8,
    avx2_x64_lu4 = 2 << 8 | 4,
    avx2_x64_lu8 = 2 << 8 | 8,
};

/**
 * Type of traced method.
 *
 * An <code>accessor</code> in this context is a method whose call alone does
 * not mutate the state of the container.  All const methods are accessors.
 * Note that some non-const methods that return non-const references to
 * internal data are still considered accessors.
 *
 * A <code>mutator</code> is a method that, when called, may change the state
 * of the stored data immediately.
 *
 * The <code>accessor_with_pos_hint</code> label signifies an accessor that
 * takes a position hint as its first argument. Likewise,
 * <code>mutator_with_pos_hint</code> signifies a mutator that takes a
 * position hint as its first argument.
 *
 * The <code>constructor</code> and <code>destructor</code> labels are
 * hopefully self-explanatory.
 */
enum class trace_method_t : int
{
    unspecified = 0,
    accessor = 1,
    accessor_with_pos_hint = 1 << 8 | 1,
    mutator = 2,
    mutator_with_pos_hint = 1 << 8 | 2,
    constructor = 3,
    destructor = 4
};

/**
 * Struct containing the information about each traced method.
 */
struct trace_method_properties_t
{
    trace_method_t type = trace_method_t::unspecified;

    /**
     * Memory address of the container instance the traced method belongs to.
     * This is essentially the <code>this</code> pointer inside the traced
     * method.
     */
    const void* instance = nullptr;

    /** Name of the method. */
    const char* function_name = nullptr;

    /**
     * String containing the argument names as well as their values if
     * available.
     */
    std::string function_args;

    /** Path of the file where the method body is. */
    const char* filepath = nullptr;

    /** Line number of the first line of the traced method body. */
    int line_number = -1;
};

/**
 * Generic exception used for errors specific to element block operations.
 */
class element_block_error : public mdds::general_error
{
public:
    element_block_error(const std::string& msg) : mdds::general_error(msg)
    {}
};

class base_element_block;
element_t get_block_type(const base_element_block&);

/**
 * Non-template common base type necessary for blocks of all types to be
 * stored in a single container.
 */
class base_element_block
{
    friend element_t get_block_type(const base_element_block&);

protected:
    element_t type;
    base_element_block(element_t _t) : type(_t)
    {}
};

template<typename _Self, element_t _TypeId, typename _Data>
class element_block : public base_element_block
{
#ifdef MDDS_UNIT_TEST
    struct print_block_array
    {
        void operator()(const _Data& val) const
        {
            std::cout << val << " ";
        }
    };
#endif

protected:
#ifdef MDDS_MULTI_TYPE_VECTOR_USE_DEQUE
    typedef std::deque<_Data> store_type;
#else
    typedef std::vector<_Data> store_type;
#endif
    store_type m_array;

    element_block() : base_element_block(_TypeId)
    {}
    element_block(size_t n) : base_element_block(_TypeId), m_array(n)
    {}
    element_block(size_t n, const _Data& val) : base_element_block(_TypeId), m_array(n, val)
    {}

    template<typename _Iter>
    element_block(const _Iter& it_begin, const _Iter& it_end) : base_element_block(_TypeId), m_array(it_begin, it_end)
    {}

public:
    static const element_t block_type = _TypeId;

    typedef typename store_type::iterator iterator;
    typedef typename store_type::reverse_iterator reverse_iterator;
    typedef typename store_type::const_iterator const_iterator;
    typedef typename store_type::const_reverse_iterator const_reverse_iterator;
    typedef _Data value_type;

    bool operator==(const _Self& r) const
    {
        return m_array == r.m_array;
    }

    bool operator!=(const _Self& r) const
    {
        return !operator==(r);
    }

    static const value_type& at(const base_element_block& block, typename store_type::size_type pos)
    {
        return get(block).m_array.at(pos);
    }

    static value_type& at(base_element_block& block, typename store_type::size_type pos)
    {
        return get(block).m_array.at(pos);
    }

    static value_type* data(base_element_block& block)
    {
        return get(block).m_array.data();
    }

    static typename store_type::size_type size(const base_element_block& block)
    {
        return get(block).m_array.size();
    }

    static iterator begin(base_element_block& block)
    {
        return get(block).m_array.begin();
    }

    static iterator end(base_element_block& block)
    {
        return get(block).m_array.end();
    }

    static const_iterator begin(const base_element_block& block)
    {
        return get(block).m_array.begin();
    }

    static const_iterator end(const base_element_block& block)
    {
        return get(block).m_array.end();
    }

    static const_iterator cbegin(const base_element_block& block)
    {
        return get(block).m_array.begin();
    }

    static const_iterator cend(const base_element_block& block)
    {
        return get(block).m_array.end();
    }

    static reverse_iterator rbegin(base_element_block& block)
    {
        return get(block).m_array.rbegin();
    }

    static reverse_iterator rend(base_element_block& block)
    {
        return get(block).m_array.rend();
    }

    static const_reverse_iterator rbegin(const base_element_block& block)
    {
        return get(block).m_array.rbegin();
    }

    static const_reverse_iterator rend(const base_element_block& block)
    {
        return get(block).m_array.rend();
    }

    static const_reverse_iterator crbegin(const base_element_block& block)
    {
        return get(block).m_array.rbegin();
    }

    static const_reverse_iterator crend(const base_element_block& block)
    {
        return get(block).m_array.rend();
    }

    static _Self& get(base_element_block& block)
    {
#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
        if (get_block_type(block) != _TypeId)
        {
            std::ostringstream os;
            os << "incorrect block type: expected block type=" << _TypeId
               << ", passed block type=" << get_block_type(block);
            throw general_error(os.str());
        }
#endif
        return static_cast<_Self&>(block);
    }

    static const _Self& get(const base_element_block& block)
    {
#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
        if (get_block_type(block) != _TypeId)
        {
            std::ostringstream os;
            os << "incorrect block type: expected block type=" << _TypeId
               << ", passed block type=" << get_block_type(block);
            throw general_error(os.str());
        }
#endif
        return static_cast<const _Self&>(block);
    }

    static void set_value(base_element_block& blk, size_t pos, const _Data& val)
    {
        get(blk).m_array[pos] = val;
    }

    static void get_value(const base_element_block& blk, size_t pos, _Data& val)
    {
        val = get(blk).m_array[pos];
    }

    static value_type get_value(const base_element_block& blk, size_t pos)
    {
        return get(blk).m_array[pos];
    }

    static void append_value(base_element_block& blk, const _Data& val)
    {
        get(blk).m_array.push_back(val);
    }

    static void prepend_value(base_element_block& blk, const _Data& val)
    {
        store_type& blk2 = get(blk).m_array;
        blk2.insert(blk2.begin(), val);
    }

    static _Self* create_block(size_t init_size)
    {
        return new _Self(init_size);
    }

    static void delete_block(const base_element_block* p)
    {
        delete static_cast<const _Self*>(p);
    }

    static void resize_block(base_element_block& blk, size_t new_size)
    {
        store_type& st = get(blk).m_array;
        st.resize(new_size);

        // Test if the vector's capacity is larger than twice its current
        // size, and if so, shrink its capacity to free up some memory.
        if (new_size < (st.capacity() / 2))
            st.shrink_to_fit();
    }

#ifdef MDDS_UNIT_TEST
    static void print_block(const base_element_block& blk)
    {
        const store_type& blk2 = get(blk).m_array;
        std::for_each(blk2.begin(), blk2.end(), print_block_array());
        std::cout << std::endl;
    }
#else
    static void print_block(const base_element_block&)
    {}
#endif

    static void erase_block(base_element_block& blk, size_t pos)
    {
        store_type& blk2 = get(blk).m_array;
        blk2.erase(blk2.begin() + pos);
    }

    static void erase_block(base_element_block& blk, size_t pos, size_t size)
    {
        store_type& blk2 = get(blk).m_array;
        blk2.erase(blk2.begin() + pos, blk2.begin() + pos + size);
    }

    static void append_values_from_block(base_element_block& dest, const base_element_block& src)
    {
        store_type& d = get(dest).m_array;
        const store_type& s = get(src).m_array;
        d.insert(d.end(), s.begin(), s.end());
    }

    static void append_values_from_block(
        base_element_block& dest, const base_element_block& src, size_t begin_pos, size_t len)
    {
        store_type& d = get(dest).m_array;
        const store_type& s = get(src).m_array;
        std::pair<const_iterator, const_iterator> its = get_iterator_pair(s, begin_pos, len);
#ifndef MDDS_MULTI_TYPE_VECTOR_USE_DEQUE
        d.reserve(d.size() + len);
#endif
        d.insert(d.end(), its.first, its.second);
    }

    static void assign_values_from_block(
        base_element_block& dest, const base_element_block& src, size_t begin_pos, size_t len)
    {
        store_type& d = get(dest).m_array;
        const store_type& s = get(src).m_array;
        std::pair<const_iterator, const_iterator> its = get_iterator_pair(s, begin_pos, len);
        d.assign(its.first, its.second);
    }

    static void prepend_values_from_block(
        base_element_block& dest, const base_element_block& src, size_t begin_pos, size_t len)
    {
        store_type& d = get(dest).m_array;
        const store_type& s = get(src).m_array;
        std::pair<const_iterator, const_iterator> its = get_iterator_pair(s, begin_pos, len);
#ifndef MDDS_MULTI_TYPE_VECTOR_USE_DEQUE
        d.reserve(d.size() + len);
#endif
        d.insert(d.begin(), its.first, its.second);
    }

    static void swap_values(base_element_block& blk1, base_element_block& blk2, size_t pos1, size_t pos2, size_t len)
    {
        store_type& st1 = get(blk1).m_array;
        store_type& st2 = get(blk2).m_array;
        assert(pos1 + len <= st1.size());
        assert(pos2 + len <= st2.size());

        typename store_type::iterator it1 = st1.begin(), it2 = st2.begin();
        std::advance(it1, pos1);
        std::advance(it2, pos2);
        for (size_t i = 0; i < len; ++i, ++it1, ++it2)
        {
#ifdef MDDS_MULTI_TYPE_VECTOR_USE_DEQUE
            std::swap(*it1, *it2);
#else
            value_type v1 = *it1, v2 = *it2;
            *it1 = v2;
            *it2 = v1;
#endif
        }
    }

    template<typename _Iter>
    static void set_values(base_element_block& block, size_t pos, const _Iter& it_begin, const _Iter& it_end)
    {
        store_type& d = get(block).m_array;
        typename store_type::iterator it_dest = d.begin();
        std::advance(it_dest, pos);
        for (_Iter it = it_begin; it != it_end; ++it, ++it_dest)
            *it_dest = *it;
    }

    template<typename _Iter>
    static void append_values(base_element_block& block, const _Iter& it_begin, const _Iter& it_end)
    {
        store_type& d = get(block).m_array;
        typename store_type::iterator it = d.end();
        d.insert(it, it_begin, it_end);
    }

    template<typename _Iter>
    static void prepend_values(base_element_block& block, const _Iter& it_begin, const _Iter& it_end)
    {
        store_type& d = get(block).m_array;
        d.insert(d.begin(), it_begin, it_end);
    }

    template<typename _Iter>
    static void assign_values(base_element_block& dest, const _Iter& it_begin, const _Iter& it_end)
    {
        store_type& d = get(dest).m_array;
        d.assign(it_begin, it_end);
    }

    template<typename _Iter>
    static void insert_values(base_element_block& block, size_t pos, const _Iter& it_begin, const _Iter& it_end)
    {
        store_type& blk = get(block).m_array;
        blk.insert(blk.begin() + pos, it_begin, it_end);
    }

    static size_t capacity(const base_element_block& block)
    {
#ifdef MDDS_MULTI_TYPE_VECTOR_USE_DEQUE
        return 0;
#else
        const store_type& blk = get(block).m_array;
        return blk.capacity();
#endif
    }

    static void shrink_to_fit(base_element_block& block)
    {
#ifndef MDDS_MULTI_TYPE_VECTOR_USE_DEQUE
        get(block).m_array.shrink_to_fit();
#endif
    }

private:
    static std::pair<const_iterator, const_iterator> get_iterator_pair(
        const store_type& array, size_t begin_pos, size_t len)
    {
        assert(begin_pos + len <= array.size());
        const_iterator it = array.begin();
        std::advance(it, begin_pos);
        const_iterator it_end = it;
        std::advance(it_end, len);
        return std::pair<const_iterator, const_iterator>(it, it_end);
    }
};

template<typename _Self, element_t _TypeId, typename _Data>
class copyable_element_block : public element_block<_Self, _TypeId, _Data>
{
    typedef element_block<_Self, _TypeId, _Data> base_type;

protected:
    copyable_element_block() : base_type()
    {}
    copyable_element_block(size_t n) : base_type(n)
    {}
    copyable_element_block(size_t n, const _Data& val) : base_type(n, val)
    {}

    template<typename _Iter>
    copyable_element_block(const _Iter& it_begin, const _Iter& it_end) : base_type(it_begin, it_end)
    {}

public:
    using base_type::get;

    static _Self* clone_block(const base_element_block& blk)
    {
        // Use copy constructor to copy the data.
        return new _Self(get(blk));
    }
};

template<typename _Self, element_t _TypeId, typename _Data>
class noncopyable_element_block : public element_block<_Self, _TypeId, _Data>
{
    typedef element_block<_Self, _TypeId, _Data> base_type;

protected:
    noncopyable_element_block() : base_type()
    {}
    noncopyable_element_block(size_t n) : base_type(n)
    {}
    noncopyable_element_block(size_t n, const _Data& val) : base_type(n, val)
    {}

    template<typename _Iter>
    noncopyable_element_block(const _Iter& it_begin, const _Iter& it_end) : base_type(it_begin, it_end)
    {}

public:
    noncopyable_element_block(const noncopyable_element_block&) = delete;
    noncopyable_element_block& operator=(const noncopyable_element_block&) = delete;

    static _Self* clone_block(const base_element_block&)
    {
        throw element_block_error("attempted to clone a noncopyable element block.");
    }
};

/**
 * Get the numerical block type ID from a given element block instance.
 *
 * @param blk element block instance
 *
 * @return numerical value representing the ID of a element block.
 */
inline element_t get_block_type(const base_element_block& blk)
{
    return blk.type;
}

/**
 * Template for default, unmanaged element block for use in
 * multi_type_vector.
 */
template<element_t _TypeId, typename _Data>
struct default_element_block : public copyable_element_block<default_element_block<_TypeId, _Data>, _TypeId, _Data>
{
    typedef copyable_element_block<default_element_block, _TypeId, _Data> base_type;
    typedef default_element_block<_TypeId, _Data> self_type;

    default_element_block() : base_type()
    {}
    default_element_block(size_t n) : base_type(n)
    {}
    default_element_block(size_t n, const _Data& val) : base_type(n, val)
    {}

    template<typename _Iter>
    default_element_block(const _Iter& it_begin, const _Iter& it_end) : base_type(it_begin, it_end)
    {}

    static self_type* create_block_with_value(size_t init_size, const _Data& val)
    {
        return new self_type(init_size, val);
    }

    template<typename _Iter>
    static self_type* create_block_with_values(const _Iter& it_begin, const _Iter& it_end)
    {
        return new self_type(it_begin, it_end);
    }

    static void overwrite_values(base_element_block&, size_t, size_t)
    {
        // Do nothing.
    }
};

/**
 * Template for element block that stores pointers to objects whose life
 * cycles are managed by the block.
 */
template<element_t _TypeId, typename _Data>
struct managed_element_block : public copyable_element_block<managed_element_block<_TypeId, _Data>, _TypeId, _Data*>
{
    typedef copyable_element_block<managed_element_block<_TypeId, _Data>, _TypeId, _Data*> base_type;
    typedef managed_element_block<_TypeId, _Data> self_type;

    using base_type::get;
    using base_type::m_array;
    using base_type::set_value;

    managed_element_block() : base_type()
    {}
    managed_element_block(size_t n) : base_type(n)
    {}
    managed_element_block(const managed_element_block& r)
    {
#ifndef MDDS_MULTI_TYPE_VECTOR_USE_DEQUE
        m_array.reserve(r.m_array.size());
#endif
        typename managed_element_block::store_type::const_iterator it = r.m_array.begin(), it_end = r.m_array.end();
        for (; it != it_end; ++it)
            m_array.push_back(new _Data(**it));
    }

    template<typename _Iter>
    managed_element_block(const _Iter& it_begin, const _Iter& it_end) : base_type(it_begin, it_end)
    {}

    ~managed_element_block()
    {
        std::for_each(m_array.begin(), m_array.end(), std::default_delete<_Data>());
    }

    static self_type* create_block_with_value(size_t init_size, _Data* val)
    {
        // Managed blocks don't support initialization with value.
        if (init_size > 1)
            throw general_error("You can't create a managed block with initial value.");

        std::unique_ptr<self_type> blk = std::make_unique<self_type>(init_size);
        if (init_size == 1)
            set_value(*blk, 0, val);

        return blk.release();
    }

    template<typename _Iter>
    static self_type* create_block_with_values(const _Iter& it_begin, const _Iter& it_end)
    {
        return new self_type(it_begin, it_end);
    }

    static void overwrite_values(base_element_block& block, size_t pos, size_t len)
    {
        managed_element_block& blk = get(block);
        typename managed_element_block::store_type::iterator it = blk.m_array.begin() + pos;
        typename managed_element_block::store_type::iterator it_end = it + len;
        std::for_each(it, it_end, std::default_delete<_Data>());
    }
};

template<element_t _TypeId, typename _Data>
struct noncopyable_managed_element_block
    : public noncopyable_element_block<noncopyable_managed_element_block<_TypeId, _Data>, _TypeId, _Data*>
{
    typedef noncopyable_element_block<noncopyable_managed_element_block<_TypeId, _Data>, _TypeId, _Data*> base_type;
    typedef managed_element_block<_TypeId, _Data> self_type;

    using base_type::get;
    using base_type::m_array;
    using base_type::set_value;

    noncopyable_managed_element_block() : base_type()
    {}
    noncopyable_managed_element_block(size_t n) : base_type(n)
    {}

    template<typename _Iter>
    noncopyable_managed_element_block(const _Iter& it_begin, const _Iter& it_end) : base_type(it_begin, it_end)
    {}

    ~noncopyable_managed_element_block()
    {
        std::for_each(m_array.begin(), m_array.end(), std::default_delete<_Data>());
    }

    static self_type* create_block_with_value(size_t init_size, _Data* val)
    {
        // Managed blocks don't support initialization with value.
        if (init_size > 1)
            throw general_error("You can't create a managed block with initial value.");

        std::unique_ptr<self_type> blk = std::make_unique<self_type>(init_size);
        if (init_size == 1)
            set_value(*blk, 0, val);

        return blk.release();
    }

    template<typename _Iter>
    static self_type* create_block_with_values(const _Iter& it_begin, const _Iter& it_end)
    {
        return new self_type(it_begin, it_end);
    }

    static void overwrite_values(base_element_block& block, size_t pos, size_t len)
    {
        noncopyable_managed_element_block& blk = get(block);
        typename noncopyable_managed_element_block::store_type::iterator it = blk.m_array.begin() + pos;
        typename noncopyable_managed_element_block::store_type::iterator it_end = it + len;
        std::for_each(it, it_end, std::default_delete<_Data>());
    }
};

using boolean_element_block = default_element_block<mtv::element_type_boolean, bool>;
using int8_element_block = default_element_block<mtv::element_type_int8, int8_t>;
using uint8_element_block = default_element_block<mtv::element_type_uint8, uint8_t>;
using int16_element_block = default_element_block<mtv::element_type_int16, int16_t>;
using uint16_element_block = default_element_block<mtv::element_type_uint16, uint16_t>;
using int32_element_block = default_element_block<mtv::element_type_int32, int32_t>;
using uint32_element_block = default_element_block<mtv::element_type_uint32, uint32_t>;
using int64_element_block = default_element_block<mtv::element_type_int64, int64_t>;
using uint64_element_block = default_element_block<mtv::element_type_uint64, uint64_t>;
using float_element_block = default_element_block<mtv::element_type_float, float>;
using double_element_block = default_element_block<mtv::element_type_double, double>;
using string_element_block = default_element_block<mtv::element_type_string, std::string>;

}} // namespace mdds::mtv

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
