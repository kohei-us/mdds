// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.

#include <mdds/multi_type_matrix.hpp>

#include <string>
#include <ostream>
#include <tuple>
#include <utility>
#include <vector>

using namespace mdds;

// User-defined value types --------------------------------------------------

struct error_value
{
    int code = 0;

    bool operator==(const error_value&) const = default;
};

std::ostream& operator<<(std::ostream& os, const error_value& v)
{
    os << "(error: " << v.code << ")";
    return os;
}

struct note_value
{
    std::string text;

    bool operator==(const note_value&) const = default;
};

std::ostream& operator<<(std::ostream& os, const note_value& v)
{
    os << "(note: " << v.text << ")";
    return os;
}

constexpr mtv::element_t element_type_error = mtv::element_type_user_start;
constexpr mtv::element_t element_type_note = mtv::element_type_user_start + 1;

using error_element_block = mtv::default_element_block<element_type_error, error_value>;
using note_element_block = mtv::default_element_block<element_type_note, note_value>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(error_value, element_type_error, error_value{}, error_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(note_value, element_type_note, note_value{}, note_element_block)

// Matrix traits -------------------------------------------------------------

struct custom_traits
{
    using numeric_element_block = mtv::double_element_block;
    using element_blocks = std::tuple<error_element_block>;
};

struct two_custom_traits
{
    using numeric_element_block = mtv::double_element_block;
    using element_blocks = std::tuple<error_element_block, note_element_block>;
};

struct float_traits
{
    using numeric_element_block = mtv::float_element_block;
    using element_blocks = std::tuple<mtv::int64_element_block>;
};

struct numeric_only_traits
{
    using numeric_element_block = mtv::double_element_block;
};

using mtx_type = multi_type_matrix<custom_traits>;
using mtx2_type = multi_type_matrix<two_custom_traits>;
using float_mtx_type = multi_type_matrix<float_traits>;
using numeric_mtx_type = multi_type_matrix<numeric_only_traits>;
using std_mtx_type = multi_type_matrix<mtm::std_string_traits>;

// Compile-time checks -------------------------------------------------------

struct no_numeric_traits
{
    using element_blocks = std::tuple<error_element_block>;
};

struct non_tuple_traits
{
    using numeric_element_block = mtv::double_element_block;
    using element_blocks = error_element_block;
};

struct non_block_traits
{
    using numeric_element_block = mtv::double_element_block;
    using element_blocks = std::tuple<int>;
};

static_assert(mtm::matrix_traits<mtm::std_string_traits>);
static_assert(mtm::matrix_traits<custom_traits>);
static_assert(mtm::matrix_traits<two_custom_traits>);
static_assert(mtm::matrix_traits<float_traits>);
static_assert(mtm::matrix_traits<numeric_only_traits>);
static_assert(!mtm::matrix_traits<no_numeric_traits>);
static_assert(!mtm::matrix_traits<non_tuple_traits>);
static_assert(!mtm::matrix_traits<non_block_traits>);

static_assert(mtx_type::element_numeric == mtv::double_element_block::block_type);
static_assert(mtx_type::element_empty == mtv::element_type_empty);
static_assert(float_mtx_type::element_numeric == mtv::float_element_block::block_type);

template<typename MtxT, typename T>
concept settable = requires(MtxT& mtx, const T& v) { mtx.set(0, 0, v); };

static_assert(settable<mtx_type, error_value>);
static_assert(!settable<mtx_type, note_value>);
static_assert(settable<mtx2_type, note_value>);
static_assert(!settable<numeric_mtx_type, error_value>);

template<typename MtxT, typename T>
concept referenceable = requires(const MtxT& mtx) { mtx.template get_ref<T>(0, 0); };

static_assert(referenceable<mtx_type, error_value>);
static_assert(referenceable<mtx_type, double>);
static_assert(!referenceable<mtx_type, note_value>); // not listed
static_assert(!referenceable<numeric_mtx_type, error_value>);
static_assert(!referenceable<std_mtx_type, bool>); // std::vector<bool> store

// Tests ---------------------------------------------------------------------

void mtm_test_custom_construction()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        mtx_type mtx;
        TEST_ASSERT(mtx.size().row == 0);
        TEST_ASSERT(mtx.size().column == 0);
        TEST_ASSERT(mtx.empty());
    }

    {
        mtx_type mtx(2, 2);
        TEST_ASSERT(mtx.size().row == 2);
        TEST_ASSERT(mtx.size().column == 2);
        TEST_ASSERT(mtx.get_type(0, 0) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(1, 0) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(0, 1) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(1, 1) == mtm::element_empty);
    }

    {
        mtx_type mtx(2, 2, error_value{7});
        TEST_ASSERT(mtx.get_type(0, 0) == error_element_block::block_type);
        TEST_ASSERT(mtx.get_type(1, 0) == error_element_block::block_type);
        TEST_ASSERT(mtx.get_type(0, 1) == error_element_block::block_type);
        TEST_ASSERT(mtx.get_type(1, 1) == error_element_block::block_type);
        TEST_ASSERT(mtx.get<error_value>(0, 0) == error_value{7});
        TEST_ASSERT(mtx.get<error_value>(1, 0) == error_value{7});
        TEST_ASSERT(mtx.get<error_value>(0, 1) == error_value{7});
        TEST_ASSERT(mtx.get<error_value>(1, 1) == error_value{7});
    }

    {
        std::vector<error_value> vals = {{1}, {2}, {3}, {4}};
        mtx_type mtx(2, 2, vals.begin(), vals.end());
        TEST_ASSERT(mtx.get_type(0, 0) == error_element_block::block_type);
        TEST_ASSERT(mtx.get_type(1, 0) == error_element_block::block_type);
        TEST_ASSERT(mtx.get_type(0, 1) == error_element_block::block_type);
        TEST_ASSERT(mtx.get_type(1, 1) == error_element_block::block_type);
        TEST_ASSERT(mtx.get<error_value>(0, 0) == error_value{1});
        TEST_ASSERT(mtx.get<error_value>(1, 0) == error_value{2});
        TEST_ASSERT(mtx.get<error_value>(0, 1) == error_value{3});
        TEST_ASSERT(mtx.get<error_value>(1, 1) == error_value{4});
    }

    {
        // A value type not listed in the traits type is rejected.
        std::vector<note_value> vals = {{"a"}, {"b"}};

        try
        {
            mtx_type mtx(1, 2, vals.begin(), vals.end());
            TEST_ASSERT(!"type_error did not get thrown.");
        }
        catch (const mdds::type_error& e)
        {
            cout << "expected exception was thrown: '" << e.what() << "'" << endl;
        }
    }
}

