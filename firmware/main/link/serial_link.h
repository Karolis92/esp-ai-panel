#pragma once

#define FW_VERSION "0.1.0"

/** Start the UART RX task that parses host NDJSON into the model. */
void serial_link_start(void);

/** Send a raw JSON object (without newline) to the host. Thread-safe. */
void serial_link_send(const char *json);
