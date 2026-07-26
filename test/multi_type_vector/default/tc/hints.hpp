/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "common.hpp"

template<typename mtv_type>
void mtv_test_hints_set()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db(3);
    db.set(db.begin(), 0, 23.4);
    TEST_ASSERT(db.template get<double>(0) == 23.4);
    db.set(db.end(), 0, std::string("test")); // passing end position should have no impact.
    TEST_ASSERT(db.template get<std::string>(0) == "test");

    mdds::mtv::position_hint pos_hint = db.set(0, 1.2);
    pos_hint = db.set(pos_hint, 1, 1.3);
    pos_hint = db.set(pos_hint, 2, 1.4);
    TEST_ASSERT(db.template get<double>(0) == 1.2);
    TEST_ASSERT(db.template get<double>(1) == 1.3);
    TEST_ASSERT(db.template get<double>(2) == 1.4);

    pos_hint = db.begin();
    pos_hint = db.set(pos_hint, 0, false);
    pos_hint = db.set(pos_hint, 1, std::string("foo"));
    pos_hint = db.set(pos_hint, 2, 34.5);
    TEST_ASSERT(db.template get<bool>(0) == false);
    TEST_ASSERT(db.template get<std::string>(1) == "foo");
    TEST_ASSERT(db.template get<double>(2) == 34.5);

    db.set(pos_hint, 0, int32_t(444)); // position hint does not precede the insertion position.
    TEST_ASSERT(db.template get<int32_t>(0) == 444); // it should still work.
}

template<typename mtv_type>
void mtv_test_hints_set_cells()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db(9);

    std::vector<int32_t> ints;
    ints.push_back(1);
    ints.push_back(2);
    ints.push_back(3);

    std::vector<double> doubles;
    doubles.push_back(1.1);
    doubles.push_back(1.2);
    doubles.push_back(1.3);

    std::vector<std::string> strings;
    strings.push_back(std::string("A"));
    strings.push_back(std::string("B"));
    strings.push_back(std::string("C"));

    mdds::mtv::position_hint pos_hint;
    pos_hint = db.set(pos_hint, 0, ints.begin(), ints.end());
    pos_hint = db.set(pos_hint, 3, doubles.begin(), doubles.end());
    pos_hint = db.set(pos_hint, 6, strings.begin(), strings.end());

    TEST_ASSERT(db.template get<int32_t>(0) == 1);
    TEST_ASSERT(db.template get<int32_t>(1) == 2);
    TEST_ASSERT(db.template get<int32_t>(2) == 3);

    TEST_ASSERT(db.template get<double>(3) == 1.1);
    TEST_ASSERT(db.template get<double>(4) == 1.2);
    TEST_ASSERT(db.template get<double>(5) == 1.3);

    TEST_ASSERT(db.template get<std::string>(6) == "A");
    TEST_ASSERT(db.template get<std::string>(7) == "B");
    TEST_ASSERT(db.template get<std::string>(8) == "C");
}

template<typename mtv_type>
void mtv_test_hints_insert_cells()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db(1, true); // We need to have at least one element to be able to insert.

    std::vector<int32_t> ints;
    ints.push_back(11);
    ints.push_back(22);

    std::vector<double> doubles;
    doubles.push_back(2.1);
    doubles.push_back(3.2);
    doubles.push_back(4.3);

    std::vector<std::string> strings;
    strings.push_back(std::string("Andy"));
    strings.push_back(std::string("Bruce"));
    strings.push_back(std::string("Charlie"));
    strings.push_back(std::string("David"));

    mdds::mtv::position_hint pos_hint = db.insert(0, ints.begin(), ints.end());
    TEST_ASSERT(db.template get<int32_t>(0) == 11);
    TEST_ASSERT(db.template get<int32_t>(1) == 22);
    TEST_ASSERT(db.template get<bool>(2) == true);

    pos_hint = db.insert(pos_hint, 2, doubles.begin(), doubles.end());
    TEST_ASSERT(db.template get<int32_t>(0) == 11);
    TEST_ASSERT(db.template get<int32_t>(1) == 22);
    TEST_ASSERT(db.template get<double>(2) == 2.1);
    TEST_ASSERT(db.template get<double>(3) == 3.2);
    TEST_ASSERT(db.template get<double>(4) == 4.3);
    TEST_ASSERT(db.template get<bool>(5) == true);

    pos_hint = db.insert(pos_hint, 4, strings.begin(), strings.end());
    TEST_ASSERT(db.template get<int32_t>(0) == 11);
    TEST_ASSERT(db.template get<int32_t>(1) == 22);
    TEST_ASSERT(db.template get<double>(2) == 2.1);
    TEST_ASSERT(db.template get<double>(3) == 3.2);
    TEST_ASSERT(db.template get<std::string>(4) == "Andy");
    TEST_ASSERT(db.template get<std::string>(5) == "Bruce");
    TEST_ASSERT(db.template get<std::string>(6) == "Charlie");
    TEST_ASSERT(db.template get<std::string>(7) == "David");
    TEST_ASSERT(db.template get<double>(8) == 4.3);
    TEST_ASSERT(db.template get<bool>(9) == true);
}

