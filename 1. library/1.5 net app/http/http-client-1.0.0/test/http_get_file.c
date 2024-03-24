/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2020-05-16 22:23:05
 * @LastEditTime: 2020-05-25 23:37:20
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */ 
#include <stdio.h>
#include <unistd.h>
#include <httpclient.h>

extern const char *ca_get();

#define URL1 "http://httpbin.org/image/png"       // tcp
#define URL2 "https://httpbin.org/image/png"       // tls
#define URL3 "https://dldir1.qq.com/weixin/Windows/WeChatSetup.exe"

#define FILE_NAME1  "pig1.png"
#define FILE_NAME2  "pig2.png"
#define FILE_NAME3  "WeChatSetup.exe"

static char buf[102]={'\0'};

static char *pbuf1 = NULL;
static char *pbuf2 = NULL;
static char *pbuf3 = NULL;

static size_t pbuf1_len = 0;
static size_t pbuf2_len = 0;
static size_t pbuf3_len = 0;

static void printf_progress_bar(size_t process, size_t total)
{
    int count = 0;
    const char* stat = {"-\\|/"};
    count = (process * 100) / total;
    if (count < 0)
        return;
    
    buf[count] = '#';

    printf("[%-101s][%%%d]%c\r", buf, count, stat[count%4]);
    fflush(stdout);
}


static int _http_cb1(void *e)
{
    static size_t process_len = 0;
    http_event_t *event = e;

    if (event->len <= 0)
        return -1;

    if (NULL == pbuf1) {
        http_client_t *client = event->context;
        http_interceptor_t *interceptor = client->interceptor;

        pbuf1_len = http_response_get_length(&interceptor->response);
        
        printf("\nfile size : %ld KB\n", pbuf1_len);
        pbuf1 = platform_memory_alloc(pbuf1_len);
    }
    
    memcpy(pbuf1 + process_len, event->data, event->len);

    process_len += event->len;

    printf_progress_bar(process_len, pbuf1_len);

    return 0;
}

static int _http_cb2(void *e)
{
    static size_t process_len = 0;
    http_event_t *event = e;


    if (event->len <= 0)
        return -1;

    if (NULL == pbuf2) {
        http_client_t *client = event->context;
        http_interceptor_t *interceptor = client->interceptor;

        pbuf2_len = http_response_get_length(&interceptor->response);

        printf("\nfile size : %ld KB\n", pbuf2_len);
        pbuf2 = platform_memory_alloc(pbuf2_len);
    }
    
    memcpy(pbuf2 + process_len, event->data, event->len);

    process_len += event->len;

    printf_progress_bar(process_len, pbuf2_len);

    return 0;
}

static int _http_cb3(void *e)
{
    static size_t process_len = 0;
    http_event_t *event = e;

    if (event->len <= 0)
        return -1;

    if (NULL == pbuf3) {
        http_client_t *client = event->context;
        http_interceptor_t *interceptor = client->interceptor;
        
        pbuf3_len = http_response_get_length(&interceptor->response);

        printf("\nfile size : %ld KB\n", pbuf3_len);
        pbuf3 = platform_memory_alloc(pbuf3_len);
    }
    
    memcpy(pbuf3 + process_len, event->data, event->len);

    process_len += event->len;

    printf_progress_bar(process_len, pbuf3_len);

    return 0;
}

void http_get_file_test(void)
{
    FILE *fp =NULL;

    printf("\n---------------------- http_get_file_test start ----------------------\n");

    http_client_get(URL1, _http_cb1);
    if (pbuf1_len) {
        fp = fopen(FILE_NAME1, "wb");
        
        if (fp == NULL) {
            printf("\nopen %s file fail ...\n", FILE_NAME1);
        } else {
            printf("\nopen %s file success ...\n", FILE_NAME1);
            fwrite(pbuf1, 1, pbuf1_len, fp);
            fflush(fp);
            fclose(fp);
            platform_memory_free(pbuf1);
            pbuf3 = NULL;
        }
    }

    http_client_get(URL2, _http_cb2);
    if (pbuf2_len) {
        fp = fopen(FILE_NAME2, "wb");
        
        if (fp == NULL) {
            printf("\nopen %s file fail ...\n", FILE_NAME2);
        } else {
            printf("\nopen %s file success ...\n", FILE_NAME2);
            fwrite(pbuf2, 1, pbuf2_len, fp);
            fflush(fp);
            fclose(fp);
            platform_memory_free(pbuf2);
            pbuf3 = NULL;
        }
    }

    http_client_get(URL3, _http_cb3);
    if (pbuf3_len) {
        fp = fopen(FILE_NAME3, "wb");
        
        if (fp == NULL) {
            printf("\nopen %s file fail ...\n", FILE_NAME3);
        } else {
            printf("\nopen %s file success ...\n", FILE_NAME3);
            fwrite(pbuf3, 1, pbuf3_len, fp);
            fflush(fp);
            fclose(fp);
            platform_memory_free(pbuf3);
            pbuf3 = NULL;
        }
    }

    printf("\n---------------------- http_get_file_test end ----------------------\n");
}

