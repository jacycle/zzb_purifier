#ifndef __QUEUE_H__
#define __QUEUE_H__

//#include "port.h"

/*****************************************************************************
 *  Global Macros & Definitions
 *****************************************************************************/
#define QUEUE_SUCCESS        0x00
#define QUEUE_PTR_ERROR      0x01
#define QUEUE_EMPTY          0x02
#define QUEUE_FULL           0x03
#define QUEUE_SIZE_ERROR     0x04

/*****************************************************************************
 *  Global Typedefs & Enums
 *****************************************************************************/
typedef struct QUEUE_STRUCT
{
    /* Define the message size that was specified in queue creation.  */
    UINT       message_size;

    /* Define the total number of messages in the queue.  */
    UINT       capacity;

    /* Define the current number of messages enqueue and the available
       queue storage space.  */
    UINT       enqueued;
    UINT       available_storage;

    /* Define pointers that represent the start and end for the queue's 
       message area.  */
    CHAR_PTR   start;
    CHAR_PTR   end;

    /* Define the queue read and write pointers.  Send requests use the write
       pointer while receive requests use the read pointer.  */
    CHAR_PTR   read;
    CHAR_PTR   write;
} QUEUE;

typedef QUEUE *QUEUE_PTR;


/*****************************************************************************
 *  Global Function Declarations
 *****************************************************************************/
UINT queue_create(QUEUE *queue_ptr, UINT message_size, 
                        void *queue_start, UINT queue_size);
UINT queue_receive(QUEUE *queue_ptr, void *destination_ptr);
UINT queue_send(QUEUE *queue_ptr, void *source_ptr);
UINT queue_get(QUEUE *queue_ptr, void *destination_ptr);
UINT queue_remove(QUEUE *queue_ptr);
UINT queue_flush(QUEUE *queue_ptr);
UINT queue_empty(QUEUE *queue_ptr);
UINT queue_full(QUEUE *queue_ptr);

#endif