template<typename mtv_type>
void mtv_test_hints_set_empty()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db(20, true);
    mdds::mtv::position_hint pos_hint = db.set_empty(db.begin(), 2, 3);
    pos_hint = db.set_empty(pos_hint, 5, 7);
    pos_hint = db.set_empty(pos_hint, 9, 12);
    pos_hint = db.set_empty(pos_hint, 14, 17);

    // Check the boundaries.
    TEST_ASSERT(!db.is_empty(0));
    TEST_ASSERT(!db.is_empty(4));
    TEST_ASSERT(db.is_empty(5));
    TEST_ASSERT(db.is_empty(7));
    TEST_ASSERT(!db.is_empty(8));
    TEST_ASSERT(db.is_empty(9));
    TEST_ASSERT(db.is_empty(12));
    TEST_ASSERT(!db.is_empty(13));
    TEST_ASSERT(db.is_empty(14));
    TEST_ASSERT(db.is_empty(17));
    TEST_ASSERT(!db.is_empty(18));
}

template<typename mtv_type>
void mtv_test_hints_insert_empty()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db(2, true);
    mdds::mtv::position_hint pos_hint;
    pos_hint = db.insert_empty(pos_hint, 1, 3); // the size becomes 5.
    pos_hint = db.insert_empty(pos_hint, 4, 2); // the size now becomes 7.

    typename mtv_type::iterator check = db.begin();
    TEST_ASSERT(check->type == mdds::mtv::element_type_boolean);
    TEST_ASSERT(check->size == 1);
    ++check;
    TEST_ASSERT(check->type == mdds::mtv::element_type_empty);
    TEST_ASSERT(check->size == 5);
    ++check;
    TEST_ASSERT(check->type == mdds::mtv::element_type_boolean);
    TEST_ASSERT(check->size == 1);
    ++check;
    TEST_ASSERT(check == db.end());
}

template<typename mtv_type>
void mtv_test_hints_default()
{
    MDDS_TEST_FUNC_SCOPE;

    // A default-constructed hint represents "no hint" and behaves as
    // unhinted on every hinted method.
    mdds::mtv::position_hint hint;

    {
        // set() a single value and a value sequence.
        mtv_type db(4);
        db.set(hint, 0, 1.1);
        db.set(hint, 1, std::string("A"));

        std::vector<int32_t> ints;
        ints.push_back(11);
        ints.push_back(22);
        db.set(hint, 2, ints.begin(), ints.end());

        TEST_ASSERT(db.template get<double>(0) == 1.1);
        TEST_ASSERT(db.template get<std::string>(1) == "A");
        TEST_ASSERT(db.template get<int32_t>(2) == 11);
        TEST_ASSERT(db.template get<int32_t>(3) == 22);
    }

    {
        // insert(), set_empty() and insert_empty().
        mtv_type db(2, true);

        std::vector<double> doubles;
        doubles.push_back(2.1);
        doubles.push_back(2.2);
        db.insert(hint, 1, doubles.begin(), doubles.end()); // the size becomes 4.

        TEST_ASSERT(db.size() == 4);
        TEST_ASSERT(db.template get<bool>(0) == true);
        TEST_ASSERT(db.template get<double>(1) == 2.1);
        TEST_ASSERT(db.template get<double>(2) == 2.2);
        TEST_ASSERT(db.template get<bool>(3) == true);

        db.set_empty(hint, 0, 1);
        TEST_ASSERT(db.is_empty(0));
        TEST_ASSERT(db.is_empty(1));
        TEST_ASSERT(db.template get<double>(2) == 2.2);

        db.insert_empty(hint, 3, 2); // the size becomes 6.
        TEST_ASSERT(db.size() == 6);
        TEST_ASSERT(db.template get<double>(2) == 2.2);
        TEST_ASSERT(db.is_empty(3));
        TEST_ASSERT(db.is_empty(4));
        TEST_ASSERT(db.template get<bool>(5) == true);
    }

    {
        // release() and release_range().
        mtv_type db(3);
        db.set(hint, 0, 5.5);
        db.set(hint, 1, 6.6);
        db.set(hint, 2, 7.7);

        double v = 0.0;
        db.release(hint, 0, v);
        TEST_ASSERT(v == 5.5);
        TEST_ASSERT(db.is_empty(0));

        db.release_range(hint, 1, 2);
        TEST_ASSERT(db.is_empty(1));
        TEST_ASSERT(db.is_empty(2));
    }

    {
        // transfer() and the two position() variants.
        mtv_type src(3), dst(3);
        src.set(hint, 0, 1.5);
        src.set(hint, 1, 2.5);

        src.transfer(hint, 0, 1, dst, 0);
        TEST_ASSERT(src.is_empty(0));
        TEST_ASSERT(src.is_empty(1));
        TEST_ASSERT(dst.template get<double>(0) == 1.5);
        TEST_ASSERT(dst.template get<double>(1) == 2.5);

        auto pos = dst.position(hint, 1);
        TEST_ASSERT(pos.first->position + pos.second == 1);

        const mtv_type& cdst = dst;
        auto cpos = cdst.position(hint, 1);
        TEST_ASSERT(cpos.first->position + cpos.second == 1);
    }

    {
        // A hinted call on an empty container degrades safely and still
        // throws for an out-of-range position.
        mtv_type db;
        bool caught = false;
        try
        {
            db.set(hint, 0, 1.0);
        }
        catch (const std::out_of_range&)
        {
            caught = true;
        }
        TEST_ASSERT(caught);
    }
}

