/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2020 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include <mdds/global.hpp>  // for MDDS_ASCII and MDDS_N_ELEMENTS
#include <mdds/trie_map.hpp>
#include <iostream>

int main()
{
    //!code-start: type
    using trie_map_type = mdds::packed_trie_map<std::string, int>;
    //!code-end: type

    // Entries must be known prior to creating the instance, and they must be
    // sorted by the key in ascending order.
    //!code-start: populate
    trie_map_type::entry entries[] =
    {
        { MDDS_ASCII("Apex"),           42214 },
        { MDDS_ASCII("Asheville"),      87236 },
        { MDDS_ASCII("Burlington"),     51510 },
        { MDDS_ASCII("Cary"),          151088 },
        { MDDS_ASCII("Chapel Hill"),    59635 },
        { MDDS_ASCII("Charlotte"),     792862 },
        { MDDS_ASCII("Concord"),        83506 },
        { MDDS_ASCII("Durham"),        245475 },
        { MDDS_ASCII("Fayetteville"),  204408 },
        { MDDS_ASCII("Gastonia"),       73209 },
        { MDDS_ASCII("Goldsboro"),      36306 },
        { MDDS_ASCII("Greensboro"),    279639 },
        { MDDS_ASCII("Greenville"),     89130 },
        { MDDS_ASCII("Hickory"),        40361 },
        { MDDS_ASCII("High Point"),    107741 },
        { MDDS_ASCII("Huntersville"),   50458 },
        { MDDS_ASCII("Jacksonville"),   69079 },
        { MDDS_ASCII("Kannapolis"),     44359 },
        { MDDS_ASCII("Raleigh"),       431746 },
        { MDDS_ASCII("Rocky Mount"),    56954 },
        { MDDS_ASCII("Wilmington"),    112067 },
        { MDDS_ASCII("Wilson"),         49628 },
        { MDDS_ASCII("Winston-Salem"), 236441 },
    };
    //!code-end: populate

    // Cities in North Carolina and their populations in 2013.
    //!code-start: inst
    trie_map_type nc_cities(entries, std::size(entries));
    //!code-end: inst

    std::cout << "Cities that start with 'Cha' and their populations:" << std::endl;
    auto results = nc_cities.prefix_search("Cha");
    for (const auto& kv : results)
    {
        std::cout << "  " << kv.first << ": " << kv.second << std::endl;
    }

    std::cout << "Cities that start with 'W' and their populations:" << std::endl;
    results = nc_cities.prefix_search("W");
    for (const auto& kv : results)
    {
        std::cout << "  " << kv.first << ": " << kv.second << std::endl;
    }

    std::cout << "Cities that start with 'C' and their populations:" << std::endl;
    results = nc_cities.prefix_search("C");
    for (const auto& kv : results)
    {
        std::cout << "  " << kv.first << ": " << kv.second << std::endl;
    }

    // Individual search.
    auto it = nc_cities.find("Wilmington");
    std::cout << "Population of Wilmington: " << it->second << std::endl;

    // You get an end position iterator when the container doesn't have the
    // specified key.
    it = nc_cities.find("Asheboro");

    std::cout << "Population of Asheboro: ";

    if (it == nc_cities.end())
        std::cout << "not found";
    else
        std::cout << it->second;

    std::cout << std::endl;

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
