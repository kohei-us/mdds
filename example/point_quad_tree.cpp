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

#include <mdds/point_quad_tree.hpp>
#include <string>
#include <iostream>

using std::cout;
using std::endl;

int main() try
{
    typedef ::mdds::point_quad_tree<double, std::string> db_type;
    db_type db;
    
    std::string chicago("Chicago");
    std::string hamburg("Hamburg");
    std::string helsinki("Helsinki");
    std::string london("London");
    std::string paris("Paris");
    std::string prague("Prague");
    std::string shanghai("Shanghai");
    std::string tokyo("Tokyo");

    // insert cities by longitude and latitude.
    db.insert(-87.755280, 41.784168, chicago);
    db.insert(10.000000, 53.633331, hamburg);
    db.insert(25.049999, 60.250000, helsinki);
    db.insert(-0.450000, 51.483334, london);
    db.insert(2.450000, 48.966667, paris);
    db.insert(14.250000, 50.099998, prague);
    db.insert(121.433334, 31.166668, shanghai);
    db.insert(139.850006, 35.633331, tokyo);

    // Perform region search.
    db_type::search_results results = db.search_region(100, 30, 140, 40);

    // Print out the result of the search.
    cout << "Cities located between longitudes 100 and 140 east and latitudes 30 and 40 north are:" << endl;
    for (const auto& res : results)
    {
        cout << "  " << res.second
             << ": (latitude=" << res.first.x
             << ", longitude=" << res.first.y << ")" << endl;
    }
}
catch (...)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
