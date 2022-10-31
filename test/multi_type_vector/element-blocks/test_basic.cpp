
#include "test_main.hpp"

#include <mdds/multi_type_vector/types.hpp>
#include <mdds/multi_type_vector.hpp>

constexpr mdds::mtv::element_t type_1 = mdds::mtv::element_type_user_start;
constexpr mdds::mtv::element_t type_2 = mdds::mtv::element_type_user_start + 1;
constexpr mdds::mtv::element_t type_3 = mdds::mtv::element_type_user_start + 2;
constexpr mdds::mtv::element_t type_4 = mdds::mtv::element_type_user_start + 3;

struct my_type_1
{
};
struct my_type_2
{
};
struct my_type_3
{
};

// Make sure these three block types can be instantiated without compile errors.
using my_block1 = mdds::mtv::default_element_block<type_1, my_type_1>;
using my_block2 = mdds::mtv::managed_element_block<type_2, my_type_2>;
using my_block3 = mdds::mtv::noncopyable_managed_element_block<type_3, my_type_3>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(my_type_1, type_1, my_type_1{}, my_block1)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(my_type_2, type_2, nullptr, my_block2)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(my_type_3, type_3, nullptr, my_block3)

struct my_trait : mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<my_block1, my_block2, my_block3>;
};

using mtv_type = mdds::multi_type_vector<my_trait>;

void mtv_test_element_blocks_buildability()
{
    stack_printer __stack_printer__(__func__);

    mtv_type con(20);
    con.set(0, my_type_1{});
    con.set(1, new my_type_2);
    con.set(2, new my_type_3);
}
