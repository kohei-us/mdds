/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2020 Kohei Yoshida
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

#include <iostream>
#include <fstream>
#include <mdds/trie_map.hpp>

using std::cout;
using std::endl;

enum affiliated_party_t : uint8_t
{
    unaffiliated = 0,
    federalist,
    democratic_republican,
    democratic,
    whig,
    republican,
    national_union,
    republican_national_union,
};

struct us_president
{
    uint16_t year;
    affiliated_party_t party;
};

std::ostream& operator<< (std::ostream& os, affiliated_party_t v)
{
    static const char* names[] = {
        "Unaffiliated",
        "Federalist",
        "Democratic Republican",
        "Democratic",
        "Whig",
        "Republican",
        "National Union",
        "Republican / National Union",
    };

    os << names[v];
    return os;
}

bool operator== (const us_president& left, const us_president& right)
{
    return left.year == right.year && left.party == right.party;
}

struct us_president_serializer
{
    union bin_buffer
    {
        char buffer[2];
        uint16_t i16;
        affiliated_party_t party;
    };

    static constexpr bool variable_size = false;
    static constexpr size_t value_size = 3;

    static void write(std::ostream& os, const us_president& v)
    {
        bin_buffer buf;

        // Write the year value first.
        buf.i16 = v.year;
        os.write(buf.buffer, 2);

        // Write the affiliated party value.
        buf.party = v.party;
        os.write(buf.buffer, 1);
    }

    static void read(std::istream& is, size_t /*n*/, us_president& v)
    {
        // For a fixed-size value type, this should equal the defined value size.
        assert(n == 3);

        bin_buffer buf;

        // Read the year value.
        is.read(buf.buffer, 2);
        v.year = buf.i16;

        // Read the affiliated party value.
        is.read(buf.buffer, 1);
        v.party = buf.party;
    }
};

int main() try
{

    using map_type = mdds::packed_trie_map<mdds::trie::std_string_trait, us_president>;

    // source: https://en.wikipedia.org/wiki/List_of_presidents_of_the_United_States
    //
    // The entries must be sorted by the keys.

    std::vector<map_type::entry> entries =
    {
        { MDDS_ASCII("Abraham Lincoln"),        { 1861, republican_national_union } },
        { MDDS_ASCII("Andrew Jackson"),         { 1829, democratic                } },
        { MDDS_ASCII("Andrew Johnson"),         { 1865, national_union            } },
        { MDDS_ASCII("Barack Obama"),           { 2009, democratic                } },
        { MDDS_ASCII("Benjamin Harrison"),      { 1889, republican                } },
        { MDDS_ASCII("Bill Clinton"),           { 1993, democratic                } },
        { MDDS_ASCII("Calvin Coolidge"),        { 1923, republican                } },
        { MDDS_ASCII("Chester A. Arthur"),      { 1881, republican                } },
        { MDDS_ASCII("Donald Trump"),           { 2017, republican                } },
        { MDDS_ASCII("Dwight D. Eisenhower"),   { 1953, republican                } },
        { MDDS_ASCII("Franklin D. Roosevelt"),  { 1933, democratic                } },
        { MDDS_ASCII("Franklin Pierce"),        { 1853, democratic                } },
        { MDDS_ASCII("George H. W. Bush"),      { 1989, republican                } },
        { MDDS_ASCII("George W. Bush"),         { 2001, republican                } },
        { MDDS_ASCII("George Washington"),      { 1789, unaffiliated              } },
        { MDDS_ASCII("Gerald Ford"),            { 1974, republican                } },
        { MDDS_ASCII("Grover Cleveland 1"),     { 1885, democratic                } },
        { MDDS_ASCII("Grover Cleveland 2"),     { 1893, democratic                } },
        { MDDS_ASCII("Harry S. Truman"),        { 1945, democratic                } },
        { MDDS_ASCII("Herbert Hoover"),         { 1929, republican                } },
        { MDDS_ASCII("James A. Garfield"),      { 1881, republican                } },
        { MDDS_ASCII("James Buchanan"),         { 1857, democratic                } },
        { MDDS_ASCII("James K. Polk"),          { 1845, democratic                } },
        { MDDS_ASCII("James Madison"),          { 1809, democratic_republican     } },
        { MDDS_ASCII("James Monroe"),           { 1817, democratic_republican     } },
        { MDDS_ASCII("Jimmy Carter"),           { 1977, democratic                } },
        { MDDS_ASCII("John Adams"),             { 1797, federalist                } },
        { MDDS_ASCII("John F. Kennedy"),        { 1961, democratic                } },
        { MDDS_ASCII("John Quincy Adams"),      { 1825, democratic_republican     } },
        { MDDS_ASCII("John Tyler"),             { 1841, whig                      } },
        { MDDS_ASCII("Lyndon B. Johnson"),      { 1963, democratic                } },
        { MDDS_ASCII("Martin Van Buren"),       { 1837, democratic                } },
        { MDDS_ASCII("Millard Fillmore"),       { 1850, whig                      } },
        { MDDS_ASCII("Richard Nixon"),          { 1969, republican                } },
        { MDDS_ASCII("Ronald Reagan"),          { 1981, republican                } },
        { MDDS_ASCII("Rutherford B. Hayes"),    { 1877, republican                } },
        { MDDS_ASCII("Theodore Roosevelt"),     { 1901, republican                } },
        { MDDS_ASCII("Thomas Jefferson"),       { 1801, democratic_republican     } },
        { MDDS_ASCII("Ulysses S. Grant"),       { 1869, republican                } },
        { MDDS_ASCII("Warren G. Harding"),      { 1921, republican                } },
        { MDDS_ASCII("William Henry Harrison"), { 1841, whig                      } },
        { MDDS_ASCII("William Howard Taft"),    { 1909, republican                } },
        { MDDS_ASCII("William McKinley"),       { 1897, republican                } },
        { MDDS_ASCII("Woodrow Wilson"),         { 1913, democratic                } },
        { MDDS_ASCII("Zachary Taylor"),         { 1849, whig                      } },
    };

    map_type us_presidents(entries.data(), entries.size());
    cout << "Number of entries: " << us_presidents.size() << endl;

    cout << endl;

    {
        std::ofstream outfile("us-presidents.bin", std::ios::binary);
        us_presidents.save_state<us_president_serializer>(outfile);
    }

    map_type us_presidents_loaded;

    {
        std::ifstream infile("us-presidents.bin", std::ios::binary);
        us_presidents_loaded.load_state<us_president_serializer>(infile);
    }

    std::ios_base::fmtflags origflags = cout.flags();
    cout << "Equal to the original? " << std::boolalpha << (us_presidents == us_presidents_loaded) << endl;
    cout.setf(origflags);

    cout << endl;

    cout << "Presidents whose first name is 'John':" << endl;
    auto results = us_presidents_loaded.prefix_search("John");
    for (const auto& entry : results)
        cout << "  * " << entry.first << " (" << entry.second.year << "; " << entry.second.party << ")" << endl;

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

