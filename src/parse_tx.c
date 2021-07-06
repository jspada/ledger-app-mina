#include <string.h>
#include <stdbool.h>

#include "parse_tx.h"

bool parse_tx(const uint8_t *dataBuffer, uint8_t dataLength, tx_t *tx, ui_t *ui)
{
    if (dataLength != 172) {
        return false;
    }

    // 0-3: from_bip44_account
    tx->account = read_uint32_be(dataBuffer);

    // 4-58: from_address
    memcpy(ui->from, dataBuffer + 4, MINA_ADDRESS_LEN - 1);
    ui->from[MINA_ADDRESS_LEN - 1] = '\0';
    if (!validate_address(ui->from)) {
        return false;
    }
    read_public_key_compressed(&tx->tx.source_pk, ui->from);

    // Always the same as from for sent-payment and delegate txs
    read_public_key_compressed(&tx->tx.fee_payer_pk, ui->from);

    // 59-113: to
    memcpy(ui->to, dataBuffer + 59, MINA_ADDRESS_LEN - 1);
    ui->to[MINA_ADDRESS_LEN - 1] = '\0';
    if (!validate_address(ui->to)) {
        return false;
    }
    read_public_key_compressed(&tx->tx.receiver_pk, ui->to);

    // 114-121: amount
    tx->tx.amount = read_uint64_be(dataBuffer + 114);
    amount_to_string(ui->amount, sizeof(ui->amount), tx->tx.amount);

    // Set to 1 until token support is released
    tx->tx.token_id = 1;

    // 122-129: fee
    tx->tx.fee = read_uint64_be(dataBuffer + 122);
    amount_to_string(ui->fee, sizeof(ui->fee), tx->tx.fee);

    // UI total
    if (tx->tx.amount + tx->tx.fee < tx->tx.amount) {
        // Overflow
        return false;
    }
    amount_to_string(ui->total, sizeof(ui->total), tx->tx.amount + tx->tx.fee);

    // Set to 1 until token support is released
    tx->tx.fee_token = 1;

    // 130-133: nonce
    tx->tx.nonce = read_uint32_be(dataBuffer + 130);
    value_to_string(ui->nonce, sizeof(ui->nonce), tx->tx.nonce);

    // 134-137: valid_until
    tx->tx.valid_until = read_uint32_be(dataBuffer + 134);
    value_to_string(ui->valid_until, sizeof(ui->valid_until), tx->tx.valid_until);

    // Fixed until token support is released
    tx->tx.token_locked = false;

    // 138-169: memo
    memcpy(ui->memo, dataBuffer + 138, sizeof(ui->memo) - 1);
    ui->memo[sizeof(ui->memo) - 1] = '\0';
    transaction_prepare_memo(tx->tx.memo, ui->memo);

    // 170: tag
    tx->tag = *(dataBuffer + 170);
    if (tx->tag != PAYMENT_TX && tx->tag != DELEGATION_TX) {
        return false;
    }
    tx->tx.tag[0] = tx->tag & 0x01;
    tx->tx.tag[1] = tx->tag & 0x02;
    tx->tx.tag[2] = tx->tag & 0x04;

    // 171: network_id
    tx->network_id = *(dataBuffer + 171);
    if (tx->network_id != TESTNET_ID && tx->network_id != MAINNET_ID) {
        return false;
    }

    return true;
}
