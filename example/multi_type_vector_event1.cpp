
#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>
#include <iostream>

using namespace std;

class event_hdl
{
public:
    void element_block_acquired(mdds::mtv::base_element_block* block)
    {
        cout << "  * element block acquired" << endl;
    }

    void element_block_released(mdds::mtv::base_element_block* block)
    {
        cout << "  * element block released" << endl;
    }
};

using mtv_type = mdds::multi_type_vector<mdds::mtv::element_block_func, event_hdl>;

int main()
{
    mtv_type db;  // starts with an empty container.

    cout << "inserting string 'foo'..." << endl;
    db.push_back(string("foo"));  // creates a new string element block.

    cout << "inserting string 'bah'..." << endl;
    db.push_back(string("bah"));  // appends to an existing string block.

    cout << "inserting int 100..." << endl;
    db.push_back(int(100)); // creates a new int element block.

    cout << "emptying the container..." << endl;
    db.clear(); // releases both the string and int element blocks.

    cout << "exiting program..." << endl;

    return EXIT_SUCCESS;
}
