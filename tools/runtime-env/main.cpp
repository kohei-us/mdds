/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2021 Kohei Yoshida
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

#include <mdds/multi_type_vector/soa/block_util.hpp>
#include <mdds/multi_type_vector/aos/block_util.hpp>

#include <iostream>
#include <vector>
#include <chrono>

using std::cout;
using std::cerr;
using std::endl;

namespace {

class section_timer
{
public:
    explicit section_timer()
    {
        m_start_time = get_time();
    }

    double get_duration() const
    {
        double end_time = get_time();
        return end_time - m_start_time;
    }

private:
    double get_time() const
    {
        unsigned long usec_since_epoch =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();

        return usec_since_epoch / 1000000.0;
    }

    double m_start_time;
};

void print_time(std::string type, int lu, int block_size, int repeats, double duration)
{
    cout << type << "," << lu << "," << block_size << "," << repeats << "," << duration << endl;
}

}

struct mtv_aos_block
{
    std::size_t position;
    std::size_t size;
    void* data;

    mtv_aos_block(std::size_t _position, std::size_t _size) :
        position(_position), size(_size) {}
};

using mtv_aos_blocks_type = std::vector<mtv_aos_block>;

template<int Factor>
void mtv_aos_measure_duration(mtv_aos_blocks_type blocks, int block_size, int repeats)
{
    using namespace mdds::mtv::aos::detail;

    section_timer st;
    for (int i = 0; i < repeats; ++i)
        adjust_block_positions<mtv_aos_blocks_type, Factor>{}(blocks, 0, 1);

    print_time("aos", Factor, block_size, repeats, st.get_duration());
}

void mtv_aos_loop_unrolling_factor(int block_size, int repeats)
{
    using namespace mdds::mtv::aos::detail;

    mtv_aos_blocks_type blocks;

    std::size_t pos = 0;
    std::size_t size = 2;
    for (int i = 0; i < block_size; ++i)
    {
        blocks.emplace_back(pos, size);
        pos += size;
    }

    mtv_aos_measure_duration<0>(blocks, block_size, repeats);
    mtv_aos_measure_duration<4>(blocks, block_size, repeats);
    mtv_aos_measure_duration<8>(blocks, block_size, repeats);
    mtv_aos_measure_duration<16>(blocks, block_size, repeats);
    mtv_aos_measure_duration<32>(blocks, block_size, repeats);
}

int main(int argc, char** argv)
{
    mtv_aos_loop_unrolling_factor(25, 500000);
    mtv_aos_loop_unrolling_factor(50, 500000);
    mtv_aos_loop_unrolling_factor(100, 500000);
    mtv_aos_loop_unrolling_factor(200, 500000);

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

