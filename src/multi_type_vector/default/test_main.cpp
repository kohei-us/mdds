/*************************************************************************
 *
 * Copyright (c) 2011-2018 Kohei Yoshida
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************/

#include "test_global.hpp" // This must be the first header to be included.
#include "test_main.hpp"

using namespace std;
using namespace mdds;

namespace {

enum test_mtv_type {
    _mtv_bool, _mtv_int8, _mtv_uint8, _mtv_int16, _mtv_uint16, _mtv_int32, _mtv_uint32, _mtv_int64, _mtv_uint64, _mtv_float, _mtv_double, _mtv_string,
};

#define TEST_TYPE(_type_,_type_enum_) test_mtv_type test_type(_type_) { return _type_enum_; }
TEST_TYPE(bool,     _mtv_bool)
TEST_TYPE(int8_t,   _mtv_int8)
TEST_TYPE(uint8_t,  _mtv_uint8)
TEST_TYPE(int16_t,  _mtv_int16)
TEST_TYPE(uint16_t, _mtv_uint16)
TEST_TYPE(int32_t,  _mtv_int32)
TEST_TYPE(uint32_t, _mtv_uint32)
TEST_TYPE(int64_t,  _mtv_int64)
TEST_TYPE(uint64_t, _mtv_uint64)
TEST_TYPE(float,    _mtv_float)
TEST_TYPE(double,   _mtv_double)
TEST_TYPE(string,   _mtv_string)

void mtv_test_types()
{
    // Test function overloading of standard types.

    stack_printer __stack_printer__(__FUNCTION__);
    {
        bool val = false;
        assert(test_type(val) == _mtv_bool);
        cout << "bool is good" << endl;
    }
    {
        int16_t val = 0;
        assert(test_type(val) == _mtv_int16);
        cout << "int16 is good" << endl;
    }
    {
        uint16_t val = 0;
        assert(test_type(val) == _mtv_uint16);
        cout << "uint16 is good" << endl;
    }
    {
        int32_t val = 0;
        assert(test_type(val) == _mtv_int32);
        cout << "int32 is good" << endl;
    }
    {
        uint32_t val = 0;
        assert(test_type(val) == _mtv_uint32);
        cout << "uint32 is good" << endl;
    }
    {
        int64_t val = 0;
        assert(test_type(val) == _mtv_int64);
        cout << "int64 is good" << endl;
    }
    {
        uint64_t val = 0;
        assert(test_type(val) == _mtv_uint64);
        cout << "uint64 is good" << endl;
    }
    {
        float val = 0;
        assert(test_type(val) == _mtv_float);
        cout << "float is good" << endl;
    }
    {
        double val = 0;
        assert(test_type(val) == _mtv_double);
        cout << "double is good" << endl;
    }
    {
        string val;
        assert(test_type(val) == _mtv_string);
        cout << "string is good" << endl;
    }
    {
        int8_t val = 0;
        assert(test_type(val) == _mtv_int8);
        cout << "int8 is good" << endl;
    }
    {
        uint8_t val = 0;
        assert(test_type(val) == _mtv_uint8);
        cout << "uint8 is good" << endl;
    }
}

}

int main (int argc, char **argv)
{
    try
    {
        mtv_test_types();
        mtv_test_construction();
        mtv_test_basic();
        mtv_test_basic_numeric();
        mtv_test_empty_cells();
        mtv_test_misc_swap();
        mtv_test_misc_equality();
        mtv_test_misc_clone();
        mtv_test_misc_resize();
        mtv_test_misc_value_type();
        mtv_test_misc_block_identifier();
        mtv_test_misc_push_back();
        mtv_test_misc_capacity();
        mtv_test_misc_position_type_end_position();
        mtv_test_misc_block_pos_adjustments();
        mtv_test_erase();
        mtv_test_insert_empty();
        mtv_test_set_cells();
        mtv_test_insert_cells();
        mtv_test_iterators();
        mtv_test_iterators_element_block();
        mtv_test_iterators_mutable_element_block();
        mtv_test_iterators_private_data();
        mtv_test_iterators_set();
        mtv_test_iterators_set_2();
        mtv_test_iterators_insert();
        mtv_test_iterators_insert_empty();
        mtv_test_iterators_set_empty();
        mtv_test_hints_set();
        mtv_test_hints_set_cells();
        mtv_test_hints_insert_cells();
        mtv_test_hints_set_empty();
        mtv_test_hints_insert_empty();
        mtv_test_position();
        mtv_test_position_next();
        mtv_test_position_advance();
        mtv_test_swap_range();
        mtv_test_transfer();
    }
    catch (const std::exception& e)
    {
        cout << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
