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
#include <string.h>

#include <unity.h>

#include "inc/ush_internal.h"
#include "inc/ush_const.h"
#include "inc/ush.h"

int g_assert_call_count;

char input_buf[256];

struct ush_object ush;
struct ush_descriptor ush_desc;

int ush_write_pointer_call_count;
int ush_prompt_start_call_count;

char *ush_write_pointer_text;
ush_state_t ush_write_pointer_state;
ush_state_t ush_prompt_start_state;

int ush_autocomp_state_prepare_call_count;
int ush_autocomp_state_candidates_start_call_count;
int ush_autocomp_state_candidates_process_call_count;
int ush_autocomp_state_candidates_finish_call_count;
int ush_autocomp_state_recall_suffix_call_count;

void setUp(void)
{
        memset((uint8_t*)&ush, 0, sizeof(ush));
        ush_desc.input_buffer = input_buf;
        ush_desc.input_buffer_size = sizeof(input_buf);
        ush_desc.path_max_length = 256;
        ush.desc = &ush_desc;

        ush_write_pointer_call_count = 0;
        ush_prompt_start_call_count = 0;

        ush_write_pointer_text = NULL;
        ush_write_pointer_state = USH_STATE__TOTAL_NUM;
        ush_prompt_start_state = USH_STATE__TOTAL_NUM;

        ush_autocomp_state_prepare_call_count = 0;
        ush_autocomp_state_candidates_start_call_count = 0;
        ush_autocomp_state_candidates_process_call_count = 0;
        ush_autocomp_state_candidates_finish_call_count = 0;
        ush_autocomp_state_recall_suffix_call_count = 0;
}

void tearDown(void)
{

}

void ush_write_pointer(struct ush_object *self, char *text, ush_state_t state)
{
        TEST_ASSERT_EQUAL(&ush, self);
        TEST_ASSERT_EQUAL_STRING(ush_write_pointer_text, text);
        TEST_ASSERT_EQUAL(ush_write_pointer_state, state);

        ush_write_pointer_call_count++;
}

void ush_prompt_start(struct ush_object *self, ush_state_t prepare_next_state)
{
        TEST_ASSERT_EQUAL(&ush, self);
        TEST_ASSERT_EQUAL(ush_prompt_start_state, prepare_next_state);

        ush_prompt_start_call_count++;
}

void ush_autocomp_state_prepare(struct ush_object *self)
{
        TEST_ASSERT_EQUAL(&ush, self);

        ush_autocomp_state_prepare_call_count++;
}

void ush_autocomp_state_candidates_start(struct ush_object *self)
{
        TEST_ASSERT_EQUAL(&ush, self);

        ush_autocomp_state_candidates_start_call_count++;
}

void ush_autocomp_state_candidates_process(struct ush_object *self)
{
        TEST_ASSERT_EQUAL(&ush, self);

        ush_autocomp_state_candidates_process_call_count++;
}

void ush_autocomp_state_candidates_finish(struct ush_object *self)
{
        TEST_ASSERT_EQUAL(&ush, self);

        ush_autocomp_state_candidates_finish_call_count++;
}

void ush_autocomp_state_recall_suffix(struct ush_object *self)
{
        TEST_ASSERT_EQUAL(&ush, self);

        ush_autocomp_state_recall_suffix_call_count++;
}

