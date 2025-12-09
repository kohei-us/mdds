// SPDX-FileCopyrightText: 2010 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.

#include <mdds/point_quad_tree.hpp>

#include <algorithm>
#include <memory>
#include <vector>
#include <sstream>
#include <boost/cstdint.hpp>

using namespace mdds;
using ::boost::uint16_t;

struct data_printer
{
    void operator()(const std::string* p)
    {
        cout << *p << " ";
    }
};

template<typename _DbType>
struct search_result_printer
{
    void operator()(const std::pair<const typename _DbType::point, const typename _DbType::value_type>& r) const
    {
        cout << "  (x=" << r.first.x << ", y=" << r.first.y << ", value='" << *r.second << "')" << endl;
    }
};

void pqt_test_basic()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef point_quad_tree<uint16_t, const std::string*> db_type;
    db_type db;

    std::string A("A");
    std::string B("B");
    std::string C("C");
    std::string D("D");
    std::string E("E");
    std::string F("F");
    std::string G("G");
    std::string H("H");
    std::string I("I");
    std::string J("J");
    std::string K("K");
    std::string L("L");
    std::string M("M");
    std::string N("N");

    db.insert(25, 32, &A);
    db.insert(5, 45, &B);
    db.insert(52, 10, &C);
    db.insert(80, 5, &D);
    db.insert(40, 50, &E);
    db.insert(10, 10, &F);
    db.insert(20, 20, &G);
    db.insert(80, 80, &H);
    db.insert(58, 46, &I);
    db.insert(36, 55, &J);
    db.insert(26, 52, &K);
    db.insert(38, 68, &L);
    db.insert(39, 78, &M);
    db.insert(72, 52, &N);

    TEST_ASSERT(db.size() == 14);

    cout << "node count = " << get_node_instance_count() << endl;
    db.dump_tree_svg("./obj/test.svg");

    {
        db_type::data_array_type result;
        db.search_region(10, 10, 60, 20, result);
        cout << "search region: (10, 10, 60, 20)" << endl;
        cout << "result: ";
        for_each(result.begin(), result.end(), data_printer());
        cout << endl;

        result.clear();
        db.search_region(10, 10, 61, 61, result);
        cout << "search region: (10, 10, 61, 61)" << endl;
        cout << "result: ";
        for_each(result.begin(), result.end(), data_printer());
        cout << endl;
    }

    db_type::search_results result = db.search_region(10, 10, 60, 20);
    db_type::search_results::const_iterator itr = result.begin(), itr_end = result.end();
    cout << "result: " << endl;
    for_each(result.begin(), result.end(), search_result_printer<db_type>());

    result = db.search_region(10, 10, 61, 61);
    itr = result.begin(), itr_end = result.end();
    cout << "result: " << endl;
    for_each(result.begin(), result.end(), search_result_printer<db_type>());

    db.remove(20, 20);
    db.remove(40, 50);
    TEST_ASSERT(db.size() == 12);
    db.dump_tree_svg("./obj/test-remove.svg");

    db.clear();
    TEST_ASSERT(db.empty());
    TEST_ASSERT(db.size() == 0);
}

void pqt_test_insertion_removal()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef point_quad_tree<int32_t, const std::string*> db_type;
    db_type db;

    // Check its empty-ness...
    TEST_ASSERT(db.empty());
    TEST_ASSERT(db.size() == 0);

    // Create all data instances.
    std::vector<std::unique_ptr<std::string>> data_store;
    data_store.reserve(100);
    for (size_t i = 0; i < 100; ++i)
    {
        std::ostringstream os;
        os << "0x" << std::hex << i;
        data_store.emplace_back(new std::string(os.str()));
    }

    std::vector<db_type::node_data> expected;

    // Insert data one by one, and verify each insertion.
    for (int32_t i = 0; i < 10; ++i)
    {
        for (int32_t j = 0; j < 10; ++j)
        {
            int32_t x = i * 10 + 1, y = j * 10 + 1;
            size_t index = i * 10 + j;
            const std::string* data_ptr = data_store[index].get();
            cout << "inserting '" << *data_ptr << "' at (" << x << "," << y << ")" << endl;
            db.insert(x, y, data_ptr);
            expected.push_back(db_type::node_data(x, y, data_ptr));

            std::vector<db_type::node_data> stored_data;
            db.get_all_stored_data(stored_data);
            TEST_ASSERT(stored_data.size() == (index + 1));
            TEST_ASSERT(db.size() == (index + 1));
            TEST_ASSERT(!db.empty());
            bool success = db.verify_data(expected);
            TEST_ASSERT(success);
        }
    }
    db.dump_tree_svg("./obj/pqt_test_insertion.svg");

    // Remove data one by one, and check the size after each removal.
    size_t node_count = 100;
    for (int32_t i = 0; i < 10; ++i)
    {
        for (int32_t j = 0; j < 10; ++j)
        {
            int32_t x = i * 10 + 1, y = j * 10 + 1;
            db.remove(x, y);
            size_t n = db.size();
            cout << "removing node at (" << x << "," << y << ")  "
                 << "size after removal: " << n << endl;
            --node_count;
            TEST_ASSERT(node_count == n);
        }
    }
}

