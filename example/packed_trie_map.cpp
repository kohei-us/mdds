
#include <mdds/global.hpp>  // for MDDS_ASCII and MDDS_N_ELEMENTS
#include <mdds/trie_map.hpp>
#include <iostream>

using namespace std;

typedef mdds::packed_trie_map<mdds::trie::std_string_trait, int> trie_map_type;

int main()
{
    // Entries must be known prior to creating the instance, and they must be
    // sorted by the key in ascending order.
    trie_map_type::entry entries[] = {
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

    // Cities in North Carolina and their populations in 2013.
    trie_map_type nc_cities(entries, MDDS_N_ELEMENTS(entries));

    cout << "Cities that start with 'Cha' and their populations:" << endl;
    auto results = nc_cities.prefix_search(MDDS_ASCII("Cha"));
    for (const trie_map_type::key_value_type& kv : results)
    {
        cout << "  " << kv.first << ": " << kv.second << endl;
    }

    cout << "Cities that start with 'W' and their populations:" << endl;
    results = nc_cities.prefix_search(MDDS_ASCII("W"));
    for (const trie_map_type::key_value_type& kv : results)
    {
        cout << "  " << kv.first << ": " << kv.second << endl;
    }

    cout << "Cities that start with 'C' and their populations:" << endl;
    results = nc_cities.prefix_search(MDDS_ASCII("C"));
    for (const trie_map_type::key_value_type& kv : results)
    {
        cout << "  " << kv.first << ": " << kv.second << endl;
    }

    // Individual search.
    auto it = nc_cities.find(MDDS_ASCII("Wilmington"));
    cout << "Population of Wilmington: " << it->second << endl;

    // You get an end position iterator when the container doesn't have the
    // specified key.
    it = nc_cities.find(MDDS_ASCII("Asheboro"));

    cout << "Population of Asheboro: ";

    if (it == nc_cities.end())
        cout << "not found";
    else
        cout << it->second;

    cout << endl;

    return EXIT_SUCCESS;
}
