# Onion CoAP — Contiki-NG Extension

This repository is a fork of [Contiki-NG](https://github.com/gunzter/contiki-ng) extended with an implementation of onion routing for constrained IoT devices. It uses CoAP as the transport protocol and OSCORE for layered encryption, targeting the Zolertia Firefly (CC2538, 32 KB RAM). This work was developed as part of an undergraduate dissertation at the University of Glasgow, supervised by Shahid Raza.

---

## Repository Structure

```text
contiki-ng/
├── examples/oscore/
│   ├── nested-oscore-client.c   # Onion CoAP client
│   ├── nested-oscore-proxy.c    # Onion CoAP proxy
│   └── nested-oscore-server.c   # Onion CoAP server
└── os/net/app-layer/coap/
    ├── coap-engine.c            # CoAP engine integration hooks
    └── oscore-support/
        ├── oscore.c             # Core nested OSCORE logic
        ├── oscore-layer.h       # Layer path structs
        └── oscore-layer.c       # Endpoint-to-path association table
```

| Path | Description |
|------|-------------|
| `examples/oscore/nested-oscore-client.c` | Onion CoAP client: constructs layered OSCORE messages |
| `examples/oscore/nested-oscore-proxy.c` | Onion CoAP proxy: decrypts one layer, forwards inner message |
| `examples/oscore/nested-oscore-server.c` | Onion CoAP server: decrypts final layer, serves resource |
| `os/net/app-layer/coap/oscore-support/oscore.c` | Core nested OSCORE logic |
| `os/net/app-layer/coap/oscore-support/oscore-layer.h` | Layer path structs (`oscore_layer_t`, `oscore_path_t`) |
| `os/net/app-layer/coap/oscore-support/oscore-layer.c` | Endpoint-to-path association table for multi-layer routing |
| `os/net/app-layer/coap/coap-engine.c` | CoAP engine integration hooks |

---

## Build Instructions

### Requirements

* 3 or more Zolertia Firefly boards (CC2538), or devices that can run Contiki-NG
* Linux or macOS host machine
* ARM GCC toolchain (`arm-none-eabi-gcc`)
* `make`
* Contiki-NG dependencies — see [Contiki-NG getting started guide](https://docs.contiki-ng.org/en/master/doc/getting-started/index.html)

### Setup

This repository is a fork of [Contiki-NG](https://github.com/gunzter/contiki-ng). Clone this repository instead of upstream Contiki-NG:
```sh
git clone https://github.com/depuf/contiki-ng
cd contiki-ng
git checkout develop
```

For toolchain setup (ARM GCC, build dependencies), follow the official [Contiki-NG getting started guide](https://docs.contiki-ng.org/en/master/doc/getting-started/index.html).

### Configuration

Before building, update `PROXY_EP` and `SERVER_EP` in `nested-oscore-client.c` to match your board MAC addresses.

The following parameters must be set in `os/net/app-layer/coap/coap-conf.h` and your network configuration header:
```c
/* Increase CoAP chunk size to accommodate layered encryption */
#define COAP_MAX_CHUNK_SIZE 256

/* Reduce network config to fit in 32 KB SRAM */
#define NBR_TABLE_CONF_MAX_NEIGHBORS 8
#define NETSTACK_MAX_ROUTE_ENTRIES   8
#define UIP_CONF_BUFFER_SIZE         512
```

### Build Steps

Each node role is selected at compile time by setting `MAKE_TARGET_MODE`. Replace `/path_to_port` with the serial port of your board (e.g. `/dev/ttyUSB0`).

**Client:**
```sh
cd examples/oscore
make TARGET=zoul BOARD=firefly PORT=/path_to_port \
     MAKE_TARGET_MODE=client nested-oscore-client.upload
```

**Proxy:**
```sh
make TARGET=zoul BOARD=firefly PORT=/path_to_port \
     MAKE_TARGET_MODE=proxy nested-oscore-proxy.upload
```

**Server:**
```sh
make TARGET=zoul BOARD=firefly PORT=/path_to_port \
     nested-oscore-server.upload
```

**Monitor serial output:**
```sh
make TARGET=zoul BOARD=firefly PORT=/path_to_port login
```

### Test Steps

1. Flash the server, proxy, and client onto three separate Firefly boards.
2. Power all three boards and open a serial monitor on each.
3. The client will automatically send a GET request to `/test/hello` after boot.
4. Expected output on the client serial monitor:

```text 
response: Hello World!
```

This confirms that layered encryption, per-hop forwarding, and response re-encryption are functioning correctly across the circuit.

---

## Known Limitations

* One concurrent circuit (`proxy_states[1]`), suitable for proof of concept only
* OSCORE sequence counters are not persisted across resets. Reflash both client, proxy, and server to resynchronise
* Logging should be disabled for four or more encryption layers
* Packets exceeding the 127-byte 802.15.4 MTU are fragmented transparently but with increased latency
