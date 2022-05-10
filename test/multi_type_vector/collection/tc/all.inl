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

void mtv_test_empty()
{
    stack_printer __stack_printer__(__FUNCTION__);
    cols_type empty;
    assert(empty.begin() == empty.end());
}

void mtv_test_pointer_size1()
{
    stack_printer __stack_printer__(__FUNCTION__);

    // Two vectors of size 1, both of which are totally empty.

    std::vector<mtv_type*> vectors;
    for (size_t i = 0; i < 2; ++i)
        vectors.push_back(new mtv_type(1));

    cols_type collection(vectors.begin(), vectors.end());
    cols_type::const_iterator it, ite;
    it = collection.begin();
    ite = collection.end();
    assert(it->type == mdds::mtv::element_type_empty);
    assert((it++)->index == 0);
    assert(it != ite);

    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->index == 1);

    assert(++it == ite);

    for_each(vectors.begin(), vectors.end(), [](const mtv_type* p) { delete p; });
}

void mtv_test_unique_pointer_size1()
{
    stack_printer __stack_printer__(__FUNCTION__);

    // Two vector of size 1, with empty and numeric values.

    std::vector<std::unique_ptr<mtv_type>> vectors;
    for (size_t i = 0; i < 2; ++i)
        vectors.push_back(std::make_unique<mtv_type>(1));

    vectors[1]->set(0, 1.1);

    cols_type collection(vectors.begin(), vectors.end());
    cols_type::const_iterator it = collection.begin(), ite = collection.end();
    assert((*it).type == mdds::mtv::element_type_empty);
    assert((*it).index == 0);

    ++it;
    assert((*it).type == mdds::mtv::element_type_double);
    assert((*it).index == 1);
    assert(it->get<mdds::mtv::double_element_block>() == 1.1);

    assert(++it == ite);
}

void mtv_test_shared_pointer_size2()
{
    stack_printer __stack_printer__(__FUNCTION__);

    std::vector<std::shared_ptr<mtv_type>> vectors;
    vectors.push_back(std::make_shared<mtv_type>(2, 2.3));
    vectors.push_back(std::make_shared<mtv_type>(2, std::string("test")));

    cols_type collection(vectors.begin(), vectors.end());
    assert(collection.size() == 2);

    cols_type::const_iterator it = collection.begin();

    assert(it->type == mdds::mtv::element_type_double);
    assert(it->index == 0);
    assert(it->position == 0);
    assert(it->get<mdds::mtv::double_element_block>() == 2.3);

    ++it;
    assert(it->type == mdds::mtv::element_type_string);
    assert(it->index == 1);
    assert(it->position == 0);
    assert(it->get<mdds::mtv::string_element_block>() == "test");

    ++it;
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->index == 0);
    assert(it->position == 1);
    assert(it->get<mdds::mtv::double_element_block>() == 2.3);

    ++it;
    assert(it->type == mdds::mtv::element_type_string);
    assert(it->index == 1);
    assert(it->position == 1);
    assert(it->get<mdds::mtv::string_element_block>() == "test");

    assert(++it == collection.end());
}

void mtv_test_non_pointer_size1()
{
    stack_printer __stack_printer__(__FUNCTION__);

    // Test 1 by 1 grid.

    std::vector<mtv_type> vectors;
    vectors.reserve(1);
    vectors.emplace_back(1, int8_t('c'));

    cols_type collection(vectors.begin(), vectors.end());
    assert(collection.size() == 1);

    auto it = collection.begin();
    assert(it->type == mdds::mtv::element_type_int8);
    assert(it->index == 0);
    assert(it->position == 0);
    assert(it->get<mdds::mtv::int8_element_block>() == 'c');

    assert(++it == collection.end());
}

void mtv_test_invalid_collection()
{
    stack_printer __stack_printer__(__FUNCTION__);

    std::vector<mtv_type> vectors;
    vectors.reserve(2);
    vectors.emplace_back(1, int8_t('c'));
    vectors.emplace_back(2);

    try
    {
        // Grouping vectors of different lengths is not allowed.
        cols_type collection(vectors.begin(), vectors.end());
        assert(!"invalid_arg_error is expected to be thrown");
    }
    catch (const mdds::invalid_arg_error&)
    {}

    vectors.clear();
    vectors.emplace_back(0);
    vectors.emplace_back(0);

    try
    {
        // Grouping of empty vectors is not allowed.
        cols_type collection(vectors.begin(), vectors.end());
        assert(false);
    }
    catch (const mdds::invalid_arg_error&)
    {
        // Good.
    }
}

