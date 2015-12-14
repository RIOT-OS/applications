#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include "coap.h"

// uncomment the line below if you want the endpoints array to be dynamically
// allocated instead of statically (or do it in the Makefile)
// #define ENDPOINT_DYNAMIC

#ifndef ENDPOINT_DYNAMIC
extern const coap_endpoint_t endpoints[];
#else
extern coap_endpoint_t *endpoints;
#endif


#ifdef DEBUG
void coap_dump_header(coap_header_t *header)
{
        printf("Header:\n");
        printf("  version  0x%02X\n",     header->version);
        printf("  type     0x%02X\n",     header->type);
        printf("  tkllen   0x%02X\n",     header->tkllen);
        printf("  code     0x%02X\n",     header->code);
        printf("  mid      0x%02X%02X\n", header->mid[0], header->mid[1]);
}
#endif


#ifdef DEBUG
void coap_dump_buffer(const uint8_t *buf, size_t buflen, bool bare)
{
        if (bare) {
                while (buflen--) {
                        printf("%02X%s", *buf++, (buflen > 0) ? " " : "");
                }
        }
        else {
                printf("Dump: ");

                while (buflen--) {
                        printf("%02X%s", *buf++, (buflen > 0) ? " " : "");
                }

                printf("\n");
        }
}
#endif


#ifdef DEBUG
void coap_dump_options(coap_option_t *opts, size_t numopt)
{
        size_t i;
        printf(" Options:\n");

        for (i = 0; i < numopt; i++) {
                printf("  0x%02X [ ", opts[i].num);
                coap_dump_buffer(opts[i].val.p, opts[i].val.len, true);
                printf(" ]\n");
        }
}
#endif


#ifdef DEBUG
void coap_dump_packet(coap_packet_t *pkt)
{
        coap_dump_header(&pkt->header);
        
        coap_dump_options(pkt->opts, pkt->numopts);
        
        printf("Payload: ");
        
        coap_dump_buffer(pkt->payload.p, pkt->payload.len, true);
        
        printf("\n");
}
#endif


static int coap_parseHeader(coap_header_t *hdr, const uint8_t *buf, size_t buflen)
{
        if (buflen < 4) {
                return COAP_ERR_HEADER_TOO_SHORT;
        }

        hdr->version = (buf[0] & 0xC0) >> 6;

        if (hdr->version != 1) {
                return COAP_ERR_VERSION_NOT_1;
        }

        hdr->type   = (buf[0] & 0x30) >> 4;
        hdr->tkllen =  buf[0] & 0x0F;
        hdr->code   =  buf[1];
        hdr->mid[0] =  buf[2];
        hdr->mid[1] =  buf[3];

        return 0;
}


static int coap_parseToken(      coap_buffer_t *tokbuf, const coap_header_t *hdr,
                           const uint8_t       *buf,          size_t         buflen)
{
        if (hdr->tkllen == 0) {
                tokbuf->p = NULL;
                tokbuf->len = 0;
                return 0;
        }
        else if (hdr->tkllen <= 8) {
                if (4U + hdr->tkllen > buflen) {
                        return COAP_ERR_TOKEN_TOO_SHORT;   // tok bigger than packet
                }

                tokbuf->p = buf + 4;   // past header
                tokbuf->len = hdr->tkllen;
                return 0;
        }
        else {
                // invalid size
                return COAP_ERR_TOKEN_TOO_SHORT;
        }
}


// advances p
static int coap_parseOption(coap_option_t *option, uint16_t *running_delta, const uint8_t **buf, size_t buflen)
{
        const uint8_t  *p       = *buf;
              uint8_t   headlen =  1;
              uint16_t  len;
              uint16_t  delta;

        if (buflen < headlen) { // too small
                return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
        }

        delta = (p[0] & 0xF0) >> 4;
        len = p[0] & 0x0F;

        // These are untested and may be buggy
        if (delta == 13) {
                headlen++;

                if (buflen < headlen) {
                        return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
                }

                delta = p[1] + 13;
                p++;
        }
        else if (delta == 14) {
                headlen += 2;

                if (buflen < headlen) {
                        return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
                }

                delta = ((p[1] << 8) | p[2]) + 269;
                p += 2;
        }
        else if (delta == 15) {
                return COAP_ERR_OPTION_DELTA_INVALID;
        }

        if (len == 13) {
                headlen++;

                if (buflen < headlen) {
                        return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
                }

                len = p[1] + 13;
                p++;
        }
        else if (len == 14) {
                headlen += 2;

                if (buflen < headlen) {
                        return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
                }

                len = ((p[1] << 8) | p[2]) + 269;
                p += 2;
        }
        else if (len == 15) {
                return COAP_ERR_OPTION_LEN_INVALID;
        }

        if ((p + 1 + len) > (*buf + buflen)) {
                return COAP_ERR_OPTION_TOO_BIG;
        }

        //printf("option num=%d\n", delta + *running_delta);
        option->num = delta + *running_delta;
        option->val.p = p + 1;
        option->val.len = len;
        //coap_dump(p+1, len, false);

        // advance buf
        *buf = p + 1 + len;
        *running_delta += delta;

        return 0;
}


