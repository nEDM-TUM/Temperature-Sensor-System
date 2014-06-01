#ifndef INTERPRET_H
#define INTERPRET_H

int16_t interpret_analyzeTSIC(uint8_t * buf);
int16_t interpret_analyzeHYTtemp(uint8_t * buf);
int16_t interpret_analyzeHYThum(uint8_t * buf);
void interpret_detectPrint(uint8_t * data);
#endif
