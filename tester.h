/**
 * @file tester.h
 * @author Niklaus Leuenberger <@NikLeberg>
 * @brief Wrapper around NVC to allow repeated tests on same invocation.
 * @version 0.1
 * @date 2024-11-23
 *
 * SPDX-License-Identifier: MIT
 *
 */


#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void fixture_setup(void);
    void fixture_teardown(void);

    void test_setup(void);
    void test_teardown(void);

    void test_run(const char *buf, size_t len);
    void test_run_forked(const char *buf, size_t len);

#ifdef __cplusplus
}
#endif
