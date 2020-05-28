
#include <mdds/trie_map.hpp>
#include <iostream>

using namespace std;

int main()
{
    using trie_map_type = mdds::trie_map<mdds::trie::std_string_trait, int>;

    // Cities in North Carolina and their populations in 2013.
    trie_map_type nc_cities;

    // Insert key-value pairs.
    nc_cities.insert("Charlotte",     792862);
    nc_cities.insert("Raleigh",       431746);
    nc_cities.insert("Greensboro",    279639);
    nc_cities.insert("Durham",        245475);
    nc_cities.insert("Winston-Salem", 236441);
    nc_cities.insert("Fayetteville",  204408);
    nc_cities.insert("Cary",          151088);
    nc_cities.insert("Wilmington",    112067);
    nc_cities.insert("High Point",    107741);
    nc_cities.insert("Greenville",    89130);
    nc_cities.insert("Asheville",     87236);
    nc_cities.insert("Concord",       83506);
    nc_cities.insert("Gastonia",      73209);
    nc_cities.insert("Jacksonville",  69079);
    nc_cities.insert("Chapel Hill",   59635);
    nc_cities.insert("Rocky Mount",   56954);
    nc_cities.insert("Burlington",    51510);
    nc_cities.insert("Huntersville",  50458);
    nc_cities.insert("Wilson",        49628);
    nc_cities.insert("Kannapolis",    44359);
    nc_cities.insert("Apex",          42214);
    nc_cities.insert("Hickory",       40361);
    nc_cities.insert("Goldsboro",     36306);

    cout << "Cities that start with 'Cha' and their populations:" << endl;
    auto results = nc_cities.prefix_search("Cha");
    for (const auto& kv : results)
    {
        cout << "  " << kv.first << ": " << kv.second << endl;
    }

    cout << "Cities that start with 'W' and their populations:" << endl;
    results = nc_cities.prefix_search("W");
    for (const auto& kv : results)
    {
        cout << "  " << kv.first << ": " << kv.second << endl;
    }

    // Create a compressed version of the container.  It works nearly identically.
    auto packed = nc_cities.pack();

    cout << "Cities that start with 'C' and their populations:" << endl;
    auto packed_results = packed.prefix_search("C");
    for (const auto& kv : packed_results)
    {
        cout << "  " << kv.first << ": " << kv.second << endl;
    }

    // Individual search.
    auto it = packed.find("Wilmington");
    cout << "Population of Wilmington: " << it->second << endl;

    // You get an end position iterator when the container doesn't have the
    // specified key.
    it = packed.find("Asheboro");

    cout << "Population of Asheboro: ";

    if (it == packed.end())
        cout << "not found";
    else
        cout << it->second;

    cout << endl;

    return EXIT_SUCCESS;
}
