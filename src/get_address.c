#include "get_address.h"
#include "utils.h"
#include "crypto.h"

static bool     _generated;
static uint32_t _account = 0;
static char     _bip44_path[27]; // max length when 44'/12586'/4294967295'/0/0
static char     _address[MINA_ADDRESS_LEN];

static uint8_t set_result_get_address(void)
{
    uint8_t tx = 0;
    memmove(G_io_apdu_buffer + tx, _address, sizeof(_address));
    tx += sizeof(_address);
    return tx;
}

static void gen_address(void)
{
    if (!_generated) {
        BEGIN_TRY {
            Keypair kp;
            TRY {
                generate_keypair(&kp, _account);
                if (!generate_address(_address, sizeof(_address), &kp.pub)) {
                    THROW(INVALID_PARAMETER);
                }
                _generated = true;

                #ifdef HAVE_ON_DEVICE_UNIT_TESTS
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

#ifdef HAVE_ON_DEVICE_UNIT_TESTS
    UX_STEP_NOCB_INIT(
        ux_get_address_done_flow_step,
        pb,
        gen_address(),
        {
            &C_icon_validate_14,
            "Done",
        }
    );

    UX_FLOW(
        ux_get_address_done_flow,
        &ux_get_address_done_flow_step
    );

    UX_STEP_TIMEOUT(
        ux_get_address_flow_unit_tests_step,
        pb,
        1,
        ux_get_address_done_flow,
        {
            &C_icon_processing,
            "Unit tests..."
        }
    );

    UX_FLOW(
        ux_get_address_unit_test_flow,
        &ux_get_address_flow_unit_tests_step
    );
#else
    UX_STEP_NOCB_INIT(
        ux_get_address_result_flow_address_step,
        bnnn_paging,
        gen_address(),
        {
            .title = "Address",
            .text = _address,
        }
    );

    UX_STEP_VALID(
        ux_get_address_result_flow_approve_step,
        pb,
        sendResponse(set_result_get_address(), true),
        {
            &C_icon_validate_14,
            "Approve",
        }
    );

    UX_STEP_VALID(
        ux_get_address_result_flow_reject_step,
        pb,
        sendResponse(0, false),
        {
            &C_icon_crossmark,
            "Reject",
        }
    );

    UX_FLOW(
        ux_get_address_result_flow,
        &ux_get_address_result_flow_address_step,
        &ux_get_address_result_flow_approve_step,
        &ux_get_address_result_flow_reject_step
    );

    UX_STEP_TIMEOUT(
        ux_get_address_comfort_flow_processing_step,
        pb,
        1,
        ux_get_address_result_flow,
        {
            &C_icon_processing,
            "Processing...",
        }
    );

    UX_FLOW(
        ux_get_address_comfort_flow,
        &ux_get_address_comfort_flow_processing_step
    );

    UX_STEP_NOCB(
        ux_get_address_flow_topic_step,
        pnn,
        {
            &C_icon_eye,
            "Get",
            "Address"
        }
    );

    UX_STEP_NOCB(
        ux_get_address_flow_path_step,
        bnnn_paging,
        {
            .title = "Path",
            .text = _bip44_path
        }
    );

    UX_STEP_VALID(
        ux_get_address_flow_generate_step,
        pb,
        ux_flow_init(0, ux_get_address_comfort_flow, NULL);,
        {
            &C_icon_validate_14,
            "Generate"
        }
    );

    UX_STEP_VALID(
        ux_get_address_flow_cancel_step,
        pb,
        sendResponse(0, false),
        {
            &C_icon_crossmark,
            "Cancel"
        }
    );

    UX_FLOW(
        ux_get_address_flow,
        &ux_get_address_flow_topic_step,
        &ux_get_address_flow_path_step,
        &ux_get_address_flow_generate_step,
        &ux_get_address_flow_cancel_step
    );
#endif

void handle_get_address(uint8_t p1, uint8_t p2, uint8_t *dataBuffer,
                        uint8_t dataLength, volatile unsigned int *flags)
{
    UNUSED(p1);
    UNUSED(p2);

    if (dataLength != 4) {
        THROW(INVALID_PARAMETER);
    }

    _generated = false;
    _address[0] = '\0';
    _account = read_uint32_be(dataBuffer);

    strncpy(_bip44_path, "44'/12586'/", sizeof(_bip44_path));              // used 11/27 (not counting null-byte)
    value_to_string(&_bip44_path[11], sizeof(_bip44_path) - 11, _account); // at most 21/27 used (max strnlen is 10 when _account = 4294967295)
    strncat(_bip44_path, "'/0/0", 6);                                      // at least 27 - 21 = 6 bytes free (just enough)

    #ifdef HAVE_ON_DEVICE_UNIT_TESTS
        ux_flow_init(0, ux_get_address_unit_test_flow, NULL);
    #else
        ux_flow_init(0, ux_get_address_flow, NULL);
    #endif

    *flags |= IO_ASYNCH_REPLY;
}
