/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

void mtv_test_empty()
{
    MDDS_TEST_FUNC_SCOPE;

    cols_type empty;
    TEST_ASSERT(empty.begin() == empty.end());
}

void mtv_test_pointer_size1()
{
    MDDS_TEST_FUNC_SCOPE;

    // Two vectors of size 1, both of which are totally empty.

    std::vector<mtv_type*> vectors;
    for (size_t i = 0; i < 2; ++i)
        vectors.push_back(new mtv_type(1));

    cols_type collection(vectors.begin(), vectors.end());
    cols_type::const_iterator it, ite;
    it = collection.begin();
    ite = collection.end();
    TEST_ASSERT(it->type == mdds::mtv::element_type_empty);
    TEST_ASSERT((it++)->index == 0);
    TEST_ASSERT(it != ite);

    TEST_ASSERT(it->type == mdds::mtv::element_type_empty);
    TEST_ASSERT(it->index == 1);

    TEST_ASSERT(++it == ite);

    for_each(vectors.begin(), vectors.end(), [](const mtv_type* p) { delete p; });
}

void mtv_test_unique_pointer_size1()
{
    MDDS_TEST_FUNC_SCOPE;

    // Two vector of size 1, with empty and numeric values.

    std::vector<std::unique_ptr<mtv_type>> vectors;
    for (size_t i = 0; i < 2; ++i)
        vectors.push_back(std::make_unique<mtv_type>(1));

    vectors[1]->set(0, 1.1);

    cols_type collection(vectors.begin(), vectors.end());
    cols_type::const_iterator it = collection.begin(), ite = collection.end();
    TEST_ASSERT((*it).type == mdds::mtv::element_type_empty);
    TEST_ASSERT((*it).index == 0);

    ++it;
    TEST_ASSERT((*it).type == mdds::mtv::element_type_double);
    TEST_ASSERT((*it).index == 1);
    TEST_ASSERT(it->get<mdds::mtv::double_element_block>() == 1.1);

    TEST_ASSERT(++it == ite);
}

void mtv_test_shared_pointer_size2()
{
    MDDS_TEST_FUNC_SCOPE;

    std::vector<std::shared_ptr<mtv_type>> vectors;
    vectors.push_back(std::make_shared<mtv_type>(2, 2.3));
    vectors.push_back(std::make_shared<mtv_type>(2, std::string("test")));

    cols_type collection(vectors.begin(), vectors.end());
    TEST_ASSERT(collection.size() == 2);

    cols_type::const_iterator it = collection.begin();

    TEST_ASSERT(it->type == mdds::mtv::element_type_double);
    TEST_ASSERT(it->index == 0);
    TEST_ASSERT(it->position == 0);
    TEST_ASSERT(it->get<mdds::mtv::double_element_block>() == 2.3);

    ++it;
    TEST_ASSERT(it->type == mdds::mtv::element_type_string);
    TEST_ASSERT(it->index == 1);
    TEST_ASSERT(it->position == 0);
    TEST_ASSERT(it->get<mdds::mtv::string_element_block>() == "test");

    ++it;
    TEST_ASSERT(it->type == mdds::mtv::element_type_double);
    TEST_ASSERT(it->index == 0);
    TEST_ASSERT(it->position == 1);
    TEST_ASSERT(it->get<mdds::mtv::double_element_block>() == 2.3);

    ++it;
    TEST_ASSERT(it->type == mdds::mtv::element_type_string);
    TEST_ASSERT(it->index == 1);
    TEST_ASSERT(it->position == 1);
    TEST_ASSERT(it->get<mdds::mtv::string_element_block>() == "test");

    TEST_ASSERT(++it == collection.end());
}

void mtv_test_non_pointer_size1()
{
    MDDS_TEST_FUNC_SCOPE;

    // Test 1 by 1 grid.

    std::vector<mtv_type> vectors;
    vectors.reserve(1);
    vectors.emplace_back(1, int8_t('c'));

    cols_type collection(vectors.begin(), vectors.end());
    TEST_ASSERT(collection.size() == 1);

    auto it = collection.begin();
    TEST_ASSERT(it->type == mdds::mtv::element_type_int8);
    TEST_ASSERT(it->index == 0);
    TEST_ASSERT(it->position == 0);
    TEST_ASSERT(it->get<mdds::mtv::int8_element_block>() == 'c');

    TEST_ASSERT(++it == collection.end());
}

void mtv_test_invalid_collection()
{
    MDDS_TEST_FUNC_SCOPE;

    std::vector<mtv_type> vectors;
    vectors.reserve(2);
    vectors.emplace_back(1, int8_t('c'));
    vectors.emplace_back(2);

    try
    {
        // Grouping vectors of different lengths is not allowed.
        cols_type collection(vectors.begin(), vectors.end());
        TEST_ASSERT(!"invalid_arg_error is expected to be thrown");
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
        TEST_ASSERT(false);
    }
    catch (const mdds::invalid_arg_error&)
    {
        // Good.
    }
}

