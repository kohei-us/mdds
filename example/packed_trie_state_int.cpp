/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2020 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <vector>
#include <fstream>
#include <mdds/trie_map.hpp>

int main() try
{
    //!code-start: type
    using map_type = mdds::packed_trie_map<std::string, int>;
    //!code-end: type

    // List of world's largest cities and their populations. The entries must
    // be sorted by the keys.
    //
    // c.f. https://en.wikipedia.org/wiki/List_of_largest_cities#cities

    //!code-start: entries
    std::vector<map_type::entry> entries =
    {
        { MDDS_ASCII("Ahmedabad"),        7681000  },
        { MDDS_ASCII("Alexandria"),       5086000  },
        { MDDS_ASCII("Atlanta"),          5572000  },
        { MDDS_ASCII("Baghdad"),          6812000  },
        { MDDS_ASCII("Bangalore"),        11440000 },
        { MDDS_ASCII("Bangkok"),          10156000 },
        { MDDS_ASCII("Barcelona"),        5494000  },
        { MDDS_ASCII("Beijing"),          19618000 },
        { MDDS_ASCII("Belo Horizonte"),   5972000  },
        { MDDS_ASCII("Bogota"),           10574000 },
        { MDDS_ASCII("Buenos Aires"),     14967000 },
        { MDDS_ASCII("Cairo"),            20076000 },
        { MDDS_ASCII("Chengdu"),          8813000  },
        { MDDS_ASCII("Chennai"),          10456000 },
        { MDDS_ASCII("Chicago"),          8864000  },
        { MDDS_ASCII("Chongqing"),        14838000 },
        { MDDS_ASCII("Dalian"),           5300000  },
        { MDDS_ASCII("Dallas"),           6099000  },
        { MDDS_ASCII("Dar es Salaam"),    6048000  },
        { MDDS_ASCII("Delhi"),            28514000 },
        { MDDS_ASCII("Dhaka"),            19578000 },
        { MDDS_ASCII("Dongguan"),         7360000  },
        { MDDS_ASCII("Foshan"),           7236000  },
        { MDDS_ASCII("Fukuoka"),          5551000  },
        { MDDS_ASCII("Guadalajara"),      5023000  },
        { MDDS_ASCII("Guangzhou"),        12638000 },
        { MDDS_ASCII("Hangzhou"),         7236000  },
        { MDDS_ASCII("Harbin"),           6115000  },
        { MDDS_ASCII("Ho Chi Minh City"), 8145000  },
        { MDDS_ASCII("Hong Kong"),        7429000  },
        { MDDS_ASCII("Houston"),          6115000  },
        { MDDS_ASCII("Hyderabad"),        9482000  },
        { MDDS_ASCII("Istanbul"),         14751000 },
        { MDDS_ASCII("Jakarta"),          10517000 },
        { MDDS_ASCII("Jinan"),            5052000  },
        { MDDS_ASCII("Johannesburg"),     5486000  },
        { MDDS_ASCII("Karachi"),          15400000 },
        { MDDS_ASCII("Khartoum"),         5534000  },
        { MDDS_ASCII("Kinshasa"),         13171000 },
        { MDDS_ASCII("Kolkata"),          14681000 },
        { MDDS_ASCII("Kuala Lumpur"),     7564000  },
        { MDDS_ASCII("Lagos"),            13463000 },
        { MDDS_ASCII("Lahore"),           11738000 },
        { MDDS_ASCII("Lima"),             10391000 },
        { MDDS_ASCII("London"),           9046000  },
        { MDDS_ASCII("Los Angeles"),      12458000 },
        { MDDS_ASCII("Luanda"),           7774000  },
        { MDDS_ASCII("Madrid"),           6497000  },
        { MDDS_ASCII("Manila"),           13482000 },
        { MDDS_ASCII("Mexico City"),      21581000 },
        { MDDS_ASCII("Miami"),            6036000  },
        { MDDS_ASCII("Moscow"),           12410000 },
        { MDDS_ASCII("Mumbai"),           19980000 },
        { MDDS_ASCII("Nagoya"),           9507000  },
        { MDDS_ASCII("Nanjing"),          8245000  },
        { MDDS_ASCII("New York City"),    18819000 },
        { MDDS_ASCII("Osaka"),            19281000 },
        { MDDS_ASCII("Paris"),            10901000 },
        { MDDS_ASCII("Philadelphia"),     5695000  },
        { MDDS_ASCII("Pune"),             6276000  },
        { MDDS_ASCII("Qingdao"),          5381000  },
        { MDDS_ASCII("Rio de Janeiro"),   13293000 },
        { MDDS_ASCII("Riyadh"),           6907000  },
        { MDDS_ASCII("Saint Petersburg"), 5383000  },
        { MDDS_ASCII("Santiago"),         6680000  },
        { MDDS_ASCII("Sao Paulo"),        21650000 },
        { MDDS_ASCII("Seoul"),            9963000  },
        { MDDS_ASCII("Shanghai"),         25582000 },
        { MDDS_ASCII("Shenyang"),         6921000  },
        { MDDS_ASCII("Shenzhen"),         11908000 },
        { MDDS_ASCII("Singapore"),        5792000  },
        { MDDS_ASCII("Surat"),            6564000  },
        { MDDS_ASCII("Suzhou"),           6339000  },
        { MDDS_ASCII("Tehran"),           8896000  },
        { MDDS_ASCII("Tianjin"),          13215000 },
        { MDDS_ASCII("Tokyo"),            37400068 },
        { MDDS_ASCII("Toronto"),          6082000  },
        { MDDS_ASCII("Washington, D.C."), 5207000  },
        { MDDS_ASCII("Wuhan"),            8176000  },
        { MDDS_ASCII("Xi'an"),            7444000  },
        { MDDS_ASCII("Yangon"),           5157000  },
    };
    //!code-end: entries

    //!code-start: inst
    map_type cities(entries.data(), entries.size());
    //!code-end: inst

    //!code-start: print-n-cities
    std::cout << "Number of cities: " << cities.size() << std::endl;
    //!code-end: print-n-cities

    std::cout << std::endl;

    {
        //!code-start: search-s
        std::cout << "Cities that begin with 'S':" << std::endl;
        auto results = cities.prefix_search("S");
        for (const auto& city : results)
            std::cout << "  * " << city.first << ": " << city.second << std::endl;
        //!code-end: search-s
    }

    std::cout << std::endl;

    {
        //!code-start: save-state-cities
        std::ofstream outfile("cities.bin", std::ios::binary);
        cities.save_state(outfile);
        //!code-end: save-state-cities
    }

    //!code-start: load-state-cities
    map_type cities_loaded;

    std::ifstream infile("cities.bin", std::ios::binary);
    cities_loaded.load_state(infile);
    //!code-end: load-state-cities

    std::ios_base::fmtflags origflags = std::cout.flags();
    //!code-start: check-equal
    std::cout << "Equal to the original? " << std::boolalpha << (cities == cities_loaded) << std::endl;
    //!code-end: check-equal
    std::cout.setf(origflags);

    std::cout << std::endl;

    //!code-start: print-n-cities-2
    std::cout << "Number of cities: " << cities_loaded.size() << std::endl;
    //!code-end: print-n-cities-2

    std::cout << std::endl;

    //!code-start: search-s-2
    std::cout << "Cities that begin with 'S':" << std::endl;
    auto results = cities_loaded.prefix_search("S");
    for (const auto& city : results)
        std::cout << "  * " << city.first << ": " << city.second << std::endl;
    //!code-end: search-s-2

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
