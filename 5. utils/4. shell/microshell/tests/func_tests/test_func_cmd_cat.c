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

void test_cmd_cat_single(void)
{
        sprintf((char*)g_file_buffer_buf, "test_data");
        g_file_buffer_buf_size = 9;
        TEST_FUNC_ASK("cat /data/buffer",
                "test_data[test /]$ "
        );

        sprintf((char*)g_file_buffer_buf, "\x01\x02""abcd\x03\xFF");
        g_file_buffer_buf_size = 8;
        TEST_FUNC_ASK("cat /data/buffer",
                "\x01\x02""abcd\x03\xFF""[test /]$ "
        );

        TEST_FUNC_ASK("cat /data/null",
                "[test /]$ "
        );
}

void test_cmd_cat_multi(void)
{
        sprintf((char*)g_file_buffer_buf, "test");
        g_file_buffer_buf_size = 4;
        TEST_FUNC_ASK("cat /data/buffer /data/buffer data/../dir/../data/buffer",
                "testtesttest[test /]$ "
        );
}

void test_cmd_cat_multi_binary(void)
{
        char buf_req[TEST_FUNC_IO_BUFFER_SIZE];
        uint8_t buf_resp[TEST_FUNC_IO_BUFFER_SIZE];

        TEST_ASSERT_EQUAL(USH_STATUS_OK, ush_node_set_current_dir(&g_ush, "/data"));

        strcpy(buf_req, "cat /data/buffer ./binary\n");

        sprintf((char*)g_file_buffer_buf, "abcd");
        g_file_buffer_buf_size = 4;
        memset(buf_resp, 0, sizeof(buf_resp));
        strcpy((char*)buf_resp, buf_req);
        buf_resp[26 + 0] = 'a';
        buf_resp[26 + 1] = 'b';
        buf_resp[26 + 2] = 'c';
        buf_resp[26 + 3] = 'd';        
        for (size_t i = 0; i < 256; i++)
                buf_resp[26 + 4 + i] = i;
        strcpy((char*)&buf_resp[26 + 4 + 256], "[test data]$ ");

        test_func_write(buf_req);
        test_func_read_all();
        TEST_ASSERT_EQUAL_UINT8_ARRAY(buf_resp, g_write_buf, (32 + 4 + 256 + 13));

        TEST_ASSERT_EQUAL(USH_STATUS_OK, ush_node_set_current_dir(&g_ush, "/dir"));
        
        strcpy(buf_req, "cat /data/binary ../data/buffer\n");

        g_file_buffer_buf[0] = '\0';
        g_file_buffer_buf[1] = 'T';
        g_file_buffer_buf_size = 2;
        memset(buf_resp, 0, sizeof(buf_resp));
        strcpy((char*)buf_resp, buf_req);
        for (size_t i = 0; i < 256; i++)
                buf_resp[32 + i] = i;
        buf_resp[32 + 256 + 0] = '\0';
        buf_resp[32 + 256 + 1] = 'T';
        strcpy((char*)&buf_resp[32 + 256 + 2], "[test dir]$ ");

        test_func_write(buf_req);
        test_func_read_all();
        TEST_ASSERT_EQUAL_UINT8_ARRAY(buf_resp, g_write_buf, (32 + 256 + 2 + 12));
}

void test_cmd_cat_help(void)
{
        TEST_FUNC_ASK("help cat",
                "usage: cat <files...>\r\n"
                "[test /]$ "
        );
}

void test_cmd_cat_error(void)
{
        TEST_FUNC_ASK("cat",
                "error: wrong arguments\r\n"
                "[test /]$ "
        );

        TEST_FUNC_ASK("cat 1",
                "error: file not found\r\n"
                "[test /]$ "
        );

        TEST_FUNC_ASK("cat test",
                "error: file not readable\r\n"
                "[test /]$ "
        );

        TEST_FUNC_ASK("cat /data/null 1",
                "error: file not found\r\n"
                "[test /]$ "
        );
}

int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

        UNITY_BEGIN();

        RUN_TEST(test_cmd_cat_single);
        RUN_TEST(test_cmd_cat_multi);
        RUN_TEST(test_cmd_cat_multi_binary);
        RUN_TEST(test_cmd_cat_help);
        RUN_TEST(test_cmd_cat_error);

        return UNITY_END();
}
