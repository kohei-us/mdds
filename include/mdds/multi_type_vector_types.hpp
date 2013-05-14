/*************************************************************************
 *
 * Copyright (c) 2012 Kohei Yoshida
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

#ifndef MDDS_MULTI_TYPE_VECTOR_TYPES_HPP
#define MDDS_MULTI_TYPE_VECTOR_TYPES_HPP

#include "default_deleter.hpp"
#include "compat/unique_ptr.hpp"
#include "global.hpp"

#include <vector>
#include <boost/noncopyable.hpp>

#if defined(MDDS_UNIT_TEST) || defined (MDDS_MULTI_TYPE_VECTOR_DEBUG)
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#endif

namespace mdds { namespace mtv {

typedef int element_t;

const element_t element_type_empty = -1;

const element_t element_type_numeric = 0;
const element_t element_type_string  = 1;
const element_t element_type_short   = 2;
const element_t element_type_ushort  = 3;
const element_t element_type_int     = 4;
const element_t element_type_uint    = 5;
const element_t element_type_long    = 6;
const element_t element_type_ulong   = 7;
const element_t element_type_boolean = 8;
const element_t element_type_char    = 9;
const element_t element_type_uchar   = 10;

const element_t element_type_user_start = 50;

/**
 * Generic exception used for errors specific to element block operations.
 */
class element_block_error : public mdds::general_error
{
public:
    element_block_error(const std::string& msg) : mdds::general_error(msg) {}
};

struct base_element_block;
element_t get_block_type(const base_element_block&);

struct base_element_block
{
    friend element_t get_block_type(const base_element_block&);
protected:
    element_t type;
    base_element_block(element_t _t) : type(_t) {}
};

template<typename _Self, element_t _TypeId, typename _Data>
class element_block : public base_element_block
{
#ifdef MDDS_UNIT_TEST
    struct print_block_array
    {
        void operator() (const _Data& val) const
        {
            std::cout << val << " ";
        }
    };
#endif

protected:
    typedef std::vector<_Data> store_type;
    store_type m_array;

    element_block() : base_element_block(_TypeId) {}
    element_block(size_t n) : base_element_block(_TypeId), m_array(n) {}
    element_block(size_t n, const _Data& val) : base_element_block(_TypeId), m_array(n, val) {}

public:
    typedef typename store_type::iterator iterator;
    typedef typename store_type::reverse_iterator reverse_iterator;
    typedef typename store_type::const_iterator const_iterator;
    typedef typename store_type::const_reverse_iterator const_reverse_iterator;
    typedef _Data value_type;

    bool operator== (const _Self& r) const
    {
        return m_array == r.m_array;
    }

    bool operator!= (const _Self& r) const
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

    static _Self& get(base_element_block& block)
    {
#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
        if (get_block_type(block) != _TypeId)
            throw general_error("incorrect block type.");
#endif
        return static_cast<_Self&>(block);
    }

    static const _Self& get(const base_element_block& block)
    {
#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
        if (get_block_type(block) != _TypeId)
            throw general_error("incorrect block type.");
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
        static_cast<_Self&>(blk).m_array.resize(new_size);
    }

#ifdef MDDS_UNIT_TEST
    static void print_block(const base_element_block& blk)
    {
        const store_type& blk2 = get(blk).m_array;
        std::for_each(blk2.begin(), blk2.end(), print_block_array());
        std::cout << std::endl;
    }
#else
    static void print_block(const base_element_block&) {}
#endif

    static void erase_block(base_element_block& blk, size_t pos)
    {
        store_type& blk2 = get(blk).m_array;
        blk2.erase(blk2.begin()+pos);
    }

    static void erase_block(base_element_block& blk, size_t pos, size_t size)
    {
        store_type& blk2 = get(blk).m_array;
        blk2.erase(blk2.begin()+pos, blk2.begin()+pos+size);
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
        typename store_type::const_iterator it = s.begin();
        std::advance(it, begin_pos);
        typename store_type::const_iterator it_end = it;
        std::advance(it_end, len);
        d.reserve(d.size() + len);
        std::copy(it, it_end, std::back_inserter(d));
    }

    static void assign_values_from_block(
        base_element_block& dest, const base_element_block& src, size_t begin_pos, size_t len)
    {
        store_type& d = get(dest).m_array;
        const store_type& s = get(src).m_array;
        assert(begin_pos + len <= s.size());
        typename store_type::const_iterator it = s.begin();
        std::advance(it, begin_pos);
        typename store_type::const_iterator it_end = it;
        std::advance(it_end, len);
        d.assign(it, it_end);
    }

    template<typename _Iter>
    static void set_values(
        base_element_block& block, size_t pos, const _Iter& it_begin, const _Iter& it_end)
    {
        store_type& d = get(block).m_array;
        for (_Iter it = it_begin; it != it_end; ++it, ++pos)
            d[pos] = *it;
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
    static void insert_values(
        base_element_block& block, size_t pos, const _Iter& it_begin, const _Iter& it_end)
    {
        store_type& blk = get(block).m_array;
        blk.insert(blk.begin()+pos, it_begin, it_end);
    }
};

template<typename _Self, element_t _TypeId, typename _Data>
class copyable_element_block : public element_block<_Self, _TypeId, _Data>
{
    typedef element_block<_Self,_TypeId,_Data> base_type;
protected:
    copyable_element_block() : base_type() {}
    copyable_element_block(size_t n) : base_type(n) {}
    copyable_element_block(size_t n, const _Data& val) : base_type(n, val) {}

public:
    using base_type::get;

