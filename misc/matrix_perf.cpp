
#include <cstdlib>
#include <iostream>
#include <sys/time.h>

#include <mdds/mixed_type_matrix.hpp>
#include <mdds/multi_type_matrix.hpp>

double get_current_time()
{
#ifdef _WIN32
    FILETIME ft;
    __int64 *time64 = reinterpret_cast<__int64 *>(&ft);
    GetSystemTimeAsFileTime(&ft);
    return *time64 / 10000000.0;
#else
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
#endif
}

class stack_watch
{
public:
    explicit stack_watch() : m_start_time(get_current_time()) {}
    void reset() { m_start_time = get_current_time(); }
    double get_duration() const { return get_current_time() - m_start_time; }

private:
    double m_start_time;
};

using namespace std;

typedef mdds::mixed_type_matrix<std::string, bool> mixed_mx_type;
typedef mdds::multi_type_matrix<mdds::mtm::std_string_trait> multi_mx_type;

namespace {

class sum_all_values
{
    double m_sum;
public:
    sum_all_values() : m_sum(0.0) {}
    void operator() (const multi_mx_type::element_block_node_type& blk)
    {
        if (!blk.data)
            return;

        if (mdds::mtv::get_block_type(*blk.data) != mdds::mtv::element_type_double)
            return;

        using mdds::mtv::double_element_block;
        numeric_element_block::const_iterator it = numeric_element_block::begin(*blk.data);
        numeric_element_block::const_iterator it_end = numeric_element_block::end(*blk.data);
        for (; it != it_end; ++it)
            m_sum += *it;
    }

    double get() const { return m_sum; }
};

class count_all_values
{
    long m_count;
public:
    count_all_values() : m_count(0) {}
    void operator() (const multi_mx_type::element_block_node_type& blk)
    {
        if (!blk.data)
            return;

        if (mdds::mtv::get_block_type(*blk.data) != mdds::mtv::element_type_double)
            return;

        m_count += blk.size;
    }

    long get() const { return m_count; }
};

template<typename _Mx>
void init_manual_loop(_Mx& mx, size_t row_size, size_t col_size)
{
    double val = 0.0;
    for (size_t row = 0; row < row_size; ++row)
    {
        for (size_t col = 0; col < col_size; ++col)
        {
            mx.set(row, col, val);
            val += 0.00001;
        }
    }
}

void init_value_vector(vector<double>& vals, size_t row_size, size_t col_size)
{
    vals.reserve(row_size*col_size);

    double val = 0.0;
    for (size_t row = 0; row < row_size; ++row)
    {
        for (size_t col = 0; col < col_size; ++col)
        {
            vals.push_back(val);
            val += 0.00001;
        }
    }
}

template<typename _Mx>
double sum_manual_loop(const _Mx& mx, size_t row_size, size_t col_size)
{
    double sum = 0.0;
    for (size_t row = 0; row < row_size; ++row)
        for (size_t col = 0; col < col_size; ++col)
            sum += mx.get_numeric(row, col);

    return sum;
}

double sum_iterator(const mixed_mx_type& mx)
{
    double sum = 0.0;
    mixed_mx_type::const_iterator it = mx.begin(), it_end = mx.end();
    for (; it != it_end; ++it)
    {
        if (it->m_type == mdds::element_numeric)
            sum += it->m_numeric;
    }
    return sum;
}

}

