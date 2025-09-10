#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// Epoch seconds now
static inline uint32_t now_s(void) {
  return (uint32_t)time(NULL);
}

// RFC1123 date (for HTTP Date:)
static inline void http_date_rfc1123(time_t t, char out[64]) {
  struct tm g; gmtime_r(&t, &g);
  // e.g., "Tue, 15 Nov 1994 08:12:31 GMT"
  strftime(out, 64, "%a, %d %b %Y %H:%M:%S GMT", &g);
}

// Percent-decode into dst (dst==src ok). Returns length written (excl NUL).
static inline size_t url_decode(char *dst, const char *src) {
  size_t di=0;
  for (size_t i=0; src[i]; ++i) {
    if (src[i]=='%' && isxdigit((unsigned char)src[i+1]) && isxdigit((unsigned char)src[i+2])) {
      int hi = isdigit((unsigned char)src[i+1]) ? src[i+1]-'0' : 10+tolower((unsigned char)src[i+1])-'a';
      int lo = isdigit((unsigned char)src[i+2]) ? src[i+2]-'0' : 10+tolower((unsigned char)src[i+2])-'a';
      dst[di++] = (char)((hi<<4) | lo);
      i += 2;
    } else if (src[i]=='+') {
      dst[di++] = ' ';
    } else {
      dst[di++] = src[i];
    }
  }
  dst[di] = 0; return di;
}

// Extract query param (?a=1&b=2). src is path or query-only.
// Returns 1 if found; 0 otherwise.
static inline int query_get_param(const char *src, const char *key, char *out, size_t outsz) {
  const char *q = strchr(src, '?');
  q = q ? q+1 : src;
  size_t klen = strlen(key);
  while (*q) {
    // find key
    if (strncmp(q, key, klen)==0 && q[klen]=='=') {
      q += klen + 1;
      const char *end = q;
      while (*end && *end!='&' && *end!='#') end++;
      size_t len = (size_t)(end - q);
      if (len >= outsz) len = outsz - 1;
      memcpy(out, q, len); out[len]=0;
      // url-decode in place
      url_decode(out, out);
      return 1;
    }
    // skip to next pair
    while (*q && *q!='&') q++;
    if (*q=='&') q++;
  }
  return 0;
}

// Parse uint32 from query param, with default.
static inline uint32_t query_get_u32(const char *src, const char *key, uint32_t defv) {
  char tmp[32];
  if (!query_get_param(src, key, tmp, sizeof(tmp))) return defv;
  uint32_t v=0; for (size_t i=0; tmp[i]; ++i) { if (!isdigit((unsigned char)tmp[i])) return defv; v = v*10 + (uint32_t)(tmp[i]-'0'); }
  return v;
}

#endif // TIMEUTIL_H
