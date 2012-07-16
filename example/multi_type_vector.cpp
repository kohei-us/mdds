
#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>
#include <iostream>
#include <vector>
#include <string>

using std::cout;
using std::endl;

typedef mdds::multi_type_vector<mdds::mtv::element_block_func> mtv_type;

template<typename _Blk>
struct print_elements
{
    void operator() (const typename _Blk::value_type& v) const
    {
        cout << " * " << v << endl;
    }
};

int main()
{
    mtv_type con(20); // Initialized with 20 empty elements.

    // Set values individually.
    con.set(0, 1.1);
    con.set(1, 1.2);
    con.set(2, 1.3);

    // Set a sequence of values in one step.
    double vals[] = { 10.1, 10.2, 10.3, 10.4, 10.5 };
    double* p = &vals[0];
    con.set(3, p, p + 5);

    // Set string values.
    con.set(10, std::string("Andy"));
    con.set(11, std::string("Bruce"));
    con.set(12, std::string("Charlie"));

    // Iterate through elements.
    mtv_type::const_iterator it_blk = con.begin(), it_blk_end = con.end();
    for (; it_blk != it_blk_end; ++it_blk)
    {
        switch (it_blk->type)
        {
            case mdds::mtv::element_type_numeric:
            {
                cout << "numeric block of size " << it_blk->size << endl;
                mdds::mtv::numeric_element_block::const_iterator it =
                    mdds::mtv::numeric_element_block::begin(*it_blk->data);
                mdds::mtv::numeric_element_block::const_iterator it_end =
                    mdds::mtv::numeric_element_block::end(*it_blk->data);
                std::for_each(it, it_end, print_elements<mdds::mtv::numeric_element_block>());
            }
            break;
            case mdds::mtv::element_type_string:
            {
                cout << "string block of size " << it_blk->size << endl;
                mdds::mtv::string_element_block::const_iterator it =
                    mdds::mtv::string_element_block::begin(*it_blk->data);
                mdds::mtv::string_element_block::const_iterator it_end =
                    mdds::mtv::string_element_block::end(*it_blk->data);
                std::for_each(it, it_end, print_elements<mdds::mtv::string_element_block>());
            }
            case mdds::mtv::element_type_empty:
                cout << "empty block of size " << it_blk->size << endl;
                cout << " - no data - " << endl;
            default:
                ;
        }
    }
}
