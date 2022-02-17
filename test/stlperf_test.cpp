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
#include <algorithm>
#include <cassert>

#include <stdio.h>
#include <string>
#include <sys/time.h>

using namespace std;

namespace {

class StackPrinter
{
public:
    explicit StackPrinter(const char* msg) : msMsg(msg)
    {
        fprintf(stdout, "%s: --begin\n", msMsg.c_str());
        mfStartTime = getTime();
    }

    ~StackPrinter()
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", msMsg.c_str(), (fEndTime - mfStartTime));
    }

    void printTime(int line) const
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --(%d) (duration: %g sec)\n", msMsg.c_str(), line, (fEndTime - mfStartTime));
    }

private:
    double getTime() const
    {
        timeval tv;
        gettimeofday(&tv, nullptr);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    ::std::string msMsg;
    double mfStartTime;
};

} // namespace

int main()
{
    size_t store_size = 100000;
    {
        StackPrinter __stack_printer__("vector non-reserved");
        vector<void*> store;
        {
            StackPrinter __stack_printer2__("  push_back");
            string* ptr = 0x00000000;
            for (size_t i = 0; i < store_size; ++i)
            {
                // coverity[dereference] - this is intentional
                store.push_back(ptr++);
            }
        }
        {
            StackPrinter __stack_printer2__("  find and pop_back");
            string* ptr = 0x00000000;
            for (size_t i = 0; i < store_size; ++i)
            {
                vector<void*>::iterator itr = find(store.begin(), store.end(), ptr);
                if (itr != store.end())
                {
                    *itr = store.back();
                    store.pop_back();
                }
                // coverity[dereference] - this is intentional
                ++ptr;
            }
        }
        assert(store.empty());
    }

    {
        StackPrinter __stack_printer__("vector reserved");
        vector<void*> store;
        {
            StackPrinter __stack_printer2__("  push_back");
            string* ptr = 0x00000000;
            store.reserve(store_size);
            for (size_t i = 0; i < store_size; ++i)
                store.push_back(ptr++);
        }
        {
            StackPrinter __stack_printer2__("  find and pop_back");
            string* ptr = 0x00000000;
            for (size_t i = 0; i < store_size; ++i)
            {
                vector<void*>::iterator itr = find(store.begin(), store.end(), ptr);
                if (itr != store.end())
                {
                    *itr = store.back();
                    store.pop_back();
                }
                ++ptr;
            }
        }
        assert(store.empty());
    }

    {
        StackPrinter __stack_printer__("list");
        list<void*> store;
        {
            StackPrinter __stack_printer2__("  push_back");
            string* ptr = 0x00000000;
            ++ptr;
            for (size_t i = 0; i < store_size; ++i)
                store.push_back(ptr++);
        }
        {
            StackPrinter __stack_printer2__("  remove");
            string* ptr = 0x00000000;
            for (size_t i = 0; i < store_size; ++i)
                store.remove(ptr++);
        }
    }

    {
        StackPrinter __stack_printer__("set");
        set<void*> store;
        {
            StackPrinter __stack_printer2__("  insert");
            string* ptr = 0x00000000;
            for (size_t i = 0; i < store_size; ++i)
                store.insert(ptr++);
        }
        {
            StackPrinter __stack_printer2__("  erase");
            string* ptr = 0x00000000;
            for (size_t i = 0; i < store_size; ++i)
                store.erase(ptr++);
        }
    }

    {
        StackPrinter __stack_printer__("unordered set");
        unordered_set<void*> store;
        {
            StackPrinter __stack_printer2__("  insert");
            string* ptr = 0x00000000;
            for (size_t i = 0; i < store_size; ++i)
                store.insert(ptr++);
        }
        {
            StackPrinter __stack_printer2__("  erase");
            string* ptr = 0x00000000;
            for (size_t i = 0; i < store_size; ++i)
                store.erase(ptr++);
        }
    }
}
