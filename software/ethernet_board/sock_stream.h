#ifdef __cplusplus
extern "C" {
#endif
#ifndef SOCK_STREAM_H
#define SOCK_STREAM_H
#include <avr/io.h>
#include <stdio.h>

extern FILE sock_stream;
#define MAX_RESPONSE_LEN 5

void sock_stream_init(void);
int sock_stream_flush();
void stream_set_sock(uint8_t sock);
uint8_t stream_get_sock();
static int sock_putchar(char c, FILE *stream);
#endif
#ifdef __cplusplus
}
#endif