void mtm_test_custom_set_get()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mtx(3, 2);
    mtx.set(0, 0, error_value{1});
    mtx.set(1, 0, 1.5);
    mtx.set(2, 0, error_value{3});

    TEST_ASSERT(mtx.get_type(0, 0) == error_element_block::block_type);
    TEST_ASSERT(mtx.get<error_value>(0, 0) == error_value{1});
    TEST_ASSERT(mtx.get_type(1, 0) == mtx_type::element_numeric);
    TEST_ASSERT(mtx.get<double>(1, 0) == 1.5);
    TEST_ASSERT(mtx.get_type(2, 0) == error_element_block::block_type);
    TEST_ASSERT(mtx.get<error_value>(2, 0) == error_value{3});

    // Overwrite a numeric element with a custom value, and back.
    mtx.set(1, 0, error_value{2});
    TEST_ASSERT(mtx.get_type(1, 0) == error_element_block::block_type);
    TEST_ASSERT(mtx.get<error_value>(1, 0) == error_value{2});
    mtx.set(1, 0, 2.5);
    TEST_ASSERT(mtx.get_type(1, 0) == mtx_type::element_numeric);
    TEST_ASSERT(mtx.get<double>(1, 0) == 2.5);

    // Position forms, using each returned position as the hint for the next call.
    mtx_type::position_type pos = mtx.position(0, 1);
    pos = mtx.set(pos, error_value{4});
    pos = mtx.position(pos, 1, 1);
    pos = mtx.set(pos, 5.5);
    pos = mtx.position(pos, 2, 1);
    pos = mtx.set(pos, error_value{6});

    TEST_ASSERT(mtx.get_type(0, 1) == error_element_block::block_type);
    TEST_ASSERT(mtx.get<error_value>(0, 1) == error_value{4});
    TEST_ASSERT(mtx.get_type(1, 1) == mtx_type::element_numeric);
    TEST_ASSERT(mtx.get<double>(1, 1) == 5.5);
    TEST_ASSERT(mtx.get_type(2, 1) == error_element_block::block_type);
    TEST_ASSERT(mtx.get<error_value>(2, 1) == error_value{6});

    mtx_type::const_position_type cpos = std::as_const(mtx).position(0, 1);
    TEST_ASSERT(mtx.get_type(cpos) == error_element_block::block_type);
    TEST_ASSERT(mtx.get<error_value>(cpos) == error_value{4});
    cpos = std::as_const(mtx).position(cpos, 1, 1);
    TEST_ASSERT(mtx.get<double>(cpos) == 5.5);

    // An empty element returns the empty value of the requested type.
    mtx.set_empty(2, 1);
    TEST_ASSERT(mtx.get_type(2, 1) == mtm::element_empty);
    TEST_ASSERT(mtx.get<error_value>(2, 1) == error_value{});
    TEST_ASSERT(mtx.get<double>(2, 1) == 0.0);

    // A type mismatch throws.
    try
    {
        mtx.get<double>(0, 1);
        TEST_ASSERT(!"type_error did not get thrown.");
    }
    catch (const mdds::type_error& e)
    {
        cout << "expected exception was thrown: '" << e.what() << "'" << endl;
    }

    try
    {
        cpos = std::as_const(mtx).position(1, 0);
        mtx.get<error_value>(cpos);
        TEST_ASSERT(!"type_error did not get thrown.");
    }
    catch (const mdds::type_error& e)
    {
        cout << "expected exception was thrown: '" << e.what() << "'" << endl;
    }
}

