#include <twr_fifo.h>
#include <twr_irq.h>

void twr_fifo_init(twr_fifo_t *fifo, void *buffer, size_t size)
{
    fifo->buffer = buffer;
    fifo->size = size;
    fifo->head = 0;
    fifo->tail = 0;
}

void twr_fifo_purge(twr_fifo_t *fifo)
{
    fifo->head = 0;
    fifo->tail = 0;
}

size_t twr_fifo_write(twr_fifo_t *fifo, const void *buffer, size_t length)
{
    // Disable interrupts
    twr_irq_disable();

    // For each byte in buffer...
    for (size_t i = 0; i < length; i++)
    {
        if ((fifo->head + 1) == fifo->tail)
        {
            // Enable interrupts
            twr_irq_enable();

            // Return number of bytes written
            return i;
        }

        if (((fifo->head + 1) == fifo->size) && (fifo->tail == 0))
        {
            // Enable interrupts
            twr_irq_enable();

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
    twr_irq_enable();

    // Return number of bytes written
    return length;
}

size_t twr_fifo_read(twr_fifo_t *fifo, void *buffer, size_t length)
{
    // Disable interrupts
    twr_irq_disable();

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
            twr_irq_enable();

            // Return number of bytes read
            return i;
        }
    }

    // Enable interrupts
    twr_irq_enable();

    // Return number of bytes read
    return length;
}

size_t twr_fifo_irq_write(twr_fifo_t *fifo, const void *buffer, size_t length)
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

size_t twr_fifo_irq_read(twr_fifo_t *fifo, void *buffer, size_t length)
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

bool twr_fifo_is_empty(twr_fifo_t *fifo)
{
    twr_irq_disable();

	bool result = fifo->tail == fifo->head;

	twr_irq_enable();

	return result;
}
