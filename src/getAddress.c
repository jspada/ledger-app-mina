#include <assert.h>

#include "getAddress.h"
#include "utils.h"
#include "crypto.h"

static uint32_t account = 0;
static char address[MINA_ADDRESS_LEN];

static uint8_t set_result_get_address() {
    uint8_t tx = 0;
    assert(strlen(address) == MINA_ADDRESS_LEN - 1);
    os_memmove(G_io_apdu_buffer + tx, address, MINA_ADDRESS_LEN);
    tx += MINA_ADDRESS_LEN;
    return tx;
}

static void gen_address()
{
    if (address[0] == '\0')
    {
        BEGIN_TRY {
            Keypair kp;
            TRY {
                generate_keypair(account, &kp);

                get_address(&kp.pub, address, sizeof(address));

                #ifdef UNIT_TESTS
                sendResponse(set_result_get_address(), true);
                #endif
            }
            FINALLY {
                os_memset(kp.priv, 0, sizeof(kp.priv));
            }
            END_TRY;
        }
    }
}

#ifdef UNIT_TESTS

UX_STEP_NOCB_INIT(
    ux_unit_tests_address_flow_4_step,
    pb,
    gen_address(),
    {
       &C_icon_validate_14,
      "Done",
    });

UX_FLOW(ux_display_public_flow,
  &ux_unit_tests_address_flow_4_step
);

#else

UX_STEP_NOCB_INIT(
    ux_display_public_flow_4_step,
    bnnn_paging,
    gen_address(),
    {
      .title = "Address",
      .text = address,
    });
UX_STEP_VALID(
    ux_display_public_flow_5_step,
    pb,
    sendResponse(set_result_get_address(), true),
    {
      &C_icon_validate_14,
      "Approve",
    });
UX_STEP_VALID(
    ux_display_public_flow_6_step,
    pb,
    sendResponse(0, false),
    {
      &C_icon_crossmark,
      "Reject",
    });

UX_FLOW(ux_display_public_flow,
  &ux_display_public_flow_4_step,
  &ux_display_public_flow_5_step,
  &ux_display_public_flow_6_step
);

#endif

UX_STEP_TIMEOUT(
    ux_processing_step,
    pb,
    1,
    ux_display_public_flow,
    {
      &C_icon_processing,
      "Processing",
    });

UX_FLOW(ux_processing_flow,
        &ux_processing_step);

void handleGetAddress(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags, volatile unsigned int *tx)
{
    UNUSED(dataLength);
    UNUSED(p2);

    address[0] = '\0';
    account = readUint32BE(dataBuffer);

    ux_flow_init(0, ux_processing_flow, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
