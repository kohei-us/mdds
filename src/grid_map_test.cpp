/*************************************************************************
 *
 * Copyright (c) 2011-2012 Kohei Yoshida
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

#include "mdds/grid_map.hpp"
#include "test_global.hpp"

#include <cassert>
#include <sstream>

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

#include <stdio.h>
#include <string>
#include <sys/time.h>

using namespace std;
using namespace mdds;

namespace {

class stack_printer
{
public:
    explicit stack_printer(const char* msg) :
        m_msg(msg)
    {
        fprintf(stdout, "%s: --begin\n", m_msg.c_str());
        m_start_time = get_time();
    }

    ~stack_printer()
    {
        double end_time = get_time();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", m_msg.c_str(), (end_time-m_start_time));
    }

    void print_time(int line) const
    {
        double end_time = get_time();
        fprintf(stdout, "%s: --(%d) (duration: %g sec)\n", m_msg.c_str(), line, (end_time-m_start_time));
    }

private:
    double get_time() const
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    std::string m_msg;
    double m_start_time;
};

enum cell_t { celltype_numeric = 0, celltype_string, celltype_index, celltype_boolean };

struct get_cell_type
{
    template<typename T>
    cell_t operator() (const T& t);
};

template<typename T>
cell_t get_cell_type::operator() (const T& t)
{
    return celltype_numeric;
}

template<>
cell_t get_cell_type::operator()<double> (const double& t)
{
    return celltype_numeric;
}

template<>
cell_t get_cell_type::operator()<std::string> (const std::string& t)
{
    return celltype_string;
}

template<>
cell_t get_cell_type::operator()<size_t> (const size_t& t)
{
    return celltype_index;
}

template<>
cell_t get_cell_type::operator()<bool> (const bool& t)
{
    return celltype_boolean;
}

struct base_cell_block
{
    cell_t type;
    base_cell_block(cell_t _t) : type(_t) {}
};

struct numeric_cell_block : public base_cell_block, public vector<double>
{
public:
    numeric_cell_block() : base_cell_block(celltype_numeric), vector<double>(1) {}
};

struct string_cell_block : public base_cell_block, public vector<string>
{
public:
    string_cell_block() : base_cell_block(celltype_string), vector<string>(1) {}
};

struct index_cell_block : public base_cell_block, public vector<size_t>
{
public:
    index_cell_block() : base_cell_block(celltype_index), vector<size_t>(1) {}
};

struct boolean_cell_block : public base_cell_block, public vector<bool>
{
public:
    boolean_cell_block() : base_cell_block(celltype_boolean), vector<bool>(1) {}
};

struct get_cell_block_type : public std::unary_function<base_cell_block, cell_t>
{
    cell_t operator() (const base_cell_block& r)
    {
        return r.type;
    }
};

numeric_cell_block* get_numeric_block(base_cell_block* block)
{
    if (!block || block->type != celltype_numeric)
        throw general_error("block is not of numeric type!");

    return static_cast<numeric_cell_block*>(block);
}

const numeric_cell_block* get_numeric_block(const base_cell_block* block)
{
    if (!block || block->type != celltype_numeric)
        throw general_error("block is not of numeric type!");

    return static_cast<const numeric_cell_block*>(block);
}

string_cell_block* get_string_block(base_cell_block* block)
{
    if (!block || block->type != celltype_string)
        throw general_error("block is not of string type!");

    return static_cast<string_cell_block*>(block);
}

const string_cell_block* get_string_block(const base_cell_block* block)
{
    if (!block || block->type != celltype_string)
        throw general_error("block is not of string type!");

    return static_cast<const string_cell_block*>(block);
}

index_cell_block* get_index_block(base_cell_block* block)
{
    if (!block || block->type != celltype_index)
        throw general_error("block is not of index type!");

    return static_cast<index_cell_block*>(block);
}

const index_cell_block* get_index_block(const base_cell_block* block)
{
    if (!block || block->type != celltype_index)
        throw general_error("block is not of index type!");

    return static_cast<const index_cell_block*>(block);
}

boolean_cell_block* get_boolean_block(base_cell_block* block)
{
    if (!block || block->type != celltype_boolean)
        throw general_error("block is not of boolean type!");

    return static_cast<boolean_cell_block*>(block);
}

const boolean_cell_block* get_boolean_block(const base_cell_block* block)
{
    if (!block || block->type != celltype_boolean)
        throw general_error("block is not of boolean type!");

    return static_cast<const boolean_cell_block*>(block);
}

struct cell_block_func
{
    static base_cell_block* create_new_block(cell_t type);

    static base_cell_block* clone_block(base_cell_block* p);

    static void delete_block(base_cell_block* p);

    static void resize_block(base_cell_block* p, size_t new_size);

    static void print_block(base_cell_block* p);

    static void erase(base_cell_block* block, size_t pos);

    static void erase(base_cell_block* block, size_t pos, size_t size);

    template<typename T>
    static void set_value(base_cell_block* block, size_t pos, const T& val);

    template<typename T>
    static void set_values(base_cell_block* block, size_t pos, const T& it_begin, const T& it_end);

    template<typename T>
    static void prepend_value(base_cell_block* block, const T& val);

    template<typename T>
    static void prepend_values(base_cell_block* block, const T& it_begin, const T& it_end);

    template<typename T>
    static void append_value(base_cell_block* block, const T& val);

    static void append_values(base_cell_block* dest, const base_cell_block* src);

    template<typename T>
    static void append_values(base_cell_block* block, const T& it_begin, const T& it_end);

    static void assign_values(base_cell_block* dest, const base_cell_block* src,
                              size_t begin_pos, size_t len);

    template<typename T>
    static void assign_values(base_cell_block* dest, const T& it_begin, const T& it_end);

    template<typename T>
    static void get_value(base_cell_block* block, size_t pos, T& val);

    template<typename T>
    static void get_empty_value(T& val);

    static bool equal_block(const base_cell_block* left, const base_cell_block* right);
};

base_cell_block* cell_block_func::create_new_block(cell_t type)
{
    switch (type)
    {
        case celltype_numeric:
            return new numeric_cell_block;
        case celltype_string:
            return new string_cell_block;
        case celltype_index:
            return new index_cell_block;
        case celltype_boolean:
            return new boolean_cell_block;
        default:
            ;
    }
    return NULL;
}

base_cell_block* cell_block_func::clone_block(base_cell_block* p)
{
    if (!p)
        return NULL;

    switch (p->type)
    {
        case celltype_numeric:
            return new numeric_cell_block(*get_numeric_block(p));
        case celltype_string:
            return new string_cell_block(*get_string_block(p));
        case celltype_index:
            return new index_cell_block(*get_index_block(p));
        case celltype_boolean:
            return new boolean_cell_block(*get_boolean_block(p));
        default:
            ;
    }
    return NULL;
}

void cell_block_func::delete_block(base_cell_block* p)
{
    if (!p)
        return;

    switch (p->type)
    {
        case celltype_numeric:
            delete static_cast<numeric_cell_block*>(p);
        break;
        case celltype_string:
            delete static_cast<string_cell_block*>(p);
        break;
        case celltype_index:
            delete static_cast<index_cell_block*>(p);
        break;
        case celltype_boolean:
            delete static_cast<boolean_cell_block*>(p);
        break;
        default:
            assert(!"attempting to delete a cell block instance of unknown type!");
    }
}

void cell_block_func::resize_block(base_cell_block* p, size_t new_size)
{
    if (!p)
        return;

    switch (p->type)
    {
        case celltype_numeric:
            static_cast<numeric_cell_block*>(p)->resize(new_size);
        break;
        case celltype_string:
            static_cast<string_cell_block*>(p)->resize(new_size);
        break;
        case celltype_index:
            static_cast<index_cell_block*>(p)->resize(new_size);
        break;
        case celltype_boolean:
            static_cast<boolean_cell_block*>(p)->resize(new_size);
        break;
        default:
            assert(!"attempting to delete a cell block instance of unknown type!");
    }
}

template<typename T>
struct print_block_array
{
    void operator() (const T& val) const
    {
        cout << val << " ";
    }
};

void cell_block_func::print_block(base_cell_block* p)
{
    if (!p)
        return;

    switch (p->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& blk = *static_cast<numeric_cell_block*>(p);
            for_each(blk.begin(), blk.end(), print_block_array<double>());
            cout << endl;
        }
        break;
        case celltype_string:
        {
            string_cell_block& blk = *static_cast<string_cell_block*>(p);
            for_each(blk.begin(), blk.end(), print_block_array<string>());
            cout << endl;
        }
        break;
        case celltype_index:
        {
            index_cell_block& blk = *static_cast<index_cell_block*>(p);
            for_each(blk.begin(), blk.end(), print_block_array<size_t>());
            cout << endl;
        }
        break;
        case celltype_boolean:
        {
            boolean_cell_block& blk = *static_cast<boolean_cell_block*>(p);
            for_each(blk.begin(), blk.end(), print_block_array<bool>());
            cout << endl;
        }
        break;
        default:
            assert(!"attempting to print a cell block instance of unknown type!");
    }
}

void cell_block_func::erase(base_cell_block* block, size_t pos)
{
    if (!block)
        return;

    switch (block->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& blk = *static_cast<numeric_cell_block*>(block);
            blk.erase(blk.begin()+pos);
        }
        break;
        case celltype_string:
        {
            string_cell_block& blk = *static_cast<string_cell_block*>(block);
            blk.erase(blk.begin()+pos);
        }
        break;
        case celltype_index:
        {
            index_cell_block& blk = *static_cast<index_cell_block*>(block);
            blk.erase(blk.begin()+pos);
        }
        break;
        case celltype_boolean:
        {
            boolean_cell_block& blk = *static_cast<boolean_cell_block*>(block);
            blk.erase(blk.begin()+pos);
        }
        break;
        default:
            assert(!"attempting to erase a cell from a block of unknown type!");
    }
}

void cell_block_func::erase(base_cell_block* block, size_t pos, size_t size)
{
    if (!block)
        return;

    switch (block->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& blk = *static_cast<numeric_cell_block*>(block);
            blk.erase(blk.begin()+pos, blk.begin()+pos+size);
        }
        break;
        case celltype_string:
        {
            string_cell_block& blk = *static_cast<string_cell_block*>(block);
            blk.erase(blk.begin()+pos, blk.begin()+pos+size);
        }
        break;
        case celltype_index:
        {
            index_cell_block& blk = *static_cast<index_cell_block*>(block);
            blk.erase(blk.begin()+pos, blk.begin()+pos+size);
        }
        break;
        case celltype_boolean:
        {
            boolean_cell_block& blk = *static_cast<boolean_cell_block*>(block);
            blk.erase(blk.begin()+pos, blk.begin()+pos+size);
        }
        break;
        default:
            assert(!"attempting to erase a cell from a block of unknown type!");
    }
}

template<typename T>
void cell_block_func::set_value(base_cell_block* block, size_t pos, const T& val)
{
    throw general_error("non-specialized version called.");
}

template<>
void cell_block_func::set_value<double>(base_cell_block* block, size_t pos, const double& val)
{
    numeric_cell_block& blk = *get_numeric_block(block);
    blk[pos] = val;
}

template<>
void cell_block_func::set_value<string>(base_cell_block* block, size_t pos, const string& val)
{
    string_cell_block& blk = *get_string_block(block);
    blk[pos] = val;
}

template<>
void cell_block_func::set_value<size_t>(base_cell_block* block, size_t pos, const size_t& val)
{
    index_cell_block& blk = *get_index_block(block);
    blk[pos] = val;
}

template<>
void cell_block_func::set_value<bool>(base_cell_block* block, size_t pos, const bool& val)
{
    boolean_cell_block& blk = *get_boolean_block(block);
    blk[pos] = val;
}

template<typename _Iter>
void _set_values(
    base_cell_block* block, size_t pos, const typename _Iter::value_type&,
    const _Iter& it_begin, const _Iter& it_end)
{
    throw general_error("non-specialized version of _set_values called.");
}

template<typename _Iter>
void _set_values(
    base_cell_block* block, size_t pos, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block& d = *get_numeric_block(block);
    for (_Iter it = it_begin; it != it_end; ++it, ++pos)
        d[pos] = *it;
}

template<typename _Iter>
void _set_values(
    base_cell_block* block, size_t pos, string, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block& d = *get_string_block(block);
    for (_Iter it = it_begin; it != it_end; ++it, ++pos)
        d[pos] = *it;
}

template<typename _Iter>
void _set_values(
    base_cell_block* block, size_t pos, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block& d = *get_index_block(block);
    for (_Iter it = it_begin; it != it_end; ++it, ++pos)
        d[pos] = *it;
}

template<typename _Iter>
void _set_values(
    base_cell_block* block, size_t pos, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block& d = *get_boolean_block(block);
    for (_Iter it = it_begin; it != it_end; ++it, ++pos)
        d[pos] = *it;
}

template<typename T>
void cell_block_func::set_values(base_cell_block* block, size_t pos, const T& it_begin, const T& it_end)
{
    if (!block)
        throw general_error("destination cell block is NULL.");

    if (it_begin == it_end)
        // Nothing to do.
        return;

    _set_values(block, pos, *it_begin, it_begin, it_end);
}

template<typename T>
void cell_block_func::prepend_value(base_cell_block* block, const T& val)
{
    throw general_error("non-specialized version called.");
}

template<>
void cell_block_func::prepend_value<double>(base_cell_block* block, const double& val)
{
    numeric_cell_block& blk = *get_numeric_block(block);
    blk.insert(blk.begin(), val);
}

template<>
void cell_block_func::prepend_value<string>(base_cell_block* block, const string& val)
{
    string_cell_block& blk = *get_string_block(block);
    blk.insert(blk.begin(), val);
}

template<>
void cell_block_func::prepend_value<size_t>(base_cell_block* block, const size_t& val)
{
    index_cell_block& blk = *get_index_block(block);
    blk.insert(blk.begin(), val);
}

template<>
void cell_block_func::prepend_value<bool>(base_cell_block* block, const bool& val)
{
    boolean_cell_block& blk = *get_boolean_block(block);
    blk.insert(blk.begin(), val);
}

template<typename _Iter>
void _prepend_values(base_cell_block* block, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block& d = *get_numeric_block(block);
    d.insert(d.begin(), it_begin, it_end);
}

template<typename _Iter>
void _prepend_values(base_cell_block* block, const string&, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block& d = *get_string_block(block);
    d.insert(d.begin(), it_begin, it_end);
}

template<typename _Iter>
void _prepend_values(base_cell_block* block, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block& d = *get_index_block(block);
    d.insert(d.begin(), it_begin, it_end);
}

template<typename _Iter>
void _prepend_values(base_cell_block* block, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block& d = *get_boolean_block(block);
    d.insert(d.begin(), it_begin, it_end);
}

template<typename T>
void cell_block_func::prepend_values(base_cell_block* block, const T& it_begin, const T& it_end)
{
    assert(it_begin != it_end);
    _prepend_values(block, *it_begin, it_begin, it_end);
}

template<typename T>
void cell_block_func::append_value(base_cell_block* block, const T& val)
{
    throw general_error("non-specialized version called.");
}

template<>
void cell_block_func::append_value<double>(base_cell_block* block, const double& val)
{
    numeric_cell_block& blk = *get_numeric_block(block);
    blk.push_back(val);
}

template<>
void cell_block_func::append_value<string>(base_cell_block* block, const string& val)
{
    string_cell_block& blk = *get_string_block(block);
    blk.push_back(val);
}

template<>
void cell_block_func::append_value<size_t>(base_cell_block* block, const size_t& val)
{
    index_cell_block& blk = *get_index_block(block);
    blk.push_back(val);
}

template<>
void cell_block_func::append_value<bool>(base_cell_block* block, const bool& val)
{
    boolean_cell_block& blk = *get_boolean_block(block);
    blk.push_back(val);
}

void cell_block_func::append_values(base_cell_block* dest, const base_cell_block* src)
{
    if (!dest)
        throw general_error("destination cell block is NULL.");

    switch (dest->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& d = *get_numeric_block(dest);
            const numeric_cell_block& s = *get_numeric_block(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        case celltype_string:
        {
            string_cell_block& d = *get_string_block(dest);
            const string_cell_block& s = *get_string_block(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        case celltype_index:
        {
            index_cell_block& d = *get_index_block(dest);
            const index_cell_block& s = *get_index_block(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        case celltype_boolean:
        {
            boolean_cell_block& d = *get_boolean_block(dest);
            const boolean_cell_block& s = *get_boolean_block(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        default:
            assert(!"unhandled cell type.");
    }
}

template<typename _Iter>
void _append_values(base_cell_block* block, const typename _Iter::value_type&, const _Iter& it_begin, const _Iter& it_end)
{
    throw general_error("non-specialized version of _append_values called.");
}

template<typename _Iter>
void _append_values(base_cell_block* block, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block& d = *get_numeric_block(block);
    numeric_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

template<typename _Iter>
void _append_values(base_cell_block* block, string, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block& d = *get_string_block(block);
    string_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

template<typename _Iter>
void _append_values(base_cell_block* block, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block& d = *get_index_block(block);
    index_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

template<typename _Iter>
void _append_values(base_cell_block* block, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block& d = *get_boolean_block(block);
    boolean_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

template<typename T>
void cell_block_func::append_values(base_cell_block* block, const T& it_begin, const T& it_end)
{
    assert(it_begin != it_end);
    _append_values(block, *it_begin, it_begin, it_end);
}

void cell_block_func::assign_values(base_cell_block* dest, const base_cell_block* src, size_t begin_pos, size_t len)
{
    if (!dest)
        throw general_error("destination cell block is NULL.");

    switch (dest->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& d = *get_numeric_block(dest);
            const numeric_cell_block& s = *get_numeric_block(src);
            numeric_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            numeric_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
        case celltype_string:
        {
            string_cell_block& d = *get_string_block(dest);
            const string_cell_block& s = *get_string_block(src);
            string_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            string_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
        case celltype_index:
        {
            index_cell_block& d = *get_index_block(dest);
            const index_cell_block& s = *get_index_block(src);
            index_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            index_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
        case celltype_boolean:
        {
            boolean_cell_block& d = *get_boolean_block(dest);
            const boolean_cell_block& s = *get_boolean_block(src);
            boolean_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            boolean_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
        default:
            assert(!"unhandled cell type.");
    }
}

template<typename _Iter>
void _assign_values(base_cell_block* dest, const typename _Iter::value_type&, const _Iter& it_begin, const _Iter& it_end)
{
    throw general_error("non-specialized version of _assign_values called.");
}

template<typename _Iter>
void _assign_values(base_cell_block* dest, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block& d = *get_numeric_block(dest);
    d.assign(it_begin, it_end);
}

template<typename _Iter>
void _assign_values(base_cell_block* dest, const string&, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block& d = *get_string_block(dest);
    d.assign(it_begin, it_end);
}

template<typename _Iter>
void _assign_values(base_cell_block* dest, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block& d = *get_index_block(dest);
    d.assign(it_begin, it_end);
}

template<typename _Iter>
void _assign_values(base_cell_block* dest, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block& d = *get_boolean_block(dest);
    d.assign(it_begin, it_end);
}

template<typename T>
void cell_block_func::assign_values(base_cell_block* dest, const T& it_begin, const T& it_end)
{
    if (!dest)
        throw general_error("destination cell block is NULL.");

    if (it_begin == it_end)
        // Nothing to do.
        return;

    _assign_values(dest, *it_begin, it_begin, it_end);
}

template<typename T>
void cell_block_func::get_value(base_cell_block* block, size_t pos, T& val)
{
    throw general_error("non-specialized version called.");
}

template<>
void cell_block_func::get_value<double>(base_cell_block* block, size_t pos, double& val)
{
    numeric_cell_block& blk = *get_numeric_block(block);
    val = blk[pos];
}

template<>
void cell_block_func::get_value<string>(base_cell_block* block, size_t pos, string& val)
{
    string_cell_block& blk = *get_string_block(block);
    val = blk[pos];
}

template<>
void cell_block_func::get_value<size_t>(base_cell_block* block, size_t pos, size_t& val)
{
    index_cell_block& blk = *get_index_block(block);
    val = blk[pos];
}

template<>
void cell_block_func::get_value<bool>(base_cell_block* block, size_t pos, bool& val)
{
    boolean_cell_block& blk = *get_boolean_block(block);
    val = blk[pos];
}

template<typename T>
void cell_block_func::get_empty_value(T& val)
{
    throw general_error("non-specialized version called.");
}

template<>
void cell_block_func::get_empty_value<double>(double& val)
{
    val = 0.0;
}

template<>
void cell_block_func::get_empty_value<string>(string& val)
{
    val = string();
}

template<>
void cell_block_func::get_empty_value<size_t>(size_t& val)
{
    val = 0;
}

template<>
void cell_block_func::get_empty_value<bool>(bool& val)
{
    val = false;
}

bool cell_block_func::equal_block(const base_cell_block* left, const base_cell_block* right)
{
    if (!left && !right)
        return true;

    if (left && !right)
        return false;

    if (!left && right)
        return false;

    assert(left && right);
    cell_t block_type = left->type;
    if (block_type != right->type)
        return false;

    switch (block_type)
    {
        case celltype_numeric:
            return *get_numeric_block(left) == *get_numeric_block(right);
        case celltype_string:
            return *get_string_block(left) == *get_string_block(right);
        case celltype_index:
            return *get_index_block(left) == *get_index_block(right);
        case celltype_boolean:
            return *get_boolean_block(left) == *get_boolean_block(right);
        default:
            ;
    }
    return false;
}

struct grid_map_trait
{
    typedef base_cell_block cell_block_type;
    typedef cell_t cell_category_type;
    typedef long sheet_key_type;
    typedef long row_key_type;
    typedef long col_key_type;

    typedef get_cell_type cell_type_inspector;
    typedef get_cell_block_type cell_block_type_inspector;

    typedef cell_block_func cell_block_modifier;
};

template<typename _ColT, typename _ValT>
bool test_cell_insertion(_ColT& col_db, typename _ColT::row_key_type row, _ValT val)
{
    _ValT test;
    col_db.set_cell(row, val);
    col_db.get_cell(row, test);
    return val == test;
}

typedef mdds::grid_map<grid_map_trait> grid_store_type;
typedef grid_store_type::sheet_type::column_type column_type;

void gridmap_test_basic()
{
    stack_printer __stack_printer__("::gridmap_test_basic");
    grid_store_type db;
    bool res;

    {
        // Single column instance with only one row.
        column_type col_db(1);

        double test = -999.0;

        // Empty cell has a numeric value of 0.0.
        col_db.get_cell(0, test);
        assert(test == 0.0);

        // Basic value setting and retrieval.
        res = test_cell_insertion(col_db, 0, 2.0);
        assert(res);
    }

    {
        // Insert first value into the top row.
        column_type col_db(2);
        double test = -999.0;

        // Test empty cell values.
        col_db.get_cell(0, test);
        assert(test == 0.0);
        test = 1.0;
        col_db.get_cell(1, test);
        assert(test == 0.0);

        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);

        col_db.get_cell(1, test);
        assert(test == 0.0); // should be empty.

        // Insert a new value to an empty row right below a non-empty one.
        res = test_cell_insertion(col_db, 1, 7.5);
        assert(res);
    }

    {
        column_type col_db(3);
        res = test_cell_insertion(col_db, 0, 4.5);
        assert(res);
        res = test_cell_insertion(col_db, 1, 5.1);
        assert(res);
        res = test_cell_insertion(col_db, 2, 34.2);
        assert(res);
    }

    {
        // Insert first value into the bottom row.
        column_type col_db(3);

        res = test_cell_insertion(col_db, 2, 5.0); // Insert into the last row.
        assert(res);

        double test = 9;
        col_db.get_cell(1, test);
        assert(test == 0.0); // should be empty.

        res = test_cell_insertion(col_db, 0, 2.5);
        assert(res);

        col_db.get_cell(1, test);
        assert(test == 0.0); // should be empty.

        res = test_cell_insertion(col_db, 1, 1.2);
        assert(res);
    }

    {
        // This time insert from bottom up one by one.
        column_type col_db(3);
        res = test_cell_insertion(col_db, 2, 1.2);
        assert(res);
        res = test_cell_insertion(col_db, 1, 0.2);
        assert(res);
        res = test_cell_insertion(col_db, 0, 23.1);
        assert(res);
    }

    {
        column_type col_db(4);
        long order[] = { 3, 1, 2, 0 };
        double val = 1.0;
        for (size_t i = 0; i < 4; ++i, ++val)
        {
            res = test_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        column_type col_db(4);
        long order[] = { 0, 3, 1, 2 };
        double val = 1.0;
        for (size_t i = 0; i < 4; ++i, ++val)
        {
            res = test_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        column_type col_db(4);
        long order[] = { 0, 2, 3, 1 };
        double val = 1.0;
        for (size_t i = 0; i < 4; ++i, ++val)
        {
            res = test_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        column_type col_db(5);
        long order[] = { 0, 4, 3, 2, 1 };
        double val = 1.0;
        for (size_t i = 0; i < 5; ++i, ++val)
        {
            res = test_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        // Insert first value into a middle row.
        column_type col_db(10);
        res = test_cell_insertion(col_db, 5, 5.0);
        assert(res);
        string str = "test";
        res = test_cell_insertion(col_db, 4, str);
        assert(res);
    }

    {
        column_type col_db(3);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        string str = "test";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
        res = test_cell_insertion(col_db, 1, 2.0);
        assert(res);
    }

    {
        column_type col_db(2);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        string str = "test";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
    }

    {
        column_type col_db(3);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        string str = "test";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
        str = "foo";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
    }

    {
        column_type col_db(3);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        res = test_cell_insertion(col_db, 2, 2.0);
        assert(res);
        string str = "foo";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
    }

    {
        column_type col_db(3);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        string str = "foo";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        str = "test";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
    }

    {
        column_type col_db(4);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        string str = "foo";
        res = test_cell_insertion(col_db, 3, str);
        assert(res);

        res = test_cell_insertion(col_db, 2, 2.0);
        assert(res);
        string test;
        col_db.get_cell(3, test); // Check the cell below.
        assert(test == "foo");

        res = test_cell_insertion(col_db, 1, -2.0);
        assert(res);
        test = "hmm";
        col_db.get_cell(3, test);
        assert(test == "foo");

        res = test_cell_insertion(col_db, 0, 7.5); // overwrite.
        assert(res);

        str = "bah";
        res = test_cell_insertion(col_db, 0, str); // overwrite with different type.
        assert(res);
        double val = -999;
        col_db.get_cell(1, val); // Check the cell below.
        assert(val == -2.0);

        str = "alpha";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        col_db.get_cell(2, val); // Check the cell below.
        assert(val == 2.0);

        col_db.get_cell(3, test);
        assert(test == "foo");

        str = "beta";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
    }

    {
        column_type col_db(1);
        res = test_cell_insertion(col_db, 0, 2.0);
        assert(res);
        string str = "foo";
        res = test_cell_insertion(col_db, 0, str);
        assert(res);
        res = test_cell_insertion(col_db, 0, 3.0);
        assert(res);
    }

    {
        column_type col_db(2);
        res = test_cell_insertion(col_db, 0, 2.0);
        assert(res);
        string str = "foo";
        res = test_cell_insertion(col_db, 0, str);
        assert(res);
        res = test_cell_insertion(col_db, 0, 3.0);
        assert(res);
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        str = "alpha";
        res = test_cell_insertion(col_db, 0, str);
        assert(res);
        string test;
        col_db.get_cell(1, test);
        assert(test == "foo");
    }

    {
        column_type col_db(3);
        string str = "alpha";
        col_db.set_cell(2, str);
        res = test_cell_insertion(col_db, 2, 5.0);
        assert(res);

        res = test_cell_insertion(col_db, 0, 1.0);
        assert(res);
        res = test_cell_insertion(col_db, 1, 2.0);
        assert(res);

        // At this point it contains one numeric block with 3 values.

        str = "beta";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
        res = test_cell_insertion(col_db, 2, 3.0);
        assert(res);
        double test;
        col_db.get_cell(0, test);
        assert(test == 1.0);
        col_db.get_cell(1, test);
        assert(test == 2.0);
        col_db.get_cell(2, test);
        assert(test == 3.0);
    }

    {
        column_type col_db(3);
        res = test_cell_insertion(col_db, 1, 5.0);
        assert(res);
        string str = "alpha";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        res = test_cell_insertion(col_db, 0, 4.0);
        assert(res);
        res = test_cell_insertion(col_db, 1, 3.0);
        assert(res);
        double test;
        col_db.get_cell(0, test);
        assert(test == 4.0);

        // The top 2 cells are numeric and the bottom cell is still empty.

        str = "beta";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        col_db.get_cell(0, test);
        assert(test == 4.0);

        res = test_cell_insertion(col_db, 1, 6.5);
        assert(res);
        col_db.get_cell(0, test);
        assert(test == 4.0);

        str = "gamma";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
        col_db.get_cell(0, test);
        assert(test == 4.0);
        col_db.get_cell(1, test);
        assert(test == 6.5);

        // The top 2 cells are numeric and the bottom cell is string.

        str = "delta";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
    }

    {
        column_type col_db(4);
        col_db.set_cell(0, 1.0);
        string str = "foo";
        col_db.set_cell(1, str);
        col_db.set_cell(2, str);
        col_db.set_cell(3, 4.0);

        res = test_cell_insertion(col_db, 2, 3.0);
        assert(res);
        double test;
        col_db.get_cell(3, test);
        assert(test == 4.0);
    }

    {
        column_type col_db(4);
        col_db.set_cell(0, 1.0);
        string str = "foo";
        col_db.set_cell(1, str);
        col_db.set_cell(2, str);
        col_db.set_cell(3, str);

        res = test_cell_insertion(col_db, 3, 3.0);
        assert(res);
    }

    {
        column_type col_db(4);
        col_db.set_cell(0, 1.0);
        string str = "foo";
        col_db.set_cell(1, str);
        col_db.set_cell(2, str);

        res = test_cell_insertion(col_db, 2, 3.0);
        assert(res);

        // Next cell should still be empty.
        double test_val;
        col_db.get_cell(3, test_val);
        assert(test_val == 0.0);
        string test_str;
        col_db.get_cell(3, test_str);
        assert(test_str.empty());
    }

    {
        column_type col_db(4);
        col_db.set_cell(0, 1.0);
        col_db.set_cell(1, 1.0);
        col_db.set_cell(2, 1.0);
        col_db.set_cell(3, 1.0);
        string str = "alpha";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
    }

    {
        column_type col_db(3);
        col_db.set_cell(0, 1.0);
        col_db.set_cell(1, 1.0);
        string str = "foo";
        col_db.set_cell(2, str);
        size_t index = 5;
        test_cell_insertion(col_db, 2, index);
    }

    {
        column_type col_db(3);
        col_db.set_cell(1, 1.0);
        string str = "foo";
        col_db.set_cell(2, str);
        str = "bah";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        res = test_cell_insertion(col_db, 1, 2.0);
        assert(res);
        size_t index = 2;
        res = test_cell_insertion(col_db, 1, index);
        assert(res);
        string test;
        col_db.get_cell(2, test);
        assert(test == "foo");
        str = "alpha";
        res = test_cell_insertion(col_db, 0, str);
        assert(res);
        double val = 3.5;
        res = test_cell_insertion(col_db, 1, val);
        assert(res);
        index = 3;
        res = test_cell_insertion(col_db, 2, index);
        assert(res);

        // At this point cells 1, 2, 3 all contain different data types.

        str = "beta";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);

        // Reset.
        val = 4.5;
        res = test_cell_insertion(col_db, 1, val);
        assert(res);

        index = 4;
        res = test_cell_insertion(col_db, 1, index);
        assert(res);
    }

    {
        column_type col_db(3);
        col_db.set_cell(0, 1.0);
        string str = "alpha";
        col_db.set_cell(1, str);
        str = "beta";
        col_db.set_cell(2, str);
        size_t index = 1;
        res = test_cell_insertion(col_db, 1, index);
        assert(res);
        string test;
        col_db.get_cell(2, test);
        assert(test == "beta");
    }

    {
        column_type col_db(3);

        // Insert 3 cells of 3 different types.
        res = test_cell_insertion(col_db, 0, true);
        assert(res);
        res = test_cell_insertion(col_db, 1, 1.2);
        assert(res);
        string str = "foo";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);

        // Now, insert a cell of the 4th type to the middle spot.
        size_t index = 2;
        res = test_cell_insertion(col_db, 1, index);
        assert(res);
    }
}

void gridmap_test_empty_cells()
{
    stack_printer __stack_printer__("::gridmap_test_empty");
    {
        column_type db(3);
        assert(db.is_empty(0));
        assert(db.is_empty(2));

        // These won't change the state of the container since it's already empty.
        db.set_empty(0, 0);
        db.set_empty(1, 1);
        db.set_empty(2, 2);
        db.set_empty(0, 2);

        db.set_cell(0, 1.0);
        db.set_cell(2, 5.0);
        assert(!db.is_empty(0));
        assert(!db.is_empty(2));
        assert(db.is_empty(1));

        db.set_cell(1, 2.3);
        assert(!db.is_empty(1));

        // Container contains a single block of numeric cells at this point.

        // Set the whole block empty.
        db.set_empty(0, 2);

        // Reset.
        db.set_cell(0, 1.0);
        db.set_cell(1, 2.0);
        db.set_cell(2, 4.0);

        // Set the upper part of the block empty.
        db.set_empty(0, 1);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(!db.is_empty(2));

        double test;
        db.get_cell(2, test);
        assert(test == 4.0);

        // Reset.
        db.set_cell(0, 5.0);
        db.set_cell(1, 5.1);
        db.set_cell(2, 5.2);

        // Set the lower part of the block empty.
        db.set_empty(1, 2);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));

        db.get_cell(0, test);
        assert(test == 5.0);

        // Reset.
        db.set_cell(0, 3.0);
        db.set_cell(1, 3.1);
        db.set_cell(2, 3.2);

        // Set the middle part of the block empty.
        db.set_empty(1, 1);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(!db.is_empty(2));
        db.get_cell(0, test);
        assert(test == 3.0);
        db.get_cell(2, test);
        assert(test == 3.2);

        bool res = test_cell_insertion(db, 1, 4.3);
        assert(res);
    }

    {
        // Empty multiple cells at the middle part of a block.
        column_type db(4);
        for (size_t i = 0; i < 4; ++i)
            db.set_cell(i, static_cast<double>(i+1));

        for (size_t i = 0; i < 4; ++i)
        {
            assert(!db.is_empty(i));
        }

        db.set_empty(1, 2);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(!db.is_empty(3));

        double test;
        db.get_cell(0, test);
        assert(test == 1.0);
        db.get_cell(3, test);
        assert(test == 4.0);
    }

    {
        // Empty multiple blocks.
        column_type db(2);
        db.set_cell(0, 1.0);
        db.set_cell(1, string("foo"));
        assert(!db.is_empty(0));
        assert(!db.is_empty(1));

        db.set_empty(0, 1);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
    }

    {
        // Empty multiple blocks, part 2 - from middle block to middle block.
        column_type db(6);
        db.set_cell(0, 1.0);
        db.set_cell(1, 2.0);
        string str = "foo";
        db.set_cell(2, str);
        db.set_cell(3, str);
        size_t index = 1;
        db.set_cell(4, index);
        index = 100;
        db.set_cell(5, index);

        db.set_empty(1, 4);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(!db.is_empty(5));
        double val;
        db.get_cell(0, val);
        assert(val == 1.0);
        size_t index_test;
        db.get_cell(5, index_test);
        assert(index_test == 100);
    }

    {
        // Empty multiple blocks, part 3 - from top block to middle block.
        column_type db(6);
        db.set_cell(0, 1.0);
        db.set_cell(1, 2.0);
        string str = "foo";
        db.set_cell(2, str);
        db.set_cell(3, str);
        size_t index = 1;
        db.set_cell(4, index);
        index = 50;
        db.set_cell(5, index);

        db.set_empty(0, 4);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(!db.is_empty(5));
        size_t test;
        db.get_cell(5, test);
        assert(test == 50);
    }

    {
        // Empty multiple blocks, part 4 - from middle block to bottom block.
        column_type db(6);
        db.set_cell(0, 1.0);
        db.set_cell(1, 2.0);
        string str = "foo";
        db.set_cell(2, str);
        db.set_cell(3, str);
        size_t index = 1;
        db.set_cell(4, index);
        db.set_cell(5, index);

        db.set_empty(1, 5);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(db.is_empty(5));
        double test;
        db.get_cell(0, test);
        assert(test == 1.0);
    }

    {
        // Empty multiple blocks, part 5 - from middle empty block to middle non-empty block.
        column_type db(6);
        db.set_cell(2, 1.0);
        db.set_cell(3, 2.0);
        string str = "foo";
        db.set_cell(4, str);
        str = "baa";
        db.set_cell(5, str);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(!db.is_empty(2));
        assert(!db.is_empty(3));
        assert(!db.is_empty(4));
        assert(!db.is_empty(5));
        assert(db.block_size() == 3);

        db.set_empty(1, 4);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(!db.is_empty(5));
        assert(db.block_size() == 2);
        string test;
        db.get_cell(5, test);
        assert(test == "baa");
    }

    {
        // Empty multiple blocks, part 6 - from middle non-empty block to middle empty block.
        column_type db(6);
        db.set_cell(0, 1.0);
        db.set_cell(1, 2.0);
        db.set_cell(2, string("foo"));
        db.set_cell(3, string("baa"));
        assert(!db.is_empty(0));
        assert(!db.is_empty(1));
        assert(!db.is_empty(2));
        assert(!db.is_empty(3));
        assert(db.is_empty(4));
        assert(db.is_empty(5));

        db.set_empty(1, 4);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(db.is_empty(5));
        double test;
        db.get_cell(0, test);
        assert(test == 1.0);
        assert(db.block_size() == 2);
    }

    {
        // Empty multiple blocks, part 7 - from middle empty block to middle empty block.
        column_type db(6);
        db.set_cell(2, 1.0);
        db.set_cell(3, string("foo"));
        assert(db.block_size() == 4);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(!db.is_empty(2));
        assert(!db.is_empty(3));
        assert(db.is_empty(4));
        assert(db.is_empty(5));

        // This should set the whole range empty.
        db.set_empty(1, 4);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(db.is_empty(5));
        assert(db.block_size() == 1);
    }
}

void gridmap_test_swap()
{
    stack_printer __stack_printer__("::gridmap_test_swap");
    column_type db1(3), db2(5);
    db1.set_cell(0, 1.0);
    db1.set_cell(1, 2.0);
    db1.set_cell(2, 3.0);

    db2.set_cell(0, 4.0);
    db2.set_cell(1, 5.0);
    db2.set_cell(4, string("foo"));
    db1.swap(db2);

    assert(db1.size() == 5 && db1.block_size() == 3);
    assert(db2.size() == 3 && db2.block_size() == 1);
}

void gridmap_test_equality()
{
    stack_printer __stack_printer__("::gridmap_test_equality");
    {
        // Two columns of equal size.
        column_type db1(3), db2(3);
        assert(db1 == db2);
        db1.set_cell(0, 1.0);
        assert(db1 != db2);
        db2.set_cell(0, 1.0);
        assert(db1 == db2);
        db2.set_cell(0, 1.2);
        assert(db1 != db2);
        db1.set_cell(0, 1.2);
        assert(db1 == db2);
    }

    {
        // Two columns of different sizes.  They are always non-equal no
        // matter what.
        column_type db1(3), db2(4);
        assert(db1 != db2);
        db1.set_cell(0, 1.2);
        db2.set_cell(0, 1.2);
        assert(db1 != db2);

        // Comparison to self.
        assert(db1 == db1);
        assert(db2 == db2);
    }
}

void gridmap_test_clone()
{
    stack_printer __stack_printer__("::gridmap_test_clone");
    column_type db1(3);
    db1.set_cell(0, 3.4);
    db1.set_cell(1, string("foo"));
    db1.set_cell(2, true);

    // copy construction

    column_type db2(db1);
    assert(db1.size() == db2.size());
    assert(db1.block_size() == db2.block_size());
    assert(db1 == db2);

    {
        double test1, test2;
        db1.get_cell(0, test1);
        db2.get_cell(0, test2);
        assert(test1 == test2);
    }
    {
        string test1, test2;
        db1.get_cell(1, test1);
        db2.get_cell(1, test2);
        assert(test1 == test2);
    }

    {
        bool test1, test2;
        db1.get_cell(2, test1);
        db2.get_cell(2, test2);
        assert(test1 == test2);
    }

    // assignment

    column_type db3 = db1;
    assert(db3 == db1);
    db3.set_cell(0, string("alpha"));
    assert(db3 != db1);

    column_type db4, db5;
    db4 = db5 = db3;
    assert(db4 == db5);
    assert(db3 == db5);
    assert(db3 == db4);
}

void gridmap_test_resize()
{
    stack_printer __stack_printer__("::gridmap_test_resize");
    column_type db(0);
    assert(db.size() == 0);
    assert(db.empty());

    // Resize to create initial empty block.
    db.resize(3);
    assert(db.size() == 3);
    assert(db.block_size() == 1);

    // Resize to increase the existing empty block.
    db.resize(5);
    assert(db.size() == 5);
    assert(db.block_size() == 1);

    for (long row = 0; row < 5; ++row)
        db.set_cell(row, static_cast<double>(row));

    assert(db.size() == 5);
    assert(db.block_size() == 1);

    // Increase its size by one.  This should append an empty cell block of size one.
    db.resize(6);
    assert(db.size() == 6);
    assert(db.block_size() == 2);
    assert(db.is_empty(5));

    // Do it again.
    db.resize(7);
    assert(db.size() == 7);
    assert(db.block_size() == 2);

    // Now, reduce its size to eliminate the last empty block.
    db.resize(5);
    assert(db.size() == 5);
    assert(db.block_size() == 1);

    // Reset.
    db.resize(7);
    assert(db.size() == 7);
    assert(db.block_size() == 2);

    // Now, resize across multiple blocks.
    db.resize(4);
    assert(db.size() == 4);
    assert(db.block_size() == 1);
    double test;
    db.get_cell(3, test);
    assert(test == 3.0);

    // Empty it.
    db.resize(0);
    assert(db.size() == 0);
    assert(db.block_size() == 0);
    assert(db.empty());
}

void gridmap_test_erase()
{
    stack_printer __stack_printer__("::gridmap_test_erase");
    {
        // Single empty block.
        column_type db(5);
        db.erase(0, 2); // erase rows 0-2.
        assert(db.size() == 2);
        db.erase(0, 1);
        assert(db.size() == 0);
        assert(db.empty());
    }

    {
        // Single non-empty block.
        column_type db(5);
        for (long i = 0; i < 5; ++i)
            db.set_cell(i, static_cast<double>(i+1));

        assert(db.block_size() == 1);
        assert(db.size() == 5);

        db.erase(0, 2); // erase rows 0-2
        assert(db.size() == 2);
        double test;
        db.get_cell(0, test);
        assert(test == 4.0);
        db.get_cell(1, test);
        assert(test == 5.0);

        db.erase(0, 1);
        assert(db.size() == 0);
        assert(db.empty());
    }

    {
        // Two blocks - non-empty to empty blocks.
        column_type db(8);
        for (long i = 0; i < 4; ++i)
            db.set_cell(i, static_cast<double>(i+1));

        assert(db.block_size() == 2);
        assert(db.size() == 8);
        assert(!db.is_empty(3));
        assert(db.is_empty(4));

        // Erase across two blocks.
        db.erase(3, 6); // 4 cells
        assert(db.block_size() == 2);
        assert(db.size() == 4);

        // Check the integrity of the data.
        double test;
        db.get_cell(2, test);
        assert(test == 3.0);
        assert(db.is_empty(3));

        // Empty it.
        db.erase(0, 3);
        assert(db.block_size() == 0);
        assert(db.size() == 0);
        assert(db.empty());
    }

    {
        // Two blocks - non-empty to non-empty blocks.
        column_type db(8);
        for (long i = 0; i < 4; ++i)
            db.set_cell(i, static_cast<double>(i+1));

        for (long i = 4; i < 8; ++i)
            db.set_cell(i, static_cast<size_t>(i+1));

        assert(db.block_size() == 2);
        assert(db.size() == 8);

        // Erase across two blocks.
        db.erase(3, 6); // 4 cells
        assert(db.block_size() == 2);
        assert(db.size() == 4);

        // Check the integrity of the data.
        double test;
        db.get_cell(2, test);
        assert(test == 3.0);

        size_t test2;
        db.get_cell(3, test2);
        assert(test2 == 8);

        // Empty it.
        db.erase(0, 3);
        assert(db.block_size() == 0);
        assert(db.size() == 0);
        assert(db.empty());
    }

    {
        // 3 blocks, all non-empty.
        column_type db(9);
        for (long i = 0; i < 3; ++i)
            db.set_cell(i, static_cast<double>(i+1));

        for (long i = 3; i < 6; ++i)
            db.set_cell(i, static_cast<size_t>(i+1));

        for (long i = 6; i < 9; ++i)
        {
            ostringstream os;
            os << i+1;
            db.set_cell(i, os.str());
        }

        assert(db.block_size() == 3);
        assert(db.size() == 9);

        db.erase(2, 7);
        assert(db.block_size() == 2);
        assert(db.size() == 3);

        // Check the integrity of the data.
        double test1;
        db.get_cell(1, test1);
        assert(test1 == 2.0);
        string test2;
        db.get_cell(2, test2);
        assert(test2 == "9");

        db.erase(2, 2); // Erase only one-block.
        assert(db.block_size() == 1);
        assert(db.size() == 2);
        test1 = -1.0;
        db.get_cell(1, test1);
        assert(test1 == 2.0);

        db.erase(0, 1);
        assert(db.size() == 0);
        assert(db.empty());
    }
}

void gridmap_test_insert_empty()
{
    stack_printer __stack_printer__("::gridmap_test_insert_empty");
    {
        column_type db(5);
        db.insert_empty(0, 5);
        assert(db.size() == 10);
        assert(db.block_size() == 1);

        // Insert data from row 0 to 4.
        for (long i = 0; i < 5; ++i)
            db.set_cell(i, static_cast<double>(i+1));

        assert(db.block_size() == 2);
        assert(db.size() == 10);

        // Now, insert an empty block of size 2 at the top.
        db.insert_empty(0, 2);
        assert(db.block_size() == 3);
        assert(db.size() == 12);

        double test;
        db.get_cell(2, test);
        assert(test == 1.0);

        // Insert an empty cell into an empty block.  This should shift the
        // data block down by one.
        db.insert_empty(1, 1);
        assert(db.block_size() == 3);
        assert(db.size() == 13);
        db.get_cell(4, test);
        assert(test == 2.0);
    }

    {
        column_type db(5);
        for (long i = 0; i < 5; ++i)
            db.set_cell(i, static_cast<double>(i+1));

        assert(db.block_size() == 1);
        assert(db.size() == 5);

        // Insert an empty block into the middle of a non-empty block.
        db.insert_empty(2, 2);

        assert(db.block_size() == 3);
        assert(db.size() == 7);
        assert(db.is_empty(2));
        assert(db.is_empty(3));

        double test;
        db.get_cell(0, test);
        assert(test == 1.0);
        db.get_cell(1, test);
        assert(test == 2.0);

        db.get_cell(4, test);
        assert(test == 3.0);
        db.get_cell(5, test);
        assert(test == 4.0);
        db.get_cell(6, test);
        assert(test == 5.0);
    }

    {
        column_type db(1);
        db.set_cell(0, 2.5);
        db.insert_empty(0, 2);
        assert(db.block_size() == 2);
        assert(db.size() == 3);
        assert(db.is_empty(1));
        assert(!db.is_empty(2));

        double test;
        db.get_cell(2, test);
        assert(test == 2.5);
    }

    {
        column_type db(2);
        db.set_cell(0, 1.2);
        db.set_cell(1, 2.3);
        db.insert_empty(1, 1);

        assert(db.block_size() == 3);
        assert(db.size() == 3);
        double test;
        db.get_cell(0, test);
        assert(test == 1.2);
        db.get_cell(2, test);
        assert(test == 2.3);
    }
}

void gridmap_test_set_cells()
{
    stack_printer __stack_printer__("::gridmap_test_set_cells");
    {
        column_type db(5);

        // Replace the whole block.

        {
            vector<double> vals;
            vals.reserve(5);
            for (size_t i = 0; i < db.size(); ++i)
                vals.push_back(i+1);

            db.set_cells(0, vals.begin(), vals.end());

            double test;
            db.get_cell(0, test);
            assert(test == 1.0);
            db.get_cell(4, test);
            assert(test == 5.0);
        }

        {
            vector<string> vals;
            vals.reserve(5);
            for (size_t i = 0; i < db.size(); ++i)
            {
                ostringstream os;
                os << (i+1);
                vals.push_back(os.str());
            }

            db.set_cells(0, vals.begin(), vals.end());

            string test;
            db.get_cell(0, test);
            assert(test == "1");
            db.get_cell(4, test);
            assert(test == "5");
        }

        {
            double vals[] = { 5.0, 6.0, 7.0, 8.0, 9.0 };
            double* p = &vals[0];
            double* p_end = p + 5;
            db.set_cells(0, p, p_end);
            double test;
            db.get_cell(0, test);
            assert(test == 5.0);
            db.get_cell(4, test);
            assert(test == 9.0);
        }

        {
            // Replace the whole block of the same type, which shouldn't
            // delete the old data array.
            double vals[] = { 5.1, 6.1, 7.1, 8.1, 9.1 };
            double* p = &vals[0];
            double* p_end = p + 5;
            db.set_cells(0, p, p_end);
            double test;
            db.get_cell(0, test);
            assert(test == 5.1);
            db.get_cell(4, test);
            assert(test == 9.1);

            double vals2[] = { 8.2, 9.2 };
            p = &vals2[0];
            p_end = p + 2;
            db.set_cells(3, p, p_end);
            db.get_cell(3, test);
            assert(test == 8.2);
            db.get_cell(4, test);
            assert(test == 9.2);
        }


        {
            // Replace the upper part of a single block.
            size_t vals[] = { 1, 2, 3 };
            size_t* p = &vals[0];
            size_t* p_end = p + 3;
            db.set_cells(0, p, p_end);
            assert(db.block_size() == 2);
            assert(db.size() == 5);
            size_t test;
            db.get_cell(0, test);
            assert(test == 1);
            db.get_cell(2, test);
            assert(test == 3);
            double test2;
            db.get_cell(3, test2);
            assert(test2 == 8.2);
        }

        {
            // Merge with the previos block and erase the whole block.
            size_t vals[] = { 4, 5 };
            size_t* p = &vals[0];
            size_t* p_end = p + 2;
            db.set_cells(3, p, p_end);
            assert(db.block_size() == 1);
            assert(db.size() == 5);
            size_t test;
            db.get_cell(2, test);
            assert(test == 3);
            db.get_cell(3, test);
            assert(test == 4);
        }

        {
            // Merge with the previous block while keeping the lower part of
            // the block.
            size_t prev_value;
            db.get_cell(2, prev_value);

            double val = 2.3;
            db.set_cell(0, val);
            assert(db.block_size() == 2);
            val = 4.5;
            double* p = &val;
            double* p_end = p + 1;
            db.set_cells(1, p, p_end);
            assert(db.block_size() == 2);
            assert(db.size() == 5);
            {
                double test;
                db.get_cell(0, test);
                assert(test == 2.3);
                db.get_cell(1, test);
                assert(test == 4.5);
            }

            size_t test;
            db.get_cell(2, test);
            assert(test == prev_value);
        }
    }

    {
        column_type db(5);
        for (size_t i = 0; i < 5; ++i)
            db.set_cell(i, static_cast<double>(i+1));

        assert(db.block_size() == 1);
        assert(db.size() == 5);

        {
            size_t vals[] = { 10, 11 };
            size_t* p = &vals[0];
            size_t* p_end = p + 2;
            db.set_cells(3, p, p_end);

            assert(db.block_size() == 2);
            assert(db.size() == 5);

            double test;
            db.get_cell(2, test);
            assert(test == 3.0);
            size_t test2;
            db.get_cell(3, test2);
            assert(test2 == 10);
            db.get_cell(4, test2);
            assert(test2 == 11);

            // Insertion into a single block but this time it needs to be
            // merged with the subsequent block.
            db.set_cells(1, p, p_end);

            assert(db.block_size() == 2);
            assert(db.size() == 5);

            db.get_cell(1, test2);
            assert(test2 == 10);
            db.get_cell(2, test2);
            assert(test2 == 11);
            db.get_cell(3, test2);
            assert(test2 == 10);
            db.get_cell(4, test2);
            assert(test2 == 11);
        }
    }

    {
        column_type db(6);
        double vals_d[] = { 1.0, 1.1, 1.2, 1.3, 1.4, 1.5 };
        size_t vals_i[] = { 12, 13, 14, 15 };
        string vals_s[] = { "a", "b" };

        {
            double* p = &vals_d[0];
            double* p_end = p + 6;
            db.set_cells(0, p, p_end);
            assert(db.block_size() == 1);
            assert(db.size() == 6);
            double test;
            db.get_cell(0, test);
            assert(test == 1.0);
            db.get_cell(5, test);
            assert(test == 1.5);
        }

        {
            size_t* p = &vals_i[0];
            size_t* p_end = p + 4;
            db.set_cells(0, p, p_end);
            assert(db.block_size() == 2);
            size_t test;
            db.get_cell(0, test);
            assert(test == 12);
            db.get_cell(3, test);
            assert(test == 15);
        }

        {
            string* p = &vals_s[0];
            string* p_end = p + 2;
            db.set_cells(2, p, p_end);
            assert(db.block_size() == 3);
            string test;
            db.get_cell(2, test);
            assert(test == "a");
            db.get_cell(3, test);
            assert(test == "b");
            double test_d;
            db.get_cell(4, test_d);
            assert(test_d == 1.4);
            size_t test_i;
            db.get_cell(1, test_i);
            assert(test_i == 13);
        }
    }

    {
        column_type db(3);
        {
            double vals[] = { 2.1, 2.2, 2.3 };
            double* p = &vals[0];
            double* p_end = p + 3;
            db.set_cells(0, p, p_end);
            assert(db.block_size() == 1);
        }

        {
            size_t val_i = 23;
            size_t* p = &val_i;
            size_t* p_end = p + 1;
            db.set_cells(1, p, p_end);
            assert(db.block_size() == 3);
            size_t test;
            db.get_cell(1, test);
            assert(test == 23);
            double test_d;
            db.get_cell(0, test_d);
            assert(test_d == 2.1);
            db.get_cell(2, test_d);
            assert(test_d == 2.3);
        }
    }

    {
        // Set cells over multiple blocks. Very simple case.

        column_type db(2);
        db.set_cell(0, static_cast<double>(1.1));
        db.set_cell(1, string("foo"));
        assert(db.block_size() == 2);

        double vals[] = { 2.1, 2.2 };
        double* p = &vals[0];
        double* p_end = p + 2;
        db.set_cells(0, p, p_end);
        assert(db.block_size() == 1);
        assert(db.size() == 2);

        double test;
        db.get_cell(0, test);
        assert(test == 2.1);
        db.get_cell(1, test);
        assert(test == 2.2);
    }

    {
        // Same as above, except that the last block is only partially replaced.

        column_type db(3);
        db.set_cell(0, static_cast<double>(1.1));
        db.set_cell(1, string("foo"));
        db.set_cell(2, string("baa"));

        double vals[] = { 2.1, 2.2 };
        double* p = &vals[0];
        double* p_end = p + 2;
        db.set_cells(0, p, p_end);
        assert(db.block_size() == 2);
        assert(db.size() == 3);

        double test_val;
        db.get_cell(0, test_val);
        assert(test_val == 2.1);
        db.get_cell(1, test_val);
        assert(test_val == 2.2);

        string test_s;
        db.get_cell(2, test_s);
        assert(test_s == "baa");
    }
}

}

int main (int argc, char **argv)
{
    cmd_options opt;
    if (!parse_cmd_options(argc, argv, opt))
        return EXIT_FAILURE;

    if (opt.test_func)
    {
        gridmap_test_basic();
        gridmap_test_empty_cells();
        gridmap_test_swap();
        gridmap_test_equality();
        gridmap_test_clone();
        gridmap_test_resize();
        gridmap_test_erase();
        gridmap_test_insert_empty();
        gridmap_test_set_cells();
    }

    if (opt.test_perf)
    {
    }

    return EXIT_SUCCESS;
}
