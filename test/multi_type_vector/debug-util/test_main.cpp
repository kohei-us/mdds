#include <iostream>

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector/soa/main.hpp>
#include <mdds/multi_type_vector/trait.hpp>

using mdds::mtv::trace_method_t;

namespace {

struct checked_method_props
{
    const void* instance = nullptr;
    std::string function_name;
    trace_method_t type = trace_method_t::unspecified;

    bool operator== (const checked_method_props& other) const
    {
        return instance == other.instance && function_name == other.function_name && type == other.type;
    }
};

class test_scope
{
    std::vector<checked_method_props>& m_expected;
    std::vector<checked_method_props>& m_observed;
public:
    test_scope(std::vector<checked_method_props>& _expected, std::vector<checked_method_props>& _observed) :
        m_expected(_expected), m_observed(_observed) {}

    ~test_scope()
    {
        m_expected.clear();
        m_observed.clear();
    }
};

std::vector<checked_method_props> observed;

struct mtv_custom_trait : public mdds::mtv::default_trait
{
    static void trace(const mdds::mtv::trace_method_properties_t& props)
    {
        std::cout << "[" << props.instance << "]: {" << props.function_name
            << ": " << props.function_args
            << "}; type=" << int(props.type)
            << std::endl;

        observed.push_back({ props.instance, props.function_name, props.type });
    }
};

using mtv_type = mdds::mtv::soa::multi_type_vector<mdds::mtv::element_block_func, mtv_custom_trait>;

} // anonymous namespace

int main()
{
    std::vector<checked_method_props> expected;

    {
        test_scope ts{expected, observed};
        {
            mtv_type db(10);
            auto pos = db.begin();
            pos = db.set<std::string>(pos, 2, "str");
            pos = db.set<int32_t>(pos, 4, 23);
            db.clear();

            expected = {
                { &db, "multi_type_vector", trace_method_t::constructor },
                { &db, "begin", trace_method_t::accessor },
                { &db, "set", trace_method_t::mutator },
                { &db, "set", trace_method_t::mutator },
                { &db, "clear", trace_method_t::mutator },
                { &db, "~multi_type_vector", trace_method_t::destructor },
            };
        }

        assert(observed == expected);
    }

    {
        test_scope ts{expected, observed};
        {
            mtv_type db(10);
            db.set<std::string>(2, "str");
            db.set<int32_t>(4, 23);
            db.clear();

            expected = {
                { &db, "multi_type_vector", trace_method_t::constructor },
                { &db, "set", trace_method_t::mutator },
                { &db, "set", trace_method_t::mutator },
                { &db, "clear", trace_method_t::mutator },
                { &db, "~multi_type_vector", trace_method_t::destructor },
            };
        }

        assert(observed == expected);
    }

    return EXIT_SUCCESS;
}
