/*
 * Copyright (c) 2016, Yasuyuki Tanaka
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "contiki.h"
#include "unit-test.h"
#include "net/linkaddr.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-asn.h"
#include "net/mac/tsch/tsch-packet.h"
#include "net/mac/tsch/tsch-schedule.h"

#include <stdio.h>
#include <string.h>


#ifndef TEST_CONFIG_TYPE
#define TEST_CONFIG_TYPE DEFAULT
#endif

typedef enum { SUCCESS, FAILURE } result_t;

typedef enum { DEFAULT = 0, SECURITY_ON, ALL_ENABLED } config_type_t;

typedef struct {
  size_t  len;
  uint8_t buf[TSCH_PACKET_MAX_LEN];
} frame_t;

#define NODE1 {{ 0xc1, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}
#define NODE2 {{ 0xc1, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 }}

/*
 * The following vectors are obtained with the former
 * examples/ipv6/rpl-tsch/rpl-tsch-z1.csc except for the enhanced beacon for
 * ALL_ENABLED. The raw frame was generated with rpl-tsch-cooja.csc because
 * there is an issue in TSCH Timeslot IE generated by z1 mote.
 */

typedef struct {
  linkaddr_t src;
  uint64_t   asn;
  uint8_t hdr_len;
  frame_t    frame;
} eb_test_vector_t;

static const eb_test_vector_t eb_test_vectors[] = {
  { /* DEFAULT */
    NODE1, 7, 18,
    { 37,  { 0x00, 0xeb, 0xcd, 0xab, 0xff, 0xff, 0xcd, 0xab,
             0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0xc1,
             0x00, 0x3f, 0x11, 0x88, 0x06, 0x1a, 0x07, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x01, 0x1c, 0x00, 0x01,
             0xc8, 0x00, 0x01, 0x1b, 0x00 }
    }
  },
  { /* SECURITY_ON */
    NODE1, 2, 20,
    { 43, { 0x08, 0xeb, 0xcd, 0xab, 0xff, 0xff, 0xcd, 0xab,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0xc1,
            0x69, 0x01, 0x00, 0x3f, 0x11, 0x88, 0x06, 0x1a,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x1c,
            0x00, 0x01, 0xc8, 0x00, 0x01, 0x1b, 0x00, 0x7d,
            0x3e, 0x39, 0x9a, 0x6f, 0x7b }
    }
  },
  { /* ALL_ENABLED */
    NODE1, 12, 18,
    { 85, { 0x00, 0xeb, 0xcd, 0xab, 0xff, 0xff, 0xcd, 0xab,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0xc1,
            0x00, 0x3f, 0x41, 0x88, 0x06, 0x1a, 0x0c, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x19, 0x1c, 0x01, 0x08,
            0x07, 0x80, 0x00, 0x48, 0x08, 0xfc, 0x03, 0x20,
            0x03, 0xe8, 0x03, 0x98, 0x08, 0x90, 0x01, 0xc0,
            0x00, 0x60, 0x09, 0xa0, 0x10, 0x10, 0x27, 0x10,
            0xc8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x04, 0x00, 0x0f, 0x19, 0x1a, 0x14, 0x00,
            0x00, 0x0a, 0x1b, 0x01, 0x00, 0x07, 0x00, 0x01,
            0x00, 0x00, 0x00, 0x00, 0x0f }
    }
  }
};

typedef struct {
  linkaddr_t src;
  linkaddr_t dest;
  uint64_t   asn;   // used only for the SECURITY_ON case
  uint8_t    seqno;
  uint16_t   drift;
  uint8_t    nack;
  frame_t    frame;
} eack_test_vector_t;

