#Na konci automaticky compil:3
# Makefile pre projekt "weather-app"
# Umiestni tento súbor do priečinka build/  (t.j. weather-app/build/Makefile)
# Spúšťaj z priečinka build/:  make        make run        make clean

# ------- C nastavenia -------
CC       := gcc
CSTD     := -std=c11
CWARN    := -Wall -Wextra -Wpedantic
COPT     := -O2
CDEF     :=
CFLAGS   := $(CSTD) $(CWARN) $(COPT) $(CDEF) -fPIC -I../common
LDFLAGS  :=
LDLIBS   :=

# Linux knižnice
DLIBS_SH := -ldl -lm
DLIBS_EX := -ldl -lpthread

# ------- Cesty -------
ROOT     := ..
COMMON   := $(ROOT)/common
SENSORS  := $(ROOT)/sensors_src
MODULES  := $(ROOT)/modules
BOARD    := $(ROOT)/board
PROC     := $(ROOT)/processor
WEB      := $(ROOT)/web

# ------- Zdroje -------
# Senzory: každý .c v sensors_src/ -> modules/sensor_*.so
SENSOR_SRCS := $(wildcard $(SENSORS)/sensor_*.c)
SENSOR_SOS  := $(patsubst $(SENSORS)/%.c,$(MODULES)/%.so,$(SENSOR_SRCS))

# Executables
BOARD_SRCS := $(BOARD)/main.c $(BOARD)/board.c
PROC_SRCS  := $(PROC)/main.c $(PROC)/processor.c
WEB_SRCS   := $(WEB)/main.c  $(WEB)/web.c

BOARD_BIN  := boardd
PROC_BIN   := procd
WEB_BIN    := webd

# ------- Predvolené ciele -------
.PHONY: all sensors bins run clean veryclean
all: sensors bins

sensors: $(SENSOR_SOS)

bins: $(BOARD_BIN) $(PROC_BIN) $(WEB_BIN)

# ------- Pravidlá pre pluginy (shared objects) -------
# modules/sensor_foo.so z sensors_src/sensor_foo.c
$(MODULES)/sensor_%.so: $(SENSORS)/sensor_%.c $(COMMON)/sensor_if.h
	@mkdir -p $(MODULES)
	$(CC) $(CFLAGS) -I$(COMMON) -shared -o $@ $< $(DLIBS_SH)

# ------- Board daemon -------
$(BOARD_BIN): $(BOARD_SRCS) $(COMMON)/http.h $(COMMON)/json.h $(COMMON)/ringbuf.h $(COMMON)/proto.h $(COMMON)/timeutil.h $(COMMON)/sensor_if.h
	$(CC) $(CFLAGS) -I$(COMMON) -o $@ $(BOARD_SRCS) $(DLIBS_EX)

# ------- Processor daemon -------
$(PROC_BIN): $(PROC_SRCS) $(COMMON)/http.h $(COMMON)/json.h $(COMMON)/proto.h $(COMMON)/timeutil.h
	$(CC) $(CFLAGS) -I$(COMMON) -o $@ $(PROC_SRCS) -lpthread

# ------- Web server -------
$(WEB_BIN): $(WEB_SRCS)
	$(CC) $(CFLAGS) -I$(COMMON) -o $@ $(WEB_SRCS)

# ------- Pomocné ciele -------
run: all
	@echo "Spúšťam boardd, procd a webd (Ctrl+C na ukončenie)…"
	@./$(BOARD_BIN) & \
	./$(PROC_BIN)  & \
	./$(WEB_BIN)   & \
	wait

clean:
	@echo "Čistenie binárok…"
	@rm -f $(BOARD_BIN) $(PROC_BIN) $(WEB_BIN)

veryclean: clean
	@echo "Čistenie modulov…"
	@rm -f $(SENSOR_SOS)
