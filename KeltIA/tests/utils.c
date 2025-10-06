#include "../include/tests/include_tests.h"
#include <stdio.h>
#include <string.h>

void print_test_result(const char *test_name, int passed)
{
    if (passed == 1)
    {
        printf("%s[PASS]%s %s\n", COLOR_GREEN, COLOR_RESET, test_name);
        return;
    }

    printf("%s[FAIL]%s %s\n", COLOR_RED, COLOR_RESET, test_name);
}

void print_centered_header(const char *title)
{
    const int total_width = 40;

    printf(COLOR_BLUE "\n========================================\n");

    // Calculate padding for centering
    int title_len = strlen(title);
    int padding = (total_width - title_len) / 2;

    // Print centered title
    for (int i = 0; i < padding; i++) { printf(" "); }
    printf("%s\n", title);

    printf("========================================\n" COLOR_RESET);
}
