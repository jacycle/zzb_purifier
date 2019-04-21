/*********************************************************************************************************
**creater    : pengjq
**data       : 2015-6-24
**description: the function of the message queue
**
**------------------------------------------------------------------------------------------------------
** Modified by:
** version    :
** date       :
** description:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/

#include "integer.h"
#include "queue.h"

UINT queue_create(QUEUE *queue_ptr, UINT message_size, 
                        void *queue_start, UINT queue_size)
{
    UINT  capacity;

    if (!queue_ptr || !queue_start)
    {
        /* Queue pointer is invalid, return appropriate error code.  */
        return (QUEUE_PTR_ERROR);
    }

    /* Save the message size in the control block.  */
    queue_ptr -> message_size =  message_size;
    
    /* Determine how many messages will fit in the queue area and the number
       of ULONGs used.  */
    capacity =  queue_size / message_size;


    /* Save the starting address and calculate the ending address of 
       the queue.  Note that the ending address is really one past the
       end!  */
    queue_ptr -> start =  (CHAR_PTR) queue_start;
    queue_ptr -> end =    ((CHAR_PTR) queue_start) + queue_size;

    /* Set the read and write pointers to the beginning of the queue
       area.  */
    queue_ptr -> read =   (CHAR_PTR) queue_start;
    queue_ptr -> write =  (CHAR_PTR) queue_start;

    /* Setup the number of enqueued messages and the number of message
       slots available in the queue.  */
    queue_ptr -> enqueued =           0;
    queue_ptr -> available_storage =  capacity;
    queue_ptr -> capacity =           capacity;

    /* Return SUCCESS.  */
    return(QUEUE_SUCCESS);
}

UINT queue_receive(QUEUE *queue_ptr, void *destination_ptr)
{
    UINT        i;
    UINT        status;                 /* Return status           */
    CHAR_PTR    source;                 /* Source pointer          */
    CHAR_PTR    destination;            /* Destination pointer     */
    
    /* Queue pointer is invalid, return appropriate error code.  */
    if (!queue_ptr || !destination_ptr)
        return(QUEUE_PTR_ERROR);

    /* Determine if there is anything in the queue.  */
    if (queue_ptr -> enqueued)
    {
        /* Setup source and destination pointers.  */
        source =  (CHAR_PTR) queue_ptr -> read;
        destination =  (CHAR_PTR) destination_ptr;

        /* Yes, there is something in the queue.  Place the oldest message in the 
           queue into the thread's area.  */
        for (i=0; i<queue_ptr->message_size; i++)
        {
            /* Copy an eight longword message from the queue.  */
            *destination++ =  *source++;
        }

        /* Adjust the read pointer.  */
        queue_ptr -> read =  
                    queue_ptr -> read + queue_ptr -> message_size;

        /* Determine if we are at the end.  */
        if (queue_ptr -> read >= queue_ptr -> end)

            /* Yes, wrap around to the beginning.  */
            queue_ptr -> read =  queue_ptr -> start;

        /* Increase the amount of available storage.  */
        queue_ptr -> available_storage++;

        /* Decrease the enqueued count.  */
        queue_ptr -> enqueued--;
        
        /* Successful service.  */
        status =  QUEUE_SUCCESS;
    }
    else
        status = QUEUE_EMPTY;

    /* Return completion status.  */
    return(status);
}

UINT queue_send(QUEUE *queue_ptr, void *source_ptr)
{
    UINT        i;
    UINT        status;                 /* Return status           */
    CHAR_PTR    source;                 /* Source pointer          */
    CHAR_PTR    destination;            /* Destination pointer     */
   
    /* Queue pointer is invalid, return appropriate error code.  */
    if (!queue_ptr || !source_ptr)
        return(QUEUE_PTR_ERROR);

    /* Determine if there is room in the queue.  */
    if (queue_ptr -> available_storage)
    {
        /* Now determine if there is a thread waiting for a message.  */
        // if (!queue_ptr -> suspension_list)
        {
            /* Simply place the message in the queue.  */
            
            /* Reduce the amount of available storage.  */
            queue_ptr -> available_storage--;

            /* Increase the enqueued count.  */
            queue_ptr -> enqueued++;

            /* Setup source and destination pointers.  */
            source =  (CHAR_PTR) source_ptr;
            destination =  (CHAR_PTR) queue_ptr -> write;

            /* Copy the message into the queue.  */
            for (i=0; i<queue_ptr -> message_size; i++)
            {
                /* Copy an eight longword message from the queue.  */
                *destination++ =  *source++;
            }

            /* Adjust the write pointer.  */
            queue_ptr -> write =  
                    queue_ptr -> write + queue_ptr -> message_size;

            /* Determine if we are at the end.  */
            if (queue_ptr -> write >= queue_ptr -> end)

                /* Yes, wrap around to the beginning.  */
                queue_ptr -> write =  queue_ptr -> start;

            /* Set status to success.  */
            status =  QUEUE_SUCCESS;
        }
    }
    else
    {
        /* Immediate return, return error completion.  */
        status =  QUEUE_FULL;
    }

    /* Return completion status.  */
    return(status);
}

