# Onion CoAP — Contiki-NG Extension

This repository is a fork of [Contiki-NG](https://github.com/gunzter/contiki-ng) extended with an implementation of onion routing for constrained IoT devices. It uses CoAP as the transport protocol and OSCORE for layered encryption, targeting the Zolertia Firefly (CC2538, 32 KB RAM).

This work was developed as part of an undergraduate dissertation at the University of Glasgow, supervised by Shahid Raza.

---

## Overview

Onion CoAP realises Tor-like anonymity on constrained hardware by applying nested OSCORE encryption at the client and forwarding through an intermediate proxy. Each hop decrypts one layer and forwards the remainder, without knowledge of the full circuit.

The core data plane (layered encryption, proxy forwarding, and response re-encryption) is implemented as an extension to the existing Contiki-NG OSCORE stack.

---

## Repository Structure

Modified and added files relative to upstream Contiki-NG:

| Path | Description |
|------|-------------|
| `examples/oscore/nested-oscore-client.c` | Onion CoAP client — constructs layered OSCORE messages |
| `examples/oscore/nested-oscore-proxy.c` | Onion CoAP proxy — decrypts one layer, forwards inner message |
| `examples/oscore/nested-oscore-server.c` | Onion CoAP server — decrypts final layer, serves resource |
| `os/net/app-layer/coap/oscore-support/oscore.c` | Core nested OSCORE logic |
| `os/net/app-layer/coap/oscore-support/oscore-layer.h` | Layer path structs (`oscore_layer_t`, `oscore_path_t`) |
| `os/net/app-layer/coap/oscore-support/oscore-layer.c` | Endpoint-to-path association table for multi-layer routing |
| `os/net/app-layer/coap/coap-engine.c` | CoAP engine integration hooks |

---

## Key Functions

| Function | Description |
|----------|-------------|
| `oscore_prepare_nested_message` | Constructs a layered OSCORE message from innermost to outermost layer |
| `oscore_decode_nested_message` | Decrypts one OSCORE layer; forwards inner message if `Proxy-Uri` is present |
| `oscore_proxy_encrypt_response` | Re-encrypts a response on the return path before forwarding to previous hop |
| `oscore_handle_message` | Entry point for OSCORE processing; dispatches to proxy or client path |

Proxy state is stored in `proxy_states[1]` - one concurrent circuit by design.

---

## Building

This project targets the Zolertia Firefly (CC2538). Ensure you have the ARM GCC toolchain installed.
Please see Contiki-NG documentation for initial setup.

### Client
```sh
cd examples/oscore
make TARGET=zoul BOARD=firefly PORT=/path_to_port MAKE_TARGET_MODE=client nested-oscore-client.upload
```
 
### Proxy
```sh
make TARGET=zoul BOARD=firefly PORT=/path_to_port MAKE_TARGET_MODE=proxy nested-oscore-proxy.upload
```
 
### Server
```sh
make TARGET=zoul BOARD=firefly PORT=/path_to_port nested-oscore-server.upload
```
 
Monitor output:
```sh
make TARGET=zoul BOARD=firefly PORT=/path_to_port login
```

---

## Configuration

Key parameters in `project-conf.h`:

```c
/* Increase CoAP chunk size to accommodate layered encryption */
/* Set in os/net/app-layer/coap/coap-conf.h */
#define COAP_MAX_CHUNK_SIZE 256

/* Reduce network config to fit in 32 KB SRAM */
#define NBR_TABLE_CONF_MAX_NEIGHBORS 8
#define NETSTACK_MAX_ROUTE_ENTRIES   8
#define UIP_CONF_BUFFER_SIZE         512

```

---

## Hardware Setup

A minimum of three Zolertia Firefly boards are required:

- **Client** : initiates the onion circuit
- **Proxy** : intermediate relay node
- **Server** : destination, serves `/test/hello`

Board addresses are hardcoded in `nested-oscore-client.c`. Update `PROXY_EP` and `SERVER_EP` to match your board MAC addresses.

---

## Known Limitations

- One concurrent circuit (`proxy_states[1]`), suitable for proof of concept only
- OSCORE sequence counters are not persisted across resets; reflashing both client and server is required to resynchronise
- Stack painting shows 68.7% peak utilisation on the client. Local `coap_message_t` variables in the encryption loop are a candidate for static allocation
- Packets exceeding the 127-byte 802.15.4 MTU are fragmented transparently but with increased latency

---

## Notes

- This README documents the Onion CoAP extension only, not the upstream Contiki-NG project
- `WITH_GROUPCOM` support is inherited from upstream and not used in this project
- Logging should be disabled for four or more encryption layers to avoid CPU pressure causing crashes on the Firefly
