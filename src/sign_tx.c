#include <assert.h>
#include <stdlib.h>

#include "menu.h"
#include "sign_tx.h"
#include "utils.h"
#include "crypto.h"
#include "random_oracle_input.h"
#include "transaction.h"

// Account variables
static uint32_t _account = 0;
static Keypair  _kp;
static char     _address[MINA_ADDRESS_LEN];

// Transaction related
static Transaction _tx;
static Field       _input_fields[3];
static uint8_t     _input_bits[TX_BITSTRINGS_BYTES];
static ROInput     _roinput;
static Signature   _sig;
static char        _sig_hex[SIGNATURE_LEN];

// UI variables
static struct ui_t {
    char sender[MINA_ADDRESS_LEN];
    char receiver[MINA_ADDRESS_LEN];
    char amount[32];
    char fee[32];
    char total[32];
    char nonce[32];
    char valid_until[32];
    char memo[MEMO_BYTES - 1];
    char type[11];
    char sender_title[10];
    char receiver_title[9];
} _ui;

static uint8_t set_result_get_signature(void)
{
    uint8_t tx = 0;
    os_memmove(G_io_apdu_buffer + tx, _sig_hex, sizeof(_sig_hex));
    tx += sizeof(_sig_hex);
    return tx;
}

static void sign_transaction(void)
{
    BEGIN_TRY {
        TRY {
            // Get the account's private key and validate corresponding
            // public key matches the sender address
            generate_keypair(&_kp, _account);
            if (!generate_address(_address, sizeof(_address), &_kp.pub)) {
                THROW(INVALID_PARAMETER);
            }
            if (os_memcmp(_address, _ui.sender, sizeof(_address)) != 0) {
                THROW(INVALID_PARAMETER);
            }

            // Create random oracle input from transaction
            _roinput.fields = _input_fields;
            _roinput.fields_capacity = ARRAY_LEN(_input_fields);
            _roinput.bits = _input_bits;
            _roinput.bits_capacity = ARRAY_LEN(_input_bits);
            transaction_to_roinput(&_roinput, &_tx);

            sign(&_sig, &_kp, &_roinput);

            uint8_t *p = (uint8_t *)&_sig;
            for (size_t i = 0; i < sizeof(_sig); i++) {
                snprintf(&_sig_hex[i*2], 3, "%02x", p[i]);
            }
            _sig_hex[SIGNATURE_LEN - 1] = '\0';

        }
        FINALLY {
            // Clear private key from memory
            explicit_bzero((void *)_kp.priv, sizeof(_kp.priv));
        }
        END_TRY;
    }

    sendResponse(set_result_get_signature(), true);
}

UX_STEP_NOCB_INIT(
    ux_signing_flow_step,
    pb,
    sign_transaction(),
    {
        &C_icon_processing,
        "Done",
    });

UX_FLOW(ux_signing_flow,
        &ux_signing_flow_step);

UX_STEP_TIMEOUT(
    ux_signing_comfort_step,
    pb,
    1,
    ux_signing_flow,
    {
      &C_icon_processing,
      "Signing...",
    });

UX_FLOW(ux_signing_comfort_flow,
       &ux_signing_comfort_step);

UX_STEP_NOCB(
   ux_sign_tx_flow_0_step,
   bnnn_paging,
   {
     .title = "Type",
     .text = _ui.type,
   });
UX_STEP_NOCB(
    ux_sign_tx_flow_1_step,
    bnnn_paging,
    {
      .title = _ui.sender_title,
      .text = _ui.sender,
    });
UX_STEP_NOCB(
    ux_sign_tx_flow_2_step,
    bnnn_paging,
    {
      .title = _ui.receiver_title,
      .text = _ui.receiver,
    });
UX_STEP_NOCB(
    ux_sign_tx_flow_amount_step,
    bn,
    {
      .line1 = "Amount",
      .line2 = _ui.amount,
    });
UX_STEP_NOCB(
    ux_sign_tx_flow_4_step,
    bn,
    {
      .line1 = "Fee",
      .line2 = _ui.fee,
    });
UX_STEP_NOCB(
    ux_sign_tx_flow_total_step,
    bn,
    {
      .line1 = "Total",
      .line2 = _ui.total,
    });
UX_STEP_NOCB(
    ux_sign_tx_flow_5_step,
    bn,
    {
      .line1 = "Nonce",
      .line2 = _ui.nonce,
    });
UX_STEP_NOCB(
    ux_sign_tx_flow_valid_until_step,
    bn,
    {
      .line1 = "Valid until",
      .line2 = _ui.valid_until,
    });
UX_STEP_NOCB(
    ux_sign_tx_flow_memo_step,
    bnnn_paging,
    {
      .title = "Memo",
      .text = _ui.memo,
    });
UX_STEP_VALID(
    ux_sign_tx_flow_6_step,
    pb,
    ux_flow_init(0, ux_signing_comfort_flow, NULL);,
    {
      &C_icon_validate_14,
      "Approve",
    });
UX_STEP_VALID(
    ux_sign_tx_flow_7_step,
    pb,
    sendResponse(0, false),
    {
      &C_icon_crossmark,
      "Reject",
    });

