
#include <mdds/multi_type_vector/collection.hpp>
#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>

#include <vector>
#include <iostream>

void example1()
{
    using mtv_type = mdds::multi_type_vector<mdds::mtv::element_block_func>;
    using collection_type = mdds::mtv::collection<mtv_type>;

    std::vector<mtv_type> columns(5);

    std::vector<std::string> headers = { "ID", "Make", "Model", "Year", "Color" };

    // Populate the header row.
    for (size_t i = 0, n = headers.size(); i < n; ++i)
        columns[i].push_back<std::string>(headers[i]);

    // Fill column 1.
    std::vector<int> c1_values = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
    mtv_type& col1 = columns[0];
    for (int v : c1_values)
        col1.push_back(v);

    // Fill column 2.
    std::vector<std::string> c2_values =
    {
        "Nissan", "Mercedes-Benz", "Nissan", "Suzuki", "Saab",
        "Subaru", "GMC", "Mercedes-Benz", "Toyota", "Nissan",
        "Mazda", "Dodge", "Ford", "Bentley", "GMC",
        "Audi", "GMC", "Mercury", "Pontiac", "BMW",
    };

    mtv_type& col2 = columns[1];
    for (const std::string& v : c2_values)
        col2.push_back(v);

    // Fill column 3.
    std::vector<std::string> c3_values =
    {
        "Frontier", "W201", "Frontier", "Equator", "9-5",
        "Tribeca", "Yukon XL 2500", "E-Class", "Camry Hybrid", "Frontier",
        "MX-5", "Ram Van 1500", "Edge", "Azure", "Sonoma Club Coupe",
        "S4", "3500 Club Coupe", "Villager", "Sunbird", "3 Series",
    };

    mtv_type& col3 = columns[2];
    for (const std::string& v : c3_values)
        col3.push_back(v);

    // Fill column 4.
    std::vector<int> c4_values =
    {
        1998, 1986, 2009, -1, -1, 2008, 2009, 2008, 2010, 2001,
        2008, 2000, -1, 2009, 1998, 2013, 1994, 2000, 1990, 1993,
    };

    mtv_type& col4 = columns[3];
    for (int v : c4_values)
    {
        if (v < 0)
            col4.push_back<std::string>("unknown");
        else
            col4.push_back(v);
    }

    // Fill column 5
    std::vector<std::string> c5_values
    {
        "Turquoise", "Fuscia", "Teal", "Fuscia", "Green",
        "Khaki", "Pink", "Goldenrod", "Turquoise", "Yellow",
        "Orange", "Goldenrod", "Fuscia", "Goldenrod", "Mauv",
        "Crimson", "Turquoise", "Teal", "Indigo", "LKhaki",
    };

    mtv_type& col5 = columns[4];
    for (const std::string& v : c5_values)
        col5.push_back(v);

    // Wrap the columns with the 'collection'...
    collection_type collection(columns.begin(), columns.end());

    for (const auto& v : collection)
    {
        if (v.index > 0)
            // Insert a column separator.
            std::cout << " | ";

        switch (v.type)
        {
            // In this example, we use two element types only.
            case mdds::mtv::element_type_int:
                std::cout << v.get<mdds::mtv::int_element_block>();
                break;
            case mdds::mtv::element_type_string:
                std::cout << v.get<mdds::mtv::string_element_block>();
                break;
            default:
                std::cout << "???"; // The default case should not hit in this example.
        }

        if (v.index == 4)
            std::cout << std::endl;
    }
}

int main()
{
    example1();

    return EXIT_SUCCESS;
}

