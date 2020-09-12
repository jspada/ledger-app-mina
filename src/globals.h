#pragma once

#include <os.h>
#include <ux.h>
#include <os_io_seproxyhal.h>

#define P1_FIRST 0x00
#define P1_MORE 0x80

extern ux_state_t ux;
// display stepped screens
extern unsigned int ux_step;
extern unsigned int ux_step_count;

typedef struct internalStorage_t {
    uint8_t initialized;
} internalStorage_t;

extern const internalStorage_t N_storage_real;
#define N_storage (*(volatile internalStorage_t*) PIC(&N_storage_real))

