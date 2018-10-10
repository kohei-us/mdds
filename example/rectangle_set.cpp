
#include <mdds/rectangle_set.hpp>
#include <string>
#include <iostream>

using namespace std;

typedef ::mdds::rectangle_set<long, string> rectset_type;

struct string_printer : public unary_function<string*, void>
{
    void operator() (const string& s) const
    {
        cout << "search hit: " << s << endl;
    }
};

int main()
{
    rectset_type db;
    string A("A");
    string B("B");
    string C("C");

    // Insert data into the tree.
    db.insert(0, 0, 12, 11, A);
    db.insert(2, 5, 20, 25, B);
    db.insert(10, 15, 40, 45, C);

    // Run search and get the result.
    rectset_type::search_result result = db.search(5, 10);

    // Print the result.
    cout << "result size: " << result.size() << endl;
    for_each(result.begin(), result.end(), string_printer());
}