void perf_construction()
{
    cout << "---" << endl;
    size_t row_size = 20000, col_size = 8000;
    {
        stack_watch sw;
        mixed_mx_type mx(row_size, col_size, mdds::matrix_density_filled_zero);
        cout << "construction filled zero: " << sw.get_duration() << " sec (mixed_type_matrix, filled)" << endl;
    }

    {
        stack_watch sw;
        mixed_mx_type mx(row_size, col_size, mdds::matrix_density_sparse_zero);
        cout << "construction filled zero: " << sw.get_duration() << " sec (mixed_type_matrix, sparse)" << endl;
    }

    {
        stack_watch sw;
        multi_mx_type mx(row_size, col_size, 0.0);
        cout << "construction filled zero: " << sw.get_duration() << " sec (multi_type_matrix)" << endl;
    }

    {
        stack_watch sw;
        mixed_mx_type mx(row_size, col_size, mdds::matrix_density_filled_empty);
        cout << "construction filled empty: " << sw.get_duration() << " sec (mixed_type_matrix, filled)" << endl;
    }

    {
        stack_watch sw;
        mixed_mx_type mx(row_size, col_size, mdds::matrix_density_sparse_empty);
        cout << "construction filled empty: " << sw.get_duration() << " sec (mixed_type_matrix, sparse)" << endl;
    }

    {
        stack_watch sw;
        multi_mx_type mx(row_size, col_size);
        cout << "construction filled empty: " << sw.get_duration() << " sec (multi_type_matrix)" << endl;
    }
}

void perf_insertion()
{
    cout << "---" << endl;
    size_t row_size = 10000;
    size_t col_size = 1000;
    {
        mixed_mx_type mx(row_size, col_size, mdds::matrix_density_filled_zero);
        stack_watch sw;
        init_manual_loop(mx, row_size, col_size);
        cout << "insertion via loop: " << sw.get_duration() << " sec (mixed_type_matrix, filled zero)" << endl;
    }

    {
        multi_mx_type mx(row_size, col_size);
        stack_watch sw;
        init_manual_loop(mx, row_size, col_size);
        cout << "insertion via loop: " << sw.get_duration() << " sec (multi_type_matrix, init empty)" << endl;
    }

    {
        multi_mx_type mx(row_size, col_size, 0.0);
        stack_watch sw;
        init_manual_loop(mx, row_size, col_size);
        cout << "insertion via loop: " << sw.get_duration() << " sec (multi_type_matrix, init zero)" << endl;
    }

    {
        multi_mx_type mx(row_size, col_size);
        std::vector<double> vals;

        stack_watch sw;
        init_value_vector(vals, row_size, col_size);
        mx.set(0, 0, vals.begin(), vals.end());
        cout << "insertion via single set call: " << sw.get_duration() << " sec (multi_type_matrix, init empty, value initialization included)" << endl;
    }

    {
        multi_mx_type mx(row_size, col_size);
        std::vector<double> vals;
        init_value_vector(vals, row_size, col_size);

        stack_watch sw;
        mx.set(0, 0, vals.begin(), vals.end());
        cout << "insertion via single set call: " << sw.get_duration() << " sec (multi_type_matrix, init empty, value initialization excluded)" << endl;
    }

    {
        multi_mx_type mx(row_size, col_size, 0.0);
        std::vector<double> vals;
        vals.reserve(row_size*col_size);

        stack_watch sw;
        init_value_vector(vals, row_size, col_size);
        mx.set(0, 0, vals.begin(), vals.end());
        cout << "insertion via single set call: " << sw.get_duration() << " sec (multi_type_matrix, init zero, value initialization included)" << endl;
    }

    {
        multi_mx_type mx(row_size, col_size, 0.0);
        std::vector<double> vals;
        init_value_vector(vals, row_size, col_size);

        stack_watch sw;
        mx.set(0, 0, vals.begin(), vals.end());
        cout << "insertion via single set call: " << sw.get_duration() << " sec (multi_type_matrix, init zero, value initialization excluded)" << endl;
    }
}