void mtm_test_custom_get_ref()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mtx(3, 1);
    mtx.set(0, 0, error_value{5});
    mtx.set(1, 0, error_value{6});
    mtx.set(2, 0, 1.5);

    TEST_ASSERT(mtx.get_ref<error_value>(0, 0) == error_value{5});
    TEST_ASSERT(mtx.get_ref<error_value>(1, 0) == error_value{6});
    TEST_ASSERT(mtx.get_ref<double>(2, 0) == 1.5);

    // Both forms reference the stored value itself.
    mtx_type::const_position_type pos = std::as_const(mtx).position(1, 0);
    const error_value& ref1 = mtx.get_ref<error_value>(1, 0);
    const error_value& ref2 = mtx.get_ref<error_value>(pos);
    TEST_ASSERT(&ref1 == &ref2);
    TEST_ASSERT(&ref1 == &error_element_block::at(*pos.first->data, pos.second));

    // The value type must match.
    try
    {
        mtx.get_ref<double>(0, 0);
        TEST_ASSERT(!"type_error did not get thrown.");
    }
    catch (const mdds::type_error& e)
    {
        cout << "expected exception was thrown: '" << e.what() << "'" << endl;
    }

    // An empty element has no value to reference.
    mtx.set_empty(0, 0);

    try
    {
        mtx.get_ref<error_value>(0, 0);
        TEST_ASSERT(!"type_error did not get thrown.");
    }
    catch (const mdds::type_error& e)
    {
        cout << "expected exception was thrown: '" << e.what() << "'" << endl;
    }

    try
    {
        pos = std::as_const(mtx).position(0, 0);
        mtx.get_ref<error_value>(pos);
        TEST_ASSERT(!"type_error did not get thrown.");
    }
    catch (const mdds::type_error& e)
    {
        cout << "expected exception was thrown: '" << e.what() << "'" << endl;
    }
}

