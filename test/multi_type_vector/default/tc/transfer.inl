/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

void mtv_test_transfer()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db1(5), db2(5);
    db1.set(0, 1.0);
    db1.set(1, 2.0);
    mtv_type::iterator it = db1.transfer(1, 2, db2, 1);

    TEST_ASSERT(db1.get<double>(0) == 1.0);
    TEST_ASSERT(db1.is_empty(1));
    TEST_ASSERT(db1.is_empty(2));
    TEST_ASSERT(db1.is_empty(3));
    TEST_ASSERT(db1.is_empty(4));

    TEST_ASSERT(db2.is_empty(0));
    TEST_ASSERT(db2.get<double>(1) == 2.0);
    TEST_ASSERT(db2.is_empty(2));
    TEST_ASSERT(db2.is_empty(3));
    TEST_ASSERT(db2.is_empty(4));

    TEST_ASSERT(it->size == 4);
    TEST_ASSERT(it->__private_data.block_index == 1);
    TEST_ASSERT(it->position == 1);

    // Reset and start over.
    db1.clear();
    db1.resize(5);
    db2.clear();
    db2.resize(5);

    db1.set(2, 1.2);
    db1.set(3, 1.3);

    // Transfer 1:2 in db1 to 2:3 in db2.
    it = db1.transfer(1, 2, db2, 2);

    TEST_ASSERT(db1.is_empty(0));
    TEST_ASSERT(db1.is_empty(1));
    TEST_ASSERT(db1.is_empty(2));
    TEST_ASSERT(db1.get<double>(3) == 1.3);
    TEST_ASSERT(db1.is_empty(4));

    TEST_ASSERT(db2.is_empty(0));
    TEST_ASSERT(db2.is_empty(1));
    TEST_ASSERT(db2.is_empty(2));
    TEST_ASSERT(db2.get<double>(3) == 1.2);
    TEST_ASSERT(db2.is_empty(4));

    TEST_ASSERT(it->size == 3);
    TEST_ASSERT(it->position == 0);
    TEST_ASSERT(it->__private_data.block_index == 0);

    // Reset and start over.
    db1.clear();
    db1.resize(4);
    db2.clear();
    db2.resize(4);

    db1.set(0, std::string("A"));
    db1.set(1, std::string("B"));
    db1.set(2, 11.1);
    db1.set(3, 11.2);

    it = db1.transfer(1, 2, db2, 1);

    TEST_ASSERT(db1.get<std::string>(0) == "A");
    TEST_ASSERT(db1.is_empty(1));
    TEST_ASSERT(db1.is_empty(2));
    TEST_ASSERT(db1.get<double>(3) == 11.2);

    TEST_ASSERT(db2.is_empty(0));
    TEST_ASSERT(db2.get<std::string>(1) == "B");
    TEST_ASSERT(db2.get<double>(2) == 11.1);
    TEST_ASSERT(db2.is_empty(3));

    TEST_ASSERT(it->size == 2);
    TEST_ASSERT(it->position == 1);
    TEST_ASSERT(it->__private_data.block_index == 1);

    // Reset and start over.
    db1.clear();
    db1.resize(4);
    db2.clear();
    db2.resize(4);

    db1.set(2, 12.8);
    it = db1.transfer(1, 2, db2, 1);

    // Reset and start over.
    db1.clear();
    db1.resize(20);
    db2.clear();
    db2.resize(20);

    db1.set(9, 1.1);

    db2.set(10, 1.2);
    db2.set(11, 1.3);

    it = db1.transfer(9, 9, db2, 9);

    // The source should be entirely empty after the transfer.
    TEST_ASSERT(db1.block_size() == 1);
    TEST_ASSERT(it == db1.begin());
    TEST_ASSERT(it->__private_data.block_index == 0);
    TEST_ASSERT(it->size == 20);
    TEST_ASSERT(it->type == mdds::mtv::element_type_empty);
    ++it;
    TEST_ASSERT(it == db1.end());

    // Check the destination as well.
    TEST_ASSERT(db2.block_size() == 3);
    it = db2.begin();
    TEST_ASSERT(it->size == 9);
    TEST_ASSERT(it->__private_data.block_index == 0);
    TEST_ASSERT(it->type == mdds::mtv::element_type_empty);
    ++it;
    TEST_ASSERT(it->size == 3);
    TEST_ASSERT(it->__private_data.block_index == 1);
    TEST_ASSERT(it->type == mdds::mtv::element_type_double);
    ++it;
    TEST_ASSERT(it->size == 8);
    TEST_ASSERT(it->__private_data.block_index == 2);
    TEST_ASSERT(it->type == mdds::mtv::element_type_empty);
    ++it;
    TEST_ASSERT(it == db2.end());
    TEST_ASSERT(db2.get<double>(9) == 1.1);
    TEST_ASSERT(db2.get<double>(10) == 1.2);
    TEST_ASSERT(db2.get<double>(11) == 1.3);

    // Reset and start over.
    db1.clear();
    db1.resize(20);
    db2.clear();
    db2.resize(20);

    db1.set(8, 1.0);
    db1.set(9, 1.1);

    db2.set(10, 1.2);
    db2.set(11, 1.3);

    it = db1.transfer(9, 9, db2, 9);
    TEST_ASSERT(it->__private_data.block_index == 2);
    TEST_ASSERT(db1.block_size() == 3);
    TEST_ASSERT(db1.get<double>(8) == 1.0);
    it = db1.begin();
    TEST_ASSERT(it->size == 8);
    TEST_ASSERT(it->type == mdds::mtv::element_type_empty);
    ++it;
    TEST_ASSERT(it->size == 1);
    TEST_ASSERT(it->type == mdds::mtv::element_type_double);
    ++it;
    TEST_ASSERT(it->size == 11);
    TEST_ASSERT(it->type == mdds::mtv::element_type_empty);
    ++it;
    TEST_ASSERT(it == db1.end());

    // Reset and start over.
    db1.clear();
    db1.resize(8);
    db2 = mtv_type(8, true);

    db1.transfer(0, 4, db2, 1);

    TEST_ASSERT(db1.is_empty(0));
    TEST_ASSERT(db1.is_empty(1));
    TEST_ASSERT(db1.is_empty(2));
    TEST_ASSERT(db1.is_empty(3));
    TEST_ASSERT(db1.is_empty(4));
    TEST_ASSERT(db1.is_empty(5));
    TEST_ASSERT(db1.is_empty(6));
    TEST_ASSERT(db1.is_empty(7));

    TEST_ASSERT(db2.get<bool>(0) == true);
    TEST_ASSERT(db2.is_empty(1));
    TEST_ASSERT(db2.is_empty(2));
    TEST_ASSERT(db2.is_empty(3));
    TEST_ASSERT(db2.is_empty(4));
    TEST_ASSERT(db2.is_empty(5));
    TEST_ASSERT(db2.get<bool>(6) == true);
    TEST_ASSERT(db2.get<bool>(7) == true);

    // Start over.
    db1 = mtv_type(8, int16_t(43));
    db2 = mtv_type(8);
    db1.transfer(3, 5, db2, 0);
    TEST_ASSERT(db1.get<int16_t>(0) == 43);
    TEST_ASSERT(db1.get<int16_t>(1) == 43);
    TEST_ASSERT(db1.get<int16_t>(2) == 43);
    TEST_ASSERT(db1.is_empty(3));
    TEST_ASSERT(db1.is_empty(4));
    TEST_ASSERT(db1.is_empty(5));
    TEST_ASSERT(db1.get<int16_t>(6) == 43);
    TEST_ASSERT(db1.get<int16_t>(7) == 43);

    TEST_ASSERT(db2.get<int16_t>(0) == 43);
    TEST_ASSERT(db2.get<int16_t>(1) == 43);
    TEST_ASSERT(db2.get<int16_t>(2) == 43);
    TEST_ASSERT(db2.is_empty(3));
    TEST_ASSERT(db2.is_empty(4));
    TEST_ASSERT(db2.is_empty(5));
    TEST_ASSERT(db2.is_empty(6));
    TEST_ASSERT(db2.is_empty(7));

    // Start over.
    db1 = mtv_type(5);
    db2 = mtv_type(5);
    db1.set<double>(3, 1.1);
    db1.set<double>(4, 1.2);
    db2.set<int32_t>(0, 1);
    db2.set<int32_t>(1, 1);

    db1.transfer(3, 4, db2, 0);

    TEST_ASSERT(db1.is_empty(0));
    TEST_ASSERT(db1.is_empty(1));
    TEST_ASSERT(db1.is_empty(2));
    TEST_ASSERT(db1.is_empty(3));
    TEST_ASSERT(db1.is_empty(4));

    TEST_ASSERT(db2.get<double>(0) == 1.1);
    TEST_ASSERT(db2.get<double>(1) == 1.2);
    TEST_ASSERT(db2.is_empty(2));
    TEST_ASSERT(db2.is_empty(3));
    TEST_ASSERT(db2.is_empty(4));

    // Start over.
    db1 = mtv_type(5);
    db2 = mtv_type(5);
    db1.set<double>(3, 1.1);
    db1.set<int32_t>(4, 892);
    db2.set<int32_t>(0, 1);
    db2.set<int32_t>(1, 1);

    db1.transfer(3, 4, db2, 0);

    TEST_ASSERT(db1.is_empty(0));
    TEST_ASSERT(db1.is_empty(1));
    TEST_ASSERT(db1.is_empty(2));
    TEST_ASSERT(db1.is_empty(3));
    TEST_ASSERT(db1.is_empty(4));

    TEST_ASSERT(db2.get<double>(0) == 1.1);
    TEST_ASSERT(db2.get<int32_t>(1) == 892);
    TEST_ASSERT(db2.is_empty(2));
    TEST_ASSERT(db2.is_empty(3));
    TEST_ASSERT(db2.is_empty(4));

    // Start over.
    db1 = mtv_type(5, false);
    db2 = mtv_type(5);
    db1.set<int32_t>(4, 234);
    db2.set<int8_t>(0, 32);
    db2.set<uint16_t>(1, 11);
    db2.set<uint16_t>(2, 22);
    db2.set<int8_t>(3, 56);

    db1.transfer(3, 4, db2, 1);

    TEST_ASSERT(db1.get<bool>(0) == false);
    TEST_ASSERT(db1.get<bool>(1) == false);
    TEST_ASSERT(db1.get<bool>(2) == false);
    TEST_ASSERT(db1.is_empty(3));
    TEST_ASSERT(db1.is_empty(4));

    TEST_ASSERT(db2.get<int8_t>(0) == 32);
    TEST_ASSERT(db2.get<bool>(1) == false);
    TEST_ASSERT(db2.get<int32_t>(2) == 234);
    TEST_ASSERT(db2.get<int8_t>(3) == 56);
    TEST_ASSERT(db2.is_empty(4));

    // Start over.
    db1 = mtv_type(5, false);
    db2 = mtv_type(6);
    db1.set<int32_t>(4, 234);
    db2.set<uint32_t>(0, 2345);

    db1.transfer(3, 4, db2, 4);

    TEST_ASSERT(db1.get<bool>(0) == false);
    TEST_ASSERT(db1.get<bool>(1) == false);
    TEST_ASSERT(db1.get<bool>(2) == false);
    TEST_ASSERT(db1.is_empty(3));
    TEST_ASSERT(db1.is_empty(4));

    TEST_ASSERT(db2.get<uint32_t>(0) == 2345);
    TEST_ASSERT(db2.is_empty(1));
    TEST_ASSERT(db2.is_empty(2));
    TEST_ASSERT(db2.is_empty(3));
    TEST_ASSERT(db2.get<bool>(4) == false);
    TEST_ASSERT(db2.get<int32_t>(5) == 234);

    // Start over
    db1.clear();
    db2 = mtv_type(10);
    db1.push_back<int8_t>(5);
    db1.push_back<int16_t>(5);
    db1.push_back<int32_t>(5);
    db1.push_back<double>(5);

    db1.transfer(0, 3, db2, 6);

    TEST_ASSERT(db1.is_empty(0));
    TEST_ASSERT(db1.is_empty(1));
    TEST_ASSERT(db1.is_empty(2));
    TEST_ASSERT(db1.is_empty(3));

    TEST_ASSERT(db2.is_empty(0));
    TEST_ASSERT(db2.is_empty(1));
    TEST_ASSERT(db2.is_empty(2));
    TEST_ASSERT(db2.is_empty(3));
    TEST_ASSERT(db2.is_empty(4));
    TEST_ASSERT(db2.is_empty(5));
    TEST_ASSERT(db2.get<int8_t>(6) == 5);
    TEST_ASSERT(db2.get<int16_t>(7) == 5);
    TEST_ASSERT(db2.get<int32_t>(8) == 5);
    TEST_ASSERT(db2.get<double>(9) == 5.0);

    // Start over.
    db1 = mtv_type(5, true);
    db2 = mtv_type(6);

    db1.transfer(0, 2, db2, 1);

    TEST_ASSERT(db1.is_empty(0));
    TEST_ASSERT(db1.is_empty(1));
    TEST_ASSERT(db1.is_empty(2));
    TEST_ASSERT(db1.get<bool>(3) == true);
    TEST_ASSERT(db1.get<bool>(4) == true);

    TEST_ASSERT(db2.is_empty(0));
    TEST_ASSERT(db2.get<bool>(1) == true);
    TEST_ASSERT(db2.get<bool>(2) == true);
    TEST_ASSERT(db2.get<bool>(3) == true);
    TEST_ASSERT(db2.is_empty(4));
    TEST_ASSERT(db2.is_empty(5));

    // Start over.
    db1 = mtv_type(5, int32_t(987));
    db1.set(3, 1.1);
    db1.set(4, 1.2);

    db2 = mtv_type(5);
    db2.set(0, true);
    db2.set(1, false);

    db1.transfer(0, 2, db2, 2);

    TEST_ASSERT(db1.is_empty(0));
    TEST_ASSERT(db1.is_empty(1));
    TEST_ASSERT(db1.is_empty(2));
    TEST_ASSERT(db1.get<double>(3) == 1.1);
    TEST_ASSERT(db1.get<double>(4) == 1.2);

    TEST_ASSERT(db2.get<bool>(0) == true);
    TEST_ASSERT(db2.get<bool>(1) == false);
    TEST_ASSERT(db2.get<int32_t>(2) == 987);
    TEST_ASSERT(db2.get<int32_t>(3) == 987);
    TEST_ASSERT(db2.get<int32_t>(4) == 987);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
