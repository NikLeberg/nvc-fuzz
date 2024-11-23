/**
 * @file fuzzer.c
 * @author Niklaus Leuenberger <@NikLeberg>
 * @brief Implements in-process fuzzer for NVC based on LLVM's libFuzzer.
 * @version 0.1
 * @date 2024-11-23
 *
 * SPDX-License-Identifier: MIT
 *
 */


#include <stdlib.h>
#include <stdint.h>

#include "tester.h"


/**
 * @brief Initialize fuzzing
 *
 * Gets called from libFuzzer once before fuzzing begins. Initializes the
 * fuzzing test fixture that is used for the duration of the repeated tests.
 *
 * @param argc unused
 * @param argv unused
 * @retval 0
 */
int LLVMFuzzerInitialize(int *argc, char ***argv) {
    (void)argc;
    (void)argv;

    fixture_setup();
    atexit(fixture_teardown);

    return 0;
}

/**
 * @brief Main fuzzer
 *
 * Gets called from libFuzzer's main() and feeds data to the API under test.
 *
 * @param data input data from fuzzer
 * @param size length of data
 * @retval 0
 */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    test_setup();
    test_run_forked((const char *)data, size);
    test_teardown();

    return 0;
}