void mtm_test_custom_range()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        // Wrap across a column boundary.
        mtx_type mtx(2, 3);
        std::vector<error_value> vals = {{1}, {2}, {3}};
        mtx.set(1, 0, vals.begin(), vals.end());

        TEST_ASSERT(mtx.get_type(0, 0) == mtm::element_empty);
        TEST_ASSERT(mtx.get<error_value>(1, 0) == error_value{1});
        TEST_ASSERT(mtx.get<error_value>(0, 1) == error_value{2});
        TEST_ASSERT(mtx.get<error_value>(1, 1) == error_value{3});
        TEST_ASSERT(mtx.get_type(0, 2) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(1, 2) == mtm::element_empty);
    }

    {
        mtx_type mtx(2, 2);
        std::vector<error_value> vals = {{5}, {6}, {7}};
        mtx.set_column(1, vals.begin(), vals.end());

        TEST_ASSERT(mtx.get_type(0, 0) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(1, 0) == mtm::element_empty);
        TEST_ASSERT(mtx.get<error_value>(0, 1) == error_value{5});
        TEST_ASSERT(mtx.get<error_value>(1, 1) == error_value{6});
    }

    {
        mtx_type mtx(3, 3, 0.5);
        std::vector<error_value> vals = {{1}, {2}, {3}, {4}};
        mtx.copy(2, 2, vals.begin(), vals.end());

        TEST_ASSERT(mtx.get<error_value>(0, 0) == error_value{1});
        TEST_ASSERT(mtx.get<error_value>(1, 0) == error_value{2});
        TEST_ASSERT(mtx.get<double>(2, 0) == 0.5);
        TEST_ASSERT(mtx.get<error_value>(0, 1) == error_value{3});
        TEST_ASSERT(mtx.get<error_value>(1, 1) == error_value{4});
        TEST_ASSERT(mtx.get<double>(2, 1) == 0.5);
        TEST_ASSERT(mtx.get<double>(0, 2) == 0.5);
        TEST_ASSERT(mtx.get<double>(1, 2) == 0.5);
        TEST_ASSERT(mtx.get<double>(2, 2) == 0.5);

        // A value type not listed in the traits type is rejected.
        std::vector<note_value> notes = {{"a"}, {"b"}};

        try
        {
            mtx.copy(2, 1, notes.begin(), notes.end());
            TEST_ASSERT(!"type_error did not get thrown.");
        }
        catch (const mdds::type_error& e)
        {
            cout << "expected exception was thrown: '" << e.what() << "'" << endl;
        }
    }
}

namespace {

/**
 * Build a 3x2 matrix with mixed element blocks.  The two error values in
 * the second column share one block.
 *
 * +-----------+-----------+
 * | error{1}  | 2.5       |
 * +-----------+-----------+
 * | 1.5       | error{2}  |
 * +-----------+-----------+
 * | (empty)   | error{3}  |
 * +-----------+-----------+
 */
mtx_type create_mixed_matrix()
{
    mtx_type mtx(3, 2);
    mtx.set(0, 0, error_value{1});
    mtx.set(1, 0, 1.5);
    mtx.set(0, 1, 2.5);
    mtx.set(1, 1, error_value{2});
    mtx.set(2, 1, error_value{3});
    return mtx;
}

} // namespace

