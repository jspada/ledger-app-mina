#include <assert.h>
#include <stdlib.h>

#include "menu.h"
#include "sign_tx.h"
#include "utils.h"
#include "crypto.h"
#include "random_oracle_input.h"
#include "parse_tx.h"

static tx_t _tx;
static ui_t _ui;

static uint8_t set_result_get_signature(void)
{
    uint8_t size = 0;
    memmove(G_io_apdu_buffer + size, &_tx.sig, sizeof(_tx.sig));
    size += sizeof(_tx.sig);
    return size;
}

static void sign_transaction(void)
{
    char address[MINA_ADDRESS_LEN];
    ROInput roinput;
    Keypair kp;
    bool error = false;

    BEGIN_TRY {
        TRY {
            // Get the account's private key and validate corresponding
            // public key matches the from address
            generate_keypair(&kp, _tx.account);
            if (!generate_address(address, sizeof(address), &kp.pub)) {
                THROW(INVALID_PARAMETER);
            }
            if (memcmp(address, _ui.from, sizeof(address)) != 0) {
                THROW(INVALID_PARAMETER);
            }

            // Create random oracle input from transaction
            roinput.fields = _tx.input_fields;
            roinput.fields_capacity = ARRAY_LEN(_tx.input_fields);
            roinput.bits = _tx.input_bits;
            roinput.bits_capacity = ARRAY_LEN(_tx.input_bits);
            transaction_to_roinput(&roinput, &_tx.tx);

            if (!sign(&_tx.sig, &kp, &roinput, _tx.network_id)) {
                THROW(INVALID_PARAMETER);
            }
        }
        CATCH_OTHER(e) {
            error = true;
        }
        FINALLY {
            // Clear private key from memory
            explicit_bzero((void *)kp.priv, sizeof(kp.priv));
        }
        END_TRY;
    }

    if (error) {
        THROW(INVALID_PARAMETER);
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

    if (!parse_tx(dataBuffer, dataLength, &_tx, &_ui)) {
        THROW(INVALID_PARAMETER);
    }

    #ifdef HAVE_ON_DEVICE_UNIT_TESTS
        ux_flow_init(0, ux_sign_tx_unit_test_flow, NULL);
    #else
        if (_tx.tag == PAYMENT_TX) {
            strncpy(_ui.type, "Payment", sizeof(_ui.type));
            strncpy(_ui.from_title, "Sender", sizeof(_ui.from_title));
            strncpy(_ui.to_title, "Receiver", sizeof(_ui.to_title));
        }
        else if (_tx.tag == DELEGATION_TX) {
            strncpy(_ui.type, "Delegation", sizeof(_ui.type));
            strncpy(_ui.from_title, "Delegator", sizeof(_ui.from_title));
            strncpy(_ui.to_title, "Delegate", sizeof(_ui.to_title));
        }

        // Select the appropriate UX flow
        int n_idx = _tx.network_id == MAINNET_ID;
        int t_idx = _tx.tag == DELEGATION_TX;
        int v_idx = _tx.tx.valid_until != (uint32_t)-1;
        int m_idx = _ui.memo[0] != '\0';

        // Run the UX flow
        ux_flow_init(0, ux_sign_tx_flow[n_idx][t_idx][v_idx][m_idx], NULL);
    #endif

    *flags |= IO_ASYNCH_REPLY;
}
