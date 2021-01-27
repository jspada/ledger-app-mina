#include "get_address.h"
#include "utils.h"
#include "crypto.h"

static uint32_t _account = 0;
static char     _address[MINA_ADDRESS_LEN];

static uint8_t set_result_get_address(void)
{
    uint8_t tx = 0;
    os_memmove(G_io_apdu_buffer + tx, _address, sizeof(_address));
    tx += sizeof(_address);
    return tx;
}

static void gen_address(void)
{
    if (_address[0] == '\0') {
        BEGIN_TRY {
            Keypair kp;
            TRY {
                generate_keypair(&kp, _account);
                if (!generate_address(_address, sizeof(_address), &kp.pub)) {
                    THROW(INVALID_PARAMETER);
                }

                #ifdef ON_DEVICE_UNIT_TESTS
                sendResponse(set_result_get_address(), true);
                #endif
            }
            FINALLY {
                explicit_bzero(kp.priv, sizeof(kp.priv));
            }
            END_TRY;
        }
    }
}

#ifdef ON_DEVICE_UNIT_TESTS

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

UX_STEP_TIMEOUT(
    ux_processing_step,
    pb,
    1,
    ux_display_public_flow,
    {
      &C_icon_processing,
      "Unit tests...",
    });

UX_FLOW(ux_processing_flow,
        &ux_processing_step);

#else

UX_STEP_NOCB_INIT(
    ux_display_public_flow_4_step,
    bnnn_paging,
    gen_address(),
    {
      .title = "Address",
      .text = _address,
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

UX_STEP_TIMEOUT(
    ux_processing_step,
    pb,
    1,
    ux_display_public_flow,
    {
      &C_icon_processing,
      "Processing...",
    });

UX_FLOW(ux_processing_flow,
        &ux_processing_step);

#endif

void handle_get_address(uint8_t p1, uint8_t p2, uint8_t *dataBuffer,
                        uint8_t dataLength, volatile unsigned int *flags,
                        volatile unsigned int *tx)
{
    UNUSED(p1);
    UNUSED(p2);

    if (dataLength != 4) {
        THROW(INVALID_PARAMETER);
    }

    _address[0] = '\0';
    _account = read_uint32_be(dataBuffer);

    ux_flow_init(0, ux_processing_flow, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
