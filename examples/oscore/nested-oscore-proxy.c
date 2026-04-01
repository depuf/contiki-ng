/*
 * Copyright (c) 2018, SICS, RISE AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \file
 *      Onion CoAP proxy - Derived from the OSCORE
 *      client example by Martin Gunnarsson.
 * \author
 *      Martin Gunnarsson <martin.gunnarsson@ri.se>
 *      Manassanan Vongprai 
 */

#include "coap-blocking-api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "coap-engine.h"
#ifdef WITH_OSCORE
#include "oscore.h"
#include "oscore-context.h"
#endif /* WITH_OSCORE */

#if PLATFORM_SUPPORTS_BUTTON_HAL
#include "dev/button-hal.h"
#else
#include "dev/button-sensor.h"
#endif

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP


#ifdef WITH_OSCORE
/* Key material, sender-ID and receiver-ID used for deriving an OSCORE-Security-Context. Note that Sender-ID and Receiver-ID is
 * mirrored in the Client and Server. */
uint8_t master_secret[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
uint8_t salt[8] = {0x9e, 0x7c, 0xa9, 0x22, 0x23, 0x78, 0x63, 0x40}; 
// 0 : client -> proxy 1 : proxy -> server 
uint8_t sender_id[2][1] = { {0x09},{0x02} };
uint8_t receiver_id[2][1] = { {0x08},{0x01} };
uint8_t id_contexts[2][1] = { {0x09},{0x05}  };
#endif /* WITH_OSCORE */

#define SERVER_EP "coap://[fe80::203:0003:0003:0003]"
#define CLIENT_EP "coap://[fe80::201:0001:0001:0001]"


PROCESS(er_example_proxy, "Nested OSCORE Example Proxy");
AUTOSTART_PROCESSES(&er_example_proxy);


PROCESS_THREAD(er_example_proxy, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  printf("Starting Nested OSCORE Example Proxy\n");

  #ifdef WITH_OSCORE
  /*Derive an OSCORE-Security-Context. */

  static oscore_ctx_t client_context;
  oscore_derive_ctx(&client_context, master_secret, 16, salt, 8, 10, sender_id[0], 1, receiver_id[0], 1, id_contexts[0], 1);

  static oscore_ctx_t server_context;
  oscore_derive_ctx(&server_context, master_secret, 16, salt, 8, 10, sender_id[1], 1, receiver_id[1], 1, id_contexts[1], 1);


  #ifdef OSCORE_PROXY_MODE
  proxy_init();
  #endif


  
  #endif /* WITH_OSCORE */

    /* Define application-specific events here. */
  while(1) {
    PROCESS_WAIT_EVENT();
  }                             /* while (1) */

  PROCESS_END();
}

