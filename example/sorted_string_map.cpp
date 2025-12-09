/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2024 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

//!code-start: header
#include <mdds/sorted_string_map.hpp>
#include <iostream>
//!code-end: header

//!code-start: enum
enum class us_state_t
{
    unknown,
    alabama,
    alaska,
    arizona,
    arkansas,
    california,
    colorado,
    connecticut,
    delaware,
    florida,
    georgia,
    hawaii,
    idaho,
    illinois,
    indiana,
    iowa,
    kansas,
    kentucky,
    louisiana,
    maine,
    maryland,
    massachusetts,
    michigan,
    minnesota,
    mississippi,
    missouri,
    montana,
    nebraska,
    nevada,
    new_hampshire,
    new_jersey,
    new_mexico,
    new_york,
    north_carolina,
    north_dakota,
    ohio,
    oklahoma,
    oregon,
    pennsylvania,
    rhode_island,
    south_carolina,
    south_dakota,
    tennessee,
    texas,
    utah,
    vermont,
    virginia,
    washington,
    west_virginia,
    wisconsin,
    wyoming,
};
//!code-end: enum

//!code-start: type
using us_state_map_type = mdds::sorted_string_map<us_state_t>;
//!code-end: type

//!code-start: entries
// Keys must be sorted in ascending order.
constexpr us_state_map_type::entry_type us_state_entries[] = {
    { "Alabama", us_state_t::alabama },
    { "Alaska", us_state_t::alaska },
    { "Arizona", us_state_t::arizona },
    { "Arkansas", us_state_t::arkansas },
    { "California", us_state_t::california },
    { "Colorado", us_state_t::colorado },
    { "Connecticut", us_state_t::connecticut },
    { "Delaware", us_state_t::delaware },
    { "Florida", us_state_t::florida },
    { "Georgia", us_state_t::georgia },
    { "Hawaii", us_state_t::hawaii },
    { "Idaho", us_state_t::idaho },
    { "Illinois", us_state_t::illinois },
    { "Indiana", us_state_t::indiana },
    { "Iowa", us_state_t::iowa },
    { "Kansas", us_state_t::kansas },
    { "Kentucky", us_state_t::kentucky },
    { "Louisiana", us_state_t::louisiana },
    { "Maine", us_state_t::maine },
    { "Maryland", us_state_t::maryland },
    { "Massachusetts", us_state_t::massachusetts },
    { "Michigan", us_state_t::michigan },
    { "Minnesota", us_state_t::minnesota },
    { "Mississippi", us_state_t::mississippi },
    { "Missouri", us_state_t::missouri },
    { "Montana", us_state_t::montana },
    { "Nebraska", us_state_t::nebraska },
    { "Nevada", us_state_t::nevada },
    { "New Hampshire", us_state_t::new_hampshire },
    { "New Jersey", us_state_t::new_jersey },
    { "New Mexico", us_state_t::new_mexico },
    { "New York", us_state_t::new_york },
    { "North Carolina", us_state_t::north_carolina },
    { "North Dakota", us_state_t::north_dakota },
    { "Ohio", us_state_t::ohio },
    { "Oklahoma", us_state_t::oklahoma },
    { "Oregon", us_state_t::oregon },
    { "Pennsylvania", us_state_t::pennsylvania },
    { "Rhode Island", us_state_t::rhode_island },
    { "South Carolina", us_state_t::south_carolina },
    { "South Dakota", us_state_t::south_dakota },
    { "Tennessee", us_state_t::tennessee },
    { "Texas", us_state_t::texas },
    { "Utah", us_state_t::utah },
    { "Vermont", us_state_t::vermont },
    { "Virginia", us_state_t::virginia },
    { "Washington", us_state_t::washington },
    { "West Virginia", us_state_t::west_virginia },
    { "Wisconsin", us_state_t::wisconsin },
    { "Wyoming", us_state_t::wyoming },
};
//!code-end: entries

int main() try
{
    auto zero = us_state_t{};
    std::cout << "unknown? " << std::boolalpha << (zero == us_state_t::unknown) << std::endl;

    //!code-start: init
    const us_state_map_type state_map(us_state_entries, std::size(us_state_entries), us_state_t::unknown);
    //!code-end: init

    //!code-start: find
    auto state = state_map.find("Virginia");
    std::cout << "virginia? " << std::boolalpha << (state == us_state_t::virginia) << std::endl;
    state = state_map.find("North Carolina");
    std::cout << "north_carolina? " << std::boolalpha << (state == us_state_t::north_carolina) << std::endl;
    //!code-end: find

    //!code-start: find-null
    state = state_map.find("Alberta");
    std::cout << "unknown? " << std::boolalpha << (state == us_state_t::unknown) << std::endl;
    //!code-end: find-null

    //!code-start: find-key
    auto key = state_map.find_key(us_state_t::rhode_island);
    std::cout << "key for rhode_island: " << key << std::endl;
    //!code-end: find-key

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

