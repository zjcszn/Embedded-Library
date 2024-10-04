/*
MIT License

Copyright (c) 2021 Marcin Borowicz <marcinbor85@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef TEST_FUNC_H
#define TEST_FUNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity.h"

#define TEST_FUNC_IO_BUFFER_SIZE        2048
#define TEST_FUNC_WORK_BUFFER_SIZE      256

extern char g_write_buf[];
extern char g_read_buf[];

extern uint8_t g_file_buffer_buf[];
extern size_t g_file_buffer_buf_size;

extern struct ush_object g_ush;

extern const struct ush_file_descriptor g_path_root_desc[];
extern const struct ush_file_descriptor g_path_data_desc[];
extern const struct ush_file_descriptor g_path_dir111_desc[];

extern struct ush_node_object g_path_root;
extern struct ush_node_object g_path_data;
extern struct ush_node_object g_path_dir;
extern struct ush_node_object g_path_dir11;
extern struct ush_node_object g_path_dir111;

void test_func_init(void);
void test_func_deinit(void);
void test_func_write(const char *text);
void test_func_read(bool reset_g_write_buf_index, int ush_service_loops);
void test_func_read_all(void);

#define TEST_FUNC_ASK(request, response) \
{\
    char buf[TEST_FUNC_IO_BUFFER_SIZE];\
    char buf_resp[TEST_FUNC_IO_BUFFER_SIZE];\
\
    sprintf(buf, "%s\n", request);\
    sprintf(buf_resp, "%s\n%s", request, response);\
    test_func_write(buf);\
    test_func_read_all();\
    TEST_ASSERT_EQUAL_STRING(\
            buf_resp,\
            g_write_buf\
    );\
}

#ifdef __cplusplus
}
#endif

#endif /* TEST_FUNC_H */