UINT queue_flush(QUEUE *queue_ptr)
{
    /* Queue pointer is invalid, return appropriate error code.  */
    if (!queue_ptr)
        return(QUEUE_PTR_ERROR);
        
    /* Determine if there is something on the queue.  */
    if (queue_ptr -> enqueued)
    {
        /* Yes, there is something in the queue.  */

        /* Reset the queue parameters to erase all of the queued messages.  */
        queue_ptr -> enqueued =           0;
        queue_ptr -> available_storage =  queue_ptr -> capacity;
        queue_ptr -> read =               queue_ptr -> start;
        queue_ptr -> write =              queue_ptr -> start;
    }

    /* Return TX_SUCCESS.  */
    return(QUEUE_SUCCESS);
}

UINT queue_get(QUEUE *queue_ptr, void *destination_ptr)
{
    UINT        i;
    UINT        status;                 /* Return status           */
    CHAR_PTR    source;                 /* Source pointer          */
    CHAR_PTR    destination;            /* Destination pointer     */
    
    /* Queue pointer is invalid, return appropriate error code.  */
    if (!queue_ptr || !destination_ptr)
        return(QUEUE_PTR_ERROR);

    /* Determine if there is anything in the queue.  */
    if (queue_ptr -> enqueued)
    {
        /* Setup source and destination pointers.  */
        source =  (CHAR_PTR) queue_ptr -> read;
        destination =  (CHAR_PTR) destination_ptr;

        /* Yes, there is something in the queue.  Place the oldest message in the 
           queue into the thread's area.  */
        for (i=0; i<queue_ptr->message_size; i++)
        {
            /* Copy an eight longword message from the queue.  */
            *destination++ =  *source++;
        }
        
        /* Successful service.  */
        status =  QUEUE_SUCCESS;
    }
    else
        status = QUEUE_EMPTY;

    /* Return completion status.  */
    return(status);
}

UINT queue_remove(QUEUE *queue_ptr)
{
    UINT        status;                 /* Return status           */
    
    /* Queue pointer is invalid, return appropriate error code.  */
    if (!queue_ptr)
        return(QUEUE_PTR_ERROR);

    /* Determine if there is anything in the queue.  */
    if (queue_ptr -> enqueued)
    {
        /* Adjust the read pointer.  */
        queue_ptr -> read =  
                    queue_ptr -> read + queue_ptr -> message_size;

        /* Determine if we are at the end.  */
        if (queue_ptr -> read >= queue_ptr -> end)

            /* Yes, wrap around to the beginning.  */
            queue_ptr -> read =  queue_ptr -> start;

        /* Increase the amount of available storage.  */
        queue_ptr -> available_storage++;

        /* Decrease the enqueued count.  */
        queue_ptr -> enqueued--;
        
        /* Successful service.  */
        status =  QUEUE_SUCCESS;
    }
    else
        status = QUEUE_EMPTY;

    /* Return completion status.  */
    return(status);
}

UINT queue_empty(QUEUE *queue_ptr)
{
    if (!queue_ptr)
        return(QUEUE_PTR_ERROR);
    
        /* Determine if there is anything in the queue.  */
    if (queue_ptr -> enqueued)
    {
        return QUEUE_SUCCESS;
    }
    else
    {
        return QUEUE_EMPTY;
    }
}

UINT queue_full(QUEUE *queue_ptr)
{
    if (!queue_ptr)
        return(QUEUE_PTR_ERROR);
      
    if (queue_ptr -> available_storage)
    {
        return QUEUE_SUCCESS;
    }
    else
    {
        return QUEUE_FULL;
    }
}
