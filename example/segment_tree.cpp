
#include <mdds/segment_tree.hpp>
#include <string>
#include <iostream>

using namespace std;

typedef ::mdds::segment_tree<long, string> db_type;

struct string_printer : public unary_function<string*, void>
{
    void operator() (const string& s) const
    {
        cout << "search hit: " << s << endl;
    }
};

int main()
{
    db_type db;
    string A("A");
    string B("B");
    string C("C");

    // Insert data into the tree.
    db.insert(0,  10, A);
    db.insert(2,  20, B);
    db.insert(10, 15, C);

    // Don't forget to build it before calling search().
    db.build_tree();

    // Run search and get the result.
    db_type::search_result result = db.search(5);

    // Print the result.
    cout << "result size: " << result.size() << endl;
    for_each(result.begin(), result.end(), string_printer());
}
