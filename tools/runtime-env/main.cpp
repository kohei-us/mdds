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
#include <iomanip>
#include <vector>
#include <chrono>
#include <map>
#include <sstream>
#include <fstream>
#include <limits>
#include <iterator>

using mdds::mtv::lu_factor_t;
using std::cerr;
using std::cout;
using std::endl;

namespace {

std::string to_string(lu_factor_t lu)
{
    int lu_value = int(lu) & 0xFF;
    bool sse2 = (int(lu) & 0x100) != 0;
    bool avx2 = (int(lu) & 0x200) != 0;

    std::ostringstream os;
    os << (sse2 ? "sse2+" : "") << (avx2 ? "avx2+" : "") << std::setw(2) << std::setfill('0') << lu_value;

    return os.str();
}

std::string reflow_text(std::string s, std::size_t width)
{
    // Tokenize the text first.
    std::vector<std::string> tokens;

    const char* p = s.data();
    const char* p_end = p + s.size();
    const char* p0 = nullptr;

    for (; p != p_end; ++p)
    {
        if (!p0)
            p0 = p;

        if (*p == ' ' && p0)
        {
            std::string token(p0, p - p0);
            tokens.emplace_back(p0, p - p0);
            p0 = nullptr;
        }
    }

    if (p0)
        tokens.emplace_back(p0, p - p0);

    std::vector<std::string> lines(1, std::string());

    for (const std::string& token : tokens)
    {
        std::string this_line = lines.back(); // copy
        if (this_line.empty())
            this_line = token;
        else
            this_line += ' ' + token;

        if (this_line.size() > width)
            lines.push_back(token);
        else
            lines.back() = this_line;
    }

    std::ostringstream os;
    std::copy(lines.begin(), lines.end(), std::ostream_iterator<std::string>(os, "\n"));
    return os.str();
}

std::string pad_right(std::string s, int width)
{
    int gap = width - int(s.size());
    if (gap < 0)
        return s;

    return s + std::string(gap, ' ');
}

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
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();

        return usec_since_epoch / 1000000.0;
    }

    double m_start_time;
};

class data_handler
{
    struct key_type
    {
        std::string type;
        lu_factor_t lu;

        key_type(const std::string& type_, lu_factor_t lu_) : type(type_), lu(lu_)
        {}

        bool operator<(const key_type& other) const
        {
            if (type != other.type)
                return type < other.type;

            return lu < other.lu;
        }

        std::string to_string() const
        {
            std::ostringstream os;
            os << '(' << type << ", " << ::to_string(lu) << ')';
            return os.str();
        }
    };

    struct duration_type
    {
        double duration = 0.0;
        int count = 0;

        duration_type(double duration_, int count_) : duration(duration_), count(count_)
        {}

        std::string to_string() const
        {
            std::ostringstream os;
            os << std::setprecision(4) << duration << '/' << count;
            return os.str();
        }
    };

    using records_type = std::map<key_type, duration_type>;
    using raw_records_type = std::vector<std::tuple<std::string, lu_factor_t, int, int, double>>;
    records_type m_records;
    raw_records_type m_raw_records;
    int m_insert_count = 0;

