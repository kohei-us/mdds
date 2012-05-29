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

#ifndef __MDDS_GRID_MAP_TYPES_HPP__
#define __MDDS_GRID_MAP_TYPES_HPP__

#include "mdds/default_deleter.hpp"

#include <vector>
#include <boost/noncopyable.hpp>

namespace mdds { namespace mtv {

typedef int element_t;

const element_t element_type_empty = -1;

const element_t element_type_numeric = 0;
const element_t element_type_string  = 1;
const element_t element_type_index   = 2;
const element_t element_type_boolean = 3;

const element_t element_type_user_start = 50;

/**
 * Generic exception used for errors specific to cell block operations.
 */
class cell_block_error : public mdds::general_error
{
public:
    cell_block_error(const std::string& msg) : mdds::general_error(msg) {}
};

struct base_cell_block;
element_t get_block_type(const base_cell_block&);

struct base_cell_block
{
    friend element_t get_block_type(const base_cell_block&);
protected:
    element_t type;
    base_cell_block(element_t _t) : type(_t) {}
};

template<typename _Self, element_t _TypeId, typename _Data>
class cell_block : public base_cell_block
{
    struct print_block_array
    {
        void operator() (const _Data& val) const
        {
            std::cout << val << " ";
        }
    };

protected:
    typedef std::vector<_Data> store_type;
    store_type m_array;

    cell_block() : base_cell_block(_TypeId) {}
    cell_block(size_t n) : base_cell_block(_TypeId), m_array(n) {}

public:
    bool operator== (const _Self& r) const
    {
        return m_array == r.m_array;
    }

    bool operator!= (const _Self& r) const
    {
        return !operator==(r);
    }

    static _Self& get(base_cell_block& block)
    {
        if (get_block_type(block) != _TypeId)
            throw general_error("incorrect block type.");

        return static_cast<_Self&>(block);
    }

    static const _Self& get(const base_cell_block& block)
    {
        if (get_block_type(block) != _TypeId)
            throw general_error("incorrect block type.");

        return static_cast<const _Self&>(block);
    }

    static void set_value(base_cell_block& blk, size_t pos, const _Data& val)
    {
        get(blk).m_array[pos] = val;
    }

    static void get_value(const base_cell_block& blk, size_t pos, _Data& val)
    {
        val = get(blk).m_array[pos];
    }

    static void append_value(base_cell_block& blk, const _Data& val)
    {
        get(blk).m_array.push_back(val);
    }

    static void prepend_value(base_cell_block& blk, const _Data& val)
    {
        store_type& blk2 = get(blk).m_array;
        blk2.insert(blk2.begin(), val);
    }

    static _Self* create_block(size_t init_size)
    {
        return new _Self(init_size);
    }

    static void delete_block(const base_cell_block* p)
    {
        delete static_cast<const _Self*>(p);
    }

    static void resize_block(base_cell_block& blk, size_t new_size)
    {
        static_cast<_Self&>(blk).m_array.resize(new_size);
    }

    static void print_block(const base_cell_block& blk)
    {
        const store_type& blk2 = get(blk).m_array;
        std::for_each(blk2.begin(), blk2.end(), print_block_array());
        std::cout << std::endl;
    }

    static void erase_block(base_cell_block& blk, size_t pos)
    {
        store_type& blk2 = get(blk).m_array;
        blk2.erase(blk2.begin()+pos);
    }

    static void erase_block(base_cell_block& blk, size_t pos, size_t size)
    {
        store_type& blk2 = get(blk).m_array;
        blk2.erase(blk2.begin()+pos, blk2.begin()+pos+size);
    }

    static void append_values_from_block(base_cell_block& dest, const base_cell_block& src)
    {
        store_type& d = get(dest).m_array;
        const store_type& s = get(src).m_array;
        d.insert(d.end(), s.begin(), s.end());
    }

    static void append_values_from_block(
        base_cell_block& dest, const base_cell_block& src, size_t begin_pos, size_t len)
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
        base_cell_block& dest, const base_cell_block& src, size_t begin_pos, size_t len)
    {
        store_type& d = get(dest).m_array;
        const store_type& s = get(src).m_array;
        typename store_type::const_iterator it = s.begin();
        std::advance(it, begin_pos);
        typename store_type::const_iterator it_end = it;
        std::advance(it_end, len);
        d.assign(it, it_end);
    }

    template<typename _Iter>
    static void set_values(
        base_cell_block& block, size_t pos, const _Iter& it_begin, const _Iter& it_end)
    {
        store_type& d = get(block).m_array;
        for (_Iter it = it_begin; it != it_end; ++it, ++pos)
            d[pos] = *it;
    }

    template<typename _Iter>
    static void append_values(base_cell_block& block, const _Iter& it_begin, const _Iter& it_end)
    {
        store_type& d = get(block).m_array;
        typename store_type::iterator it = d.end();
        d.insert(it, it_begin, it_end);
    }

