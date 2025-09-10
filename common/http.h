#ifndef HTTP_H
#define HTTP_H

// Tiny HTTP helpers: parse request line + headers from a small buffer,
// and craft minimal JSON / static-file responses.
// Networking (accept/read/write) is up to your server .c code.

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#ifndef HTTP_MAX_HEADERS
#define HTTP_MAX_HEADERS 24
#endif
#ifndef HTTP_MAX_METHOD
#define HTTP_MAX_METHOD 8
#endif
#ifndef HTTP_MAX_PATH
#define HTTP_MAX_PATH 256
#endif
#ifndef HTTP_MAX_VER
#define HTTP_MAX_VER 16
#endif
#ifndef HTTP_MAX_HEADER_NAME
#define HTTP_MAX_HEADER_NAME 32
#endif
#ifndef HTTP_MAX_HEADER_VALUE
#define HTTP_MAX_HEADER_VALUE 256
#endif

typedef struct {
  char name[HTTP_MAX_HEADER_NAME];
  char value[HTTP_MAX_HEADER_VALUE];
} http_header_t;

typedef struct {
  char method[HTTP_MAX_METHOD];   // "GET" / "POST"
  char path[HTTP_MAX_PATH];       // "/api/v1/..."
  char version[HTTP_MAX_VER];     // "HTTP/1.1"
  const char *body;               // may point into the original buffer
  size_t body_len;
  int header_count;
  http_header_t headers[HTTP_MAX_HEADERS];
} http_request_t;

// Returns index or -1
static inline int http_get_header(const http_request_t *req, const char *name) {
  for (int i = 0; i < req->header_count; ++i) {
    if (strcasecmp(req->headers[i].name, name) == 0) return i;
  }
  return -1;
}

// Very small parser: expects full request in buf (headers only; body optional).
// Returns 0 on success; -1 on parse error; -2 if need more data (no \r\n\r\n yet).
static inline int http_parse_request(char *buf, size_t len, http_request_t *out) {
  memset(out, 0, sizeof(*out));
  // find header/body split
  char *end = NULL;
  for (size_t i = 3; i < len; ++i) {
    if (buf[i-3]=='\r' && buf[i-2]=='\n' && buf[i-1]=='\r' && buf[i]=='\n') {
      end = &buf[i];
      break;
    }
  }
  if (!end) return -2;

  // request line
  char *p = buf;
  char *eol = strstr(p, "\r\n");
  if (!eol) return -1;
  *eol = 0;
  if (sscanf(p, "%7s %255s %15s", out->method, out->path, out->version) != 3) return -1;
  p = eol + 2;

  // headers
  int hc = 0;
  while (p < end - 2 && hc < HTTP_MAX_HEADERS) {
    char *line_end = strstr(p, "\r\n");
    if (!line_end) break;
    if (line_end == p) { // empty line (shouldn't happen before end)
      p += 2; break;
    }
    *line_end = 0;
    char *colon = strchr(p, ':');
    if (colon) {
      size_t nlen = (size_t)(colon - p);
      size_t vlen = strlen(colon + 1);
      while (vlen && (colon[1 + (int)(vlen-1)]==' ' || colon[1 + (int)(vlen-1)]=='\t')) vlen--;
      size_t off = 1;
      while (off < vlen && (colon[off]==' ' || colon[off]=='\t')) off++;
      if (nlen >= HTTP_MAX_HEADER_NAME) nlen = HTTP_MAX_HEADER_NAME-1;
      if (vlen-off >= HTTP_MAX_HEADER_VALUE) vlen = off + HTTP_MAX_HEADER_VALUE-1;
      memcpy(out->headers[hc].name, p, nlen); out->headers[hc].name[nlen]=0;
      memcpy(out->headers[hc].value, colon+off, vlen-off); out->headers[hc].value[vlen-off]=0;
      hc++;
    }
    p = line_end + 2;
  }
  out->header_count = hc;

  // body (if any)
  size_t header_bytes = (size_t)((end - buf) + 4);
  if (header_bytes < len) {
    out->body = buf + header_bytes;
    out->body_len = len - header_bytes;
  } else {
    out->body = NULL; out->body_len = 0;
  }
  return 0;
}

static inline ssize_t http_write_all(int fd, const void *buf, size_t len) {
  const uint8_t *p = (const uint8_t*)buf;
  size_t left = len;
  while (left) {
    ssize_t n = write(fd, p, left);
    if (n <= 0) return n;
    p += (size_t)n; left -= (size_t)n;
  }
  return (ssize_t)len;
}

// Minimal response helpers
static inline int http_send_status_json(int fd, int status, const char *status_text,
                                        const char *json_body,
                                        const char *extra_headers /* may be NULL */) {
  char head[512];
  size_t blen = json_body ? strlen(json_body) : 0;
  int n = snprintf(head, sizeof(head),
    "HTTP/1.1 %d %s\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: %zu\r\n"
    "Connection: close\r\n"
    "%s%s"
    "\r\n",
    status, status_text, blen,
    extra_headers ? extra_headers : "",
    extra_headers ? "\r\n" : "");
  if (n < 0) return -1;
  if (http_write_all(fd, head, (size_t)n) < 0) return -1;
  if (blen && http_write_all(fd, json_body, blen) < 0) return -1;
  return 0;
}

static inline int http_send_status_text(int fd, int status, const char *status_text,
                                        const char *mime, const char *body) {
  size_t blen = body ? strlen(body) : 0;
  char head[512];
  int n = snprintf(head, sizeof(head),
    "HTTP/1.1 %d %s\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %zu\r\n"
    "Connection: close\r\n\r\n",
    status, status_text, mime ? mime : "text/plain", blen);
  if (n < 0) return -1;
  if (http_write_all(fd, head, (size_t)n) < 0) return -1;
  if (blen && http_write_all(fd, body, blen) < 0) return -1;
  return 0;
}

#endif // HTTP_H