// http://tools.ietf.org/html/rfc7252#section-3.1
static int coap_parseOptionsAndPayload(coap_option_t *options, uint8_t *numOptions, coap_buffer_t *payload,
                                       const coap_header_t *hdr, const uint8_t *buf, size_t buflen)
{
        const uint8_t  *p           = buf + 4 + hdr->tkllen;
        const uint8_t  *end         = buf + buflen;
              size_t    optionIndex = 0;
              uint16_t  delta       = 0;
              int       rc;

        if (p > end) {
                return COAP_ERR_OPTION_OVERRUNS_PACKET;        // out of bounds
        }

        //coap_dump(p, end - p);

        // 0xFF is payload marker
        while ((optionIndex < *numOptions) && (p < end) && (*p != 0xFF)) {
                if (0 != (rc = coap_parseOption(&options[optionIndex], &delta, &p, end - p))) {
                        return rc;
                }

                optionIndex++;
        }

        *numOptions = optionIndex;

        if (p + 1 < end && *p == 0xFF) { // payload marker
                payload->p = p + 1;
                payload->len = end - (p + 1);
        }
        else {
                payload->p = NULL;
                payload->len = 0;
        }

        return 0;
}


int coap_parse(coap_packet_t *pkt, const uint8_t *buf, size_t buflen)
{
        int rc;

        // coap_dump(buf, buflen, false);

        if (0 != (rc = coap_parseHeader(&pkt->header, buf, buflen))) {
                return rc;
        }

        //    coap_dumpHeader(&hdr);
        if (0 != (rc = coap_parseToken(&pkt->token, &pkt->header, buf, buflen))) {
                return rc;
        }

        pkt->numopts = MAXOPT;

        if (0 != (rc = coap_parseOptionsAndPayload(pkt->opts, &(pkt->numopts),
                                                   &(pkt->payload), &pkt->header, buf, buflen))) {
                return rc;
        }

        //    coap_dumpOptions(opts, numopt);
        return 0;
}


// options are always stored consecutively, so can return a block with same option num
const coap_option_t *coap_find_options(const coap_packet_t *pkt, uint8_t num, uint8_t *count)
{
        // FIXME, options is always sorted, can find faster than this
        size_t i;
        const coap_option_t *first = NULL;
        *count = 0;

        for (i = 0; i < pkt->numopts; i++) {
                if (pkt->opts[i].num == num) {
                        if (NULL == first) {
                                first = &pkt->opts[i];
                        }

                        (*count)++;
                }
                else {
                        if (NULL != first) {
                                break;
                        }
                }
        }

        return first;
}


int coap_buffer_to_string(char *strbuf, size_t strbuflen, const coap_buffer_t *buf)
{
        if (buf->len + 1 > strbuflen) {
                return COAP_ERR_BUFFER_TOO_SMALL;
        }

        memcpy(strbuf, buf->p, buf->len);
        strbuf[buf->len] = 0;
        return 0;
}


static void coap_option_nibble(uint32_t value, uint8_t *nibble)
{
        if (value < 13) {
                *nibble = (0xFF & value);
        }
        else if (value <= 0xFF + 13) {
                *nibble = 13;
        }
        else if (value <= 0xFFFF + 269) {
                *nibble = 14;
        }
}


