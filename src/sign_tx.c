#include <assert.h>
#include <stdlib.h>

#include "menu.h"
#include "sign_tx.h"
#include "utils.h"
#include "crypto.h"
#include "random_oracle_input.h"
#include "transaction.h"

// Blockchain instance
uint8_t _network_id;

// Account variables
static uint32_t _account;
static Keypair  _kp;
static char     _address[MINA_ADDRESS_LEN];

// Transaction related
static Transaction _tx;
static Field       _input_fields[3];
static uint8_t     _input_bits[TX_BITSTRINGS_BYTES];
static ROInput     _roinput;
static Signature   _sig;

// UI variables
static struct ui_t {
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
} _ui;

static uint8_t set_result_get_signature(void)
{
    uint8_t tx = 0;
    memmove(G_io_apdu_buffer + tx, &_sig, sizeof(_sig));
    tx += sizeof(_sig);
    return tx;
}

static void sign_transaction(void)
{
    BEGIN_TRY {
        TRY {
            // Get the account's private key and validate corresponding
            // public key matches the from address
            generate_keypair(&_kp, _account);
            if (!generate_address(_address, sizeof(_address), &_kp.pub)) {
                THROW(INVALID_PARAMETER);
            }
            if (memcmp(_address, _ui.from, sizeof(_address)) != 0) {
                THROW(INVALID_PARAMETER);
            }

            // Create random oracle input from transaction
            _roinput.fields = _input_fields;
            _roinput.fields_capacity = ARRAY_LEN(_input_fields);
            _roinput.bits = _input_bits;
            _roinput.bits_capacity = ARRAY_LEN(_input_bits);
            transaction_to_roinput(&_roinput, &_tx);

            sign(&_sig, &_kp, &_roinput, _network_id);
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
    ux_sign_tx_done_flow_done_step,
    pb,
    sign_transaction(),
    {
        &C_icon_validate_14,
        "Done"
    }
);

UX_FLOW(
    ux_sign_tx_done_flow,
    &ux_sign_tx_done_flow_done_step
);

#ifdef HAVE_ON_DEVICE_UNIT_TESTS
    UX_STEP_TIMEOUT(
        ux_sign_tx_flow_unit_tests_step,
        pb,
        1,
        ux_sign_tx_done_flow,
        {
            &C_icon_processing,
            "Unit tests..."
        }
    );

    UX_FLOW(
        ux_sign_tx_unit_test_flow,
        &ux_sign_tx_flow_unit_tests_step
    );
#else
    UX_STEP_TIMEOUT(
        ux_sign_tx_comfort_flow_signing_step,
        pb,
        1,
        ux_sign_tx_done_flow,
        {
            &C_icon_processing,
            "Signing..."
        }
    );

    UX_FLOW(
        ux_sign_tx_comfort_flow,
        &ux_sign_tx_comfort_flow_signing_step
    );

    UX_STEP_NOCB(
        ux_sign_tx_flow_topic_step,
        pnn,
        {
            &C_icon_eye,
            "Sign",
            "Transaction"
        }
    );

    UX_STEP_NOCB(
        ux_sign_tx_flow_network_step,
        bn,
        {
            "Network",
            "testnet"
        }
    );

    UX_STEP_NOCB(
        ux_sign_tx_flow_type_step,
        bn,
        {
            "Type",
            _ui.type
        }
    );

    UX_STEP_NOCB(
        ux_sign_tx_flow_from_step,
        bnnn_paging,
        {
            .title = _ui.from_title,
            .text = _ui.from
        }
    );

    UX_STEP_NOCB(
        ux_sign_tx_flow_to_step,
        bnnn_paging,
        {
            .title = _ui.to_title,
            .text = _ui.to
        }
    );

    UX_STEP_NOCB(
        ux_sign_tx_flow_amount_step,
        bn,
        {
            .line1 = "Amount",
            .line2 = _ui.amount
        }
    );

    UX_STEP_NOCB(
        ux_sign_tx_flow_fee_step,
        bn,
        {
           "Fee",
           _ui.fee
        }
    );

    UX_STEP_NOCB(
        ux_sign_tx_flow_total_step,
        bn,
        {
            "Total",
            _ui.total
        }
    );

    UX_STEP_NOCB(
        ux_sign_tx_flow_nonce_step,
        bn,
        {
            "Nonce",
            _ui.nonce
        }
    );

    UX_STEP_NOCB(
        ux_sign_tx_flow_valid_until_step,
        bn,
        {
            "Valid until",
            _ui.valid_until
        }
    );

    UX_STEP_NOCB(
        ux_sign_tx_flow_memo_step,
        bnnn_paging,
        {
            .title = "Memo",
            .text = _ui.memo
        }
    );

    UX_STEP_VALID(
        ux_sign_tx_flow_approve_step,
        pb,
        ux_flow_init(0, ux_sign_tx_comfort_flow, NULL);,
        {
            &C_icon_validate_14,
            "Approve"
        }
    );

    UX_STEP_VALID(
        ux_sign_tx_flow_reject_step,
        pb,
        sendResponse(0, false),
        {
            &C_icon_crossmark,
            "Reject"
        }
    );

    // Unfortunately ux flows cannot be generated dynamically
    // so we must define 16 static flows...

    UX_FLOW(ux_sign_tx_flow_testnet_payment_0_0,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_network_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_amount_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_total_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_testnet_payment_0_memo,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_network_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_amount_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_total_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_memo_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_testnet_payment_valid_until_0,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_network_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_amount_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_total_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_valid_until_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_testnet_payment_valid_until_memo,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_network_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_amount_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_total_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_valid_until_step,
            &ux_sign_tx_flow_memo_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_testnet_delegation_0_0,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_network_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_testnet_delegation_0_memo,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_network_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_memo_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_testnet_delegation_valid_until_0,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_network_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_valid_until_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_testnet_delegation_valid_until_memo,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_network_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_valid_until_step,
            &ux_sign_tx_flow_memo_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_mainnet_payment_0_0,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_amount_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_total_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_mainnet_payment_0_memo,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_amount_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_total_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_memo_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_mainnet_payment_valid_until_0,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_amount_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_total_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_valid_until_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_mainnet_payment_valid_until_memo,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_amount_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_total_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_valid_until_step,
            &ux_sign_tx_flow_memo_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_mainnet_delegation_0_0,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_mainnet_delegation_0_memo,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_memo_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_mainnet_delegation_valid_until_0,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_valid_until_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    UX_FLOW(ux_sign_tx_flow_mainnet_delegation_valid_until_memo,
            &ux_sign_tx_flow_topic_step,
            &ux_sign_tx_flow_type_step,
            &ux_sign_tx_flow_from_step,
            &ux_sign_tx_flow_to_step,
            &ux_sign_tx_flow_fee_step,
            &ux_sign_tx_flow_nonce_step,
            &ux_sign_tx_flow_valid_until_step,
            &ux_sign_tx_flow_memo_step,
            &ux_sign_tx_flow_approve_step,
            &ux_sign_tx_flow_reject_step);

    // Create an ux flow index to simplify application logic
    //
    #define GET_FLOW_PTR(x) ((const ux_flow_step_t** const )&x)
    //
    //     n - network_id       (0 testnet, 1 mainnet)
    //     t - transaction type (0 payment, 1 delegation)
    //     v - valid_until      (0 omitted, 1 present)
    //     m - memo             (0 omitted, 1 present)
    //                                                  n  t  v  m
    static const ux_flow_step_t** const ux_sign_tx_flow[2][2][2][2] = {
        { // testnet
            { // payment
                {
                    GET_FLOW_PTR(ux_sign_tx_flow_testnet_payment_0_0),
                    GET_FLOW_PTR(ux_sign_tx_flow_testnet_payment_0_memo)
                },
                {
                    GET_FLOW_PTR(ux_sign_tx_flow_testnet_payment_valid_until_0),
                    GET_FLOW_PTR(ux_sign_tx_flow_testnet_payment_valid_until_memo)
                }
            },
            { // delegation
                {
                    GET_FLOW_PTR(ux_sign_tx_flow_testnet_delegation_0_0),
                    GET_FLOW_PTR(ux_sign_tx_flow_testnet_delegation_0_memo)
                },
                {
                    GET_FLOW_PTR(ux_sign_tx_flow_testnet_delegation_valid_until_0),
                    GET_FLOW_PTR(ux_sign_tx_flow_testnet_delegation_valid_until_memo)
                }
            }
        },
        { // mainnet
            { // payment
                {
                    GET_FLOW_PTR(ux_sign_tx_flow_mainnet_payment_0_0),
                    GET_FLOW_PTR(ux_sign_tx_flow_mainnet_payment_0_memo)
                },
                {
                    GET_FLOW_PTR(ux_sign_tx_flow_mainnet_payment_valid_until_0),
                    GET_FLOW_PTR(ux_sign_tx_flow_mainnet_payment_valid_until_memo)
                }
            },
            { // delegation
                {
                    GET_FLOW_PTR(ux_sign_tx_flow_mainnet_delegation_0_0),
                    GET_FLOW_PTR(ux_sign_tx_flow_mainnet_delegation_0_memo)
                },
                {
                    GET_FLOW_PTR(ux_sign_tx_flow_mainnet_delegation_valid_until_0),
                    GET_FLOW_PTR(ux_sign_tx_flow_mainnet_delegation_valid_until_memo)
                }
            }
        }
    };
#endif

void handle_sign_tx(uint8_t p1, uint8_t p2, uint8_t *dataBuffer,
                    uint8_t dataLength, volatile unsigned int *flags)
{
    UNUSED(p1);
    UNUSED(p2);

    if (dataLength != 172) {
        THROW(INVALID_PARAMETER);
    }

    // 0-3: from_bip44_account
    _account = read_uint32_be(dataBuffer);

    // 4-58: from_address
    memcpy(_ui.from, dataBuffer + 4, MINA_ADDRESS_LEN - 1);
    _ui.from[MINA_ADDRESS_LEN - 1] = '\0';
    if (!validate_address(_ui.from)) {
        THROW(INVALID_PARAMETER);
    }
    read_public_key_compressed(&_tx.source_pk, _ui.from);

    // Always the same as from for sent-payment and delegate txs
    read_public_key_compressed(&_tx.fee_payer_pk, _ui.from);

    // 59-113: to
    memcpy(_ui.to, dataBuffer + 59, MINA_ADDRESS_LEN - 1);
    _ui.to[MINA_ADDRESS_LEN - 1] = '\0';
    if (!validate_address(_ui.to)) {
        THROW(INVALID_PARAMETER);
    }
    read_public_key_compressed(&_tx.receiver_pk, _ui.to);

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
    memcpy(_ui.memo, dataBuffer + 138, sizeof(_ui.memo) - 1);
    _ui.memo[sizeof(_ui.memo) - 1] = '\0';
    transaction_prepare_memo(_tx.memo, _ui.memo);

    // 170: tag
    uint8_t tag = *(dataBuffer + 170);
    if (tag != PAYMENT_TX && tag != DELEGATION_TX) {
        THROW(INVALID_PARAMETER);
    }
    _tx.tag[0] = tag & 0x01;
    _tx.tag[1] = tag & 0x02;
    _tx.tag[2] = tag & 0x04;

    // 171: network_id
    _network_id = *(dataBuffer + 171);
    if (_network_id != TESTNET_ID && _network_id != MAINNET_ID) {
        THROW(INVALID_PARAMETER);
    }

    #ifdef HAVE_ON_DEVICE_UNIT_TESTS
        ux_flow_init(0, ux_sign_tx_unit_test_flow, NULL);
    #else
        if (tag == PAYMENT_TX) {
            strncpy(_ui.type, "Payment", sizeof(_ui.type));
            strncpy(_ui.from_title, "Sender", sizeof(_ui.from_title));
            strncpy(_ui.to_title, "Receiver", sizeof(_ui.to_title));
        }
        else if (tag == DELEGATION_TX) {
            strncpy(_ui.type, "Delegation", sizeof(_ui.type));
            strncpy(_ui.from_title, "Delegator", sizeof(_ui.from_title));
            strncpy(_ui.to_title, "Delegate", sizeof(_ui.to_title));
        }

        // Select the appropriate UX flow
        int n_idx = _network_id == MAINNET_ID;
        int t_idx = tag == DELEGATION_TX;
        int v_idx = _tx.valid_until != (uint32_t)-1;
        int m_idx = _ui.memo[0] != '\0';

        // Run the UX flow
        ux_flow_init(0, ux_sign_tx_flow[n_idx][t_idx][v_idx][m_idx], NULL);
    #endif

    *flags |= IO_ASYNCH_REPLY;
}