    void draw_graph() const
    {
        if (m_records.empty())
            return;

        // Convert to sequence of average durations.
        std::vector<std::tuple<key_type, double>> averages;
        averages.reserve(m_records.size());

        for (const auto& kv : m_records)
        {
            const duration_type& v = kv.second;
            averages.emplace_back(kv.first, v.duration / v.count);
        }

        // Sort by durations.
        std::sort(averages.begin(), averages.end(), [](const auto& left, const auto& right) -> bool {
            return std::get<1>(left) < std::get<1>(right);
        });

        const int n_ticks_max = 55;

        double min_v = std::numeric_limits<double>::max();
        double max_v = std::numeric_limits<double>::min();

        for (const auto& average : averages)
        {
            double v = std::get<1>(average);
            min_v = std::min(min_v, v);
            max_v = std::max(max_v, v);
        }

        double tick_width = max_v / n_ticks_max;

        std::vector<std::pair<std::string, std::string>> lines;
        lines.reserve(averages.size());
        int max_label_width = 0;

        for (const auto& average : averages)
        {
            const key_type& k = std::get<0>(average);
            double v = std::get<1>(average);

            lines.emplace_back();
            auto& line = lines.back();

            int n_ticks = v / tick_width;

            // Create and store the label first.
            std::ostringstream label;
            label << '(' << k.type << ", " << to_string(k.lu) << ')';
            line.first = label.str();

            char tick = k.type == "SoA" ? 'o' : '+';

            // Create and store the bar next.
            std::ostringstream bar;
            bar << std::string(n_ticks, tick) << ' ' << std::setprecision(4) << v;
            line.second = bar.str();

            max_label_width = std::max<int>(max_label_width, line.first.size());
        }

        for (auto& line : lines)
            line.first = pad_right(std::move(line.first), max_label_width);

        std::ostringstream graph_out;

        {
            // Print the top label and axis. Make sure to rewind to erase the
            // progress text.
            std::string line = pad_right(" Category", max_label_width);
            line += " | Average duration (seconds)";
            graph_out << line << "\n";

            line = std::string(max_label_width, '-') + "-+-" + std::string(n_ticks_max, '-') + '>';
            graph_out << line << "\n";
        }

        for (const auto& line : lines)
            graph_out << line.first << " | " << line.second << "\n";

        graph_out << "\n";

        const key_type& top_key = std::get<0>(averages[0]);
        std::ostringstream os;
        os << "Storage of " << top_key.type << " with the LU factor of " << to_string(top_key.lu)
           << " appears to be the best choice in this environment.";

        graph_out << reflow_text(os.str(), 70) << endl;

        std::string graph_out_s = graph_out.str();

        cout << "\r" << graph_out_s;

        std::ofstream of("graph-output.txt");
        of << graph_out_s << std::flush;
    }

    void write_csv() const
    {
        std::ofstream of("raw-data.csv");
        of << "storage,factor,block count,repeat count,duration\n";

        for (const auto& rec : m_raw_records)
        {
            const std::string& type = std::get<0>(rec);
            lu_factor_t lu = std::get<1>(rec);
            int block_size = std::get<2>(rec);
            int repeats = std::get<3>(rec);
            double duration = std::get<4>(rec);

            of << type << "," << to_string(lu) << "," << block_size << "," << repeats << "," << duration << "\n";
        }

        of << std::flush;
    }

public:
    data_handler()
    {}

    void start()
    {
        m_insert_count = 0;
    }

    void end()
    {
        draw_graph();
        write_csv();
    }

    void record_time(const std::string& type, lu_factor_t lu, int block_size, int repeats, double duration)
    {
        key_type key(type, lu);

        auto it = m_records.lower_bound(key);
        if (it != m_records.end() && !m_records.key_comp()(key, it->first))
        {
            // key exists
            duration_type& v = it->second;
            v.duration += duration;
            ++v.count;
        }
        else
        {
            // key doesn't yet exist.
            m_records.insert(it, records_type::value_type(key, {duration, 1}));
        }

        if (!(++m_insert_count & 0x1F))
        {
            int n_ticks = m_insert_count >> 5;
            cout << "\rprogress: " << std::string(n_ticks, '#') << std::flush;
        }

        m_raw_records.emplace_back(type, lu, block_size, repeats, duration);
    }
};

/**
 * Class designed to measure run-time performance of AoS multi_type_vector
 * with various loop-unrolling factors.
 */
class mtv_aos_luf_runner
{
    data_handler& m_handler;

    struct block
    {
        std::size_t position;
        std::size_t size;
        void* data;

        block(std::size_t _position, std::size_t _size) : position(_position), size(_size)
        {}
    };

    using blocks_type = std::vector<block>;

