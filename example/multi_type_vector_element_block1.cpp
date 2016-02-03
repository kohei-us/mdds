
#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using mdds::mtv::numeric_element_block;
using mdds::mtv::string_element_block;

typedef mdds::multi_type_vector<mdds::mtv::element_block_func> mtv_type;

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
    // 'at' method and taking the address of the returned reference.
    const double* p = &numeric_element_block::at(*it->data, 0);

    // Print the elements from this raw array pointer.
    for (const double* p_end = p + it->size; p != p_end; ++p)
        cout << *p << endl;

    cout << "--" << endl;

    ++it; // move to the next block, which is a string block.

    // Get a pointer to the raw array of the string element block.
    const string* pz = &string_element_block::at(*it->data, 0);

    // Print out the string elements.
    for (const string* pz_end = pz + it->size; pz != pz_end; ++pz)
        cout << *pz << endl;

    return EXIT_SUCCESS;
}

