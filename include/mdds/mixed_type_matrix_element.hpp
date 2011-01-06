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

#ifndef __MDDS_MIXED_TYPE_MATRIX_ELEMENT_HPP__
#define __MDDS_MIXED_TYPE_MATRIX_ELEMENT_HPP__

namespace mdds {

enum matrix_element_t
{ 
    element_empty   = 0, 
    element_numeric = 1, 
    element_boolean = 2, 
    element_string  = 3 
};

template<typename _String>
struct element
{
    typedef _String     string_type;

    matrix_element_t m_type

    union
    {
        double       m_numeric;
        bool         m_boolean;
        string_type* mp_string;
    };

    element() : m_type(element_empty) {}
    element(const element& r) : m_type(r.m_type)
    {
        switch (m_type)
        {
            case element_boolean:
                m_boolean = r.m_boolean;
                break;
            case element_numeric:
                m_numeric = r.m_numeric;
                break;
            case element_string:
                mp_string = new string_type(*r.mp_string);
                break;
            case element_empty:
            default:
                ;
        }
    }

    explicit element(double v) : m_type(element_numeric), m_numeric(v) {}
    explicit element(bool v) : m_type(element_boolean), m_boolean(v) {}
    explicit element(string_type* p) : m_type(element_string), mp_string(p) {}

    bool operator== (const element& r) const
    {
        if (m_type != r.m_type)
            return false;

        switch (m_type)
        {
            case element_boolean:
                return m_boolean == r.m_boolean;
            case element_numeric:
                return m_numeric == r.m_numeric;
            case element_string:
                return *mp_string == *r.mp_string;
            case element_empty:
            default:
                ;
        }

        return true;
    }

    element& operator= (const element& r)
    {
        if (m_type == element_string)
            delete mp_string;

        m_type = r.m_type;

        switch (m_type)
        {
            case element_boolean:
                m_boolean = r.m_boolean;
                break;
            case element_numeric:
                m_numeric = r.m_numeric;
                break;
            case element_string:
                mp_string = new string_type(*r.mp_string);
                break;
            case element_empty:
            default:
                ;
        }   
        return *this;
    }

    ~element()
    {
        clear_string();
    }

    void clear_string()
    {
        if (m_type == element_string)
            delete mp_string;
    }

    void set_empty()
    {
        clear_string();
        m_type = element_empty;
    }

    void set_numeric(double val)
    {
        clear_string();
        m_type = element_numeric;
        m_numeric = val;
    }

    void set_boolean(bool val)
    {
        clear_string();
        m_type = element_boolean;
        m_boolean = val;
    }

    void set_string(string_type* str)
    {
        clear_string();
        m_type = element_string;
        mp_string = str;
    }
};

}

#endif