void mtm_test_custom_transpose()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mtx = create_mixed_matrix();
    mtx.transpose();

    TEST_ASSERT(mtx.size().row == 2);
    TEST_ASSERT(mtx.size().column == 3);
    TEST_ASSERT(mtx.get_type(0, 0) == error_element_block::block_type);
    TEST_ASSERT(mtx.get<error_value>(0, 0) == error_value{1});
    TEST_ASSERT(mtx.get_type(0, 1) == mtx_type::element_numeric);
    TEST_ASSERT(mtx.get<double>(0, 1) == 1.5);
    TEST_ASSERT(mtx.get_type(0, 2) == mtm::element_empty);
    TEST_ASSERT(mtx.get_type(1, 0) == mtx_type::element_numeric);
    TEST_ASSERT(mtx.get<double>(1, 0) == 2.5);
    TEST_ASSERT(mtx.get_type(1, 1) == error_element_block::block_type);
    TEST_ASSERT(mtx.get<error_value>(1, 1) == error_value{2});
    TEST_ASSERT(mtx.get_type(1, 2) == error_element_block::block_type);
    TEST_ASSERT(mtx.get<error_value>(1, 2) == error_value{3});
}

void mtm_test_custom_copy()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type src = create_mixed_matrix();
    mtx_type dest(4, 3, error_value{9});
    dest.copy(src);

    TEST_ASSERT(dest.get_type(0, 0) == error_element_block::block_type);
    TEST_ASSERT(dest.get<error_value>(0, 0) == error_value{1});
    TEST_ASSERT(dest.get_type(1, 0) == mtx_type::element_numeric);
    TEST_ASSERT(dest.get<double>(1, 0) == 1.5);
    TEST_ASSERT(dest.get_type(2, 0) == mtm::element_empty);
    TEST_ASSERT(dest.get<error_value>(3, 0) == error_value{9});
    TEST_ASSERT(dest.get_type(0, 1) == mtx_type::element_numeric);
    TEST_ASSERT(dest.get<double>(0, 1) == 2.5);
    TEST_ASSERT(dest.get_type(1, 1) == error_element_block::block_type);
    TEST_ASSERT(dest.get<error_value>(1, 1) == error_value{2});
    TEST_ASSERT(dest.get_type(2, 1) == error_element_block::block_type);
    TEST_ASSERT(dest.get<error_value>(2, 1) == error_value{3});
    TEST_ASSERT(dest.get<error_value>(3, 1) == error_value{9});
    TEST_ASSERT(dest.get<error_value>(0, 2) == error_value{9});
    TEST_ASSERT(dest.get<error_value>(1, 2) == error_value{9});
    TEST_ASSERT(dest.get<error_value>(2, 2) == error_value{9});
    TEST_ASSERT(dest.get<error_value>(3, 2) == error_value{9});
}

void mtm_test_custom_resize()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mtx = create_mixed_matrix();

    mtx.resize(4, 3);
    TEST_ASSERT(mtx.size().row == 4);
    TEST_ASSERT(mtx.size().column == 3);
    TEST_ASSERT(mtx.get<error_value>(0, 0) == error_value{1});
    TEST_ASSERT(mtx.get<double>(1, 0) == 1.5);
    TEST_ASSERT(mtx.get_type(2, 0) == mtm::element_empty);
    TEST_ASSERT(mtx.get_type(3, 0) == mtm::element_empty);
    TEST_ASSERT(mtx.get<double>(0, 1) == 2.5);
    TEST_ASSERT(mtx.get<error_value>(1, 1) == error_value{2});
    TEST_ASSERT(mtx.get<error_value>(2, 1) == error_value{3});
    TEST_ASSERT(mtx.get_type(3, 1) == mtm::element_empty);
    TEST_ASSERT(mtx.get_type(0, 2) == mtm::element_empty);
    TEST_ASSERT(mtx.get_type(1, 2) == mtm::element_empty);
    TEST_ASSERT(mtx.get_type(2, 2) == mtm::element_empty);
    TEST_ASSERT(mtx.get_type(3, 2) == mtm::element_empty);

    mtx.resize(1, 2);
    TEST_ASSERT(mtx.size().row == 1);
    TEST_ASSERT(mtx.size().column == 2);
    TEST_ASSERT(mtx.get<error_value>(0, 0) == error_value{1});
    TEST_ASSERT(mtx.get<double>(0, 1) == 2.5);
}

