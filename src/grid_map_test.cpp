/*************************************************************************
 *
 * Copyright (c) 2011 Kohei Yoshida
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

#include "mdds/grid_map.hpp"
#include "test_global.hpp"

#include <cassert>

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

#include <stdio.h>
#include <string>
#include <sys/time.h>

using namespace std;
using namespace mdds;

namespace {

class stack_printer
{
public:
    explicit stack_printer(const char* msg) :
        m_msg(msg)
    {
        fprintf(stdout, "%s: --begin\n", m_msg.c_str());
        m_start_time = get_time();
    }

    ~stack_printer()
    {
        double end_time = get_time();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", m_msg.c_str(), (end_time-m_start_time));
    }

    void print_time(int line) const
    {
        double end_time = get_time();
        fprintf(stdout, "%s: --(%d) (duration: %g sec)\n", m_msg.c_str(), line, (end_time-m_start_time));
    }

private:
    double get_time() const
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    std::string m_msg;
    double m_start_time;
};

enum cell_t { celltype_numeric = 0, celltype_string };

struct base_cell
{
    cell_t type;
    base_cell(cell_t _t) : type(_t) {}
};

struct numeric_cell : public base_cell
{
    double value;
    numeric_cell(double _v) : base_cell(celltype_numeric), value(_v) {}
};

struct string_cell : public base_cell
{
    std::string str;
    string_cell(const std::string& _s) : base_cell(celltype_string), str(_s) {}
};

struct cell_deleter : public std::unary_function<base_cell*, void>
{
    void operator() (base_cell* p)
    {
        switch (p->type)
        {
            case celltype_numeric:
                delete static_cast<numeric_cell*>(p);
            break;
            case celltype_string:
                delete static_cast<string_cell*>(p);
            break;
            default:
                assert(!"attempting to delete a cell instance of unknown type!");
        }
    }
};

struct grid_map_trait
{
    typedef base_cell cell_type;
    typedef cell_t cell_category_type;
    typedef long sheet_key_type;
    typedef long row_key_type;
    typedef long col_key_type;

    typedef cell_deleter cell_delete_handler;
};

}

void gridmap_test_basic()
{
    stack_printer __stack_printer__("::gridmap_test_basic");
    typedef mdds::grid_map<grid_map_trait> grid_store_type;
    typedef grid_store_type::sheet_type::column_type column_type;
    grid_store_type db;

    // Single column instance with 100 rows.
    column_type col_db(100);
    const column_type::cell_type* cell = col_db.get_cell(0);
    assert(!cell);

    col_db.set_cell(0, celltype_numeric, new numeric_cell(1.0));
}

int main (int argc, char **argv)
{
    cmd_options opt;
    if (!parse_cmd_options(argc, argv, opt))
        return EXIT_FAILURE;

    if (opt.test_func)
    {
        gridmap_test_basic();
    }

    if (opt.test_perf)
    {
    }

    return EXIT_SUCCESS;
}