UX_FLOW(ux_sign_payment_tx_flow,
    &ux_sign_tx_flow_0_step,
    &ux_sign_tx_flow_1_step,
    &ux_sign_tx_flow_2_step,
    &ux_sign_tx_flow_amount_step,
    &ux_sign_tx_flow_4_step,
    &ux_sign_tx_flow_total_step,
    &ux_sign_tx_flow_5_step,
    &ux_sign_tx_flow_6_step,
    &ux_sign_tx_flow_7_step
);

UX_FLOW(ux_sign_payment_tx_flow_valid_until,
    &ux_sign_tx_flow_0_step,
    &ux_sign_tx_flow_1_step,
    &ux_sign_tx_flow_2_step,
    &ux_sign_tx_flow_amount_step,
    &ux_sign_tx_flow_4_step,
    &ux_sign_tx_flow_total_step,
    &ux_sign_tx_flow_5_step,
    &ux_sign_tx_flow_valid_until_step,
    &ux_sign_tx_flow_6_step,
    &ux_sign_tx_flow_7_step
);

UX_FLOW(ux_sign_payment_tx_flow_memo,
    &ux_sign_tx_flow_0_step,
    &ux_sign_tx_flow_1_step,
    &ux_sign_tx_flow_2_step,
    &ux_sign_tx_flow_amount_step,
    &ux_sign_tx_flow_4_step,
    &ux_sign_tx_flow_total_step,
    &ux_sign_tx_flow_5_step,
    &ux_sign_tx_flow_memo_step,
    &ux_sign_tx_flow_6_step,
    &ux_sign_tx_flow_7_step
);

UX_FLOW(ux_sign_payment_tx_flow_all,
    &ux_sign_tx_flow_0_step,
    &ux_sign_tx_flow_1_step,
    &ux_sign_tx_flow_2_step,
    &ux_sign_tx_flow_amount_step,
    &ux_sign_tx_flow_4_step,
    &ux_sign_tx_flow_total_step,
    &ux_sign_tx_flow_5_step,
    &ux_sign_tx_flow_valid_until_step,
    &ux_sign_tx_flow_memo_step,
    &ux_sign_tx_flow_6_step,
    &ux_sign_tx_flow_7_step
);

UX_FLOW(ux_sign_delegate_tx_flow,
    &ux_sign_tx_flow_0_step,
    &ux_sign_tx_flow_1_step,
    &ux_sign_tx_flow_2_step,
    &ux_sign_tx_flow_4_step,
    &ux_sign_tx_flow_5_step,
    &ux_sign_tx_flow_6_step,
    &ux_sign_tx_flow_7_step
);

UX_FLOW(ux_sign_delegate_tx_flow_valid_until,
    &ux_sign_tx_flow_0_step,
    &ux_sign_tx_flow_1_step,
    &ux_sign_tx_flow_2_step,
    &ux_sign_tx_flow_4_step,
    &ux_sign_tx_flow_5_step,
    &ux_sign_tx_flow_valid_until_step,
    &ux_sign_tx_flow_6_step,
    &ux_sign_tx_flow_7_step
);

UX_FLOW(ux_sign_delegate_tx_flow_memo,
    &ux_sign_tx_flow_0_step,
    &ux_sign_tx_flow_1_step,
    &ux_sign_tx_flow_2_step,
    &ux_sign_tx_flow_4_step,
    &ux_sign_tx_flow_5_step,
    &ux_sign_tx_flow_memo_step,
    &ux_sign_tx_flow_6_step,
    &ux_sign_tx_flow_7_step
);

UX_FLOW(ux_sign_delegate_tx_flow_all,
    &ux_sign_tx_flow_0_step,
    &ux_sign_tx_flow_1_step,
    &ux_sign_tx_flow_2_step,
    &ux_sign_tx_flow_4_step,
    &ux_sign_tx_flow_5_step,
    &ux_sign_tx_flow_valid_until_step,
    &ux_sign_tx_flow_memo_step,
    &ux_sign_tx_flow_6_step,
    &ux_sign_tx_flow_7_step
);

#ifdef ON_DEVICE_UNIT_TESTS

UX_STEP_NOCB_INIT(
    ux_signing_step_2,
    pb,
    sign_transaction(),
    {
        &C_icon_validate_14,
        "Done",
    });

UX_FLOW(ux_unit_test_flow_2,
    &ux_signing_step_2
);

UX_STEP_TIMEOUT(
    ux_signing_step_1,
    pb,
    1,
    ux_unit_test_flow_2,
    {
        &C_icon_processing,
        "Unit tests...",
    });

UX_FLOW(ux_unit_test_flow_1,
        &ux_signing_step_1);
#endif

