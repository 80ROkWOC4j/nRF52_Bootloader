/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdbool.h>
#include <stdint.h>

#define TOTEM_DFU_RETURN_MARKER 0x54444655UL

/* Dedicated NOLOAD RAM above the SoftDevice and upstream fault record.
 * Unlike GPREGRET2, SRAM can retain this record across a physical pin reset.
 * A magic/complement pair rejects uninitialized or corrupted RAM contents.
 */
typedef struct {
    uint32_t magic;
    uint32_t inverse;
} totem_reset_state_t;

static inline void totem_reset_clear(volatile totem_reset_state_t *state) {
    state->magic = 0;
    state->inverse = 0;
}

static inline void totem_reset_mark_dfu(volatile totem_reset_state_t *state) {
    state->inverse = (uint32_t)~TOTEM_DFU_RETURN_MARKER;
    state->magic = TOTEM_DFU_RETURN_MARKER;
}

static inline bool totem_reset_consume_dfu(volatile totem_reset_state_t *state) {
    bool const valid = state->magic == TOTEM_DFU_RETURN_MARKER &&
                       state->inverse == (uint32_t)~TOTEM_DFU_RETURN_MARKER;
    totem_reset_clear(state);
    return valid;
}

/* Evaluated AFTER the upstream double-reset window and explicit mode checks.
 * RESETREAS is snapshotted and cleared on every boot, including DFU boots.
 * The RAM marker is consumed on entry and set only while waiting in DFU.
 */
static inline bool totem_reset_enters_ota(uint32_t reason, bool was_in_dfu,
                                        bool valid_app, bool explicit_dfu) {
    return reason == 1u && !was_in_dfu &&
           valid_app && !explicit_dfu;
}
