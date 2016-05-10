
#include <mdds/multi_type_matrix.hpp>
#include <vector>

typedef mdds::multi_type_matrix<mdds::mtm::std_string_trait> mtm_type;

int main()
{
    // Create a 3x3 matrix with empty elements.
    mtm_type mx(3, 3);

    // Store values of four different types.
    mx.set(0, 0, 1.1);  // numeric value (double)
    mx.set(1, 0, std::string("text")); // string value
    mx.set(0, 1, true); // boolean value
    mx.set(1, 1, int(12)); // integer value

    std::vector<double> values = {
        1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9
    };

    // Set values for all elements in one step.
    mx.set(0, 0, values.begin(), values.end());

    return EXIT_SUCCESS;
}