void mtm_test_custom_walk()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mtx = create_mixed_matrix();

    {
        std::vector<mtx_type::element_block_node_type> nodes;
        mtx.walk([&nodes](const mtx_type::element_block_node_type& node) { nodes.push_back(node); });

        TEST_ASSERT(nodes.size() == 5);
        TEST_ASSERT(nodes[0].type == error_element_block::block_type);
        TEST_ASSERT(nodes[0].size == 1);
        TEST_ASSERT(*nodes[0].begin<error_element_block>() == error_value{1});
        TEST_ASSERT(nodes[1].type == mtx_type::element_numeric);
        TEST_ASSERT(nodes[1].size == 1);
        TEST_ASSERT(nodes[2].type == mtm::element_empty);
        TEST_ASSERT(nodes[2].size == 1);
        TEST_ASSERT(nodes[3].type == mtx_type::element_numeric);
        TEST_ASSERT(nodes[3].size == 1);
        TEST_ASSERT(nodes[4].type == error_element_block::block_type);
        TEST_ASSERT(nodes[4].size == 2);

        auto it = nodes[4].begin<error_element_block>();
        auto ite = nodes[4].end<error_element_block>();
        TEST_ASSERT(*it == error_value{2});
        ++it;
        TEST_ASSERT(*it == error_value{3});
        ++it;
        TEST_ASSERT(it == ite);
    }

    {
        // Sub-range covering only row 2 of column 1, which is the second
        // element of the error block.
        std::vector<mtx_type::element_block_node_type> nodes;
        mtx.walk([&nodes](const mtx_type::element_block_node_type& node) { nodes.push_back(node); }, {2, 1}, {2, 1});

        TEST_ASSERT(nodes.size() == 1);
        TEST_ASSERT(nodes[0].type == error_element_block::block_type);
        TEST_ASSERT(nodes[0].offset == 1);
        TEST_ASSERT(nodes[0].size == 1);
        TEST_ASSERT(*nodes[0].begin<error_element_block>() == error_value{3});
    }

    {
        // Parallel walk against a matrix filled with one error value.
        mtx_type right(3, 2, error_value{9});
        std::vector<mtx_type::element_block_node_type> lnodes, rnodes;
        mtx.walk(
            [&](const mtx_type::element_block_node_type& l, const mtx_type::element_block_node_type& r) {
                lnodes.push_back(l);
                rnodes.push_back(r);
            },
            right);

        TEST_ASSERT(lnodes.size() == 5);
        TEST_ASSERT(rnodes.size() == 5);
        TEST_ASSERT(lnodes[0].type == error_element_block::block_type);
        TEST_ASSERT(lnodes[4].type == error_element_block::block_type);
        TEST_ASSERT(lnodes[4].size == 2);
        TEST_ASSERT(rnodes[0].type == error_element_block::block_type);
        TEST_ASSERT(rnodes[0].offset == 0);
        TEST_ASSERT(rnodes[0].size == 1);
        TEST_ASSERT(*rnodes[0].begin<error_element_block>() == error_value{9});
        TEST_ASSERT(rnodes[4].type == error_element_block::block_type);
        TEST_ASSERT(rnodes[4].offset == 4);
        TEST_ASSERT(rnodes[4].size == 2);
        TEST_ASSERT(*rnodes[4].begin<error_element_block>() == error_value{9});
    }

    {
        // Parallel walk over a sub-range covering rows 0-1 of column 0.
        mtx_type right(3, 2, error_value{9});
        std::vector<mtx_type::element_block_node_type> lnodes, rnodes;
        mtx.walk(
            [&](const mtx_type::element_block_node_type& l, const mtx_type::element_block_node_type& r) {
                lnodes.push_back(l);
                rnodes.push_back(r);
            },
            right, {0, 0}, {1, 0});

        TEST_ASSERT(lnodes.size() == 2);
        TEST_ASSERT(lnodes[0].type == error_element_block::block_type);
        TEST_ASSERT(lnodes[1].type == mtx_type::element_numeric);
        TEST_ASSERT(rnodes.size() == 2);
        TEST_ASSERT(rnodes[0].type == error_element_block::block_type);
        TEST_ASSERT(rnodes[0].offset == 0);
        TEST_ASSERT(rnodes[1].type == error_element_block::block_type);
        TEST_ASSERT(rnodes[1].offset == 1);
        TEST_ASSERT(*rnodes[1].begin<error_element_block>() == error_value{9});
    }
}