static const eack_test_vector_t eack_test_vectors[]  = {
  { /* DEFAULT */
    NODE1, NODE2, 0, 1, 214, 0,
    { 17, { 0x02, 0x2e, 0x01, 0xcd, 0xab, 0x02, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x0c, 0xc1, 0x02, 0x0f, 0xd6,
            0x00 }
    }
  },
  { /* SECURITY_ON */
    NODE1, NODE2, 108, 1, 214, 0,
    { 23, { 0x0a, 0x2e, 0x01, 0xcd, 0xab, 0x02, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x0c, 0xc1, 0x6d, 0x02, 0x02,
            0x0f, 0xd6, 0x00, 0x5e, 0x20, 0x84, 0xda }
    }
  },
  { /* ALL_ENABLED */
    NODE1, NODE2, 0, 1, 214, 0,
    { 25, { 0x02, 0xee, 0x01, 0xcd, 0xab, 0x02, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x0c, 0xc1, 0x01, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x0c, 0xc1, 0x02, 0x0f, 0xd6,
            0x00 }
    }
  }
};

PROCESS(test_process, "tsch-packet-create test");
AUTOSTART_PROCESSES(&test_process);

static void
print_hex(const uint8_t *p, size_t len)
{
  int i;
  for(i = 0; i < len; i++) {
    printf("%02x", p[i]);
  }
}

static void
update_current_asn(uint64_t asn)
{
  tsch_current_asn.ls4b = (uint32_t)(asn & 0xffffffff);
  tsch_current_asn.ms1b = (uint8_t)((asn >> 32) & 0xff);
}

static result_t
test_create_eb(const eb_test_vector_t *v)
{
  uint8_t buf[TSCH_PACKET_MAX_LEN];
  int len;
  uint8_t hdr_len;
  uint8_t tsch_sync_ie_offset;

  memset(buf, 0, sizeof(buf));

  linkaddr_copy(&linkaddr_node_addr, &v->src);
  update_current_asn(v->asn);

  len = tsch_packet_create_eb(buf, sizeof(buf),
                              &hdr_len, &tsch_sync_ie_offset);
  tsch_packet_update_eb(buf, len, tsch_sync_ie_offset);
#if WITH_SECURITY_ON
  len += tsch_security_secure_frame(buf, buf,
                                    hdr_len, len - hdr_len,
                                    &tsch_current_asn);
#endif

  printf("%s: len=%u, hdr_len=%u, buf=", __func__, len, hdr_len);
  print_hex(buf, len);
  printf("\n");

  if(len != v->frame.len ||
     hdr_len != v->hdr_len ||
     memcmp(buf, v->frame.buf, len) != 0) {
    return FAILURE;
  }

  return SUCCESS;
}

static result_t
test_parse_eb(const eb_test_vector_t *v)
{
  frame802154_t frame;
  struct ieee802154_ies ies;
  uint8_t hdr_len;
  int frame_without_mic;
  int len;
  uint64_t asn;
  linkaddr_t src_addr;

#if WITH_SECURITY_ON
  frame_without_mic = 0;
  update_current_asn(v->asn);
#else
  frame_without_mic = 1;
#endif

  memset(&frame, 0, sizeof(frame));
  memset(&ies, 0, sizeof(ies));
  hdr_len = 0;

  len = tsch_packet_parse_eb(v->frame.buf, v->frame.len, &frame, &ies, &hdr_len,
                             frame_without_mic);
  asn = ((uint64_t)ies.ie_asn.ms1b << 32) + ies.ie_asn.ls4b;
  printf("%s: len=%u, hdr_len=%u, asn=%llu\n", __func__, len, hdr_len, asn);

#if WITH_SECURITY_ON
  /* adjust 'len' with the length of MIC which is included in a raw frame */
  len += tsch_security_mic_len(&frame);
#endif

  if(frame.fcf.frame_type != FRAME802154_BEACONFRAME ||
     frame.fcf.frame_version != FRAME802154_IEEE802154E_2012) {
    return FAILURE;
  }

  if(len != v->frame.len ||
     hdr_len != v->hdr_len ||
     asn != v->asn) {
    return FAILURE;
  }

  if(frame802154_extract_linkaddr(&frame, &src_addr, NULL) == 0||
     linkaddr_cmp(&src_addr, &v->src) == 0) {
    return FAILURE;
  }

  return SUCCESS;
}