void perf_sum_all_values()
{
    cout << "---" << endl;
    size_t row_size = 10000;
    size_t col_size = 1000;
    {
        mixed_mx_type mx(row_size, col_size, mdds::matrix_density_filled_zero);
        init_manual_loop(mx, row_size, col_size);

        {
            stack_watch sw;
            double sum = sum_manual_loop(mx, row_size, col_size);
            cout << "sum all values (" << sum << ") : " << sw.get_duration() << " sec (mixed_type_matrix, manual loop)" << endl;
        }

        {
            stack_watch sw;
            double sum = sum_iterator(mx);
            cout << "sum all values (" << sum << ") : " << sw.get_duration() << " sec (mixed_type_matrix, iterator)" << endl;
        }

        {
            stack_watch sw;
            long count = 0;
            for (size_t row = 0; row < row_size; ++row)
                for (size_t col = 0; col < col_size; ++col)
                {
                    if (mx.get_type(row, col) == mdds::element_numeric)
                        ++count;
                }

            cout << "count all values (" << count << ") : " << sw.get_duration() << " sec (mixed_type_matrix, manual loop)" << endl;
        }

        {
            stack_watch sw;
            long count = 0;
            mixed_mx_type::const_iterator it = mx.begin(), it_end = mx.end();
            for (; it != it_end; ++it)
            {
                if (it->m_type == mdds::element_numeric)
                    ++count;
            }

            cout << "count all values (" << count << ") : " << sw.get_duration() << " sec (mixed_type_matrix, iterator)" << endl;
        }
    }

    {
        multi_mx_type mx(row_size, col_size, 0.0);
        init_manual_loop(mx, row_size, col_size);

        {
            stack_watch sw;
            double sum = sum_manual_loop(mx, row_size, col_size);
            cout << "sum all values (" << sum << ") : " << sw.get_duration() << " sec (multi_type_matrix, manual loop)" << endl;
        }

        {
            stack_watch sw;
            sum_all_values func;
            mx.walk(func);
            cout << "sum all values (" << func.get() << ") : " << sw.get_duration() << " sec (multi_type_matrix, walk)" << endl;
        }

        {
            stack_watch sw;
            count_all_values func;
            mx.walk(func);
            cout << "count all values (" << func.get() << ") : " << sw.get_duration() << " sec (multi_type_matrix, walk)" << endl;
        }
    }
}

void perf_sum_all_values_multi_block()
{
    cout << "---" << endl;
    size_t row_size = 10000;
    size_t col_size = 1000;
    double step = 0.00001;
    {
        mixed_mx_type mx(row_size, col_size, mdds::matrix_density_filled_zero);
        double val = 0.0;
        for (size_t row = 0; row < row_size; ++row)
        {
            for (size_t col = 0; col < col_size; ++col)
            {
                if ((col % 3) == 0)
                {
                    mx.set_empty(row, col);
                }
                else
                {
                    mx.set(row, col, val);
                    val += step;
                }
            }
        }

        {
            stack_watch sw;
            double sum = sum_manual_loop(mx, row_size, col_size);
            cout << "sum all values multi-block (" << sum << ") : " << sw.get_duration() << " sec (mixed_type_matrix, manual loop)" << endl;
        }

        {
            stack_watch sw;
            double sum = sum_iterator(mx);
            cout << "sum all values multi-block (" << sum << ") : " << sw.get_duration() << " sec (mixed_type_matrix, iterator)" << endl;
        }

        {
            stack_watch sw;
            long count = 0;
            for (size_t row = 0; row < row_size; ++row)
                for (size_t col = 0; col < col_size; ++col)
                {
                    if (mx.get_type(row, col) == mdds::element_numeric)
                        ++count;
                }

            cout << "count all values multi-block (" << count << ") : " << sw.get_duration() << " sec (mixed_type_matrix, manual loop)" << endl;
        }

        {
            stack_watch sw;
            long count = 0;
            mixed_mx_type::const_iterator it = mx.begin(), it_end = mx.end();
            for (; it != it_end; ++it)
            {
                if (it->m_type == mdds::element_numeric)
                    ++count;
            }

            cout << "count all values multi-block (" << count << ") : " << sw.get_duration() << " sec (mixed_type_matrix, iterator)" << endl;
        }
    }

    {
        multi_mx_type mx(row_size, col_size);
        double val = 0.0;
        vector<double> vals;
        vals.reserve(row_size);
        for (size_t col = 0; col < col_size; ++col)
        {
            if ((col % 3) == 0)
                continue;

            vals.clear();
            for (size_t row = 0; row < row_size; ++row)
            {
                vals.push_back(val);
                val += step;
            }

            mx.set(0, col, vals.begin(), vals.end());
        }

        {
            stack_watch sw;
            double sum = sum_manual_loop(mx, row_size, col_size);
            cout << "sum all values multi-block (" << sum << ") : " << sw.get_duration() << " sec (multi_type_matrix, manual loop)" << endl;
        }

        {
            stack_watch sw;
            sum_all_values func;
            mx.walk(func);
            cout << "sum all values multi-block (" << func.get() << ") : " << sw.get_duration() << " sec (multi_type_matrix, walk)" << endl;
        }

        {
            stack_watch sw;
            count_all_values func;
            mx.walk(func);
            cout << "count all values multi-block (" << func.get() << ") : " << sw.get_duration() << " sec (multi_type_matrix, walk)" << endl;
        }
    }
}

