#ifndef INTERPRET_H
#define INTERPRET_H

#include <inttypes.h>

int16_t interpret_analyzeTSIC(uint8_t * buf);
int16_t interpret_analyzeHYTtemp(uint8_t * buf);
int16_t interpret_analyzeHYThum(uint8_t * buf);
void interpret_detectPrintSingle(uint8_t * data);
void interpret_detectPrintAll(uint8_t * data, uint8_t connected);
#endif