static result_t
test_create_eack(const eack_test_vector_t *v)
{
  uint8_t buf[TSCH_PACKET_MAX_LEN];
  int len;
#if WITH_SECURITY_ON
  int data_len = 0;
#endif

  memset(buf, 0, sizeof(buf));
  linkaddr_copy(&linkaddr_node_addr, &v->src);

  len = tsch_packet_create_eack(buf, sizeof(buf),
                                &v->dest, v->seqno, v->drift, v->nack);
#if WITH_SECURITY_ON
  update_current_asn(v->asn);
  len += tsch_security_secure_frame(buf, buf,
                                    len, data_len,
                                    &tsch_current_asn);
#endif

  printf("%s: len=%u, buf=", __func__, len);
  print_hex(buf, len);
  printf("\n");

  if(len != v->frame.len ||
     memcmp(buf, v->frame.buf, len) != 0) {
    return FAILURE;
  }

  return SUCCESS;
}

static result_t
test_parse_eack(const eack_test_vector_t *v)
{
  frame802154_t frame;
  struct ieee802154_ies ies;
  uint8_t hdr_len;
  int len;
#if TSCH_PACKET_EACK_WITH_SRC_ADDR
  linkaddr_t src_addr;
#endif
#if TSCH_PACKET_EACK_WITH_DEST_ADDR
  linkaddr_t dest_addr;
#endif

#if WITH_SECURITY_ON
  update_current_asn(v->asn);
#endif

  memset(&frame, 0, sizeof(frame));
  memset(&ies, 0, sizeof(ies));
  hdr_len = 0;

  linkaddr_copy(&linkaddr_node_addr, &v->dest);
  len = tsch_packet_parse_eack(v->frame.buf, v->frame.len, v->seqno,
                               &frame, &ies, &hdr_len);
  printf("%s: len=%u, seqno=%u, drift=%u, nack=%u\n",
         __func__, len, frame.seq, ies.ie_time_correction, ies.ie_is_nack);

#if WITH_SECURITY_ON
  /* adjust 'len' with the length of MIC which is included in a raw frame */
  len += tsch_security_mic_len(&frame);
#endif

  if(frame.fcf.frame_type != FRAME802154_ACKFRAME ||
     frame.fcf.frame_version != FRAME802154_IEEE802154E_2012) {
    return FAILURE;
  }

  if(len != v->frame.len ||
     frame.seq != v->seqno ||
     ies.ie_time_correction != v->drift ||
     ies.ie_is_nack != v->nack) {
    return FAILURE;
  }

#if TSCH_PACKET_EACK_WITH_SRC_ADDR
  if(frame802154_extract_linkaddr(&frame, &src_addr, NULL) == 0||
     linkaddr_cmp(&src_addr, &v->src) == 0) {
    return FAILURE;
  }
#endif

#if TSCH_PACKET_EACK_WITH_DEST_ADDR
  if(frame802154_extract_linkaddr(&frame, NULL, &dest_addr) == 0||
     linkaddr_cmp(&dest_addr, &v->dest) == 0) {
    return FAILURE;
  }
#endif

  return SUCCESS;
}

PROCESS_THREAD(test_process, ev, data)
{
  static struct etimer et;
  const eb_test_vector_t *eb_v;
  const eack_test_vector_t *eack_v;

  PROCESS_BEGIN();

  tsch_set_coordinator(1);

#if WITH_SECURITY_ON
  tsch_set_pan_secured(1);
#endif

  etimer_set(&et, CLOCK_SECOND);

  /* wait for minimal schedule installed */
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    if(tsch_schedule_get_link_by_handle(0) != NULL) {
      break;
    }
    etimer_reset(&et);
  }

  eb_v = &eb_test_vectors[TEST_CONFIG_TYPE];
  printf("==check-me== %s\n",
         test_create_eb(eb_v) == SUCCESS ? "SUCCEEDED" : "FAILED");
  printf("==check-me== %s\n",
         test_parse_eb(eb_v) == SUCCESS ? "SUCCEEDED" : "FAILED");

  eack_v = &eack_test_vectors[TEST_CONFIG_TYPE];
  printf("==check-me== %s\n",
         test_create_eack(eack_v) == SUCCESS ? "SUCCEEDED" : "FAILED");
  printf("==check-me== %s\n",
         test_parse_eack(eack_v) == SUCCESS ? "SUCCEEDED" : "FAILED");

  printf("==check-me== DONE\n");

  PROCESS_END();
}