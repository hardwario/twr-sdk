#include <bc_fifo.h>
#include <bc_irq.h>

void bc_fifo_init(bc_fifo_t *fifo, void *buffer, size_t size)
{
    fifo->buffer = buffer;
    fifo->size = size;
    fifo->head = 0;
    fifo->tail = 0;
}

void bc_fifo_purge(bc_fifo_t *fifo)
{
    fifo->head = 0;
    fifo->tail = 0;
}

size_t bc_fifo_write(bc_fifo_t *fifo, const void *buffer, size_t length)
{
    // Disable interrupts
    bc_irq_disable();

    // For each byte in buffer...
    for (size_t i = 0; i < length; i++)
    {
        if ((fifo->head + 1) == fifo->tail)
        {
            // Enable interrupts
            bc_irq_enable();

            // Return number of bytes written
            return i;
        }

        if (((fifo->head + 1) == fifo->size) && (fifo->tail == 0))
        {
            // Enable interrupts
            bc_irq_enable();

            // Return number of bytes written
            return i;
        }

        *((uint8_t *) fifo->buffer + fifo->head) = *(uint8_t *) buffer;

        buffer = (uint8_t *) buffer + 1;

        fifo->head++;

        if (fifo->head == fifo->size)
        {
            fifo->head = 0;
        }
    }

    // Enable interrupts
    bc_irq_enable();

    // Return number of bytes written
    return length;
}

size_t bc_fifo_read(bc_fifo_t *fifo, void *buffer, size_t length)
{
    // Disable interrupts
    bc_irq_disable();

    // For desired number of bytes...
    for (size_t i = 0; i < length; i++)
    {
        if (fifo->tail != fifo->head)
        {
            *(uint8_t *) buffer = *((uint8_t *) fifo->buffer + fifo->tail);

            buffer = (uint8_t *) buffer + 1;

            fifo->tail++;

            if (fifo->tail == fifo->size)
            {
                fifo->tail = 0;
            }
        }
        else
        {
            // Enable interrupts
            bc_irq_enable();

            // Return number of bytes read
            return i;
        }
    }

    // Enable interrupts
    bc_irq_enable();

    // Return number of bytes read
    return length;
}

size_t bc_fifo_irq_write(bc_fifo_t *fifo, const void *buffer, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        if ((fifo->head + 1) == fifo->tail)
        {
            // Return number of bytes written
            return i;
        }

        if (((fifo->head + 1) == fifo->size) && (fifo->tail == 0))
        {
            // Return number of bytes written
            return i;
        }

        *((uint8_t *) fifo->buffer + fifo->head) = *(uint8_t *) buffer;

        buffer = (uint8_t *) buffer + 1;

        fifo->head++;

        if (fifo->head == fifo->size)
        {
            fifo->head = 0;
        }
    }

    // Return number of bytes written
    return length;
}

size_t bc_fifo_irq_read(bc_fifo_t *fifo, void *buffer, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        if (fifo->tail != fifo->head)
        {
            *(uint8_t *) buffer = *((uint8_t *) fifo->buffer + fifo->tail);

            buffer = (uint8_t *) buffer + 1;

            fifo->tail++;

            if (fifo->tail == fifo->size)
            {
                fifo->tail = 0;
            }
        }
        else
        {
            // Return number of bytes read
            return i;
        }
    }

    // Return number of bytes read
    return length;
}

bool bc_fifo_is_empty(bc_fifo_t *fifo)
{
    bc_irq_disable();

	bool result = fifo->tail == fifo->head;

	bc_irq_enable();

	return result;
}
