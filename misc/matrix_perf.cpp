
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

void perf_construction()
{
    {
        stack_watch sw;
        mixed_mx_type mx(20000, 5000, mdds::matrix_density_filled_zero);
        cout << "construction filled zero: " << sw.get_duration() << " sec (mixed_type_matrix, filled)" << endl;
    }

    {
        stack_watch sw;
        mixed_mx_type mx(20000, 5000, mdds::matrix_density_sparse_zero);
        cout << "construction filled zero: " << sw.get_duration() << " sec (mixed_type_matrix, sparse)" << endl;
    }

    {
        stack_watch sw;
        multi_mx_type mx(20000, 5000, 0.0);
        cout << "construction filled zero: " << sw.get_duration() << " sec (multi_type_matrix)" << endl;
    }

    {
        stack_watch sw;
        mixed_mx_type mx(20000, 5000, mdds::matrix_density_filled_empty);
        cout << "construction filled empty: " << sw.get_duration() << " sec (mixed_type_matrix, filled)" << endl;
    }

    {
        stack_watch sw;
        mixed_mx_type mx(20000, 5000, mdds::matrix_density_sparse_empty);
        cout << "construction filled empty: " << sw.get_duration() << " sec (mixed_type_matrix, sparse)" << endl;
    }

    {
        stack_watch sw;
        multi_mx_type mx(20000, 5000);
        cout << "construction filled empty: " << sw.get_duration() << " sec (multi_type_matrix)" << endl;
    }
}

int main()
{
    perf_construction();
    return EXIT_SUCCESS;
}
