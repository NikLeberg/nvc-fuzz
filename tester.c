/**
 * @file tester.c
 * @author Niklaus Leuenberger <@NikLeberg>
 * @brief Wrapper around NVC to allow repeated tests on same invocation.
 * @version 0.1
 * @date 2024-11-23
 *
 * SPDX-License-Identifier: MIT
 *
 */


#include <stdlib.h>

#include "nvc/src/common.h"
#include "nvc/src/option.h"
#include "nvc/src/phase.h"
#include "nvc/src/rt/mspace.h"
#include "nvc/src/thread.h"
#include "nvc/src/scan.h"
#include "nvc/src/lib.h"
#include "nvc/src/diag.h"

#include "tester.h"

const char version_string[] = "fuzzer"; // required for util.c show_stacktrace()

static void diag_consumer(diag_t *diag, void *ctx);
static void *run_thread(void *arg);

typedef struct test_input_s {
    const char *buf;
    size_t len;
} test_input_t;

void fixture_setup(void) {
    term_init();
    thread_init();
    set_default_options();
    intern_strings();
    mspace_stack_limit(MSPACE_CURRENT_FRAME);
    diag_set_consumer(diag_consumer, NULL);

    opt_set_int(OPT_UNIT_TEST, 1);
    opt_set_int(OPT_IGNORE_TIME, 1);
    opt_set_int(OPT_RELAXED, 1);

    if (getenv("NVC_LIBPATH") == NULL) {
        // CMake specific location to where NVC is built
        setenv("NVC_LIBPATH", "./nvc-prefix/src/nvc-build/lib", 1);
    }

    set_standard(STD_08);
    // ToDo: do we want to help the fuzzer and exit early on errors?
    // set_error_limit(1);
}

void fixture_teardown(void) {
    // ...
}

static lib_t work_lib;
void test_setup(void) {
    work_lib = lib_tmp("work");
    lib_set_work(work_lib);
    reset_error_count();
}

void test_teardown(void) {
    lib_set_work(NULL);
    lib_free(work_lib);
    work_lib = NULL;
}

void test_run(const char *buf, size_t len) {
    input_from_buffer(buf, len, SOURCE_VHDL);
    tree_t t = parse();
}

void test_run_forked(const char *buf, size_t len) {
    test_input_t *input = malloc(sizeof(test_input_t));
    input->buf = buf;
    input->len = len;
    nvc_thread_t *thread = thread_create(run_thread, (void*)input, "fuzzing thread");
    thread_join(thread);
    free(input);
}

static void diag_consumer(diag_t *diag, void *ctx) {
    (void)ctx;
    (void)diag;
}

static void *run_thread(void *arg) {
    diag_set_consumer(diag_consumer, NULL);
    test_input_t *input = (test_input_t*)arg;
    test_run(input->buf, input->len);
    return NULL;
}