void pqt_test_remove_root()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef point_quad_tree<int32_t, const std::string*> db_type;
    std::string O("O");
    std::string NW("NW");
    std::string NE("NE");
    std::string SW("SW");
    std::string SE("SE");
    db_type db;

    // Insert all data and verify their storage.
    db.insert(10, 10, &O);
    db.insert(20, 0, &NE);
    db.insert(0, 0, &NW);
    db.insert(20, 20, &SE);
    db.insert(0, 20, &SW);
    db.dump_tree_svg("./obj/pqt_test_remove_root-1.svg");

    std::vector<db_type::node_data> expected;
    expected.push_back(db_type::node_data(10, 10, &O));
    expected.push_back(db_type::node_data(20, 0, &NE));
    expected.push_back(db_type::node_data(0, 0, &NW));
    expected.push_back(db_type::node_data(20, 20, &SE));
    expected.push_back(db_type::node_data(0, 20, &SW));
    bool success = db.verify_data(expected);
    TEST_ASSERT(success);
    TEST_ASSERT(db.size() == 5);

    // Now, remove the root node.
    db.remove(10, 10);
    db.dump_tree_svg("./obj/pqt_test_remove_root-2.svg");
    expected.clear();
    expected.push_back(db_type::node_data(20, 0, &NE));
    expected.push_back(db_type::node_data(0, 0, &NW));
    expected.push_back(db_type::node_data(20, 20, &SE));
    expected.push_back(db_type::node_data(0, 20, &SW));
    success = db.verify_data(expected);
    TEST_ASSERT(success);
    TEST_ASSERT(db.size() == 4);
}

void pqt_test_equality()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef point_quad_tree<int32_t, const std::string*> db_type;
    db_type db1, db2;

    std::string A("A");
    std::string B("B");
    std::string C("C");
    std::string D("D");
    std::string E("E");
    std::string F("F");

    TEST_ASSERT(db1 == db2); // both are empty.

    db1.insert(0, 0, &A);
    db2.insert(0, 0, &A);
    TEST_ASSERT(db1 == db2);
    db1.remove(0, 0);
    TEST_ASSERT(db1 != db2);
    db1.insert(0, 0, &B);
    TEST_ASSERT(db1 != db2);
    db2.insert(0, 0, &B); // B overwrites A.
    TEST_ASSERT(db1 == db2); // Both should have B at (0,0).
    db1.insert(1, 1, &C);
    db2.insert(2, 2, &C);
    TEST_ASSERT(db1 != db2);
    db1.insert(2, 2, &C);
    db2.insert(1, 1, &C);
    TEST_ASSERT(db1 == db2);

    // Inserting data in different orders should make no difference in equality.
    db1.insert(1, 3, &D);
    db1.insert(1, 4, &E);
    db1.insert(1, 5, &F);

    db2.insert(1, 5, &F);
    db2.insert(1, 4, &E);
    db2.insert(1, 3, &D);
    TEST_ASSERT(db1 == db2);
    db1.remove(1, 4);
    db2.remove(1, 4);
    TEST_ASSERT(db1 == db2);

    // Make them empty again.
    db1.clear();
    db2.clear();
    TEST_ASSERT(db1 == db2);
}

void pqt_test_assignment()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef point_quad_tree<int32_t, const std::string*> db_type;
    db_type db1, db2;
    std::string A("A");
    std::string B("B");
    std::string C("C");
    std::string D("D");
    std::string E("E");
    std::string F("F");

    db1.insert(0, 10, &A);
    db1.insert(2, 5, &B);
    db1.insert(-10, 2, &C);
    db1.insert(5, 7, &D);
    std::vector<db_type::node_data> expected;
    expected.push_back(db_type::node_data(0, 10, &A));
    expected.push_back(db_type::node_data(2, 5, &B));
    expected.push_back(db_type::node_data(-10, 2, &C));
    expected.push_back(db_type::node_data(5, 7, &D));
    bool success = db1.verify_data(expected);
    TEST_ASSERT(success);

    db2 = db1;
    success = db2.verify_data(expected);
    TEST_ASSERT(success);
    success = db1.verify_data(expected);
    TEST_ASSERT(success);

    db2.insert(12, 45, &E);
    db2.insert(20, 42, &F);
    success = db2.verify_data(expected); // This should fail.
    TEST_ASSERT(!success);
    db2 = db1; // Assign once again.
    success = db2.verify_data(expected); // This now should succeed.
    TEST_ASSERT(success);
}

