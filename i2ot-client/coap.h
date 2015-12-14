#ifndef COAP_H
#define COAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define COAP_PORT 5683

#define MAXOPT 8

typedef struct
{
        uint8_t version;   // version number
        uint8_t type;      // message type
        uint8_t tkllen;    // token length
        uint8_t code;      // status code
        uint8_t mid[2];    // message ID
} coap_header_t;


typedef struct
{
        const uint8_t *p;      // byte array that holds some data, immutable
              size_t   len;    // length of the array
} coap_buffer_t;


typedef struct
{
        uint8_t *p;     // byte array that holds some data, mutable
        size_t   len;   // length of the array
} coap_rw_buffer_t;


typedef struct
{
        coap_buffer_t val;   // option value
        uint8_t       num;   // option number
} coap_option_t;


typedef struct
{
        coap_header_t header;         // header of the packet
        coap_buffer_t token;          // token value, size as specified by header.tkllen
        uint8_t       numopts;        // number of options
        coap_option_t opts[MAXOPT];   // options of the packet
        coap_buffer_t payload;        // payload carried by the packet
} coap_packet_t;


typedef enum
{
        COAP_OPTION_IF_MATCH       = 1,
        COAP_OPTION_URI_HOST       = 3,
        COAP_OPTION_ETAG           = 4,
        COAP_OPTION_IF_NONE_MATCH  = 5,
        COAP_OPTION_OBSERVE        = 6,
        COAP_OPTION_URI_PORT       = 7,
        COAP_OPTION_LOCATION_PATH  = 8,
        COAP_OPTION_URI_PATH       = 11,
        COAP_OPTION_CONTENT_FORMAT = 12,
        COAP_OPTION_MAX_AGE        = 14,
        COAP_OPTION_URI_QUERY      = 15,
        COAP_OPTION_ACCEPT         = 17,
        COAP_OPTION_LOCATION_QUERY = 20,
        COAP_OPTION_PROXY_URI      = 35,
        COAP_OPTION_PROXY_SCHEME   = 39
} coap_option_num_t;


typedef enum
{
        COAP_METHOD_GET    = 1,
        COAP_METHOD_POST   = 2,
        COAP_METHOD_PUT    = 3,
        COAP_METHOD_DELETE = 4
} coap_method_t;


typedef enum
{
        COAP_TYPE_CON    = 0,   // confirmable message
        COAP_TYPE_NONCON = 1,   // non-confirmable message
        COAP_TYPE_ACK    = 2,   // acknowledgement for a received message
        COAP_TYPE_RESET  = 3    // reset
} coap_msgtype_t;


#define MAKE_RSPCODE(clas, det) ((clas << 5) | (det))

typedef enum
{
        COAP_RSPCODE_EMPTY_MSG             = MAKE_RSPCODE(0, 0),
        COAP_RSPCODE_CREATED               = MAKE_RSPCODE(2, 1),
        COAP_RSPCODE_DELETED               = MAKE_RSPCODE(2, 2),
        COAP_RSPCODE_VALID                 = MAKE_RSPCODE(2, 3),
        COAP_RSPCODE_CHANGED               = MAKE_RSPCODE(2, 4),
        COAP_RSPCODE_CONTENT               = MAKE_RSPCODE(2, 5),
        COAP_RSPCODE_BAD_REQUEST           = MAKE_RSPCODE(4, 0),
        COAP_RSPCODE_UNAUTHORIZED          = MAKE_RSPCODE(4, 1),
        COAP_RSPCODE_BAD_OPTION            = MAKE_RSPCODE(4, 2),
        COAP_RSPCODE_FORBIDDEN             = MAKE_RSPCODE(4, 3),
        COAP_RSPCODE_NOT_FOUND             = MAKE_RSPCODE(4, 4),
        COAP_RSPCODE_METHOD_NOT_ALLOWED    = MAKE_RSPCODE(4, 5),
        COAP_RSPCODE_NOT_ACCEPTABLE        = MAKE_RSPCODE(4, 6),
        COAP_RSPCODE_INTERNAL_SERVER_ERROR = MAKE_RSPCODE(5, 0),
        COAP_RSPCODE_NOT_IMPLEMENTED       = MAKE_RSPCODE(5, 1),
        COAP_RSPCODE_SERVICE_UNAVAILABLE   = MAKE_RSPCODE(5, 3)
} coap_responsecode_t;


typedef enum
{
        COAP_CONTENTTYPE_NONE                   = -1,   // bodge to allow us not to send option block
        COAP_CONTENTTYPE_TEXT_PLAIN             =  0,
        COAP_CONTENTTYPE_APPLICATION_LINKFORMAT = 40
} coap_content_type_t;


