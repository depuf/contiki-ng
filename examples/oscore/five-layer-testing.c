#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "energest.h"
#if PLATFORM_SUPPORTS_BUTTON_HAL
#include "dev/button-hal.h"
#else
#include "dev/button-sensor.h"
#endif

#ifdef WITH_OSCORE
#include "oscore.h"
#include "oscore-layer.h"
uint8_t master_secret[16] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10};
uint8_t salt[8] = {0x9e,0x7c,0xa9,0x22,0x23,0x78,0x63,0x40};
// sender/receiver ids — one per layer
uint8_t sender_id_0[1] = { 0x01 };
uint8_t sender_id_1[1] = { 0x03 };
uint8_t sender_id_2[1] = { 0x05 };
uint8_t sender_id_3[1] = { 0x07 };
uint8_t sender_id_4[1] = { 0x09 };
uint8_t receiver_id_0[1] = { 0x02 };
uint8_t receiver_id_1[1] = { 0x04 };
uint8_t receiver_id_2[1] = { 0x06 };
uint8_t receiver_id_3[1] = { 0x08 };
uint8_t receiver_id_4[1] = { 0x0a };
uint8_t id_ctx_0[1] = { 0x01 };
uint8_t id_ctx_1[1] = { 0x02 };
uint8_t id_ctx_2[1] = { 0x03 };
uint8_t id_ctx_3[1] = { 0x04 };
uint8_t id_ctx_4[1] = { 0x05 };
#endif

#include "coap-log.h"
#define LOG_MODULE "client"
#define LOG_LEVEL  LOG_LEVEL_NONE
#define TOGGLE_INTERVAL 10

#define PROXY_EP  "coap://[fe80::212:4b00:9df:8ecb]"
#define SERVER_EP "coap://[fe80::212:4b00:9df:904f]"

PROCESS(er_example_client, "Nested OSCORE Example Client 5 layers");
AUTOSTART_PROCESSES(&er_example_client);

static struct etimer et;
char *service_urls[] = { ".well-known/core", "test/hello" };

static rtimer_clock_t rtt_start;

void client_chunk_handler(coap_message_t *response)
{
  rtimer_clock_t rtt = RTIMER_NOW() - rtt_start;
  const uint8_t *chunk;
  int len = coap_get_payload(response, &chunk);
  if(len <= 0) return;
  printf("RTT: %lu us\n", (uint32_t)(rtt * 1000000 / RTIMER_ARCH_SECOND));
  printf("response: |%.*s\n", len, (char *)chunk);
}

static oscore_ctx_t ctx_0;
static oscore_ctx_t ctx_1;
static oscore_ctx_t ctx_2;
static oscore_ctx_t ctx_3;
static oscore_ctx_t ctx_4;

PROCESS_THREAD(er_example_client, ev, data)
{
  PROCESS_BEGIN();
  static coap_message_t request[1];
  static coap_endpoint_t proxy_ep;

  coap_endpoint_parse(PROXY_EP, strlen(PROXY_EP), &proxy_ep);

#ifdef WITH_OSCORE
  oscore_derive_ctx(&ctx_0, master_secret, 16, salt, 8, 10, sender_id_0, 1, receiver_id_0, 1, id_ctx_0, 1);
  oscore_derive_ctx(&ctx_1, master_secret, 16, salt, 8, 10, sender_id_1, 1, receiver_id_1, 1, id_ctx_1, 1);
  oscore_derive_ctx(&ctx_2, master_secret, 16, salt, 8, 10, sender_id_2, 1, receiver_id_2, 1, id_ctx_2, 1);
  oscore_derive_ctx(&ctx_3, master_secret, 16, salt, 8, 10, sender_id_3, 1, receiver_id_3, 1, id_ctx_3, 1);
  oscore_derive_ctx(&ctx_4, master_secret, 16, salt, 8, 10, sender_id_4, 1, receiver_id_4, 1, id_ctx_4, 1);

  oscore_ep_ctx_set_association(&proxy_ep, service_urls[1], &ctx_0);
  oscore_ep_ctx_set_association(&proxy_ep, service_urls[1], &ctx_1);
  oscore_ep_ctx_set_association(&proxy_ep, service_urls[1], &ctx_2);
  oscore_ep_ctx_set_association(&proxy_ep, service_urls[1], &ctx_3);
  oscore_ep_ctx_set_association(&proxy_ep, service_urls[1], &ctx_4);
#endif

  etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);

  while(1) {
    PROCESS_YIELD();
    if(etimer_expired(&et)) {

      coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
      coap_set_header_uri_path(request, service_urls[1]);

      static oscore_layer_t layers[5];
      static oscore_path_t client_path;

      layers[0].next_hop_uri = PROXY_EP;
      layers[0].ctx = &ctx_0;
      layers[1].next_hop_uri = PROXY_EP;
      layers[1].ctx = &ctx_1;
      layers[2].next_hop_uri = PROXY_EP;
      layers[2].ctx = &ctx_2;
      layers[3].next_hop_uri = PROXY_EP;
      layers[3].ctx = &ctx_3;
      layers[4].next_hop_uri = SERVER_EP;
      layers[4].ctx = &ctx_4;

      client_path.layers = layers;
      client_path.num_layers = 5;
      request->dest_ep = &proxy_ep;
      oscore_ep_path_set(&proxy_ep, &client_path);

      const char msg[] = "Toggle!";
      coap_set_payload(request, (uint8_t *)msg, sizeof(msg) - 1);

      rtt_start = RTIMER_NOW();
      printf("\n--starting timer--\n");
      COAP_BLOCKING_REQUEST(&proxy_ep, request, client_chunk_handler);
      printf("\n--Done--\n");

      etimer_reset(&et);
    }
  }
  PROCESS_END();
}