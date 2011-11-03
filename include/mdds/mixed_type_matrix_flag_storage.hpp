/*************************************************************************
 *
 * Copyright (c) 2010 Kohei Yoshida
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

#if UNIT_TEST
#include <iostream>
#endif

#include "mdds/hash_container/map.hpp"

namespace mdds { namespace __mtm {

template<typename _Flag, typename _Key, typename _KeyHash>
class flag_storage
{
    typedef _Flag       flag_type;
    typedef _Key        key_type;
    typedef _KeyHash    key_hash_type;

    typedef _mdds_unordered_map_type<key_type, flag_type, key_hash_type> flag_store_type;

public:
    flag_storage() {}
    flag_storage(const flag_storage& r) : m_flags(r.m_flags) {}

    void set_flag(size_t row, size_t col, flag_type flag)
    {
        key_type pos = key_type(row, col);
        typename flag_store_type::iterator itr = m_flags.find(pos);
        if (itr == m_flags.end())
        {
            // flag not stored for this position.
            m_flags.insert(typename flag_store_type::value_type(pos, flag));
            return;
        }
        itr->second = flag;
    }

    flag_type get_flag(size_t row, size_t col)
    {
        key_type pos = key_type(row, col);
        typename flag_store_type::iterator itr = m_flags.find(pos);
        return itr == m_flags.end() ? static_cast<flag_type>(0) : itr->second;
    }

    void clear_flag(size_t row, size_t col)
    {
        key_type pos = key_type(row, col);
        typename flag_store_type::iterator itr = m_flags.find(pos);
        if (itr != m_flags.end())
            // Flag is stored at this position.  Remove it.
            m_flags.erase(itr);
    }
#if UNIT_TEST
    void dump() const
    {
        using namespace std;
        if (m_flags.empty())
        {
            cout << "no flags stored" << endl;
            return;
        }

        cout << "flags stored:" << endl;
        typename flag_store_type::const_iterator itr = m_flags.begin(), itr_end = m_flags.end();
        for (; itr != itr_end; ++itr)
        {
            const key_type& pos = itr->first;
            flag_type val = itr->second;
            cout << "(row=" << pos.first << ",col=" << pos.second << ") = 0x" << hex << static_cast<size_t>(val) << endl;
        }
    }
#endif
private:
    flag_store_type m_flags;
};

}}
