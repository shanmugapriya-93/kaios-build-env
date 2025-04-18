/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"
#include "ecrio_gz.h"
#include "EcrioPAL.h"
#ifndef _WIN32
#include <unistd.h>
#endif

#if defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#else
#  define LSEEK lseek
#endif

/* Local functions */
local void gz_reset OF((gz_statep));
#ifndef ECRIO_ZLIB
local gzFile gz_open OF((const char *, int, const char *));
#else
local gzFile gz_open OF((const char *, char**,unsigned int*));
#endif /* ECRIO_ZLIB */


#if defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
char ZLIB_INTERNAL *gz_strwinerror
(
     DWORD error
)
{
    static char buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        NULL);
    if (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        if (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        if (chars > sizeof (buf) - 1) {
            chars = sizeof (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    else {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    return buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local void gz_reset
(
    gz_statep state
)
{
    if (state->mode == GZ_READ) {   /* for reading ... */
        state->have = 0;            /* no output data available */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->direct = 1;          /* default for empty file */
    }
    state->seek = 0;                /* no seek request pending */
    gz_error(state, Z_OK, NULL);    /* clear error */
    state->pos = 0;                 /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
#ifndef ECRIO_ZLIB
local gzFile gz_open
(
    const char *path,
    int fd,
    const char *mode
)
{
    gz_statep state;

    /* allocate gzFile structure to return */
    state = malloc(sizeof(gz_state));
    if (state == NULL)
        return NULL;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->msg = NULL;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    while (*mode) {
        if (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        else
            switch (*mode) {
            case 'r':
                state->mode = GZ_READ;
                break;
#ifndef NO_GZCOMPRESS
            case 'w':
                state->mode = GZ_WRITE;
                break;
            case 'a':
                state->mode = GZ_APPEND;
                break;
#endif
            case '+':       /* can't read and write at the same time */
                free(state);
                return NULL;
            case 'b':       /* ignore -- will request binary anyway */
                break;
            case 'f':
                state->strategy = Z_FILTERED;
                break;
            case 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                break;
            case 'R':
                state->strategy = Z_RLE;
                break;
            case 'F':
                state->strategy = Z_FIXED;
            default:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    if (state->mode == GZ_NONE) {
        free(state);
        return NULL;
    }

    /* save the path name for error messages */
    state->path = malloc(strlen(path) + 1);
    if (state->path == NULL) {
        free(state);
        return NULL;
    }
    strcpy(state->path, path);

    /* open the file with the appropriate mode (or just use fd) */
    state->fd = fd != -1 ? fd :
        open(path,
#ifdef O_LARGEFILE
            O_LARGEFILE |
#endif
#ifdef O_BINARY
            O_BINARY |
#endif
            (state->mode == GZ_READ ?
                O_RDONLY :
                (O_WRONLY | O_CREAT | (
                    state->mode == GZ_WRITE ?
                        O_TRUNC :
                        O_APPEND))),
            0666);
    if (state->fd == -1) {
        free(state->path);
        free(state);
        return NULL;
    }
    if (state->mode == GZ_APPEND)
        state->mode = GZ_WRITE;         /* simplify later checks */

    /* save the current position for rewinding (only if reading) */
    if (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, SEEK_CUR);
        if (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    return (gzFile)state;
}
#else
local gzFile gz_open
(
    const char *mode,
    char** ppbuf,
    unsigned int* plen
)
{
    gz_statep state;

    /* allocate gzFile structure to return */
    state = malloc(sizeof(gz_state));
    if (state == NULL)
        return NULL;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->msg = NULL;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    while (*mode) {
        if (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        else
            switch (*mode) {
            case 'r':
                state->mode = GZ_READ;
                break;
#ifndef NO_GZCOMPRESS
            case 'w':
                state->mode = GZ_WRITE;
                break;
            case 'a':
                state->mode = GZ_APPEND;
                break;
#endif
            case '+':       /* can't read and write at the same time */
                free(state);
                return NULL;
            case 'b':       /* ignore -- will request binary anyway */
                break;
            case 'f':
                state->strategy = Z_FILTERED;
                break;
            case 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                break;
            case 'R':
                state->strategy = Z_RLE;
                break;
            case 'F':
                state->strategy = Z_FIXED;
            default:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    if (state->mode == GZ_NONE) {
        free(state);
        return NULL;
    }

    /* save the path name for error messages */
    state->path = malloc(( strlen(ECRIO_DUMMY_PATH) ) + 1);
    if (state->path == NULL) {
        free(state);
        return NULL;
    }

    if (NULL == pal_StringNCopy((u_char*)state->path, strlen(ECRIO_DUMMY_PATH) + 1, (u_char*)ECRIO_DUMMY_PATH, pal_StringLength((u_char*)ECRIO_DUMMY_PATH)))
    {
        free(state);
        return NULL;
    }

    state->e_fd = ecrio_set(NULL,ppbuf,(int*)plen);

    if (state->mode == GZ_APPEND)
        state->mode = GZ_WRITE;         /* simplify later checks */

    /* save the current position for rewinding (only if reading) */
    if (state->mode == GZ_READ) {
        state->start = ecrio_lseek(state->e_fd, 0, SEEK_CUR);
        if (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    return (gzFile)state;
}
#endif /* ECRIO_ZLIB */
/* -- see zlib.h -- */
#ifndef ECRIO_ZLIB
gzFile ZEXPORT gzopen
(
    const char *path,
    const char *mode
)
{
    return gz_open(path, -1, mode);
}
#else
gzFile ZEXPORT gzopen
(
    const char *mode,
    char** ppbuf,
    unsigned int* plen
)
{
    return gz_open(mode,ppbuf,plen);
}
#endif /* ECRIO_ZLIB */
/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64
(
    const char *path,
    const char *mode
)
{
#ifndef ECRIO_ZLIB
	return gz_open(path, -1, mode);
#else
	(const char*)path;
	return gz_open(mode,NULL,NULL);
#endif
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen
(
    int fd,
    const char *mode
)
{
    char *path;         /* identifier for error messages */
    gzFile gz;

    if (fd == -1 || (path = malloc(7 + 3 * sizeof(int))) == NULL)
        return NULL;
	if (0 >= pal_StringSNPrintf(path, (7 + 3 * sizeof(int)), "<fd:%d>", fd))   /* for debugging */
	{
		free(path);
		return NULL;
	}
#ifndef ECRIO_ZLIB
    gz = gz_open(path, fd, mode);
#else
	gz = gz_open(mode,NULL,NULL);
#endif /* ECRIO_ZLIB */

    free(path);
    return gz;
}

/* -- see zlib.h -- */
int ZEXPORT gzbuffer
(
    gzFile file,
    unsigned size
)
{
    gz_statep state;

    /* get internal structure and check integrity */
    if (file == NULL)
        return -1;
    state = (gz_statep)file;
    if (state->mode != GZ_READ && state->mode != GZ_WRITE)
        return -1;

    /* make sure we haven't already allocated memory */
    if (state->size != 0)
        return -1;

    /* check and set requested size */
    if (size == 0)
        return -1;
    state->want = size;
    return 0;
}

/* -- see zlib.h -- */
int ZEXPORT gzrewind
(
    gzFile file
)
{
    gz_statep state;

    /* get internal structure */
    if (file == NULL)
        return -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    if (state->mode != GZ_READ || state->err != Z_OK)
        return -1;

    /* back up and start over */
    if (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        return -1;
    gz_reset(state);
    return 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64
(
    gzFile file,
    z_off64_t offset,
    int whence
)
{
    unsigned n;
    z_off64_t ret;
    gz_statep state;

    /* get internal structure and check integrity */
    if (file == NULL)
        return -1;
    state = (gz_statep)file;
    if (state->mode != GZ_READ && state->mode != GZ_WRITE)
        return -1;

    /* check that there's no error */
    if (state->err != Z_OK)
        return -1;

    /* can only seek from start or relative to current position */
    if (whence != SEEK_SET && whence != SEEK_CUR)
        return -1;

    /* normalize offset to a SEEK_CUR specification */
    if (whence == SEEK_SET)
        offset -= state->pos;
    else if (state->seek)
        offset += state->skip;
    state->seek = 0;

    /* if within raw area while reading, just go there */
    if (state->mode == GZ_READ && state->how == COPY &&
        state->pos + offset >= state->raw) {
        ret = LSEEK(state->fd, offset - state->have, SEEK_CUR);
        if (ret == -1)
            return -1;
        state->have = 0;
        state->eof = 0;
        state->seek = 0;
        gz_error(state, Z_OK, NULL);
        state->strm.avail_in = 0;
        state->pos += offset;
        return state->pos;
    }

    /* calculate skip amount, rewinding if needed for back seek when reading */
    if (offset < 0) {
        if (state->mode != GZ_READ)         /* writing -- can't go backwards */
            return -1;
        offset += state->pos;
        if (offset < 0)                     /* before start of file! */
            return -1;
        if (gzrewind(file) == -1)           /* rewind, then skip to offset */
            return -1;
    }

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    if (state->mode == GZ_READ) {
        n = GT_OFF(state->have) || (z_off64_t)state->have > offset ?
            (unsigned)offset : state->have;
        state->have -= n;
        state->next += n;
        state->pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    if (offset) {
        state->seek = 1;
        state->skip = offset;
    }
    return state->pos + offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek
(
    gzFile file,
    z_off_t offset,
    int whence
)
{
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    return ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64
(
    gzFile file
)
{
    gz_statep state;

    /* get internal structure and check integrity */
    if (file == NULL)
        return -1;
    state = (gz_statep)file;
    if (state->mode != GZ_READ && state->mode != GZ_WRITE)
        return -1;

    /* return position */
    return state->pos + (state->seek ? state->skip : 0);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell
(
    gzFile file
)
{
    z_off64_t ret;

    ret = gztell64(file);
    return ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64
(
    gzFile file
)
{
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    if (file == NULL)
        return -1;
    state = (gz_statep)file;
    if (state->mode != GZ_READ && state->mode != GZ_WRITE)
        return -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, SEEK_CUR);
    if (offset == -1)
        return -1;
    if (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    return offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset
(
    gzFile file
)
{
    z_off64_t ret;

    ret = gzoffset64(file);
    return ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
int ZEXPORT gzeof
(
    gzFile file
)
{
    gz_statep state;

    /* get internal structure and check integrity */
    if (file == NULL)
        return 0;
    state = (gz_statep)file;
    if (state->mode != GZ_READ && state->mode != GZ_WRITE)
        return 0;

    /* return end-of-file state */
    return state->mode == GZ_READ ?
        (state->eof && state->strm.avail_in == 0 && state->have == 0) : 0;
}

/* -- see zlib.h -- */
const char * ZEXPORT gzerror
(
    gzFile file,
    int *errnum
)
{
    gz_statep state;

    /* get internal structure and check integrity */
    if (file == NULL)
        return NULL;
    state = (gz_statep)file;
    if (state->mode != GZ_READ && state->mode != GZ_WRITE)
        return NULL;

    /* return error information */
    if (errnum != NULL)
        *errnum = state->err;
    return state->msg == NULL ? "" : state->msg;
}

/* -- see zlib.h -- */
void ZEXPORT gzclearerr
(
    gzFile file
)
{
    gz_statep state;

    /* get internal structure and check integrity */
    if (file == NULL)
        return;
    state = (gz_statep)file;
    if (state->mode != GZ_READ && state->mode != GZ_WRITE)
        return;

    /* clear error and end-of-file */
    if (state->mode == GZ_READ)
        state->eof = 0;
    gz_error(state, Z_OK, NULL);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
void ZLIB_INTERNAL gz_error
(
    gz_statep state,
    int err,
    const char *msg
)
{
	unsigned int uSize = 0;

    /* free previously allocated message and clear */
    if (state->msg != NULL) {
        if (state->err != Z_MEM_ERROR)
            free(state->msg);
        state->msg = NULL;
    }

    /* set error code, and if no message, then done */
    state->err = err;
    if (msg == NULL)
        return;

    /* for an out of memory error, save as static string */
    if (err == Z_MEM_ERROR) {
        state->msg = (char *)msg;
        return;
    }

    /* construct error message with path */
    if ((state->msg = malloc(strlen(state->path) + strlen(msg) + 3)) == NULL) {
        state->err = Z_MEM_ERROR;
        state->msg = (char *)"out of memory";
        return;
    }
    uSize = strlen(state->path) + strlen(msg) + 3;
	if (NULL == pal_StringNCopy((u_char*)state->msg, uSize, (u_char*)state->path, pal_StringLength((u_char*)state->path)))
	{
		state->err = Z_MEM_ERROR;
		if (state->msg != NULL)
		{
			pal_MemoryFree((void**)&state->msg);
			return;
		}
	}
	if (NULL == pal_StringNConcatenate((u_char*)state->msg, strlen(state->path) + strlen(msg) + 3, (u_char*)": ", pal_StringLength((u_char*)": ")))
	{
		state->err = Z_MEM_ERROR;
		if (state->msg != NULL)
		{
			pal_MemoryFree((void**)&state->msg);
			return;
		}
	}
	if (NULL == pal_StringNConcatenate((u_char*)state->msg, strlen(state->path) + strlen(msg) + 3, (u_char*)msg, pal_StringLength((u_char*)msg)))
	{
		state->err = Z_MEM_ERROR;
		if (state->msg != NULL)
		{
			pal_MemoryFree((void**)&state->msg);
			return;
		}
	}
    return;
}

#ifndef INT_MAX
/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
unsigned ZLIB_INTERNAL gz_intmax()
{
    unsigned p, q;

    p = 1;
    do {
        q = p;
        p <<= 1;
        p++;
    } while (p > q);
    return q >> 1;
}
#endif
