
#include <vector>
#include <boost/pool/object_pool.hpp>
#include <iostream>

#include <stdio.h>
#include <string>
#include <sys/time.h>

namespace {

class StackPrinter
{
public:
    explicit StackPrinter(const char* msg) :
        msMsg(msg)
    {
        fprintf(stdout, "%s: --begin\n", msMsg.c_str());
        mfStartTime = getTime();
    }

    ~StackPrinter()
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

using namespace std;
using ::boost::object_pool;

struct element
{
    double number;
    element() : number(0.0) {}
    element(double num) : number(num) {}
};

int main()
{
    size_t n = 100000000;
    {
        StackPrinter __stack_printer__("::main object pool with vector");
        object_pool<element> elem_pool;
        vector<element*> vec;
        vec.reserve(n);
        for (size_t i = 0; i < n; ++i)
            vec.push_back(elem_pool.construct());

        cout << "size of vector: " << vec.size() << endl;
    }

    {
        StackPrinter __stack_printer__("::main array new");
        element* parray = new element[n];
        for (size_t i = 0; i < n; ++i)
            parray[i].number = 1.0;
        delete[] parray;
    }
    return 0;
}
