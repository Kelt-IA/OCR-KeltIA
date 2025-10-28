#include "../include/tests/include_tests.h"
#include <stdio.h>
#include <unistd.h>

int main()
{
    print_centered_header("Save and Load NNs");
    print_test_result("write nn", test_write_nn());
    print_test_result("load nn", test_load_nn());
    print_test_result("write nn, load", test_write_and_load(0));

    print_centered_header("XOR NN for demo");
    print_test_result("test XOR nn", test_xor_nn());

    test_xor_nn_train(50, 10);
    return 0;
}