void perf_init_with_value()
{
    cout << "---" << endl;
    size_t row_size = 10000;
    size_t col_size = 2000;
    double val = 12.3;
    {
        stack_watch sw;
        mixed_mx_type mx(row_size, col_size, mdds::matrix_density_filled_zero);
        for (size_t row = 0; row < row_size; ++row)
            for (size_t col = 0; col < col_size; ++col)
                mx.set(row, col, val);
        cout << "init with value: " << sw.get_duration() << " sec (mixed_type_matrix, filled zero, manual loop)" << endl;
    }

    {
        stack_watch sw;
        multi_mx_type mx(row_size, col_size, 0.0);
        for (size_t row = 0; row < row_size; ++row)
            for (size_t col = 0; col < col_size; ++col)
                mx.set(row, col, val);
        cout << "init with value: " << sw.get_duration() << " sec (multi_type_matrix, manual loop)" << endl;
    }

    {
        stack_watch sw;
        multi_mx_type(row_size, col_size, val);
        cout << "init with value: " << sw.get_duration() << " sec (multi_type_matrix, constructor)" << endl;
    }

    {
        stack_watch sw;
        multi_mx_type mx(row_size, col_size);
        vector<double> vals(row_size*col_size, val);
        mx.set(0, 0, vals.begin(), vals.end());
        cout << "init with value: " << sw.get_duration() << " sec (multi_type_matrix, vector + set)" << endl;
    }
}

void perf_heap_vs_array()
{
    cout << "---" << endl;
    size_t row_size = 20000, col_size = 3000;
    {
        mixed_mx_type mx(row_size, col_size, mdds::matrix_density_filled_empty);
        init_manual_loop(mx, row_size, col_size);
        stack_watch sw;
        double sum = 0.0;
        for (size_t row = 0; row < row_size; ++row)
            for (size_t col = 0; col < col_size; ++col)
                sum += mx.get_numeric(row, col);
        cout << "sum with values on the heap (mixed_type_matrix): " << sw.get_duration() << " sec (" << sum << ")" << endl;
    }

    {
        mixed_mx_type mx(row_size, col_size, mdds::matrix_density_filled_zero);
        init_manual_loop(mx, row_size, col_size);
        stack_watch sw;
        double sum = 0.0;
        for (size_t row = 0; row < row_size; ++row)
            for (size_t col = 0; col < col_size; ++col)
                sum += mx.get_numeric(row, col);
        cout << "sum with array values (mixed_type_matrix): " << sw.get_duration() << " sec (" << sum << ")" << endl;
    }

    {
        multi_mx_type mx(row_size, col_size, 0.0);
        init_manual_loop(mx, row_size, col_size);
        sum_all_values func;
        stack_watch sw;
        mx.walk(func);
        double sum = func.get();
        cout << "sum with array values (multi_type_matrix): " << sw.get_duration() << " sec (" << sum << ")" << endl;
    }
}

int main()
{
    perf_construction();
    perf_insertion();
    perf_sum_all_values();
    perf_sum_all_values_multi_block();
    perf_init_with_value();
    perf_heap_vs_array();
    return EXIT_SUCCESS;
}
