#include "../include/tests/include_tests.h"
#include <unistd.h>

int main()
{
    print_centered_header("Save and Load NNs");
    print_test_result("write nn", test_write_nn());
    print_test_result("load nn", test_load_nn());
    print_test_result("write nn, load", test_write_and_load(0));

    test_xnor_nn_train(50000);
    return 0;
}
