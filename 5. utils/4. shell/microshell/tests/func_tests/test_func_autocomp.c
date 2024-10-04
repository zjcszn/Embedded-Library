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

void setUp(void)
{
        test_func_init();
}

void tearDown(void)
{
        test_func_deinit();
}

void test_autocomp_full(void)
{
        test_func_write("\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "\r\n"
                "help\r\n"
                "ls\r\n"
                "cd\r\n"
                "pwd\r\n"
                "cat\r\n"
                "xxd\r\n"
                "echo\r\n"
                "test\r\n"
                "dir\r\n"
                "data\r\n"
                "[test /]$ ",
                g_write_buf
        );
}

void test_autocomp_part(void)
{
        test_func_write("d\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "d\r\n"
                "dir\r\n"
                "data\r\n"
                "[test /]$ d",
                g_write_buf
        );
}

void test_autocomp_finish(void)
{
        test_func_write("da\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "data",
                g_write_buf
        );

        tearDown();
        setUp();

        test_func_write("h\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "help",
                g_write_buf
        );

        tearDown();
        setUp();

        test_func_write("di\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "dir",
                g_write_buf
        );

        tearDown();
        setUp();

        test_func_write("p\t\n");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "pwd\n"
                "/\r\n"
                "[test /]$ ",
                g_write_buf
        );
}

void test_autocomp_unknown(void)
{
        test_func_write("q\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "q",
                g_write_buf
        );
}

void test_autocomp_args(void)
{
        test_func_write("abcd d\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "abcd d\r\n"
                "dir\r\n"
                "data\r\n"
                "[test /]$ abcd d",
                g_write_buf
        );

        test_func_write("i\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "ir",
                g_write_buf
        );

        tearDown();
        setUp();
        
        TEST_ASSERT_EQUAL(USH_STATUS_OK, ush_node_set_current_dir(&g_ush, "/dir"));

        test_func_write("\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "\r\n"
                "help\r\n"
                "ls\r\n"
                "cd\r\n"
                "pwd\r\n"
                "cat\r\n"
                "xxd\r\n"
                "echo\r\n"
                "2\r\n"
                "1\r\n"
                "[test dir]$ ",
                g_write_buf
        );

        test_func_write("qwe qwe   pw\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "qwe qwe   pwd",
                g_write_buf
        );
        
        test_func_write("\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "",
                g_write_buf
        );

        test_func_write(" \t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                " \r\n"
                "help\r\n"
                "ls\r\n"
                "cd\r\n"
                "pwd\r\n"
                "cat\r\n"
                "xxd\r\n"
                "echo\r\n"
                "2\r\n"
                "1\r\n"
                "[test dir]$ qwe qwe   pwd ",
                g_write_buf
        );
}

void test_autocomp_complex(void)
{
        TEST_ASSERT_EQUAL(USH_STATUS_OK, ush_node_set_current_dir(&g_ush, "/data"));

        test_func_write("te\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "text",
                g_write_buf
        );

        test_func_write("\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "\r\n"
                "text\r\n"
                "text_file1\r\n"
                "text_file2\r\n"
                "[test data]$ text",
                g_write_buf
        );

        test_func_write("_\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "_file",
                g_write_buf
        );

        test_func_write("\t");
        test_func_read_all();
        TEST_ASSERT_EQUAL_STRING(
                "\r\n"
                "text_file1\r\n"
                "text_file2\r\n"
                "[test data]$ text_file",
                g_write_buf
        );
}

int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

        UNITY_BEGIN();

        RUN_TEST(test_autocomp_full);
        RUN_TEST(test_autocomp_part);
        RUN_TEST(test_autocomp_finish);
        RUN_TEST(test_autocomp_unknown);
        RUN_TEST(test_autocomp_args);
        RUN_TEST(test_autocomp_complex);

        return UNITY_END();
}