    static _Self* clone_block(const base_element_block& blk)
    {
        return new _Self(get(blk));
    }
};

template<typename _Self, element_t _TypeId, typename _Data>
class noncopyable_element_block : public element_block<_Self, _TypeId, _Data>, private boost::noncopyable
{
    typedef element_block<_Self,_TypeId,_Data> base_type;
protected:
    noncopyable_element_block() : base_type() {}
    noncopyable_element_block(size_t n) : base_type(n) {}
    noncopyable_element_block(size_t n, const _Data& val) : base_type(n, val) {}

public:
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
struct default_element_block : public copyable_element_block<default_element_block<_TypeId,_Data>, _TypeId, _Data>
{
    typedef copyable_element_block<default_element_block, _TypeId, _Data> base_type;
    typedef default_element_block<_TypeId,_Data> self_type;

    default_element_block() : base_type() {}
    default_element_block(size_t n) : base_type(n) {}
    default_element_block(size_t n, const _Data& val) : base_type(n, val) {}

    static self_type* create_block_with_value(size_t init_size, const _Data& val)
    {
        return new self_type(init_size, val);
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
struct managed_element_block : public copyable_element_block<managed_element_block<_TypeId,_Data>, _TypeId, _Data*>
{
    typedef copyable_element_block<managed_element_block<_TypeId,_Data>, _TypeId, _Data*> base_type;
    typedef managed_element_block<_TypeId,_Data> self_type;

    using base_type::get;
    using base_type::set_value;
    using base_type::m_array;

    managed_element_block() : base_type() {}
    managed_element_block(size_t n) : base_type(n) {}
    managed_element_block(const managed_element_block& r)
    {
        m_array.reserve(r.m_array.size());
        typename managed_element_block::store_type::const_iterator it = r.m_array.begin(), it_end = r.m_array.end();
        for (; it != it_end; ++it)
            m_array.push_back(new _Data(**it));
    }

    ~managed_element_block()
    {
        std::for_each(m_array.begin(), m_array.end(), mdds::default_deleter<_Data>());
    }

    static self_type* create_block_with_value(size_t init_size, _Data* val)
    {
        // Managed blocks don't support initialization with value.
        if (init_size > 1)
            throw general_error("You can't create a managed block with initial value.");

        unique_ptr<self_type> blk(new self_type(init_size));
        if (init_size == 1)
            set_value(*blk, 0, val);

        return blk.release();
    }

    static void overwrite_values(base_element_block& block, size_t pos, size_t len)
    {
        managed_element_block& blk = get(block);
        typename managed_element_block::store_type::iterator it = blk.m_array.begin() + pos;
        typename managed_element_block::store_type::iterator it_end = it + len;
        std::for_each(it, it_end, mdds::default_deleter<_Data>());
    }
};

template<element_t _TypeId, typename _Data>
struct noncopyable_managed_element_block : public noncopyable_element_block<noncopyable_managed_element_block<_TypeId,_Data>, _TypeId, _Data*>
{
    typedef noncopyable_element_block<noncopyable_managed_element_block<_TypeId,_Data>, _TypeId, _Data*> base_type;
    typedef managed_element_block<_TypeId,_Data> self_type;

    using base_type::get;
    using base_type::m_array;
    using base_type::set_value;

    noncopyable_managed_element_block() : base_type() {}
    noncopyable_managed_element_block(size_t n) : base_type(n) {}

    ~noncopyable_managed_element_block()
    {
        std::for_each(m_array.begin(), m_array.end(), mdds::default_deleter<_Data>());
    }

    static self_type* create_block_with_value(size_t init_size, _Data* val)
    {
        // Managed blocks don't support initialization with value.
        if (init_size > 1)
            throw general_error("You can't create a managed block with initial value.");

        unique_ptr<self_type> blk(new self_type(init_size));
        if (init_size == 1)
            set_value(*blk, 0, val);

        return blk.release();
    }

    static void overwrite_values(base_element_block& block, size_t pos, size_t len)
    {
        noncopyable_managed_element_block& blk = get(block);
        typename noncopyable_managed_element_block::store_type::iterator it = blk.m_array.begin() + pos;
        typename noncopyable_managed_element_block::store_type::iterator it_end = it + len;
        std::for_each(it, it_end, mdds::default_deleter<_Data>());
    }
};

typedef default_element_block<mtv::element_type_numeric, double>        numeric_element_block;
typedef default_element_block<mtv::element_type_string, std::string>    string_element_block;
typedef default_element_block<mtv::element_type_short, short>           short_element_block;
typedef default_element_block<mtv::element_type_ushort, unsigned short> ushort_element_block;
typedef default_element_block<mtv::element_type_int, int>               int_element_block;
typedef default_element_block<mtv::element_type_uint, unsigned int>     uint_element_block;
typedef default_element_block<mtv::element_type_long, long>             long_element_block;
typedef default_element_block<mtv::element_type_ulong, unsigned long>   ulong_element_block;
typedef default_element_block<mtv::element_type_boolean, bool>          boolean_element_block;
typedef default_element_block<mtv::element_type_char, char>             char_element_block;
typedef default_element_block<mtv::element_type_uchar, unsigned char>   uchar_element_block;

}}

#endif
