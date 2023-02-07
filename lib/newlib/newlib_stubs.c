#include <stddef.h>
#include <sys/stat.h>

extern int _end;

void *_sbrk(int incr)
{
    static unsigned char *heap = NULL;
    unsigned char *prev_heap;

    if (heap == NULL)
    {
        heap = (unsigned char *)&_end;
    }

    prev_heap = heap;

    heap += incr;

    return prev_heap;
}

int _close(int file)
{
    (void)file;

    return -1;
}

int _fstat(int file, struct stat *st)
{
    (void)file;

    st->st_mode = S_IFCHR;

    return 0;
}

int _isatty(int file)
{
    (void)file;

    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;

    return 0;
}

void _exit(int status)
{
    (void)status;

    __asm("BKPT #0");
}

void _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;

    return;
}

int _getpid(void)
{
    return -1;
}

int _write(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;

    return -1;
}

int _read(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;

    return -1;
}
