/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-comm' Communication stack
	Copyright (C) 2016 Dephy, Inc. <http://dephy.com/>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************
	[Lead developper] Jean-Francois (JF) Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] flexsea_circular_buffer.c: simple circular buffer implementation
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-03-21 | dudds4 | Initial GPL-3.0 release
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <flexsea_circular_buffer.h>
#include <string.h>

void circ_buff_init(circularBuffer_t* cb)
{
	cb->head = -1;
	cb->tail = 0;
	cb->size= 0;

	int i = 0;
	uint8_t* b = cb->bytes;
	for(i=0; i<CB_BUF_LEN;i++)
		b[i]=0;
}

int circ_buff_write(circularBuffer_t* cb, uint8_t *writeFrom, uint16_t numBytes)
{
    const int MSG_BIGGER_THAN_BUFFER = 1;
    const int SUCCESS = 0;
    const int OVERWROTE = 2;

	if(numBytes > CB_BUF_LEN) { return MSG_BIGGER_THAN_BUFFER; }

	if(cb->tail + numBytes <= CB_BUF_LEN)
	{
		memcpy(cb->bytes + cb->tail, writeFrom, numBytes);
	}
	else
	{
		uint16_t bytesUntilEnd = CB_BUF_LEN - cb->tail;
		memcpy(cb->bytes + cb->tail, writeFrom, bytesUntilEnd);
		memcpy(cb->bytes, writeFrom + bytesUntilEnd, numBytes - bytesUntilEnd);
	}

	if(cb->head < 0) cb->head = 0;
	cb->tail = (cb->tail + numBytes) % CB_BUF_LEN;
	cb->size += numBytes;

	if(cb->size > CB_BUF_LEN)
	{
		//we've overwritten
		cb->size = CB_BUF_LEN;
		cb->head = cb->tail;
		return OVERWROTE;
	}
	return SUCCESS;
}

uint8_t circ_buff_peak(circularBuffer_t* cb, uint16_t offset)
{
    if(offset >= cb->size) return 0;
    return cb->bytes[((cb->head + offset) % CB_BUF_LEN)];
}

int32_t circ_buff_search(circularBuffer_t* cb, uint8_t value, uint16_t start)
{
    if(start > cb->size) return -1;
    int i = start;
    int index = cb->head + start;

    while(i < cb->size && index < CB_BUF_LEN)
    {
        if(cb->bytes[index] == value) return i;
        i++;
        index++;
    }

    index %= CB_BUF_LEN;

    while(i < cb->size)
    {
        if(cb->bytes[index] == value) return i;
        i++;
        index++;
    }

    return -1;
}

uint8_t circ_buff_checksum(circularBuffer_t* cb, uint16_t start, uint16_t end)
{
    if(start >= cb->size || end > cb->size) return 0;

    uint8_t checksum = 0;

    int i = (cb->head + start); //no modulo on purpose. (it would have no ultimate effect)
    int j = (cb->head + end) % CB_BUF_LEN;

    while(i < j && i < CB_BUF_LEN)
        checksum += cb->bytes[i++];

    i %= CB_BUF_LEN;

    while(i < j)
        checksum += cb->bytes[i++];

    return checksum;
}

int circ_buff_read(circularBuffer_t* cb, uint8_t* readInto, uint16_t numBytes)
{
    const int SUCCESS = 0;
    const int NOT_ENOUGH_BUFFERED = 1;

    if(numBytes > cb->size) { return NOT_ENOUGH_BUFFERED; }

    if(cb->head + numBytes < CB_BUF_LEN)
    {
        memcpy(readInto, cb->bytes + cb->head, numBytes);
    }
    else
    {
		uint16_t bytesUntilEnd = CB_BUF_LEN - cb->head;
        memcpy(readInto, cb->bytes + cb->head, bytesUntilEnd);
        memcpy(readInto + bytesUntilEnd, cb->bytes, numBytes - bytesUntilEnd);
    }

    return SUCCESS;
}

int circ_buff_read_section(circularBuffer_t* cb, uint8_t* readInto, uint16_t start, uint16_t end)
{
    const int SUCCESS = 0;
    const int INVALID_ARGS = 1;

    if(end > cb->size || start >= cb->size || start > end) { return INVALID_ARGS; }

    uint16_t numBytes = end - start;
    uint16_t startOffset = cb->head + start;
    if(cb->head + end < CB_BUF_LEN || startOffset >= CB_BUF_LEN)
    {
        memcpy(readInto, cb->bytes + (startOffset % CB_BUF_LEN), numBytes);
    }
    else
    {
		int bytesUntilEnd = CB_BUF_LEN - startOffset;
        memcpy(readInto, cb->bytes + cb->head, bytesUntilEnd);
        memcpy(readInto + bytesUntilEnd, cb->bytes, numBytes - bytesUntilEnd);
    }

    return SUCCESS;

    int i = 0;
    int h = cb->head + start; // no modulo on purpose, it would have same affect

    while(i < numBytes && h < CB_BUF_LEN)
        readInto[i++] = (cb->bytes)[h++];

    h %= CB_BUF_LEN;

    while(i < numBytes)
        readInto[i++] = (cb->bytes)[h++];

    return SUCCESS;
}



int circ_buff_move_head(circularBuffer_t* cb, uint16_t numBytes)
{
    const int SUCCESS = 0;
    const int MOVED_MORE_THAN_BUFFERED = 1;

    int originalNumBytes = numBytes;
    numBytes = numBytes < CB_BUF_LEN ? numBytes : CB_BUF_LEN;

	if(cb->head < 0) //buffer is already empty
		return (numBytes == originalNumBytes ? SUCCESS : MOVED_MORE_THAN_BUFFERED);

    //write zeros
	int h = cb->head;
    int i = 0;
    
    while(h < CB_BUF_LEN && i < numBytes)
    {
    	(cb->bytes)[h++] = 0;
    	i++;
    }
    
    h %= CB_BUF_LEN;
    
    while(i < numBytes)
    {
    	(cb->bytes)[h++] = 0;
    	i++;
    }
    
    cb->head = h;
    cb->size -= numBytes;
    if(cb->size < 1) //if the buffer is empty, we can reinitialize the buffer
    {
    	cb->head = -1;
    	cb->tail = 0;
    	cb->size = 0;
    }

    return (numBytes == originalNumBytes ? SUCCESS : MOVED_MORE_THAN_BUFFERED);
}

int circ_buff_get_size(circularBuffer_t* cb)  { return cb->size; }

#ifdef __cplusplus
}
#endif
