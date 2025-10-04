#include "../include/tests/include_tests.h"
#include <stdio.h>
#include <unistd.h>

#define COLOR_GREEN "\x1b[32m"
#define COLOR_RED "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

void print_test_result(const char *test_name, int passed)
{
    if (passed == 1)
    {
        printf("%s[PASS]%s %s\n", COLOR_GREEN, COLOR_RESET, test_name);
        return;
    }

    printf("%s[FAIL]%s %s\n", COLOR_RED, COLOR_RESET, test_name);
}

int main()
{
    print_test_result("write nn", test_write_nn());
    print_test_result("load nn", test_load_nn());
    print_test_result("write nn, load", test_write_and_load());
    return 0;
}