typedef enum
{
        COAP_ERR_NONE                        = 0,
        COAP_ERR_HEADER_TOO_SHORT            = 1,
        COAP_ERR_VERSION_NOT_1               = 2,
        COAP_ERR_TOKEN_TOO_SHORT             = 3,
        COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER = 4,
        COAP_ERR_OPTION_TOO_SHORT            = 5,
        COAP_ERR_OPTION_OVERRUNS_PACKET      = 6,
        COAP_ERR_OPTION_TOO_BIG              = 7,
        COAP_ERR_OPTION_LEN_INVALID          = 8,
        COAP_ERR_BUFFER_TOO_SMALL            = 9,
        COAP_ERR_UNSUPPORTED                 = 10,
        COAP_ERR_OPTION_DELTA_INVALID        = 11
} coap_error_t;


typedef int (*coap_endpoint_func)(      coap_rw_buffer_t *scratch,
                                  const coap_packet_t    *inpkt,
                                        coap_packet_t    *outpkt,
                                        uint8_t           id_hi,
                                        uint8_t           id_lo);


#define MAX_SEGMENTS 8   // e.g. 2 = /foo/bar, 3 = /foo/bar/baz

typedef struct
{
              int   count;
        const char *elems[MAX_SEGMENTS];
} coap_endpoint_path_t;


typedef struct
{
              coap_method_t         method;      // (i.e. POST, PUT or GET)
              coap_endpoint_func    handler;     // callback function which handles this type of endpoint (and calls coap_make_response() at some point)
        const coap_endpoint_path_t *path;        // path towards a resource (i.e. foo/bar/)
        const char                 *core_attr;   /* the 'ct' attribute, as defined in RFC7252, section 7.2.1.:
                                                  * "The Content-Format code "ct" attribute 
                                                  * provides a hint about the 
                                                  * Content-Formats this resource returns." 
                                                  * (Section 12.3. lists possible ct values.) */
} coap_endpoint_t;



//////////////////////////////////////////////////////////////////////
//////////               FUNCTION DEFINITIONS               //////////
//////////////////////////////////////////////////////////////////////


/**
 * TODO document me
 */
void coap_dump_buffer(const uint8_t *buf,
                            size_t   buflen,
                            bool     bare);


/**
 * TODO document me
 */
void coap_dump_header(coap_header_t *header);


/**
 * TODO document me
 */
void coap_dump_options(coap_option_t *opts,
                       size_t         numopt);


/**
 * TODO document me
 */
void coap_dump_packet(coap_packet_t *pkt);


/**
 * TODO document me
 */
int coap_parse(       coap_packet_t *pkt,
               const  uint8_t       *buf,
                      size_t         buflen);


/**
 * Converts the data in <code>buf</code> into a null-terminated C string and
 * copies the result to <code>strbuf</code>.
 * 
 * @return 0 on success, or COAP_ERR_BUFFER_TOO_SMALL if <code>strbuflen</code>
 * is smaller than the size of <code>buf</code>
 */
int coap_buffer_to_string(      char          *strbuf,
                                size_t         strbuflen,
                          const coap_buffer_t *buf);


/**
 * TODO document me
 */
const coap_option_t *coap_find_options(const coap_packet_t *pkt,
                                             uint8_t        num,
                                             uint8_t       *count);


/**
 * Creates a CoAP message from the data in <code>pkt</code> and copies the
 * result in <code>buf</code>. The actual size of the whole message (which
 * may be smaller than the size of the buffer) will be written to
 * <code>buflen</code>. You should use that value (and not <code>buflen</code>)
 * when you send the message.
 * 
 * @return 0 on success, or COAP_ERR_BUFFER_TOO_SMALL if the size of
 * <code>buf</code> is not sufficient, or COAP_ERR_UNSUPPORTED if
 * the token length specified in the header does not match the
 * token length specified in the buffer that actually holds the
 * tokens
 */
int coap_build(      uint8_t       *buf,
                     size_t        *buflen,
               const coap_packet_t *pkt);


/**
 * TODO document me
 */
int coap_make_ack(      coap_packet_t    *pkt,
                        uint8_t           msgid_hi,
                        uint8_t           msgid_lo,
                  const coap_buffer_t    *tok);


/**
 * TODO document me
 */
int coap_make_response(      coap_rw_buffer_t    *scratch,
                             coap_packet_t       *pkt,
                       const uint8_t             *content,
                             size_t               content_len,
                             uint8_t              msgid_hi,
                             uint8_t              msgid_lo,
                       const coap_buffer_t       *tok,
                             coap_responsecode_t  rspcode,
                             coap_content_type_t  content_type,
                             bool                 confirmable);


/**
 * TODO document me
 */
int coap_make_pb_response(      coap_rw_buffer_t    *scratch,
                                coap_packet_t       *pkt,
                          const uint8_t             *content,
                                size_t               content_len,
                                uint8_t              msgid_hi,
                                uint8_t              msgid_lo,
                          const coap_buffer_t       *tok,
                                coap_responsecode_t  rspcode,
                                coap_content_type_t  content_type);


/**
  * TODO document me
  */
int coap_handle_req(      coap_rw_buffer_t *scratch,
                    const coap_packet_t    *inpkt,
                          coap_packet_t    *outpkt,
                          bool              pb,
                          bool              con);


#ifdef __cplusplus
}
#endif

#endif   // #ifndef COAP_H