void mtv_test_sub_element_ranges()
{
    stack_printer __stack_printer__(__FUNCTION__);

    std::deque<mtv_type> vectors;
    vectors.emplace_back(0);
    vectors.emplace_back(0);
    vectors.emplace_back(0);

    mtv_type* p = &vectors[0];
    p->push_back<int>(0);
    p->push_back<int>(1);
    p->push_back<int>(2);

    p = &vectors[1];
    p->push_back<int>(3);
    p->push_back<int>(4);
    p->push_back<int>(5);

    p = &vectors[2];
    p->push_back<int>(6);
    p->push_back<int>(7);
    p->push_back<int>(8);

    // +---+---+---+
    // | 0 | 3 | 6 |
    // | 1 | 4 | 7 |
    // | 2 | 5 | 8 |
    // +---+---+---+

    cols_type collection(vectors.begin(), vectors.end());

    // Limit the element range.

    collection.set_element_range(1, 2);

    cols_type::const_iterator it = collection.begin();
    assert(it->type == mdds::mtv::element_type_int32);
    assert(it->get<mdds::mtv::int32_element_block>() == 1);
    assert(it->index == 0);
    assert(it->position == 1);

    ++it;
    assert(it->type == mdds::mtv::element_type_int32);
    assert(it->get<mdds::mtv::int32_element_block>() == 4);
    assert(it->index == 1);
    assert(it->position == 1);

    ++it;
    assert(it->type == mdds::mtv::element_type_int32);
    assert(it->get<mdds::mtv::int32_element_block>() == 7);
    assert(it->index == 2);
    assert(it->position == 1);

    ++it;
    assert(it->type == mdds::mtv::element_type_int32);
    assert(it->get<mdds::mtv::int32_element_block>() == 2);
    assert(it->index == 0);
    assert(it->position == 2);

    ++it;
    assert(it->type == mdds::mtv::element_type_int32);
    assert(it->get<mdds::mtv::int32_element_block>() == 5);
    assert(it->index == 1);
    assert(it->position == 2);

    ++it;
    assert(it->type == mdds::mtv::element_type_int32);
    assert(it->get<mdds::mtv::int32_element_block>() == 8);
    assert(it->index == 2);
    assert(it->position == 2);

    assert(++it == collection.end());

    // Limit the collection range.
    collection.set_collection_range(1, 1);
    it = collection.begin();
    assert(it->type == mdds::mtv::element_type_int32);
    assert(it->get<mdds::mtv::int32_element_block>() == 4);
    assert(it->index == 1);
    assert(it->position == 1);

    ++it;
    assert(it->type == mdds::mtv::element_type_int32);
    assert(it->get<mdds::mtv::int32_element_block>() == 5);
    assert(it->index == 1);
    assert(it->position == 2);

    assert(++it == collection.end());

    // Swap and try again.
    cols_type swapped;
    collection.swap(swapped);
    it = swapped.begin();
    assert(it->type == mdds::mtv::element_type_int32);
    assert(it->get<mdds::mtv::int32_element_block>() == 4);
    assert(it->index == 1);
    assert(it->position == 1);

    ++it;
    assert(it->type == mdds::mtv::element_type_int32);
    assert(it->get<mdds::mtv::int32_element_block>() == 5);
    assert(it->index == 1);
    assert(it->position == 2);

    assert(++it == swapped.end());
}

void mtv_test_sub_element_ranges_invalid()
{
    stack_printer __stack_printer__(__FUNCTION__);

    std::deque<mtv_type> vectors;
    vectors.emplace_back(5);
    vectors.emplace_back(5);
    vectors.emplace_back(5);

    cols_type collection(vectors.begin(), vectors.end());
    try
    {
        // Empty size.
        collection.set_element_range(0, 0);
        assert(!"invalid_arg_error is expected to be thrown");
    }
    catch (const mdds::invalid_arg_error&)
    {}

    try
    {
        // out-of-range start position.
        collection.set_element_range(5, 1);
        assert(!"invalid_arg_error is expected to be thrown");
    }
    catch (const mdds::invalid_arg_error&)
    {}

    try
    {
        // out-of-range end position.
        collection.set_element_range(0, 6);
        assert(!"invalid_arg_error is expected to be thrown");
    }
    catch (const mdds::invalid_arg_error&)
    {}
}

void mtv_test_sub_collection_ranges_invalid()
{
    stack_printer __stack_printer__(__FUNCTION__);

    std::deque<mtv_type> vectors;
    vectors.emplace_back(1);
    vectors.emplace_back(1);
    vectors.emplace_back(1);
    vectors.emplace_back(1);
    vectors.emplace_back(1);

    cols_type collection(vectors.begin(), vectors.end());
    try
    {
        // Empty size.
        collection.set_collection_range(0, 0);
        assert(!"invalid_arg_error is expected to be thrown");
    }
    catch (const mdds::invalid_arg_error&)
    {}

    try
    {
        // out-of-range start position.
        collection.set_collection_range(5, 1);
        assert(!"invalid_arg_error is expected to be thrown");
    }
    catch (const mdds::invalid_arg_error&)
    {}

    try
    {
        // out-of-range end position.
        collection.set_collection_range(0, 6);
        assert(!"invalid_arg_error is expected to be thrown");
    }
    catch (const mdds::invalid_arg_error&)
    {}
}

void mtv_test_boolean_block()
{
    stack_printer __stack_printer__(__FUNCTION__);

    std::vector<mtv_type> vectors;
    vectors.reserve(2);
    vectors.emplace_back(1, true);
    vectors.emplace_back(1, false);

    cols_type collection(vectors.begin(), vectors.end());

    auto it = collection.begin();
    assert(it->type == mdds::mtv::element_type_boolean);
    assert(it->index == 0);
    assert(it->position == 0);
    assert(it->get<mdds::mtv::boolean_element_block>() == true);

    ++it;
    assert(it->type == mdds::mtv::element_type_boolean);
    assert(it->index == 1);
    assert(it->position == 0);
    assert(it->get<mdds::mtv::boolean_element_block>() == false);

    assert(++it == collection.end());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
