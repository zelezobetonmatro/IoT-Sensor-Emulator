#ifndef PROTO_H
#define PROTO_H

// Shared constants & structs across services.

#include <stdint.h>

#define SENSOR_WINDOW_SEC 256
#define MAX_SENSORS 8

typedef enum { S_TEMP=0, S_HUMID=1, S_PRESS=2, S_WIND=3, S_CUSTOM=255 } sensor_type_t;

// from ringbuf.h (kept duplicate typedef guard-friendly)
typedef struct {
  double value;
  uint32_t ts;
} sample_t;

typedef struct {
  sensor_type_t type;
  char name[16];
  sample_t buf[SENSOR_WINDOW_SEC];
  uint16_t head;
  uint16_t count;
} proto_sensor_slot_t;

typedef struct {
  char name[16];
  uint32_t from_ts;
  uint32_t to_ts;
  double last;
  double avg;
  double minv;
  double maxv;
} agg_record_t;

// Endpoint paths
#define EP_SENSORS   "/api/v1/sensors"
#define EP_WINDOW    "/api/v1/window"   // ?name=temp0
#define EP_SUMMARY   "/api/v1/summary"  // ?from=&to=

// Simple status JSON
#define JSON_ERR_NOT_FOUND    "{\"error\":\"not found\"}"
#define JSON_ERR_BAD_REQUEST  "{\"error\":\"bad request\"}"
#define JSON_OK               "{\"ok\":true}"

#endif // PROTO_H