int coap_build(uint8_t *buf, size_t *buflen, const coap_packet_t *pkt)
{
        size_t    opts_len      = 0;
        size_t    i;
        uint8_t  *p;
        uint16_t  running_delta = 0;

        // build header
        if (*buflen < (4U + pkt->header.tkllen)) {
                return COAP_ERR_BUFFER_TOO_SMALL;
        }

        buf[0] = (pkt->header.version & 0x03) << 6;
        buf[0] |= (pkt->header.type & 0x03) << 4;
        buf[0] |= (pkt->header.tkllen & 0x0F);
        buf[1] = pkt->header.code;
        buf[2] = pkt->header.mid[0];
        buf[3] = pkt->header.mid[1];

        // inject token
        p = buf + 4;

        if ((pkt->header.tkllen > 0) && (pkt->header.tkllen != pkt->token.len)) {
                return COAP_ERR_UNSUPPORTED;
        }

        if (pkt->header.tkllen > 0) {
                memcpy(p, pkt->token.p, pkt->header.tkllen);
        }

        // inject options
        p += pkt->header.tkllen;

        for (i = 0; i < pkt->numopts; i++) {
                uint32_t optDelta;
                uint8_t  len;
                uint8_t  delta = 0;

                if (((size_t)(p - buf)) > *buflen) {
                        return COAP_ERR_BUFFER_TOO_SMALL;
                }

                optDelta = pkt->opts[i].num - running_delta;
                coap_option_nibble(optDelta, &delta);
                coap_option_nibble((uint32_t)pkt->opts[i].val.len, &len);

                *p++ = (0xFF & (delta << 4 | len));

                if (delta == 13) {
                        *p++ = (optDelta - 13);
                }
                else if (delta == 14) {
                        *p++ = ((optDelta - 269) >> 8);
                        *p++ = (0xFF & (optDelta - 269));
                }

                if (len == 13) {
                        *p++ = (pkt->opts[i].val.len - 13);
                }
                else if (len == 14) {
                        *p++ = (pkt->opts[i].val.len >> 8);
                        *p++ = (0xFF & (pkt->opts[i].val.len - 269));
                }

                memcpy(p, pkt->opts[i].val.p, pkt->opts[i].val.len);
                p += pkt->opts[i].val.len;
                running_delta = pkt->opts[i].num;
        }

        opts_len = (p - buf) - 4;   // number of bytes used by options

        if (pkt->payload.len > 0) {
                if (*buflen < 4 + 1 + pkt->payload.len + opts_len) {
                        return COAP_ERR_BUFFER_TOO_SMALL;
                }

                buf[4 + opts_len] = 0xFF;  // payload marker
                memcpy(buf + 5 + opts_len, pkt->payload.p, pkt->payload.len);
                *buflen = opts_len + 5 + pkt->payload.len;
        }
        else {
                *buflen = opts_len + 4;
        }

        return 0;
}


int coap_make_ack(      coap_packet_t    *pkt,
                        uint8_t           msgid_hi,
                        uint8_t           msgid_lo,
                  const coap_buffer_t    *tok)
{
        pkt->header.version = 0x01;
        pkt->header.type    = COAP_TYPE_ACK;
        pkt->header.code    = COAP_RSPCODE_EMPTY_MSG;
        pkt->header.mid[0]  = msgid_hi;
        pkt->header.mid[1]  = msgid_lo;
        pkt->numopts        = 0;
        
        if (tok) {
                pkt->header.tkllen =  tok->len;
                pkt->token         = *tok;
        } else {
                pkt->header.tkllen = 0;
                pkt->token         = (coap_buffer_t){.p = NULL, .len = 0};
        }
        
        pkt->payload.p   = NULL;
        pkt->payload.len = 0;
        
        return 0;
}


// FIXME This function always sends the content format option, even when the
// response code is an error (e.g. 4.04 NOT FOUND)
int coap_make_response(      coap_rw_buffer_t    *scratch,
                             coap_packet_t       *pkt,
                       const uint8_t             *content,
                             size_t               content_len,
                             uint8_t              msgid_hi,
                             uint8_t              msgid_lo,
                       const coap_buffer_t       *tok,
                             coap_responsecode_t  rspcode,
                             coap_content_type_t  content_type,
                             bool                 confirmable)
{
        if (scratch->len < 2) {
                return COAP_ERR_BUFFER_TOO_SMALL;
        }
        
        pkt->header.version = 0x01;
        pkt->header.code    = rspcode;
        pkt->header.mid[0]  = msgid_hi;
        pkt->header.mid[1]  = msgid_lo;
        pkt->numopts        = 1;
        
        if (confirmable) {
                pkt->header.type = COAP_TYPE_CON;
        } else {
                pkt->header.type = COAP_TYPE_NONCON;
        }
        
        if (tok) {
                pkt->header.tkllen =  tok->len;
                pkt->token         = *tok;
        } else {
                pkt->header.tkllen = 0;
                pkt->token         = (coap_buffer_t){.p = NULL, .len = 0};
        }
        
        pkt->opts[0].num   = COAP_OPTION_CONTENT_FORMAT;
        pkt->opts[0].val.p = scratch->p;
        
        scratch->p[0] = ((uint16_t)content_type & 0xFF00) >> 8;
        scratch->p[1] = ((uint16_t)content_type & 0x00FF);
        
        pkt->opts[0].val.len = 2;
        
        pkt->payload.p   = content;
        pkt->payload.len = content_len;
        
        return 0;
}


