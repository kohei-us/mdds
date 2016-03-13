
#include <mdds/point_quad_tree.hpp>
#include <string>
#include <iostream>

using namespace std;

int main()
{
    typedef ::mdds::point_quad_tree<double, const string*> db_type;
    db_type db;
    
    string chicago("Chicago");
    string hamburg("Hamburg");
    string helsinki("Helsinki");
    string london("London");
    string paris("Paris");
    string prague("Prague");
    string shanghai("Shanghai");
    string tokyo("Tokyo");

    // insert cities by longitude and latitude.
    db.insert(-87.755280, 41.784168, &chicago);
    db.insert(10.000000, 53.633331, &hamburg);
    db.insert(25.049999, 60.250000, &helsinki);
    db.insert(-0.450000, 51.483334, &london);
    db.insert(2.450000, 48.966667, &paris);
    db.insert(14.250000, 50.099998, &prague);
    db.insert(121.433334, 31.166668, &shanghai);
    db.insert(139.850006, 35.633331, &tokyo);

    // Perform region search.
    db_type::search_results result = db.search_region(100, 30, 140, 40);

    // Print out the result of the search.
    cout << "Cities located between longitudes 100 and 140 east and latitudes 30 and 40 north are:" << endl;
    db_type::search_results::const_iterator itr = result.begin(), itr_end = result.end();
    for (; itr != itr_end; ++itr)
    {
        cout << "  " << *itr->second 
             << ": (latitude=" << itr->first.x 
             << ", longitude=" << itr->first.y << ")" << endl;
    }
}
