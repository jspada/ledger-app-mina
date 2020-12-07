#include <assert.h>
#include <stdlib.h>

#include "signPaymentTx.h"
#include "utils.h"
#include "crypto.h"
#include "random_oracle_input.h"

// Account variables
static uint32_t _account = 0;
static Keypair  _kp;
static char     _address[MINA_ADDRESS_LEN];

// Transaction related
static Transaction _tx;
static Field       _input_fields[3];
static uint8_t     _input_bits[TX_BITS_LEN];
static ROInput     _roinput;
static Signature   _sig;
static char        _signature[65];

// UI variables
static struct ui_t {
    char sender[MINA_ADDRESS_LEN];
    char receiver[MINA_ADDRESS_LEN];
    char amount[32];
    char fee[32];
    char nonce[32];
} _ui;

static uint8_t set_result_get_signature()
{
    uint8_t tx = 0;
    os_memmove(G_io_apdu_buffer + tx, _signature, sizeof(_signature));
    tx += sizeof(_signature);
    return tx;
}

static void sign_transaction()
{
    if (_signature[0] == '\0')
    {
        BEGIN_TRY {
            TRY {

                // Get the private key and validate corresponding public key
                // matches the sender
                generate_keypair(&_kp, _account);
                int result = get_address(_address, sizeof(_address), &_kp.pub);
                switch (result) {
                    case -2:
                        THROW(EXCEPTION_OVERFLOW);

                    case -1:
                        THROW(INVALID_PARAMETER);

                    default:
                        ; // SUCCESS
                }
                if (strncmp(_address, _ui.sender, sizeof(_address)) != 0) {
                    THROW(INVALID_PARAMETER);
                }

                // Create random oracle input from transaction
                _roinput.fields = _input_fields;
                _roinput.fields_capacity = ARRAY_LEN(_input_fields);
                _roinput.bits = _input_bits;
                _roinput.bits_capacity = ARRAY_LEN(_input_bits);
                roinput_from_transaction(&_roinput, &_tx);

                sign(&_sig, &_kp, &_roinput);
                for (size_t i = 0; i < sizeof(_sig.s); i++) {
                    snprintf(&_signature[i*2], 3, "%02x", _sig.s[i]);
                }
                _signature[64] = '\0';

            }
            FINALLY {
                os_memset((void *)_kp.priv, 0, sizeof(_kp.priv));
            }
            END_TRY;
        }
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
    ux_sign_payment_tx_flow_1_step,
    bnnn_paging,
    {
      .title = "From",
      .text = _ui.sender,
    });
UX_STEP_NOCB(
    ux_sign_payment_tx_flow_2_step,
    bnnn_paging,
    {
      .title = "To",
      .text = _ui.receiver,
    });
UX_STEP_NOCB(
    ux_sign_payment_tx_flow_3_step,
    bn,
    {
      .line1 = "Amount",
      .line2 = _ui.amount,
    });
UX_STEP_NOCB(
    ux_sign_payment_tx_flow_4_step,
    bn,
    {
      .line1 = "Fee",
      .line2 = _ui.fee,
    });
UX_STEP_NOCB(
    ux_sign_payment_tx_flow_5_step,
    bn,
    {
      .line1 = "Nonce",
      .line2 = _ui.nonce,
    });
UX_STEP_VALID(
    ux_sign_payment_tx_flow_6_step,
    pb,
    ux_flow_init(0, ux_signing_comfort_flow, NULL);,
    {
      &C_icon_validate_14,
      "Approve",
    });
UX_STEP_VALID(
    ux_sign_payment_tx_flow_7_step,
    pb,
    sendResponse(0, false),
    {
      &C_icon_crossmark,
      "Reject",
    });

UX_FLOW(ux_sign_payment_tx_flow,
        &ux_sign_payment_tx_flow_1_step,
        &ux_sign_payment_tx_flow_2_step,
        &ux_sign_payment_tx_flow_3_step,
        &ux_sign_payment_tx_flow_4_step,
        &ux_sign_payment_tx_flow_5_step,
        &ux_sign_payment_tx_flow_6_step,
        &ux_sign_payment_tx_flow_7_step
    );

void handleSignPaymentTx(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint32_t dataLength, volatile unsigned int *flags, volatile unsigned int *unused)
{
    UNUSED(dataLength);
    UNUSED(p2);

    // TODO: Check dataLength is valid
    if (dataLength != 638) {
        THROW(INVALID_PARAMETER);
    }

    _signature[0] = '\0';

    // 0-3: sender_bip44_account
    _account = read_uint32_be(dataBuffer);

    // 4-58: sender_address
    memcpy(_ui.sender, dataBuffer + 4, MINA_ADDRESS_LEN - 1);
    _ui.sender[MINA_ADDRESS_LEN - 1] = '\0';
    read_public_key_compressed(&_tx.source_pk, _ui.sender);

    // TODO fee_payer
    read_public_key_compressed(&_tx.fee_payer_pk, _ui.sender);

    // 59-113: receiver
    memcpy(_ui.receiver, dataBuffer + 59, MINA_ADDRESS_LEN - 1);
    _ui.receiver[MINA_ADDRESS_LEN - 1] = '\0';
    read_public_key_compressed(&_tx.receiver_pk, _ui.receiver);

    // 114-121:
    _tx.amount = read_uint64_be(dataBuffer + 114);
    amount_to_string(_ui.amount, sizeof(_ui.amount), _tx.amount);

    // TODO token_id
    _tx.token_id = 1; // TODO

    // 122-129: fee
    _tx.fee = read_uint64_be(dataBuffer + 122);
    amount_to_string(_ui.fee, sizeof(_ui.fee), _tx.fee);

    // TODO fee_token
    _tx.fee_token = 1;

    // 130-137: nonce // TODO: change to 32bits
    _tx.nonce = (uint32_t)read_uint64_be(dataBuffer + 130);
    value_to_string(_ui.nonce, sizeof(_ui.nonce), (uint64_t)_tx.nonce);

    // TODO
    _tx.valid_until = 10000;
    _tx.tag[0] = 0;
    _tx.tag[1] = 0;
    _tx.tag[2] = 0;
    _tx.token_locked = false;
    prepare_memo(_tx.memo, "this is a memo");

    ux_flow_init(0, ux_sign_payment_tx_flow, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