void mtm_test_custom_get_numeric()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mtx(3, 1);
    mtx.set(0, 0, 2.5);
    mtx.set(2, 0, error_value{1});

    TEST_ASSERT(mtx.get_numeric(0, 0) == 2.5);
    TEST_ASSERT(mtx.get_numeric(1, 0) == 0.0);

    try
    {
        mtx.get_numeric(2, 0);
        TEST_ASSERT(!"type_error did not get thrown.");
    }
    catch (const mdds::type_error&)
    {
        // Custom element is not numeric.
    }

    try
    {
        mtx.get_numeric(std::as_const(mtx).position(2, 0));
        TEST_ASSERT(!"type_error did not get thrown.");
    }
    catch (const mdds::type_error&)
    {
        // Custom element is not numeric.
    }
}

void mtm_test_custom_numeric()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mtx(2, 2, 1.0);
    TEST_ASSERT(mtx.numeric());

    mtx.set(1, 1, error_value{1});
    TEST_ASSERT(!mtx.numeric());

    mtx.set(1, 1, 2.0);
    TEST_ASSERT(mtx.numeric());

    mtx.clear();
    TEST_ASSERT(!mtx.numeric());
}

void mtm_test_custom_all_of()
{
    MDDS_TEST_FUNC_SCOPE;

    // A custom block counts like any other.
    mtx_type mtx(2, 2, 1.0);
    TEST_ASSERT(mtx.all_of<mtv::double_element_block>());
    TEST_ASSERT(!mtx.all_of<error_element_block>());

    mtx.set(0, 1, error_value{1});
    TEST_ASSERT(!mtx.all_of<mtv::double_element_block>());
    TEST_ASSERT(!mtx.all_of<error_element_block>());
    TEST_ASSERT((mtx.all_of<mtv::double_element_block, error_element_block>()));

    mtx.set(0, 0, error_value{2});
    mtx.set(1, 0, error_value{3});
    mtx.set(1, 1, error_value{4});
    TEST_ASSERT(mtx.all_of<error_element_block>());
}

void mtm_test_custom_float_numeric()
{
    MDDS_TEST_FUNC_SCOPE;

    float_mtx_type mtx(2, 2);
    mtx.set(0, 0, 1.5f);
    mtx.set(1, 0, int64_t(-3));

    TEST_ASSERT(mtx.get_type(0, 0) == mtv::element_type_float);
    TEST_ASSERT(mtx.get_numeric(0, 0) == 1.5f);
    TEST_ASSERT(mtx.get<float>(0, 0) == 1.5f);
    TEST_ASSERT(mtx.get_type(1, 0) == mtv::element_type_int64);
    TEST_ASSERT(mtx.get<int64_t>(1, 0) == -3);
    TEST_ASSERT(mtx.get_numeric(0, 1) == 0.0f);

    // A double converts to the numeric type.
    mtx.set(1, 1, 2.5);
    TEST_ASSERT(mtx.get_type(1, 1) == mtv::element_type_float);
    TEST_ASSERT(mtx.get_numeric(1, 1) == 2.5f);
}

