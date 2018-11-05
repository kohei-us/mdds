
#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>
#include <iostream>

using namespace std;
using mdds::mtv::double_element_block;
using mdds::mtv::string_element_block;

using mtv_type = mdds::multi_type_vector<mdds::mtv::element_block_func>;

int main()
{
    mtv_type db;  // starts with an empty container.

    db.push_back(1.1);
    db.push_back(1.2);
    db.push_back(1.3);
    db.push_back(1.4);
    db.push_back(1.5);

    db.push_back(string("A"));
    db.push_back(string("B"));
    db.push_back(string("C"));
    db.push_back(string("D"));
    db.push_back(string("E"));

    // At this point, you have 2 blocks in the container.
    cout << "block size: " << db.block_size() << endl;
    cout << "--" << endl;

    // Get an iterator that points to the first block in the primary array.
    mtv_type::const_iterator it = db.begin();

    // Get a pointer to the raw array of the numeric element block using the
    // 'data' method.
    const double* p = double_element_block::data(*it->data);

    // Print the elements from this raw array pointer.
    for (const double* p_end = p + it->size; p != p_end; ++p)
        cout << *p << endl;

    cout << "--" << endl;

    ++it; // move to the next block, which is a string block.

    // Get a pointer to the raw array of the string element block.
    const string* pz = string_element_block::data(*it->data);

    // Print out the string elements.
    for (const string* pz_end = pz + it->size; pz != pz_end; ++pz)
        cout << *pz << endl;

    return EXIT_SUCCESS;
}

