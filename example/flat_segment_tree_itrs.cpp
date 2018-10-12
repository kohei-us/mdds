
#include <mdds/flat_segment_tree.hpp>
#include <iostream>

using namespace std;

using fst_type = mdds::flat_segment_tree<long, int>;

void iterate_nodes(const fst_type& db)
{
    for (auto it = db.begin(); it != db.end(); ++it)
    {
        cout << "key: " << it->first << "; value: " << it->second << endl;
    }
}

void loop_nodes(const fst_type& db)
{
    for (const auto& node : db)
    {
        cout << "key: " << node.first << "; value: " << node.second << endl;
    }
}

void iterate_segments(const fst_type& db)
{
    for (auto it = db.begin_segment(); it != db.end_segment(); ++it)
    {
        cout << "start: " << it->start << "; end: " << it->end << "; value: " << it->value << endl;
    }
}

int main()
{
    fst_type db(0, 500, 0);
    
    db.insert_front(10, 20, 10);
    db.insert_back(50, 70, 15);
    db.insert_back(60, 65, 5);

    iterate_nodes(db);
    loop_nodes(db);
    iterate_segments(db);
}
