#include <assert.h>
#include <stdlib.h>

#include "signPaymentTx.h"
#include "utils.h"
#include "crypto.h"

static uint32_t account = 0;
static char sender[MINA_ADDRESS_LEN];
static char receiver[MINA_ADDRESS_LEN];
static char amount[32];
static char fee[32];
static char nonce[32];
static char random_oracle_input[358];

UX_STEP_NOCB(
    ux_sign_payment_tx_flow_1_step,
    bnnn_paging,
    {
      .title = "From",
      .text = sender,
    });
UX_STEP_NOCB(
    ux_sign_payment_tx_flow_2_step,
    bnnn_paging,
    {
      .title = "To",
      .text = receiver,
    });
UX_STEP_NOCB(
    ux_sign_payment_tx_flow_3_step,
    bn,
    {
      .line1 = "Amount",
      .line2 = amount,
    });
UX_STEP_NOCB(
    ux_sign_payment_tx_flow_4_step,
    bn,
    {
      .line1 = "Fee",
      .line2 = fee,
    });
UX_STEP_NOCB(
    ux_sign_payment_tx_flow_5_step,
    bn,
    {
      .line1 = "Nonce",
      .line2 = nonce,
    });
UX_STEP_VALID(
    ux_sign_payment_tx_flow_6_step,
    pb,
    sendResponse(0, false),
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

void handleSignPaymentTx(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags, volatile unsigned int *tx)
{
    UNUSED(dataLength);
    UNUSED(p2);

    // TODO: Check dataLength is valid

    // 0-3: sender_bip44_account
    account = readUint32BE(dataBuffer);

    // 4-58: sender_address
    memcpy(sender, dataBuffer + 4, MINA_ADDRESS_LEN - 1);
    sender[MINA_ADDRESS_LEN - 1] = '\0';

    // 59-113: receiver
    memcpy(receiver, dataBuffer + 59, MINA_ADDRESS_LEN - 1);
    receiver[MINA_ADDRESS_LEN - 1] = '\0';

    // 114-121: amount
    amountToString(amount, sizeof(amount), readUint64BE(dataBuffer + 114));

    // 122-129: fee
    amountToString(fee, sizeof(fee), readUint64BE(dataBuffer + 122));

    // 130-137: nonce
    valueToString(nonce, sizeof(nonce), readUint64BE(dataBuffer + 130));

    // 138-495: random_oracle_input
    memcpy(random_oracle_input, dataBuffer + 138, sizeof(random_oracle_input));

    ux_flow_init(0, ux_sign_payment_tx_flow, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
