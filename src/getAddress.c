#include "getAddress.h"
#include "os.h"
#include "ux.h"
#include "utils.h"

static uint32_t account = 0;
static char address[FULL_ADDRESS_LENGTH];

static uint8_t set_result_get_address() {
    uint8_t tx = 0;
    const uint8_t address_size = strlen(address);
    G_io_apdu_buffer[tx++] = address_size;
    os_memmove(G_io_apdu_buffer + tx, address, address_size);
    tx += address_size;
    return tx;
}

void genAddress()
{
    if (address[0] == '\0')
    {
        // Simulate Coda Tweedle slowness
        float x = 123.92;
        for (size_t i = 0; i < 2000000; i++)
            for (size_t j = 0; j < 10; j++)
                x *= 1.002682;

        uint8_t publicKey[32];
        getPublicKey(account, publicKey);
        getAddressStringFromBinary(publicKey, address);

    }
}

//////////////////////////////////////////////////////////////////////

UX_STEP_NOCB_INIT(
    ux_display_public_flow_5_step, 
    bnnn_paging,
    genAddress(),
    {
      .title = "Address",
      .text = address,
    });
UX_STEP_VALID(
    ux_display_public_flow_6_step, 
    pb, 
    sendResponse(set_result_get_address(), true),
    {
      &C_icon_validate_14,
      "Approve",
    });
UX_STEP_VALID(
    ux_display_public_flow_7_step, 
    pb, 
    sendResponse(0, false),
    {
      &C_icon_crossmark,
      "Reject",
    });

UX_FLOW(ux_display_public_flow,
  &ux_display_public_flow_5_step,
  &ux_display_public_flow_6_step,
  &ux_display_public_flow_7_step
);

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

void handleGetAddress(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags, volatile unsigned int *tx) {
    UNUSED(dataLength);
    UNUSED(p2);
    address[0] = '\0';
    account = readUint32BE(dataBuffer);

    ux_flow_init(0, ux_processing_flow, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
