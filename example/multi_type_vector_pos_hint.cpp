
#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>

#include <stdio.h>
#include <string>
#include <sys/time.h>

namespace {

class stack_printer
{
public:
    explicit stack_printer(const char* msg) :
        msMsg(msg)
    {
        fprintf(stdout, "%s: --begin\n", msMsg.c_str());
        mfStartTime = getTime();
    }

    ~stack_printer()
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", msMsg.c_str(), (fEndTime-mfStartTime));
    }

    void printTime(int line) const
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --(%d) (duration: %g sec)\n", msMsg.c_str(), line, (fEndTime-mfStartTime));
    }

private:
    double getTime() const
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    ::std::string msMsg;
    double mfStartTime;
};

}


void run_no_position_hint()
{
    stack_printer __stack_printer__("::run_no_position_hint");

    using mtv_type = mdds::multi_type_vector<mdds::mtv::element_block_func>;

    size_t size = 50000;

    // Initialize the container with one empty block of size 50000.
    mtv_type db(size);

    // Set non-empty value at every other logical position from top down.
    for (size_t i = 0; i < size; ++i)
    {
        if (i % 2)
            db.set<double>(i, 1.0);
    }
}

void run_with_position_hint()
{
    stack_printer __stack_printer__("::run_with_position_hint");

    using mtv_type = mdds::multi_type_vector<mdds::mtv::element_block_func>;

    size_t size = 50000;

    // Initialize the container with one empty block of size 50000.
    mtv_type db(size);
    mtv_type::iterator pos = db.begin();

    // Set non-empty value at every other logical position from top down.
    for (size_t i = 0; i < size; ++i)
    {
        if (i % 2)
            // Pass the position hint as the first argument, and receive a new
            // one returned from the method for the next call.
            pos = db.set<double>(pos, i, 1.0);
    }
}

int main()
{
    run_no_position_hint();
    run_with_position_hint();

    return EXIT_SUCCESS;
}
