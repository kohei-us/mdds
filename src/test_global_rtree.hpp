/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2018 Kohei Yoshida
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

#ifndef INCLUDED_MDDS_TEST_GLOBAL_RTREE_HPP
#define INCLUDED_MDDS_TEST_GLOBAL_RTREE_HPP

#define MDDS_RTREE_DEBUG 1
#include <mdds/rtree.hpp>
#include <fstream>

struct tiny_trait_1d
{
    constexpr static size_t dimensions = 1;
    constexpr static size_t min_node_size = 2;
    constexpr static size_t max_node_size = 5;
    constexpr static size_t max_tree_depth = 100;

    constexpr static bool enable_forced_reinsertion = false;
    constexpr static size_t reinsertion_size = 2;
};

struct tiny_trait_2d
{
    constexpr static size_t dimensions = 2;
    constexpr static size_t min_node_size = 2;
    constexpr static size_t max_node_size = 5;
    constexpr static size_t max_tree_depth = 100;

    constexpr static bool enable_forced_reinsertion = false;
    constexpr static size_t reinsertion_size = 2;
};

struct tiny_trait_2d_forced_reinsertion
{
    constexpr static size_t dimensions = 2;
    constexpr static size_t min_node_size = 2;
    constexpr static size_t max_node_size = 5;
    constexpr static size_t max_tree_depth = 100;

    constexpr static bool enable_forced_reinsertion = true;
    constexpr static size_t reinsertion_size = 2;
};

class only_movable
{
    double m_value;
public:
    only_movable() : m_value(0.0) {}
    only_movable(double v) : m_value(v) {}
    only_movable(const only_movable&) = delete;
    only_movable(only_movable&& other) : m_value(other.m_value)
    {
        other.m_value = 0.0;
    }

    double get() const { return m_value; }
};

class only_copyable
{
    double m_value;
public:
    only_copyable() : m_value(0.0) {}
    only_copyable(double v) : m_value(v) {}
    only_copyable(const only_copyable& other) : m_value(other.m_value) {}

    only_copyable(only_copyable&&) = delete;

    void set(double v) { m_value = v; }
    double get() const { return m_value; }
};

template<typename T>
void export_tree(const T& tree, const std::string& basename)
{
    {
        std::ofstream fout(basename + ".obj");
        fout << tree.export_tree(T::export_tree_type::extent_as_obj);
    }

    {
        std::ofstream fout(basename + ".svg");
        fout << tree.export_tree(T::export_tree_type::extent_as_svg);
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