void test_ush_autocomp_service(void)
{
        for (int i = 0; i < USH_STATE__TOTAL_NUM; i++) {
                ush_state_t state = (ush_state_t)i;

                setUp();
                ush.state = state;

                switch (state) {
                case USH_STATE_AUTOCOMP_PREPARE:
                        TEST_ASSERT_TRUE(ush_autocomp_service(&ush));
                        TEST_ASSERT_EQUAL(1, ush_autocomp_state_prepare_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_start_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_process_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_finish_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_recall_suffix_call_count);
                        TEST_ASSERT_EQUAL(0, ush_write_pointer_call_count);
                        TEST_ASSERT_EQUAL(0, ush_prompt_start_call_count);
                        break;

                case USH_STATE_AUTOCOMP_CANDIDATES_START:
                        TEST_ASSERT_TRUE(ush_autocomp_service(&ush));
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_prepare_call_count);
                        TEST_ASSERT_EQUAL(1, ush_autocomp_state_candidates_start_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_process_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_finish_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_recall_suffix_call_count);
                        TEST_ASSERT_EQUAL(0, ush_write_pointer_call_count);
                        TEST_ASSERT_EQUAL(0, ush_prompt_start_call_count);
                        break;

                case USH_STATE_AUTOCOMP_CANDIDATES_COUNT:
                case USH_STATE_AUTOCOMP_CANDIDATES_OPTIMISE:
                case USH_STATE_AUTOCOMP_CANDIDATES_PRINT: {
                        TEST_ASSERT_TRUE(ush_autocomp_service(&ush));
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_prepare_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_start_call_count);
                        TEST_ASSERT_EQUAL(1, ush_autocomp_state_candidates_process_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_finish_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_recall_suffix_call_count);
                        TEST_ASSERT_EQUAL(0, ush_write_pointer_call_count);
                        TEST_ASSERT_EQUAL(0, ush_prompt_start_call_count);
                        break;
                }

                case USH_STATE_AUTOCOMP_CANDIDATES_FINISH:
                        TEST_ASSERT_TRUE(ush_autocomp_service(&ush));
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_prepare_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_start_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_process_call_count);
                        TEST_ASSERT_EQUAL(1, ush_autocomp_state_candidates_finish_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_recall_suffix_call_count);
                        TEST_ASSERT_EQUAL(0, ush_write_pointer_call_count);
                        TEST_ASSERT_EQUAL(0, ush_prompt_start_call_count);
                        break;

                case USH_STATE_AUTOCOMP_PROMPT_PREPARE:
                        ush_write_pointer_state = USH_STATE_AUTOCOMP_PROMPT;
                        ush_write_pointer_text = "\r\n";
                        TEST_ASSERT_TRUE(ush_autocomp_service(&ush));
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_prepare_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_start_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_process_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_finish_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_recall_suffix_call_count);
                        TEST_ASSERT_EQUAL(1, ush_write_pointer_call_count);
                        TEST_ASSERT_EQUAL(0, ush_prompt_start_call_count);
                        break;

                case USH_STATE_AUTOCOMP_PROMPT:
                        ush_prompt_start_state = USH_STATE_AUTOCOMP_RECALL;
                        TEST_ASSERT_TRUE(ush_autocomp_service(&ush));
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_prepare_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_start_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_process_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_finish_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_recall_suffix_call_count);
                        TEST_ASSERT_EQUAL(0, ush_write_pointer_call_count);
                        TEST_ASSERT_EQUAL(1, ush_prompt_start_call_count);
                        break;

                case USH_STATE_AUTOCOMP_RECALL:
                        ush_write_pointer_state = USH_STATE_READ_CHAR;
                        strcpy(input_buf, "test");
                        ush_write_pointer_text = "test";
                        TEST_ASSERT_TRUE(ush_autocomp_service(&ush));
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_prepare_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_start_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_process_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_finish_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_recall_suffix_call_count);
                        TEST_ASSERT_EQUAL(1, ush_write_pointer_call_count);
                        TEST_ASSERT_EQUAL(0, ush_prompt_start_call_count);
                        break;
                
                case USH_STATE_AUTOCOMP_RECALL_SUFFIX: {                
                        TEST_ASSERT_TRUE(ush_autocomp_service(&ush));
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_prepare_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_start_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_process_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_finish_call_count);
                        TEST_ASSERT_EQUAL(1, ush_autocomp_state_recall_suffix_call_count);
                        TEST_ASSERT_EQUAL(0, ush_write_pointer_call_count);
                        TEST_ASSERT_EQUAL(0, ush_prompt_start_call_count);
                        break;
                }
                default:
                        TEST_ASSERT_FALSE(ush_autocomp_service(&ush));
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_prepare_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_start_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_process_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_candidates_finish_call_count);
                        TEST_ASSERT_EQUAL(0, ush_autocomp_state_recall_suffix_call_count);
                        TEST_ASSERT_EQUAL(0, ush_write_pointer_call_count);
                        TEST_ASSERT_EQUAL(0, ush_prompt_start_call_count);
                        break;
                }
        }
}

int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

        UNITY_BEGIN();

        RUN_TEST(test_ush_autocomp_service);

        return UNITY_END();
}