    template<lu_factor_t Factor>
    void measure_duration(blocks_type blocks, int block_size, int repeats)
    {
        using namespace mdds::mtv::aos::detail;

        section_timer st;
        for (int i = 0; i < repeats; ++i)
            adjust_block_positions<blocks_type, Factor>{}(blocks, 0, 1);

        m_handler.record_time("AoS", Factor, block_size, repeats, st.get_duration());
    }

public:
    mtv_aos_luf_runner(data_handler& dh) : m_handler(dh)
    {}

    void run(int block_size, int repeats)
    {
        blocks_type blocks;

        std::size_t pos = 0;
        std::size_t size = 2;
        for (int i = 0; i < block_size; ++i)
        {
            blocks.emplace_back(pos, size);
            pos += size;
        }

        measure_duration<lu_factor_t::none>(blocks, block_size, repeats);
        measure_duration<lu_factor_t::lu4>(blocks, block_size, repeats);
        measure_duration<lu_factor_t::lu8>(blocks, block_size, repeats);
        measure_duration<lu_factor_t::lu16>(blocks, block_size, repeats);
        measure_duration<lu_factor_t::lu32>(blocks, block_size, repeats);
    }
};

class mtv_soa_luf_runner
{
    data_handler& m_handler;

    struct blocks_type
    {
        std::vector<std::size_t> positions;
        std::vector<std::size_t> sizes;
        std::vector<void*> element_blocks;
    };

    template<lu_factor_t Factor>
    void measure_duration(blocks_type blocks, int block_size, int repeats)
    {
        using namespace mdds::mtv::soa::detail;

        section_timer st;
        for (int i = 0; i < repeats; ++i)
            adjust_block_positions<blocks_type, Factor>{}(blocks, 0, 1);

        m_handler.record_time("SoA", Factor, block_size, repeats, st.get_duration());
    }

public:
    mtv_soa_luf_runner(data_handler& dh) : m_handler(dh)
    {}

    void run(int block_size, int repeats)
    {
        blocks_type blocks;

        std::size_t pos = 0;
        std::size_t size = 2;
        for (int i = 0; i < block_size; ++i)
        {
            blocks.positions.push_back(pos);
            blocks.sizes.push_back(size);
            pos += size;
        }

        measure_duration<lu_factor_t::none>(blocks, block_size, repeats);
        measure_duration<lu_factor_t::lu4>(blocks, block_size, repeats);
        measure_duration<lu_factor_t::lu8>(blocks, block_size, repeats);
        measure_duration<lu_factor_t::lu16>(blocks, block_size, repeats);
        measure_duration<lu_factor_t::lu32>(blocks, block_size, repeats);
#if SIZEOF_VOID_P == 8 && defined(__SSE2__)
        measure_duration<lu_factor_t::sse2_x64>(blocks, block_size, repeats);
        measure_duration<lu_factor_t::sse2_x64_lu4>(blocks, block_size, repeats);
        measure_duration<lu_factor_t::sse2_x64_lu8>(blocks, block_size, repeats);
        measure_duration<lu_factor_t::sse2_x64_lu16>(blocks, block_size, repeats);
#endif
#if SIZEOF_VOID_P == 8 && defined(__AVX2__)
        measure_duration<lu_factor_t::avx2_x64>(blocks, block_size, repeats);
        measure_duration<lu_factor_t::avx2_x64_lu4>(blocks, block_size, repeats);
        measure_duration<lu_factor_t::avx2_x64_lu8>(blocks, block_size, repeats);
#endif
    }
};

} // namespace

int main()
{
    int block_count_init = 5;
    int block_count_step = 5;
    int block_count_max = 300;
    int repeats = 1000000;

    data_handler dh;
    dh.start();

    for (int block_count = block_count_init; block_count <= block_count_max; block_count += block_count_step)
    {
        mtv_aos_luf_runner runner(dh);
        runner.run(block_count, repeats);
    }

    for (int block_count = block_count_init; block_count <= block_count_max; block_count += block_count_step)
    {
        mtv_soa_luf_runner runner(dh);
        runner.run(block_count, repeats);
    }

    dh.end();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