void mtm_test_custom_numeric_only()
{
    MDDS_TEST_FUNC_SCOPE;

    numeric_mtx_type mtx(2, 1);
    mtx.set(0, 0, 3.5);
    TEST_ASSERT(mtx.get_type(0, 0) == numeric_mtx_type::element_numeric);
    TEST_ASSERT(mtx.get_numeric(0, 0) == 3.5);
    TEST_ASSERT(mtx.get_type(1, 0) == mtm::element_empty);
    TEST_ASSERT(mtx.get_numeric(1, 0) == 0.0);

    // Without a boolean block, a bool converts to the numeric type.
    mtx.set(1, 0, true);
    TEST_ASSERT(mtx.get_type(1, 0) == numeric_mtx_type::element_numeric);
    TEST_ASSERT(mtx.get_numeric(1, 0) == 1.0);
    TEST_ASSERT(mtx.numeric());
}

void mtm_test_custom_equality_swap_clear()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mtx1 = create_mixed_matrix();
    mtx_type mtx2 = create_mixed_matrix();
    TEST_ASSERT(mtx1 == mtx2);

    mtx2.set(2, 1, error_value{4});
    TEST_ASSERT(mtx1 != mtx2);

    mtx_type mtx3(1, 1, error_value{5});
    mtx3.swap(mtx2);
    TEST_ASSERT(mtx2.size().row == 1);
    TEST_ASSERT(mtx2.size().column == 1);
    TEST_ASSERT(mtx2.get<error_value>(0, 0) == error_value{5});
    TEST_ASSERT(mtx3.size().row == 3);
    TEST_ASSERT(mtx3.size().column == 2);
    TEST_ASSERT(mtx3.get<error_value>(2, 1) == error_value{4});

    mtx3.clear();
    TEST_ASSERT(mtx3.empty());
    TEST_ASSERT(mtx3.size().row == 0);
    TEST_ASSERT(mtx3.size().column == 0);
}

void mtm_test_custom_two_blocks()
{
    MDDS_TEST_FUNC_SCOPE;

    static_assert(error_element_block::block_type != note_element_block::block_type);

    mtx2_type mtx(3, 1);
    mtx.set(0, 0, error_value{1});
    mtx.set(1, 0, note_value{"memo"});
    mtx.set(2, 0, 4.5);

    TEST_ASSERT(mtx.get_type(0, 0) == error_element_block::block_type);
    TEST_ASSERT(mtx.get<error_value>(0, 0) == error_value{1});
    TEST_ASSERT(mtx.get_type(1, 0) == note_element_block::block_type);
    TEST_ASSERT(mtx.get<note_value>(1, 0) == note_value{"memo"});
    TEST_ASSERT(mtx.get_type(2, 0) == mtx2_type::element_numeric);
    TEST_ASSERT(mtx.get<double>(2, 0) == 4.5);

    std::vector<mtx2_type::element_block_node_type> nodes;
    mtx.walk([&nodes](const mtx2_type::element_block_node_type& node) { nodes.push_back(node); });

    TEST_ASSERT(nodes.size() == 3);
    TEST_ASSERT(nodes[0].type == error_element_block::block_type);
    TEST_ASSERT(*nodes[0].begin<error_element_block>() == error_value{1});
    TEST_ASSERT(nodes[1].type == note_element_block::block_type);
    TEST_ASSERT(*nodes[1].begin<note_element_block>() == note_value{"memo"});
    TEST_ASSERT(nodes[2].type == mtx2_type::element_numeric);
}

int main(int argc, char** argv)
{
    try
    {
        cmd_options opt;
        if (!parse_cmd_options(argc, argv, opt))
            return EXIT_FAILURE;

        if (opt.test_func)
        {
            mtm_test_custom_construction();
            mtm_test_custom_set_get();
            mtm_test_custom_get_ref();
            mtm_test_custom_range();
            mtm_test_custom_transpose();
            mtm_test_custom_copy();
            mtm_test_custom_resize();
            mtm_test_custom_walk();
            mtm_test_custom_get_numeric();
            mtm_test_custom_numeric();
            mtm_test_custom_all_of();
            mtm_test_custom_float_numeric();
            mtm_test_custom_numeric_only();
            mtm_test_custom_equality_swap_clear();
            mtm_test_custom_two_blocks();
        }
    }
    catch (const std::exception& e)
    {
        cerr << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