void handle_sign_tx(uint8_t p1, uint8_t p2, uint8_t *dataBuffer,
                    uint8_t dataLength, volatile unsigned int *flags,
                    volatile unsigned int *unused)
{
    UNUSED(p1);
    UNUSED(p2);

    if (dataLength != 171) {
        THROW(INVALID_PARAMETER);
    }

    _sig_hex[0] = '\0';

    // 0-3: sender_bip44_account
    _account = read_uint32_be(dataBuffer);

    // 4-58: sender_address
    os_memcpy(_ui.sender, dataBuffer + 4, MINA_ADDRESS_LEN - 1);
    _ui.sender[MINA_ADDRESS_LEN - 1] = '\0';
    if (!validate_address(_ui.sender)) {
        THROW(INVALID_PARAMETER);
    }
    read_public_key_compressed(&_tx.source_pk, _ui.sender);

    // Always the same as sender for sent-payment and delegate txs
    read_public_key_compressed(&_tx.fee_payer_pk, _ui.sender);

    // 59-113: receiver
    os_memcpy(_ui.receiver, dataBuffer + 59, MINA_ADDRESS_LEN - 1);
    _ui.receiver[MINA_ADDRESS_LEN - 1] = '\0';
    if (!validate_address(_ui.receiver)) {
        THROW(INVALID_PARAMETER);
    }
    read_public_key_compressed(&_tx.receiver_pk, _ui.receiver);

    // 114-121: amount
    _tx.amount = read_uint64_be(dataBuffer + 114);
    amount_to_string(_ui.amount, sizeof(_ui.amount), _tx.amount);

    // Set to 1 until token support is released
    _tx.token_id = 1;

    // 122-129: fee
    _tx.fee = read_uint64_be(dataBuffer + 122);
    amount_to_string(_ui.fee, sizeof(_ui.fee), _tx.fee);

    // UI total
    if (_tx.amount + _tx.fee < _tx.amount) {
        // Overflow
        THROW(INVALID_PARAMETER);
    }
    amount_to_string(_ui.total, sizeof(_ui.total), _tx.amount + _tx.fee);

    // Set to 1 until token support is released
    _tx.fee_token = 1;

    // 130-133: nonce
    _tx.nonce = read_uint32_be(dataBuffer + 130);
    value_to_string(_ui.nonce, sizeof(_ui.nonce), _tx.nonce);

    // 134-137: valid_until
    _tx.valid_until = read_uint32_be(dataBuffer + 134);
    value_to_string(_ui.valid_until, sizeof(_ui.valid_until), _tx.valid_until);

    // Fixed until token support is released
    _tx.token_locked = false;

    // 138-169: memo
    os_memcpy(_ui.memo, dataBuffer + 138, sizeof(_ui.memo) - 1);
    _ui.memo[sizeof(_ui.memo) - 1] = '\0';
    transaction_prepare_memo(_tx.memo, _ui.memo);

    // 170: tag
    uint8_t tag = *(dataBuffer + 170);
    if (tag != 0x00 && tag != 0x04) {
        THROW(INVALID_PARAMETER);
    }
    _tx.tag[0] = tag & 0x01;
    _tx.tag[1] = tag & 0x02;
    _tx.tag[2] = tag & 0x04;

    #ifdef ON_DEVICE_UNIT_TESTS
    ux_flow_init(0, ux_unit_test_flow_1, NULL);
    *flags |= IO_ASYNCH_REPLY;
    return;
    #endif

    if (tag == 0x00) {
        strncpy(_ui.type, "Payment", sizeof(_ui.type));
        strncpy(_ui.sender_title, "Sender", sizeof(_ui.sender_title));
        strncpy(_ui.receiver_title, "Receiver", sizeof(_ui.receiver_title));

        if (_tx.valid_until == (uint32_t)-1 && _ui.memo[0] == '\0') {
            // Valid forever and no memo
            ux_flow_init(0, ux_sign_payment_tx_flow, NULL);
        }
        else if (_ui.memo[0] == '\0') {
            // Valid until and no memo
            ux_flow_init(0, ux_sign_payment_tx_flow_valid_until, NULL);
        }
        else if (_tx.valid_until == (uint32_t)-1) {
            // Valid forever and memo
            ux_flow_init(0, ux_sign_payment_tx_flow_memo, NULL);
        }
        else {
            // Valid until and memo
            ux_flow_init(0, ux_sign_payment_tx_flow_all, NULL);
        }
    }
    else if (tag == 0x04) {
        strncpy(_ui.type, "Delegation", sizeof(_ui.type));
        strncpy(_ui.sender_title, "Delegator", sizeof(_ui.sender_title));
        strncpy(_ui.receiver_title, "Delegate", sizeof(_ui.receiver_title));

        if (_tx.valid_until == (uint32_t)-1 && _ui.memo[0] == '\0') {
            // Valid forever and no memo
            ux_flow_init(0, ux_sign_delegate_tx_flow, NULL);
        }
        else if (_ui.memo[0] == '\0') {
            // Valid until and no memo
            ux_flow_init(0, ux_sign_delegate_tx_flow_valid_until, NULL);
        }
        else if (_tx.valid_until == (uint32_t)-1) {
            // Valid forever and memo
            ux_flow_init(0, ux_sign_delegate_tx_flow_memo, NULL);
        }
        else {
            // Valid until and memo
            ux_flow_init(0, ux_sign_delegate_tx_flow_all, NULL);
        }
    }

    *flags |= IO_ASYNCH_REPLY;
}