template<typename mtv_type>
void mtv_test_hints_from_other_container()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        // Use a hint originating from another live container.  The hint gets
        // ignored and the lookup falls back to an unhinted search.
        mtv_type db1(6);
        db1.set(0, 1.1);
        db1.set(2, std::string("A"));
        mdds::mtv::position_hint hint = db1.set(4, int32_t(40)); // block index in bounds for db2 too
        TEST_ASSERT(hint.container_id == db1.identity());

        mtv_type db2(6);
        TEST_ASSERT(db2.identity() != db1.identity());
        db2.set(hint, 0, 2.1);
        db2.set(hint, 5, std::string("Z"));
        TEST_ASSERT(db2.template get<double>(0) == 2.1);
        TEST_ASSERT(db2.template get<std::string>(5) == "Z");

        TEST_ASSERT(db1.template get<double>(0) == 1.1);
        TEST_ASSERT(db1.template get<std::string>(2) == "A");
        TEST_ASSERT(db1.template get<int32_t>(4) == 40);
    }

    {
        // Reuse a hint whose originating container has been destroyed, with
        // a new container created afterwards.  With the old iterator-based
        // hints a new container allocated at the same address would pass the
        // parent pointer check and crash; the container identity check
        // detects this and falls back to an unhinted search.
        mdds::mtv::position_hint hint;
        {
            mtv_type db1(4);
            db1.set(0, 1.1);
            db1.set(1, std::string("A"));
            hint = db1.set(2, int32_t(30));
        }

        mtv_type db2(4);
        db2.set(hint, 1, 4.5);
        db2.set(hint, 3, std::string("Y"));
        TEST_ASSERT(db2.template get<double>(1) == 4.5);
        TEST_ASSERT(db2.template get<std::string>(3) == "Y");
    }
}

template<typename mtv_type>
void mtv_test_hints_stale_block_index()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        // Take a hint at a high block index, then collapse the container to
        // a single block so that the stored block index goes out of bounds.
        mtv_type db(10);
        db.set(0, 1.1);
        db.set(2, std::string("A"));
        db.set(4, int32_t(40));
        db.set(6, 2.2);
        db.set(8, std::string("B"));
        mdds::mtv::position_hint hint = db.set(9, 3.3);

        std::vector<double> doubles(10, 1.0);
        db.set(0, doubles.begin(), doubles.end());
        TEST_ASSERT(db.block_size() == 1);

        db.set(hint, 9, std::string("Z"));
        TEST_ASSERT(db.template get<std::string>(9) == "Z");
        TEST_ASSERT(db.template get<double>(8) == 1.0);
    }

    {
        // A hint that points past the target position still yields correct
        // results via the walk-back.
        mtv_type db(10);
        db.set(0, 1.1);
        db.set(2, std::string("A"));
        db.set(4, int32_t(40));
        db.set(6, 2.2);
        mdds::mtv::position_hint hint = db.set(8, std::string("B"));

        db.set(hint, 1, 5.5);
        TEST_ASSERT(db.template get<double>(1) == 5.5);
        db.set(hint, 0, 6.6);
        TEST_ASSERT(db.template get<double>(0) == 6.6);
    }
}