void mtv_test_sub_element_ranges()
{
    MDDS_TEST_FUNC_SCOPE;

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
    TEST_ASSERT(it->type == mdds::mtv::element_type_int32);
    TEST_ASSERT(it->get<mdds::mtv::int32_element_block>() == 1);
    TEST_ASSERT(it->index == 0);
    TEST_ASSERT(it->position == 1);

    ++it;
    TEST_ASSERT(it->type == mdds::mtv::element_type_int32);
    TEST_ASSERT(it->get<mdds::mtv::int32_element_block>() == 4);
    TEST_ASSERT(it->index == 1);
    TEST_ASSERT(it->position == 1);

    ++it;
    TEST_ASSERT(it->type == mdds::mtv::element_type_int32);
    TEST_ASSERT(it->get<mdds::mtv::int32_element_block>() == 7);
    TEST_ASSERT(it->index == 2);
    TEST_ASSERT(it->position == 1);

    ++it;
    TEST_ASSERT(it->type == mdds::mtv::element_type_int32);
    TEST_ASSERT(it->get<mdds::mtv::int32_element_block>() == 2);
    TEST_ASSERT(it->index == 0);
    TEST_ASSERT(it->position == 2);

    ++it;
    TEST_ASSERT(it->type == mdds::mtv::element_type_int32);
    TEST_ASSERT(it->get<mdds::mtv::int32_element_block>() == 5);
    TEST_ASSERT(it->index == 1);
    TEST_ASSERT(it->position == 2);

    ++it;
    TEST_ASSERT(it->type == mdds::mtv::element_type_int32);
    TEST_ASSERT(it->get<mdds::mtv::int32_element_block>() == 8);
    TEST_ASSERT(it->index == 2);
    TEST_ASSERT(it->position == 2);

    TEST_ASSERT(++it == collection.end());

    // Limit the collection range.
    collection.set_collection_range(1, 1);
    it = collection.begin();
    TEST_ASSERT(it->type == mdds::mtv::element_type_int32);
    TEST_ASSERT(it->get<mdds::mtv::int32_element_block>() == 4);
    TEST_ASSERT(it->index == 1);
    TEST_ASSERT(it->position == 1);

    ++it;
    TEST_ASSERT(it->type == mdds::mtv::element_type_int32);
    TEST_ASSERT(it->get<mdds::mtv::int32_element_block>() == 5);
    TEST_ASSERT(it->index == 1);
    TEST_ASSERT(it->position == 2);

    TEST_ASSERT(++it == collection.end());

    // Swap and try again.
    cols_type swapped;
    collection.swap(swapped);
    it = swapped.begin();
    TEST_ASSERT(it->type == mdds::mtv::element_type_int32);
    TEST_ASSERT(it->get<mdds::mtv::int32_element_block>() == 4);
    TEST_ASSERT(it->index == 1);
    TEST_ASSERT(it->position == 1);

    ++it;
    TEST_ASSERT(it->type == mdds::mtv::element_type_int32);
    TEST_ASSERT(it->get<mdds::mtv::int32_element_block>() == 5);
    TEST_ASSERT(it->index == 1);
    TEST_ASSERT(it->position == 2);

    TEST_ASSERT(++it == swapped.end());
}

void mtv_test_sub_element_ranges_invalid()
{
    MDDS_TEST_FUNC_SCOPE;

    std::deque<mtv_type> vectors;
    vectors.emplace_back(5);
    vectors.emplace_back(5);
    vectors.emplace_back(5);

    cols_type collection(vectors.begin(), vectors.end());
    try
    {
        // Empty size.
        collection.set_element_range(0, 0);
        TEST_ASSERT(!"invalid_arg_error is expected to be thrown");
    }
    catch (const mdds::invalid_arg_error&)
    {}

    try
    {
        // out-of-range start position.
        collection.set_element_range(5, 1);
        TEST_ASSERT(!"invalid_arg_error is expected to be thrown");
    }
    catch (const mdds::invalid_arg_error&)
    {}

    try
    {
        // out-of-range end position.
        collection.set_element_range(0, 6);
        TEST_ASSERT(!"invalid_arg_error is expected to be thrown");
    }
    catch (const mdds::invalid_arg_error&)
    {}
}

void mtv_test_sub_collection_ranges_invalid()
{
    MDDS_TEST_FUNC_SCOPE;

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
        TEST_ASSERT(!"invalid_arg_error is expected to be thrown");
    }
    catch (const mdds::invalid_arg_error&)
    {}

    try
    {
        // out-of-range start position.
        collection.set_collection_range(5, 1);
        TEST_ASSERT(!"invalid_arg_error is expected to be thrown");
    }
    catch (const mdds::invalid_arg_error&)
    {}

    try
    {
        // out-of-range end position.
        collection.set_collection_range(0, 6);
        TEST_ASSERT(!"invalid_arg_error is expected to be thrown");
    }
    catch (const mdds::invalid_arg_error&)
    {}
}

void mtv_test_boolean_block()
{
    MDDS_TEST_FUNC_SCOPE;

    std::vector<mtv_type> vectors;
    vectors.reserve(2);
    vectors.emplace_back(1, true);
    vectors.emplace_back(1, false);

    cols_type collection(vectors.begin(), vectors.end());

    auto it = collection.begin();
    TEST_ASSERT(it->type == mdds::mtv::element_type_boolean);
    TEST_ASSERT(it->index == 0);
    TEST_ASSERT(it->position == 0);
    TEST_ASSERT(it->get<mdds::mtv::boolean_element_block>() == true);

    ++it;
    TEST_ASSERT(it->type == mdds::mtv::element_type_boolean);
    TEST_ASSERT(it->index == 1);
    TEST_ASSERT(it->position == 0);
    TEST_ASSERT(it->get<mdds::mtv::boolean_element_block>() == false);

    TEST_ASSERT(++it == collection.end());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
