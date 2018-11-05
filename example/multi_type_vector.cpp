
#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>
#include <iostream>
#include <vector>
#include <string>

using std::cout;
using std::endl;

using mtv_type = mdds::multi_type_vector<mdds::mtv::element_block_func>;

template<typename _Blk>
void print_block(const mtv_type::value_type& v)
{
    // Each element block has static begin() and end() methods that return
    // begin and end iterators, respectively, from the passed element block
    // instance.
    auto it = _Blk::begin(*v.data);
    auto it_end = _Blk::end(*v.data);

    std::for_each(it, it_end,
        [](const typename _Blk::value_type& elem)
        {
            cout << " * " << elem << endl;
        }
    );
}

int main()
{
    mtv_type con(20); // Initialized with 20 empty elements.

    // Set values individually.
    con.set(0, 1.1);
    con.set(1, 1.2);
    con.set(2, 1.3);

    // Set a sequence of values in one step.
    std::vector<double> vals = { 10.1, 10.2, 10.3, 10.4, 10.5 };
    con.set(3, vals.begin(), vals.end());

    // Set string values.
    con.set(10, std::string("Andy"));
    con.set(11, std::string("Bruce"));
    con.set(12, std::string("Charlie"));

    // Iterate through all blocks and print all elements.
    for (const mtv_type::value_type& v : con)
    {
        switch (v.type)
        {
            case mdds::mtv::element_type_double:
            {
                cout << "numeric block of size " << v.size << endl;
                print_block<mdds::mtv::double_element_block>(v);
                break;
            }
            case mdds::mtv::element_type_string:
            {
                cout << "string block of size " << v.size << endl;
                print_block<mdds::mtv::string_element_block>(v);
                break;
            }
            case mdds::mtv::element_type_empty:
                cout << "empty block of size " << v.size << endl;
                cout << " - no data - " << endl;
            default:
                ;
        }
    }

    return EXIT_SUCCESS;
}
