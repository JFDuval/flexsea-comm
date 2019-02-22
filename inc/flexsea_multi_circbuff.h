/*
 * flexsea_comm_multi.h
 *
 *  Created on: August 14, 2018
 *      Author: Dephy Inc
 */

#ifndef FLEXSEA_COMM_INC_FLEXSEA_MULTI_CIRCBUFF_H_
#define FLEXSEA_COMM_INC_FLEXSEA_MULTI_CIRCBUFF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define HEADER  						0xED	//237d
#define FOOTER  						0xEE	//238d
#define ESCAPE  						0xE9	//233d
#define COMM_STR_BUF_LEN				48
typedef enum state {NoMessage = 0, Head, MultiByteInfo, Reading, Escaped, End} State;

struct MultiWrapper_struct;
typedef struct MultiWrapper_struct MultiWrapper;

struct circularBuffer;
typedef struct circularBuffer circularBuffer_t;

uint16_t unpack_multi_payload_cb(circularBuffer_t *cb, MultiWrapper* p);
uint16_t unpack_multi_payload_cb_cached(circularBuffer_t *cb, MultiWrapper* p, int *cacheStart);

#ifdef __cplusplus
}
#endif



#endif /* FLEXSEA_COMM_INC_FLEXSEA_MULTI_CIRCBUFF_H_ */