    template<typename _Iter>
    static void prepend_values(base_cell_block& block, const _Iter& it_begin, const _Iter& it_end)
    {
        store_type& d = get(block).m_array;
        d.insert(d.begin(), it_begin, it_end);
    }

    template<typename _Iter>
    static void assign_values(base_cell_block& dest, const _Iter& it_begin, const _Iter& it_end)
    {
        store_type& d = get(dest).m_array;
        d.assign(it_begin, it_end);
    }

    template<typename _Iter>
    static void insert_values(
        base_cell_block& block, size_t pos, const _Iter& it_begin, const _Iter& it_end)
    {
        store_type& blk = get(block).m_array;
        blk.insert(blk.begin()+pos, it_begin, it_end);
    }
};

template<typename _Self, element_t _TypeId, typename _Data>
class copyable_cell_block : public cell_block<_Self, _TypeId, _Data>
{
    typedef cell_block<_Self,_TypeId,_Data> base_type;
protected:
    copyable_cell_block() : base_type() {}
    copyable_cell_block(size_t n) : base_type(n) {}

public:
    using base_type::get;

    static _Self* clone_block(const base_cell_block& blk)
    {
        return new _Self(get(blk));
    }
};

template<typename _Self, element_t _TypeId, typename _Data>
class noncopyable_cell_block : public cell_block<_Self, _TypeId, _Data>, private boost::noncopyable
{
    typedef cell_block<_Self,_TypeId,_Data> base_type;
protected:
    noncopyable_cell_block() : base_type() {}
    noncopyable_cell_block(size_t n) : base_type(n) {}

public:
    static _Self* clone_block(const base_cell_block& blk)
    {
        throw cell_block_error("attempted to clone a noncopyable cell block.");
    }
};

/**
 * Get the numerical block type ID from a given cell block instance.
 *
 * @param blk cell block instance
 *
 * @return numerical value representing the ID of a cell block.
 */
inline element_t get_block_type(const base_cell_block& blk)
{
    return blk.type;
}

/**
 * Template for default, unmanaged cell block for use in grid_map.
 */
template<element_t _TypeId, typename _Data>
struct default_cell_block : public copyable_cell_block<default_cell_block<_TypeId,_Data>, _TypeId, _Data>
{
    typedef copyable_cell_block<default_cell_block, _TypeId, _Data> base_type;

    default_cell_block() : base_type() {}
    default_cell_block(size_t n) : base_type(n) {}

    static void overwrite_cells(base_cell_block&, size_t, size_t)
    {
        // Do nothing.
    }
};

/**
 * Template for cell block that stores pointers to objects whose life cycles
 * are managed by the block.
 */
template<element_t _TypeId, typename _Data>
struct managed_cell_block : public copyable_cell_block<managed_cell_block<_TypeId,_Data>, _TypeId, _Data*>
{
    typedef copyable_cell_block<managed_cell_block<_TypeId,_Data>, _TypeId, _Data*> base_type;

    using base_type::get;
    using base_type::m_array;

    managed_cell_block() : base_type() {}
    managed_cell_block(size_t n) : base_type(n) {}
    managed_cell_block(const managed_cell_block& r)
    {
        m_array.reserve(r.m_array.size());
        typename managed_cell_block::store_type::const_iterator it = r.m_array.begin(), it_end = r.m_array.end();
        for (; it != it_end; ++it)
            m_array.push_back(new _Data(**it));
    }

    ~managed_cell_block()
    {
        std::for_each(m_array.begin(), m_array.end(), mdds::default_deleter<_Data>());
    }

    static void overwrite_cells(base_cell_block& block, size_t pos, size_t len)
    {
        managed_cell_block& blk = get(block);
        typename managed_cell_block::store_type::iterator it = blk.m_array.begin() + pos;
        typename managed_cell_block::store_type::iterator it_end = it + len;
        std::for_each(it, it_end, mdds::default_deleter<_Data>());
    }
};

template<element_t _TypeId, typename _Data>
struct noncopyable_managed_cell_block : public noncopyable_cell_block<noncopyable_managed_cell_block<_TypeId,_Data>, _TypeId, _Data*>
{
    typedef noncopyable_cell_block<noncopyable_managed_cell_block<_TypeId,_Data>, _TypeId, _Data*> base_type;

    using base_type::get;
    using base_type::m_array;

    noncopyable_managed_cell_block() : base_type() {}
    noncopyable_managed_cell_block(size_t n) : base_type(n) {}

    ~noncopyable_managed_cell_block()
    {
        std::for_each(m_array.begin(), m_array.end(), mdds::default_deleter<_Data>());
    }

    static void overwrite_cells(base_cell_block& block, size_t pos, size_t len)
    {
        noncopyable_managed_cell_block& blk = get(block);
        typename noncopyable_managed_cell_block::store_type::iterator it = blk.m_array.begin() + pos;
        typename noncopyable_managed_cell_block::store_type::iterator it_end = it + len;
        std::for_each(it, it_end, mdds::default_deleter<_Data>());
    }
};

}}

#endif