void pqt_test_swap()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef point_quad_tree<int32_t, const std::string*> db_type;
    db_type db1, db2;
    std::string A("A");
    std::string B("B");
    std::string C("C");
    std::string D("D");
    std::string E("E");
    std::string F("F");

    db1.insert(0, 10, &A);
    db1.insert(2, 5, &B);
    db1.insert(-10, 2, &C);
    db1.insert(5, 7, &D);
    std::vector<db_type::node_data> expected;
    expected.push_back(db_type::node_data(0, 10, &A));
    expected.push_back(db_type::node_data(2, 5, &B));
    expected.push_back(db_type::node_data(-10, 2, &C));
    expected.push_back(db_type::node_data(5, 7, &D));
    bool success = db1.verify_data(expected);
    TEST_ASSERT(success);
    TEST_ASSERT(db2.empty());

    db1.swap(db2);
    TEST_ASSERT(db1.empty());
    TEST_ASSERT(!db2.empty());
    success = db2.verify_data(expected);
    TEST_ASSERT(success);
}

template<typename _DbType>
bool verify_find(
    const _DbType& db, typename _DbType::key_type x, typename _DbType::key_type y,
    const typename _DbType::value_type data)
{
    try
    {
        typename _DbType::value_type found = db.find(x, y);
        cout << "found at (" << x << "," << y << "): " << found << endl;
        if (found == data)
            return true;
    }
    catch (const typename _DbType::data_not_found&)
    {
        cout << "nothing found at (" << x << "," << y << ")" << endl;
    }
    return false;
}

void pqt_test_find()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef point_quad_tree<int32_t, const std::string*> db_type;
    db_type db;
    std::string A("A");
    std::string B("B");
    std::string C("C");
    std::string D("D");
    std::string E("E");
    std::string F("F");
    db.insert(92, 27, &A);
    db.insert(53, 26, &B);
    db.insert(69, 18, &C);
    db.insert(0, 78, &D);
    db.insert(17, 7, &E);
    db.insert(91, 88, &F);
    TEST_ASSERT(db.size() == 6);
    db.dump_tree_svg("obj/pqt_test_find.svg");

    bool check;
    check = verify_find(db, 92, 27, &A);
    TEST_ASSERT(check);
    check = verify_find(db, 53, 26, &B);
    TEST_ASSERT(check);
    check = verify_find(db, 69, 18, &C);
    TEST_ASSERT(check);
    check = verify_find(db, 0, 78, &D);
    TEST_ASSERT(check);
    check = verify_find(db, 17, 7, &E);
    TEST_ASSERT(check);
    check = verify_find(db, 91, 88, &F);
    TEST_ASSERT(check);

    // Check for non-existent data.
    check = verify_find(db, 34, 86, &A);
    TEST_ASSERT(!check);
    check = verify_find(db, -1, 7, &A);
    TEST_ASSERT(!check);
    check = verify_find(db, 91, 27, &A);
    TEST_ASSERT(!check);
}

void pqt_test_node_access()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef point_quad_tree<int32_t, const std::string*> db_type;
    db_type db;
    db_type::node_access nac = db.get_node_access();
    TEST_ASSERT(!nac);
    std::string A("A");
    std::string B("B");
    std::string C("C");
    std::string D("D");
    std::string E("E");
    std::string F("F");
    db.insert(92, 27, &A);
    db.insert(53, 26, &B);
    db.insert(69, 18, &C);
    db.insert(0, 78, &D);
    db.insert(17, 7, &E);
    db.insert(91, 88, &F);
    TEST_ASSERT(db.size() == 6);

    nac = db.get_node_access();
    // Test root node.
    TEST_ASSERT(nac);
    TEST_ASSERT(nac.x() == 92);
    TEST_ASSERT(nac.y() == 27);
    TEST_ASSERT(nac.data() == &A);

    bool success = db.verify_node_iterator(nac);
    TEST_ASSERT(success);
}

int main()
{
    try
    {
        pqt_test_basic();
        pqt_test_insertion_removal();
        pqt_test_remove_root();
        pqt_test_equality();
        pqt_test_assignment();
        pqt_test_swap();
        pqt_test_find();
        pqt_test_node_access();
        TEST_ASSERT(get_node_instance_count() == 0);
    }
    catch (const std::exception& e)
    {
        cout << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
