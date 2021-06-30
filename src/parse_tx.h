#pragma once

#include "transaction.h"

typedef struct {
    // Blockchain instance
    uint8_t network_id;

    // Account variables
    uint32_t account;

    // Transaction related
    Transaction tx;
    Field       input_fields[3];
    uint8_t     input_bits[TX_BITSTRINGS_BYTES];
    uint8_t     tag;
} tx_t;

typedef struct {
    char from[MINA_ADDRESS_LEN];
    char to[MINA_ADDRESS_LEN];
    char amount[32];
    char fee[32];
    char total[32];
    char nonce[32];
    char valid_until[32];
    char memo[MEMO_BYTES - 1];
    char type[11];
    char from_title[10];
    char to_title[9];
} ui_t;

bool parse_tx(const uint8_t *dataBuffer, uint8_t dataLength, tx_t *tx, ui_t *ui);