// FIXME This function always sends the content format option, even when the response code
// is an error (e.g. 4.04 NOT FOUND)
int coap_make_pb_response(      coap_rw_buffer_t    *scratch,
                                coap_packet_t       *pkt,
                          const uint8_t             *content,
                                size_t               content_len,
                                uint8_t              msgid_hi,
                                uint8_t              msgid_lo,
                          const coap_buffer_t       *tok,
                                coap_responsecode_t  rspcode,
                                coap_content_type_t  content_type)
{
        if (scratch->len < 2) {
                return COAP_ERR_BUFFER_TOO_SMALL;
        }
        
        pkt->header.version = 0x01;
        pkt->header.type    = COAP_TYPE_ACK;
        pkt->header.code    = rspcode;
        pkt->header.mid[0]  = msgid_hi;
        pkt->header.mid[1]  = msgid_lo;
        pkt->numopts        = 1;

        // need token in response
        if (tok) {
                pkt->header.tkllen =  tok->len;
                pkt->token         = *tok;
        } else {
                pkt->header.tkllen = 0;
                pkt->token         = (coap_buffer_t){.p = NULL, .len = 0};
        }

        // safe because 1 < MAXOPT
        pkt->opts[0].num   = COAP_OPTION_CONTENT_FORMAT;
        pkt->opts[0].val.p = scratch->p;

        scratch->p[0] = ((uint16_t)content_type & 0xFF00) >> 8;
        scratch->p[1] = ((uint16_t)content_type & 0x00FF);
        
        pkt->opts[0].val.len = 2;
        
        pkt->payload.p   = content;
        pkt->payload.len = content_len;
        
        return 0;
}


int coap_handle_req(      coap_rw_buffer_t *scratch,
                    const coap_packet_t    *inpkt,
                          coap_packet_t    *outpkt,
                          bool              pb,
                          bool              con)
{
        const coap_endpoint_t *ep   = endpoints;
        const coap_option_t   *opt;
        
        uint8_t count;
        int     i;
                
        coap_responsecode_t rsp_code;
        
        if (ep->handler == NULL) {   // no handler exists at all, set state to 5.01
                rsp_code = COAP_RSPCODE_NOT_IMPLEMENTED;
        } else {
                rsp_code = COAP_RSPCODE_NOT_FOUND;
        }
        
        while (ep->handler != NULL) {
                opt = coap_find_options(inpkt, COAP_OPTION_URI_PATH, &count);
                
                if (opt != NULL) {
                        if (count != ep->path->count) {
                                goto next;
                        }
                        
                        for (i = 0; i < count; i++) {
                                if (opt[i].val.len != strlen(ep->path->elems[i])) {
                                        goto next;
                                }
                                
                                if (memcmp(ep->path->elems[i], opt[i].val.p, opt[i].val.len) != 0) {
                                        goto next;
                                }
                        }
                        
                        // URI in request matches an endpoint URI, now check if methods match
                        
                        if (inpkt->header.code != ep->method) {
                                rsp_code = COAP_RSPCODE_METHOD_NOT_ALLOWED;
                                goto next;
                        }
                        
                        // valid request, now call handler
                                                
                        return ep->handler(scratch, inpkt, outpkt,
                                           inpkt->header.mid[0], inpkt->header.mid[1]);
                }
                
                next:
                
                ep++;
        }
        
        if (pb) {
                coap_make_pb_response(scratch, outpkt, NULL, 0, inpkt->header.mid[0],
                                      inpkt->header.mid[1], &inpkt->token, rsp_code,
                                      COAP_CONTENTTYPE_NONE);
        } else {
                coap_make_response(scratch, outpkt, NULL, 0, inpkt->header.mid[0],
                                   inpkt->header.mid[1], &inpkt->token, rsp_code,
                                   COAP_CONTENTTYPE_NONE, con);
        }
        
        return 0;
}