template<typename mtv_type>
void mtv_test_hints_container_identity()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        // The container identity moves with the block data: a hint taken
        // before a move remains usable with the move destination, and the
        // moved-from instance is left with no identity.
        mtv_type src(6);
        src.set(0, 1.1);
        src.set(2, std::string("A"));
        mdds::mtv::position_hint hint = src.set(4, int32_t(40));
        TEST_ASSERT(hint.container_id == src.identity());

        mdds::mtv::container_id_t src_id = src.identity();
        mtv_type moved(std::move(src));
        TEST_ASSERT(moved.identity() == src_id);
        TEST_ASSERT(src.identity() == 0);

        moved.set(hint, 5, 2.2);
        TEST_ASSERT(moved.template get<double>(5) == 2.2);
        TEST_ASSERT(moved.template get<int32_t>(4) == 40);
    }

    {
        // Same via move assignment.
        mtv_type src(6);
        src.set(0, 1.1);
        mdds::mtv::position_hint hint = src.set(4, int32_t(40));

        mdds::mtv::container_id_t src_id = src.identity();
        mtv_type moved;
        moved = std::move(src);
        TEST_ASSERT(moved.identity() == src_id);
        TEST_ASSERT(src.identity() == 0);

        moved.set(hint, 5, 2.2);
        TEST_ASSERT(moved.template get<double>(5) == 2.2);
        TEST_ASSERT(moved.template get<int32_t>(4) == 40);
    }

    {
        // swap() exchanges the identities along with the data, so hints
        // follow their blocks to the other container.
        mtv_type db1(6);
        db1.set(0, 1.1);
        mdds::mtv::position_hint hint1 = db1.set(4, int32_t(40));

        mtv_type db2(3);
        db2.set(0, std::string("A"));
        mdds::mtv::position_hint hint2 = db2.set(2, std::string("B"));

        mdds::mtv::container_id_t db1_id = db1.identity();
        mdds::mtv::container_id_t db2_id = db2.identity();
        db1.swap(db2);
        TEST_ASSERT(db1.identity() == db2_id);
        TEST_ASSERT(db2.identity() == db1_id);

        db2.set(hint1, 5, 2.2);
        TEST_ASSERT(db2.template get<double>(5) == 2.2);
        TEST_ASSERT(db2.template get<int32_t>(4) == 40);
        db1.set(hint2, 1, std::string("C"));
        TEST_ASSERT(db1.template get<std::string>(1) == "C");
        TEST_ASSERT(db1.template get<std::string>(2) == "B");
    }

    {
        // A moved-from container has no identity; a hint created from its
        // iterator is an empty hint that never validates anywhere.
        mtv_type src(4);
        src.set(0, 1.1);
        mtv_type moved(std::move(src));

        mdds::mtv::position_hint hint = src.begin();
        TEST_ASSERT(hint.container_id == 0);

        moved.set(hint, 1, 2.2);
        TEST_ASSERT(moved.template get<double>(1) == 2.2);
    }

    {
        // Copies and clones get an identity of their own: a hint from the
        // source is not honored by them, but operations still yield correct
        // results via the fallback, and the source keeps honoring its own
        // hint.
        mtv_type src(6);
        src.set(0, 1.1);
        src.set(2, std::string("A"));
        mdds::mtv::position_hint hint = src.set(4, int32_t(40));

        mtv_type copied(src);
        TEST_ASSERT(copied.identity() != 0);
        TEST_ASSERT(copied.identity() != src.identity());
        copied.set(hint, 5, 2.2);
        TEST_ASSERT(copied.template get<double>(5) == 2.2);
        TEST_ASSERT(copied.template get<int32_t>(4) == 40);

        mtv_type cloned = src.clone();
        TEST_ASSERT(cloned.identity() != 0);
        TEST_ASSERT(cloned.identity() != src.identity());
        TEST_ASSERT(cloned.identity() != copied.identity());
        cloned.set(hint, 5, 3.3);
        TEST_ASSERT(cloned.template get<double>(5) == 3.3);
        TEST_ASSERT(cloned.template get<int32_t>(4) == 40);

        src.set(hint, 5, 4.4);
        TEST_ASSERT(src.template get<double>(5) == 4.4);
    }
}

template<typename mtv_type>
void mtv_test_hints_iterator_conversion()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db(4);
    db.set(0, 1.1);
    db.set(2, std::string("A"));

    {
        // From a non-const iterator.
        mdds::mtv::position_hint hint = db.begin();
        TEST_ASSERT(hint.container_id == db.identity());
        db.set(hint, 1, 2.2);
        TEST_ASSERT(db.template get<double>(1) == 2.2);
    }

    {
        // From a const iterator.
        mdds::mtv::position_hint hint = db.cbegin();
        TEST_ASSERT(hint.container_id == db.identity());
        const mtv_type& cdb = db;
        auto pos = cdb.position(hint, 2);
        TEST_ASSERT(pos.first->position + pos.second == 2);
    }

    {
        // From a mutator return value in the feed-back pattern.
        mdds::mtv::position_hint hint = db.set(1, 3.3);
        hint = db.set(hint, 3, std::string("B"));
        TEST_ASSERT(db.template get<double>(1) == 3.3);
        TEST_ASSERT(db.template get<std::string>(3) == "B");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
