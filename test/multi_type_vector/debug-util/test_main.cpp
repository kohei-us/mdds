#include <iostream>

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector/soa/main.hpp>
#include <mdds/multi_type_vector/trait.hpp>

struct mtv_custom_trait : public mdds::mtv::default_trait
{
    static void trace(const char* funcname, const char* filename, int lineno, const void* self)
    {
        (void)filename;
        (void)lineno;
        std::cout << "[" << self << "]: func=" << funcname << std::endl;
    }
};

using mtv_type = mdds::mtv::soa::multi_type_vector<mdds::mtv::element_block_func, mtv_custom_trait>;

int main()
{
    mtv_type db(10);
    db.set<std::string>(2, "str");
    db.set<int32_t>(4, 23);
    db.clear();

    mtv_type db2(10);
    db2.set<std::string>(2, "str");
    db2.set<int32_t>(4, 23);
    db2.clear();

    return EXIT_SUCCESS;
}
