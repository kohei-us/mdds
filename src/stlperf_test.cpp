/*************************************************************************
 *
 * Copyright (c) 2010 Kohei Yoshida
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

#include <vector>
#include <unordered_set>
#include <set>
#include <list>

#include <stdio.h>
#include <string>
#include <sys/time.h>

using namespace std;

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

struct fake
{
};

int main()
{
    size_t store_size = 50000000;
    {
        StackPrinter __stack_printer__("vector non-reserved");
        fake* ptr = 0x000000000;
        vector<void*> store;
        for (size_t i = 0; i < store_size; ++i)
            store.push_back(ptr++);
    }

    {
        StackPrinter __stack_printer__("vector reserved");
        fake* ptr = 0x000000000;
        vector<void*> store;
        store.reserve(store_size);
        for (size_t i = 0; i < store_size; ++i)
            store.push_back(ptr++);
    }

    {
        StackPrinter __stack_printer__("list");
        fake* ptr = 0x000000000;
        list<void*> store;
        for (size_t i = 0; i < store_size; ++i)
            store.push_back(ptr++);
    }

    {
        StackPrinter __stack_printer__("set");
        fake* ptr = 0x000000000;
        set<void*> store;   
        for (size_t i = 0; i < store_size; ++i)
            store.insert(ptr++);
    }

    {
        StackPrinter __stack_printer__("unordered set");
        fake* ptr = 0x000000000;
        unordered_set<void*> store;
        for (size_t i = 0; i < store_size; ++i)
            store.insert(ptr++);
    }
}
