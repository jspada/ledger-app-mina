#pragma once

#include "globals.h"

void handleSignPaymentTx(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint32_t dataLength, volatile unsigned int *flags, volatile unsigned int *tx);
