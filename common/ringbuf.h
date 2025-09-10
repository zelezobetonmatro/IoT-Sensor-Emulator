#ifndef RINGBUF_H
#define RINGBUF_H

// Fixed-size ring buffer specialized for 256-capacity sample_t,
// but trivially generic via macros if you wish.

#include <stdint.h>
#include <string.h>

#ifndef RB_CAP
#define RB_CAP 256
#endif

typedef struct {
  double value;
  uint32_t ts; // epoch seconds
} sample_t;

typedef struct {
  char name[16];
  uint16_t head;   // next write index
  uint16_t count;  // 0..RB_CAP
  int type;        // optional: sensor_type_t
  sample_t buf[RB_CAP];
} sensor_slot_t;

static inline void rb_clear(sensor_slot_t *s) { s->head=0; s->count=0; }

static inline void rb_push(sensor_slot_t *s, sample_t v) {
  s->buf[s->head] = v;
  s->head = (uint16_t)((s->head + 1) & (RB_CAP-1));   // RB_CAP must be power of two (256)
  if (s->count < RB_CAP) s->count++;
}

// Iterate chronologically: call cb(sample*, idx, user) oldest→newest
static inline void rb_foreach(const sensor_slot_t *s,
                              void (*cb)(const sample_t*, uint16_t, void*),
                              void *user) {
  uint16_t start = (s->count == RB_CAP) ? s->head : 0;
  for (uint16_t i = 0; i < s->count; ++i) {
    uint16_t idx = (uint16_t)((start + i) & (RB_CAP-1));
    cb(&s->buf[idx], idx, user);
  }
}

// Copy out up to n chronological samples into dst; returns number copied.
static inline uint16_t rb_copy_chrono(const sensor_slot_t *s, sample_t *dst, uint16_t n) {
  if (n > s->count) n = s->count;
  uint16_t start = (s->count == RB_CAP) ? s->head : 0;
  for (uint16_t i = 0; i < n; ++i) {
    uint16_t idx = (uint16_t)((start + i) & (RB_CAP-1));
    dst[i] = s->buf[idx];
  }
  return n;
}

#endif // RINGBUF_H
