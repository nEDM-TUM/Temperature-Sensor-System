#ifdef __cplusplus
extern "C" {
#endif
#ifndef RESPONSE_H
#define RESPONSE_H
#include <avr/io.h>
#include <stdio.h>

extern FILE res_stream;
#define MAX_RESPONSE_LEN 5

void res_init(void);
int res_flush();
void res_set_sock(uint8_t sock);
static int res_putchar(char c, FILE *stream);
#endif
#ifdef __cplusplus
}
#endif
