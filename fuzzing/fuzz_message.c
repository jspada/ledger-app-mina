#include <assert.h>
#include <string.h>

#include "parse_tx.h"

// mock validate_address to avoid crypto function calls
bool validate_address(const char *address)
{
    if (strnlen(address, MINA_ADDRESS_LEN) != MINA_ADDRESS_LEN - 1) {
        return false;
    }

    return true;
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    tx_t tx;
    ui_t ui;

    if (!parse_tx(Data, Size, &tx, &ui)) {
        return 0;
    }

    // determines the length of each UI field to ensure that uninitialized
    // memory will be catched by sanitizers
    size_t len = 0;

    len += strlen(ui.from);
    len += strlen(ui.to);
    len += strlen(ui.amount);
    len += strlen(ui.fee);
    len += strlen(ui.total);
    len += strlen(ui.nonce);
    len += strlen(ui.valid_until);
    len += strlen(ui.memo);
    len += strlen(ui.type);
    len += strlen(ui.from_title);
    len += strlen(ui.to_title);

    // use the resulting length to prevents compiler optimization
    assert(len > 0 && len < 4096);

    return 0;
}
