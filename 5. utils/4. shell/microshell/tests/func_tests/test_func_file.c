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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <microshell.h>
#include "test_func.h"

static struct ush_node_object node = {0};

static struct ush_file_descriptor files[] = {
        {
                .name = "cmd1",
                .description = "additional cmd1",
        },
        {
                .name = "cmd2",
                .description = "additional cmd2",
        },
};

void setUp(void)
{
        test_func_init();
}

void tearDown(void)
{
        test_func_deinit();
}

void test_file_find_cmd(void)
{
        TEST_ASSERT_NULL(ush_file_find_by_name(&g_ush, "cmd1"));
        TEST_ASSERT_EQUAL(USH_STATUS_OK, ush_commands_add(&g_ush, &node, files, sizeof(files) / sizeof(files[0])));
        TEST_ASSERT_EQUAL(&files[0], ush_file_find_by_name(&g_ush, "cmd1"));
        TEST_ASSERT_EQUAL(&files[1], ush_file_find_by_name(&g_ush, "cmd2"));

        TEST_ASSERT_EQUAL(&g_ush_buildin_commands[0], ush_file_find_by_name(&g_ush, "help"));
        TEST_ASSERT_EQUAL(&g_ush_buildin_commands[1], ush_file_find_by_name(&g_ush, "ls"));
        TEST_ASSERT_EQUAL(&g_ush_buildin_commands[2], ush_file_find_by_name(&g_ush, "cd"));
        TEST_ASSERT_EQUAL(&g_ush_buildin_commands[3], ush_file_find_by_name(&g_ush, "pwd"));
        TEST_ASSERT_EQUAL(&g_ush_buildin_commands[4], ush_file_find_by_name(&g_ush, "cat"));
        TEST_ASSERT_EQUAL(&g_ush_buildin_commands[5], ush_file_find_by_name(&g_ush, "xxd"));
        TEST_ASSERT_EQUAL(&g_ush_buildin_commands[6], ush_file_find_by_name(&g_ush, "echo"));
}

void test_file_find_cmd_deeper(void)
{
        TEST_ASSERT_EQUAL(USH_STATUS_OK, ush_node_set_current_dir(&g_ush, "/dir/1/11/111"));

        test_file_find_cmd();
}

void test_file_find_abs(void)
{
        TEST_ASSERT_EQUAL(&g_path_root_desc[0], ush_file_find_by_name(&g_ush, "/test"));
        TEST_ASSERT_EQUAL(&g_path_data_desc[0], ush_file_find_by_name(&g_ush, "/data/text"));
        TEST_ASSERT_EQUAL(&g_path_data_desc[5], ush_file_find_by_name(&g_ush, "/data/binary"));
        TEST_ASSERT_EQUAL(&g_path_dir111_desc[0], ush_file_find_by_name(&g_ush, "/dir/1/11/111/joke"));

        TEST_ASSERT_EQUAL(USH_STATUS_OK, ush_node_set_current_dir(&g_ush, "/dir/1/11/111"));

        TEST_ASSERT_EQUAL(&g_path_root_desc[0], ush_file_find_by_name(&g_ush, "/test"));
        TEST_ASSERT_EQUAL(&g_path_data_desc[0], ush_file_find_by_name(&g_ush, "/data/text"));
        TEST_ASSERT_EQUAL(&g_path_data_desc[5], ush_file_find_by_name(&g_ush, "/data/binary"));
        TEST_ASSERT_EQUAL(&g_path_dir111_desc[0], ush_file_find_by_name(&g_ush, "/dir/1/11/111/joke"));
}

void test_file_find_rel(void)
{
        TEST_ASSERT_EQUAL(&g_path_root_desc[0], ush_file_find_by_name(&g_ush, "test"));
        TEST_ASSERT_EQUAL(&g_path_data_desc[0], ush_file_find_by_name(&g_ush, "data/text"));
        TEST_ASSERT_EQUAL(&g_path_data_desc[5], ush_file_find_by_name(&g_ush, "data/binary"));
        TEST_ASSERT_EQUAL(&g_path_dir111_desc[0], ush_file_find_by_name(&g_ush, "dir/1/11/111/joke"));

        TEST_ASSERT_EQUAL(USH_STATUS_OK, ush_node_set_current_dir(&g_ush, "/dir/1/11/111"));

        TEST_ASSERT_NULL(ush_file_find_by_name(&g_ush, "test"));
        TEST_ASSERT_NULL(ush_file_find_by_name(&g_ush, "data/text"));
        TEST_ASSERT_NULL(ush_file_find_by_name(&g_ush, "data/binary"));
        TEST_ASSERT_NULL(ush_file_find_by_name(&g_ush, "dir/1/11/111/joke"));

        TEST_ASSERT_EQUAL(&g_path_root_desc[0], ush_file_find_by_name(&g_ush, "../../../../test"));
        TEST_ASSERT_EQUAL(&g_path_data_desc[0], ush_file_find_by_name(&g_ush, "../../../../data/text"));
        TEST_ASSERT_EQUAL(&g_path_data_desc[5], ush_file_find_by_name(&g_ush, "../../../../data/binary"));
        TEST_ASSERT_EQUAL(&g_path_dir111_desc[0], ush_file_find_by_name(&g_ush, "joke"));
}

void test_file_error(void)
{
        TEST_ASSERT_NULL(ush_file_find_by_name(&g_ush, "qwerty"));
        TEST_ASSERT_NULL(ush_file_find_by_name(&g_ush, "./help"));
        TEST_ASSERT_NULL(ush_file_find_by_name(&g_ush, "/data/test"));
        TEST_ASSERT_NULL(ush_file_find_by_name(&g_ush, "./data/../binary"));
        TEST_ASSERT_NULL(ush_file_find_by_name(&g_ush, "/joke"));
}

int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

        UNITY_BEGIN();

        RUN_TEST(test_file_find_cmd);
        RUN_TEST(test_file_find_cmd_deeper);
        RUN_TEST(test_file_find_abs);
        RUN_TEST(test_file_find_rel);
        RUN_TEST(test_file_error);

        return UNITY_END();
}
