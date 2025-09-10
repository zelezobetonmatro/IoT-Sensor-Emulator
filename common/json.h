#ifndef JSON_H
#define JSON_H

// Tiny JSON writer: minimal escaping + append-only buffer.
// Usage:
//   jsonw jw; jsonw_init(&jw);
//   jsonw_obj_begin(&jw);
//   jsonw_pair_string(&jw, "name", "temp0");
//   jsonw_pair_number(&jw, "value", 21.5);
//   jsonw_obj_end(&jw);
//   printf("%s\n", jw.buf);
//   jsonw_free(&jw);

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

typedef struct {
  char *buf;
  size_t len, cap;
  int need_comma_stack[32];
  int depth;
} jsonw;

static inline void jsonw_init(jsonw *w) {
  w->cap = 256; w->len = 0; w->buf = (char*)malloc(w->cap); w->depth = 0;
  if (w->buf) w->buf[0] = 0;
  memset(w->need_comma_stack, 0, sizeof(w->need_comma_stack));
}

static inline void jsonw_free(jsonw *w) { free(w->buf); w->buf=NULL; w->len=w->cap=w->depth=0; }

static inline int jsonw_reserve(jsonw *w, size_t add) {
  if (w->len + add + 1 <= w->cap) return 0;
  size_t ncap = w->cap ? w->cap : 256;
  while (w->len + add + 1 > ncap) ncap *= 2;
  char *nb = (char*)realloc(w->buf, ncap);
  if (!nb) return -1;
  w->buf = nb; w->cap = ncap; return 0;
}

static inline int jsonw_append(jsonw *w, const char *s, size_t n) {
  if (jsonw_reserve(w, n) != 0) return -1;
  memcpy(w->buf + w->len, s, n);
  w->len += n; w->buf[w->len] = 0;
  return 0;
}

static inline int jsonw_putc(jsonw *w, char c) { return jsonw_append(w, &c, 1); }

static inline int jsonw_comma_if_needed(jsonw *w) {
  if (w->depth > 0 && w->need_comma_stack[w->depth-1]) {
    return jsonw_putc(w, ',');
  }
  return 0;
}

static inline void jsonw_mark_need_comma(jsonw *w) {
  if (w->depth > 0) w->need_comma_stack[w->depth-1] = 1;
}

static inline int jsonw_string_escaped(jsonw *w, const char *s) {
  if (jsonw_putc(w, '\"')) return -1;
  for (const unsigned char *p=(const unsigned char*)s; *p; ++p) {
    unsigned char c = *p;
    switch (c) {
      case '\"': if (jsonw_append(w, "\\\"", 2)) return -1; break;
      case '\\': if (jsonw_append(w, "\\\\", 2)) return -1; break;
      case '\b': if (jsonw_append(w, "\\b", 2)) return -1; break;
      case '\f': if (jsonw_append(w, "\\f", 2)) return -1; break;
      case '\n': if (jsonw_append(w, "\\n", 2)) return -1; break;
      case '\r': if (jsonw_append(w, "\\r", 2)) return -1; break;
      case '\t': if (jsonw_append(w, "\\t", 2)) return -1; break;
      default:
        if (c < 0x20) {
          char u[7]; // \u00xx
          snprintf(u, sizeof(u), "\\u%04x", (unsigned)c);
          if (jsonw_append(w, u, strlen(u))) return -1;
        } else {
          if (jsonw_putc(w, (char)c)) return -1;
        }
    }
  }
  return jsonw_putc(w, '\"');
}

static inline int jsonw_raw(jsonw *w, const char *raw) {
  return jsonw_append(w, raw, strlen(raw));
}

static inline int jsonw_obj_begin(jsonw *w) {
  if (jsonw_comma_if_needed(w)) return -1;
  if (jsonw_putc(w, '{')) return -1;
  w->need_comma_stack[w->depth++] = 0;
  return 0;
}

static inline int jsonw_obj_end(jsonw *w) {
  if (w->depth==0) return -1;
  w->depth--;
  return jsonw_putc(w, '}');
}

static inline int jsonw_arr_begin(jsonw *w) {
  if (jsonw_comma_if_needed(w)) return -1;
  if (jsonw_putc(w, '[')) return -1;
  w->need_comma_stack[w->depth++] = 0;
  return 0;
}

static inline int jsonw_arr_end(jsonw *w) {
  if (w->depth==0) return -1;
  w->depth--;
  return jsonw_putc(w, ']');
}

static inline int jsonw_key(jsonw *w, const char *k) {
  if (jsonw_comma_if_needed(w)) return -1;
  if (jsonw_string_escaped(w, k)) return -1;
  if (jsonw_putc(w, ':')) return -1;
  // after a key, the value comes; commas appear after the value
  return 0;
}

static inline int jsonw_pair_string(jsonw *w, const char *k, const char *v) {
  if (jsonw_key(w, k)) return -1;
  if (jsonw_string_escaped(w, v)) return -1;
  jsonw_mark_need_comma(w);
  return 0;
}

static inline int jsonw_pair_number(jsonw *w, const char *k, double v) {
  if (jsonw_key(w, k)) return -1;
  char tmp[64];
  int n = snprintf(tmp, sizeof(tmp), "%.10g", v);
  if (n < 0) return -1;
  if (jsonw_append(w, tmp, (size_t)n)) return -1;
  jsonw_mark_need_comma(w);
  return 0;
}

static inline int jsonw_pair_uint(jsonw *w, const char *k, uint32_t v) {
  if (jsonw_key(w, k)) return -1;
  char tmp[32];
  int n = snprintf(tmp, sizeof(tmp), "%u", v);
  if (n < 0) return -1;
  if (jsonw_append(w, tmp, (size_t)n)) return -1;
  jsonw_mark_need_comma(w);
  return 0;
}

static inline int jsonw_bool(jsonw *w, int b) {
  return jsonw_raw(w, b ? "true" : "false");
}

#endif // JSON_H
