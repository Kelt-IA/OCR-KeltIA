#include "../include/tests/include_tests.h"
#include <unistd.h>

int main()
{
    print_centered_header("Save and Load NNs");
    print_test_result("write nn", test_write_nn());
    print_test_result("load nn", test_load_nn());
    print_test_result("write nn, load", test_write_and_load(0));
    print_test_result("write cnn with kernels!!, load", test_cnn_save_load(0));
    print_test_result(
        "save - load stride and padding", test_cnn_save_load_stride_padding(0)
    );

    return 0;
}
