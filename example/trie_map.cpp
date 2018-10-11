
#include <mdds/global.hpp>  // for MDDS_ASCII
#include <mdds/trie_map.hpp>
#include <iostream>

using namespace std;

typedef mdds::trie_map<mdds::trie::std_string_trait, int> trie_map_type;

int main()
{
    // Cities in North Carolina and their populations in 2013.
    trie_map_type nc_cities;

    // Insert key-value pairs.
    nc_cities.insert(MDDS_ASCII("Charlotte"),     792862);
    nc_cities.insert(MDDS_ASCII("Raleigh"),       431746);
    nc_cities.insert(MDDS_ASCII("Greensboro"),    279639);
    nc_cities.insert(MDDS_ASCII("Durham"),        245475);
    nc_cities.insert(MDDS_ASCII("Winston-Salem"), 236441);
    nc_cities.insert(MDDS_ASCII("Fayetteville"),  204408);
    nc_cities.insert(MDDS_ASCII("Cary"),          151088);
    nc_cities.insert(MDDS_ASCII("Wilmington"),    112067);
    nc_cities.insert(MDDS_ASCII("High Point"),    107741);
    nc_cities.insert(MDDS_ASCII("Greenville"),    89130);
    nc_cities.insert(MDDS_ASCII("Asheville"),     87236);
    nc_cities.insert(MDDS_ASCII("Concord"),       83506);
    nc_cities.insert(MDDS_ASCII("Gastonia"),      73209);
    nc_cities.insert(MDDS_ASCII("Jacksonville"),  69079);
    nc_cities.insert(MDDS_ASCII("Chapel Hill"),   59635);
    nc_cities.insert(MDDS_ASCII("Rocky Mount"),   56954);
    nc_cities.insert(MDDS_ASCII("Burlington"),    51510);
    nc_cities.insert(MDDS_ASCII("Huntersville"),  50458);
    nc_cities.insert(MDDS_ASCII("Wilson"),        49628);
    nc_cities.insert(MDDS_ASCII("Kannapolis"),    44359);
    nc_cities.insert(MDDS_ASCII("Apex"),          42214);
    nc_cities.insert(MDDS_ASCII("Hickory"),       40361);
    nc_cities.insert(MDDS_ASCII("Goldsboro"),     36306);

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

    // Create a compressed version of the container.  It works nearly identically.
    auto packed = nc_cities.pack();

    cout << "Cities that start with 'C' and their populations:" << endl;
    auto packed_results = packed.prefix_search(MDDS_ASCII("C"));
    for (const trie_map_type::key_value_type& kv : packed_results)
    {
        cout << "  " << kv.first << ": " << kv.second << endl;
    }

    // Individual search.
    auto it = packed.find(MDDS_ASCII("Wilmington"));
    cout << "Population of Wilmington: " << it->second << endl;

    // You get an end position iterator when the container doesn't have the
    // specified key.
    it = packed.find(MDDS_ASCII("Asheboro"));

    cout << "Population of Asheboro: ";

    if (it == packed.end())
        cout << "not found";
    else
        cout << it->second;

    cout << endl;

    return EXIT_SUCCESS;
}
