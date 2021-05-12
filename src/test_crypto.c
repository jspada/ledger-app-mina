#include "globals.h"
#include "curve_checks.h"

uint8_t set_result_test_crypto(void)
{
    uint8_t tx = 0;
    memmove(G_io_apdu_buffer + tx, "Success", 8);
    tx += 8;
    return tx;
}

void test_crypto(void)
{
    curve_checks();
    sendResponse(set_result_test_crypto(), true);
}

UX_STEP_NOCB_INIT(
    ux_test_crypto_done_flow_step,
    pb,
    test_crypto(),
    {
       &C_icon_validate_14,
      "Done",
    });

UX_FLOW(ux_test_crypto_done_flow,
        &ux_test_crypto_done_flow_step);

#ifdef HAVE_ON_DEVICE_UNIT_TESTS

UX_STEP_TIMEOUT(
    ux_test_crypto_testing_step,
    pb,
    1,
    ux_test_crypto_done_flow,
    {
      &C_icon_processing,
      "Unit tests...",
    });

UX_FLOW(ux_test_crypto_testing_flow,
        &ux_test_crypto_testing_step);
#else

UX_STEP_TIMEOUT(
    ux_test_crypto_testing_step,
    pb,
    1,
    ux_test_crypto_done_flow,
    {
      &C_icon_processing,
      "Testing crypto...",
    });

UX_FLOW(ux_test_crypto_testing_flow,
        &ux_test_crypto_testing_step);

#endif

void handle_test_crypto(uint8_t p1, uint8_t p2, uint8_t *dataBuffer,
                        uint8_t dataLength, volatile unsigned int *flags)
{
    UNUSED(p1);
    UNUSED(p2);
    UNUSED(dataBuffer);

    if (dataLength != 0) {
        THROW(INVALID_PARAMETER);
    }

    ux_flow_init(0, ux_test_crypto_testing_flow, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
