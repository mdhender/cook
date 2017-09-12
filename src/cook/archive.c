/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997-1999, 2001, 2005 Peter Miller;
 *      All rights reserved.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate archive files
 */

#include <ac/ctype.h>
#include <ac/stdio.h>
#include <ac/errno.h>
#include <ac/ar.h>
#include <ac/fcntl.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ac/utime.h>
#include <ac/unistd.h>

#include <archive.h>
#include <fp.h>
#include <mem.h>
#include <trace.h>


struct archive_file_ty;

typedef struct method_ty method_ty;
struct method_ty
{
    size_t  header_size;
    int (*magic)_((struct archive_file_ty *));
    int (*advance)_((struct archive_file_ty *));
    int (*name_compare)_((struct archive_file_ty *, string_ty *));
    int (*do_stat)_((struct archive_file_ty *, struct stat *));
    int (*utime)_((struct archive_file_ty *, struct utimbuf *));
    void (*close)_((struct archive_file_ty *));
};

typedef struct archive_file_ty archive_file_ty;
struct archive_file_ty
{
    method_ty       *method;
    int             fd;
    long            start;
    long            finish;
    long            current;
    void            *header;
    size_t          header_size;
    long            data;
    long            size;
    long            next;
    char            *name_map;
    long            name_map_len;
};


#ifndef AIAMAG


static int look_for_name_map _((archive_file_ty *, char *, size_t));

static int
look_for_name_map(afp, name, len)
    archive_file_ty *afp;
    char            *name;
    size_t          len;
{
    int             nbytes;
    char            *cp;
    char            *ep;

    if (afp->name_map)
	return 0;
    if
    (
	memcmp(name, "//", 2)
    &&
	memcmp(name, "ARFILENAMES/", 12)
    )
	return 0;

    /*
     * read the data
     */
    afp->name_map_len = afp->size;
    afp->name_map = mem_alloc(afp->size);
    if (lseek(afp->fd, afp->data, SEEK_SET) == -1)
	return -1;
    nbytes = read(afp->fd, afp->name_map, afp->size);
    if (nbytes < 0)
	return -1;
    if (nbytes != afp->size)
    {
	errno = EINVAL;
	return -1;
    }

    /*
     * The names are terminated by newlines.
     * Replace the newlines wil NULs.
     */
    ep = afp->name_map + afp->name_map_len;
    for (cp = afp->name_map; cp < ep; ++cp)
	if (*cp == '\n')
    	    *cp = 0;
    return 1;
}


static int grope_name _((archive_file_ty *, char *, size_t, char **, size_t *,
    int *));

static int
grope_name(afp, hnam, hnamlen, name_p, len_p, trunc_p)
    archive_file_ty *afp;
    char            *hnam;
    size_t          hnamlen;
    char            **name_p;
    size_t          *len_p;
    int             *trunc_p;
{
    char            *ep;

    /*
     * see if it is in the name map
     */
    if
    (
	afp->name_map
    &&
	(hnam[0] == ' ' || hnam[0] == '/')
    &&
	isdigit(hnam[1])
    )
    {
	size_t          offset;

	offset = atoi(hnam + 1);
	if (offset >= afp->name_map_len)
	{
	    errno = EINVAL;
	    return -1;
	}
	*name_p = afp->name_map + offset;
	*len_p = strlen(*name_p);
	*trunc_p = 0;
	return 0;
    }

    /*
     * see of the name follows the header
     */
    if (!memcmp(hnam, "#1/", 3) && isdigit(hnam[3]))
    {
	static char     *buf;
	static size_t   bufmax;
	size_t          buflen;
	int             nbytes;

	buflen = atoi(hnam + 3);
	if (buflen > bufmax)
	{
	    bufmax = buflen;
	    buf = mem_change_size(buf, bufmax);
	}
	nbytes = read(afp->fd, buf, buflen);
	if (nbytes < 0)
	    return -1;
	if (nbytes != nbytes)
	{
	    errno = EINVAL;
	    return -1;
	}
	*name_p = buf;
	*len_p = buflen;
	*trunc_p = 0;

	/*
	 * adjust data address
	 */
	afp->data += nbytes;
	afp->next = afp->data + afp->size;
	if (afp->next & 1)
	    afp->next++;
	return 0;
    }

    /*
     * see if there is a '/' terminator
     */
    ep = memchr(hnam, '/', hnamlen);
    if (ep)
    {
	*name_p = hnam;
	*len_p = ep - hnam;
	*trunc_p = (*len_p >= hnamlen - 2);
	return 0;
    }

    /*
     * see if there is a ' ' terminator
     */
    ep = memchr(hnam, ' ', hnamlen);
    if (ep)
    {
	*name_p = hnam;
	*len_p = ep - hnam;
	*trunc_p = 0;
	return 0;
    }

    /*
     * no terminator found,
     * length is whole field,
     * need to truncate
     */
    *name_p = hnam;
    *len_p = hnamlen;
    *trunc_p = 1;
    return 0;
}


static int cmp_grope_name _((archive_file_ty *, string_ty *, char *, size_t));

static int
cmp_grope_name(afp, member, hnam, hnamlen)
    archive_file_ty *afp;
    string_ty       *member;
    char            *hnam;
    size_t          hnamlen;
{
    char            *name;
    size_t          len;
    int             trunc;
    int             flag;

    flag = look_for_name_map(afp, hnam, hnamlen);
    if (flag < 0)
	return -1;
    if (flag)
	return 0;
    if (grope_name(afp, hnam, hnamlen, &name, &len, &trunc))
	return -1;

    return
    (
	(trunc ? member->str_length >= len : member->str_length == len)
    &&
	!memcmp(member->str_text, name, len)
    );
}

#endif /* !AIAMAG */
#if (defined(SARMAG) && !PORT5AR) || defined(AIAMAG)

static long number _((char *, size_t len));

static long
number(s, len)
    char            *s;
    size_t          len;
{
    int             ndigits;
    long            n;

    while (len > 0 && isspace(*s))
    {
	--len;
	++s;
    }
    ndigits = 0;
    n = 0;
    while (len > 0 && isdigit(*s))
    {
	n = n * 10 + *s - '0';
	--len;
	++s;
	++ndigits;
    }
    if (!ndigits)
	return -1;
    while (len > 0 && isspace(*s))
    {
	--len;
	++s;
    }
    if (len && *s)
	return -1;
    return n;
}


static long octal _((char *, size_t len));

static long
octal(s, len)
    char            *s;
    size_t          len;
{
    int             ndigits;
    long            n;

    while (len > 0 && isspace(*s))
    {
	--len;
	++s;
    }
    ndigits = 0;
    n = 0;
    while (len > 0 && isdigit(*s) && *s != '8' && *s != '9')
    {
	n = n * 8 + *s - '0';
	--len;
	++s;
	++ndigits;
    }
    if (!ndigits)
	return -1;
    while (len > 0 && isspace(*s))
    {
	--len;
	++s;
    }
    if (len && *s)
	return -1;
    return n;
}


static void numset _((char *, size_t, long));

static void
numset(s, len, n)
    char            *s;
    size_t          len;
    long            n;
{
    sprintf(s, "%ld", n);
    while (len > 0 && *s)
    {
	++s;
	--len;
    }
    while (len > 0)
    {
	*s++ = ' ';
	--len;
    }
}


#endif /* SARMAG || AIAMAG */
#if PORT5AR

static int port5_magic _((archive_file_ty *));

static int
port5_magic(afp)
    archive_file_ty *afp;
{
    int             nbytes;
    struct ar_hdr   magic;

    nbytes = read(afp->fd, &magic, sizeof(magic));
    if (nbytes < 0)
	return -1;
    if (nbytes != sizeof(magic) || memcmp(magic.ar_magic, ARMAG, SARMAG))
    {
	errno = EINVAL;
	return -1;
    }
    afp->start = sizeof(magic);
    return 0;
}


static long b4r _((char *));

static long
b4r(cp)
    char            *cp;
{
    return
    (
	((long)(unsigned char)cp[0] << 24)
    ||
	((long)(unsigned char)cp[1] << 16)
    ||
	((unsigned char)cp[2] << 8)
    ||
	(unsigned char)cp[3]
    );
}


static void b4w _((char *, long));

static void
b4w(cp, n)
    char            *cp;
    long            n;
{
    cp[0] = n >> 24;
    cp[1] = n >> 16;
    cp[2] = n >> 8;
    cp[3] = n;
}


static int port5_advance _((archive_file_ty *));

static int
port5_advance(afp)
    archive_file_ty *afp;
{
    int             nbytes;
    struct arf_hdr  *h;

    h = afp->header;
    assert(h);
    nbytes = read(afp->fd, h, sizeof(*h));
    if (nbytes < 0)
	return -1;
    if (nbytes == 0)
    {
	errno = ENOENT;
	return -1;
    }
    if (nbytes != sizeof(*h))
    {
	broken:
	errno = EINVAL;
	return -1;
    }

    afp->size = b4r(h->arf_size);
    if (afp->size < 0)
	goto broken;
    afp->data = afp->current + sizeof(*h);
    afp->next = afp->data + afp->size;
    if (afp->size & 1)
	afp->next++;
    return 0;
}


static int port5_name_compare _((archive_file_ty *, string_ty *));

static int
port5_name_compare(afp, name)
    archive_file_ty *afp;
    string_ty       *name;
{
    struct arf_hdr  *h;

    h = afp->header;
    assert(h);
    return cmp_grope_name(afp, name, h->arf_name, sizeof(h->arf_name));
}


static int port5_stat _((archive_file_ty *, struct stat *));

static int
port5_stat(afp, st)
    archive_file_ty *afp;
    struct stat     *st;
{
    struct arf_hdr  *h;

    h = afp->header;
    assert(h);
    st->st_mtime = b4r(h->arf_date);
    st->st_atime = st->st_mtime;
    st->st_ctime = st->st_mtime;
    st->st_uid = b4r(h->arf_uid);
    st->st_gid = b4r(h->arf_gid);
    st->st_mode = b4r(h->arf_mode);
    st->st_size = afp->size;
    return 0;
}


static int port5_utime _((archive_file_ty *, struct utimbuf *));

static int
port5_utime(afp, ut)
    archive_file_ty *afp;
    struct utimbuf  *ut;
{
    struct arf_hdr  *h;
    int             nbytes;
    struct ar_hdr   fh;
    struct stat     st;

    /*
     * set this file entry
     */
    h = afp->header;
    assert(h);
    b4w(h->arf_date, ut->modtime);
    assert(b4r(h->arf_date) == ut->modtime);
    nbytes = write(afp->fd, h, sizeof(*h));
    if (nbytes < 0)
	return -1;
    if (nbytes != sizeof(*h))
    {
	errno = EIO;
	return -1;
    }

    /*
     * must also set the file header
     */
    if (fstat(afp->fd, &st))
	return -1;
    if (lseek(afp->fd, 0L, SEEK_SET) == -1)
	return -1;
    nbytes = read(afp->fd, &fh, sizeof(fh));
    if (nbytes < 0)
	return -1;
    if (nbytes != sizeof(fh))
    {
	errno = EIO;
	return -1;
    }
    b4w(fh.ar_date, st.st_mtime);
    if (lseek(afp->fd, 0L, SEEK_SET) == -1)
	return -1;
    nbytes = write(afp->fd, &fh, sizeof(fh));
    if (nbytes < 0)
	return -1;
    if (nbytes != sizeof(fh))
    {
	errno = EIO;
	return -1;
    }

    /*
     * done
     */
    return 0;
}


static method_ty port5 =
{
    sizeof(struct arf_hdr),
    port5_magic,
    port5_advance,
    port5_name_compare,
    port5_stat,
    port5_utime,
    0, /* close */
};

#endif /* PORT5AR */
#if defined(SARMAG) && !PORT5AR

static int standard_magic _((archive_file_ty *));

static int
standard_magic(afp)
    archive_file_ty *afp;
{
    int             nbytes;
    char            magic[SARMAG];

    nbytes = read(afp->fd, magic, sizeof(magic));
    if (nbytes < 0)
	return -1;
    if (nbytes != sizeof(magic) || memcmp(magic, ARMAG, sizeof(magic)))
    {
	errno = EINVAL;
	return -1;
    }
    afp->start = sizeof(magic);
    return 0;
}


static int standard_advance _((archive_file_ty *));

static int
standard_advance(afp)
    archive_file_ty *afp;
{
    int             nbytes;
    struct ar_hdr   *h;

    h = afp->header;
    assert(h);
    nbytes = read(afp->fd, h, sizeof(*h));
    if (nbytes < 0)
	return -1;
    if (nbytes == 0)
    {
	errno = ENOENT;
	return -1;
    }
    if (nbytes != sizeof(*h))
    {
	broken:
	errno = EINVAL;
	return -1;
    }
#ifdef ARFMAG
    if (memcmp(h->ar_fmag, ARFMAG, sizeof(h->ar_fmag)))
	goto broken;
#endif

    afp->size = number(h->ar_size, sizeof(h->ar_size));
    if (afp->size < 0)
	goto broken;
    afp->data = afp->current + sizeof(*h);
    afp->next = afp->data + afp->size;
    if (afp->next & 1)
	afp->next++;
    return 0;
}


static int standard_name_compare _((archive_file_ty *, string_ty *));

static int
standard_name_compare(afp, name)
    archive_file_ty *afp;
    string_ty       *name;
{
    struct ar_hdr   *h;

    h = afp->header;
    assert(h);
    return cmp_grope_name(afp, name, h->ar_name, sizeof(h->ar_name));
}


static int standard_stat _((archive_file_ty *, struct stat *));

static int
standard_stat(afp, st)
    archive_file_ty *afp;
    struct stat     *st;
{
    struct ar_hdr   *h;
    long            value;

    h = afp->header;
    assert(h);
    value = number(h->ar_date, sizeof(h->ar_date));
    if (value < 0)
    {
	broken:
	errno = EINVAL;
	return -1;
    }
    st->st_mtime = value;
    st->st_atime = value;
    st->st_ctime = value;

    value = number(h->ar_uid, sizeof(h->ar_uid));
    if (value < 0)
	goto broken;
    st->st_uid = value;

    value = octal(h->ar_mode, sizeof(h->ar_mode));
    if (value < 0)
	goto broken;
    st->st_mode = value;

    st->st_size = afp->size;
    return 0;
}


static int standard_utime _((archive_file_ty *, struct utimbuf *));

static int
standard_utime(afp, ut)
    archive_file_ty *afp;
    struct utimbuf  *ut;
{
    struct ar_hdr   *h;
    int             nbytes;

    h = afp->header;
    assert(h);
    numset(h->ar_date, sizeof(h->ar_date), ut->modtime);
    assert(number(h->ar_date, sizeof(h->ar_date)) == ut->modtime);
    nbytes = write(afp->fd, h, sizeof(*h));
    if (nbytes < 0)
	return -1;
    if (nbytes != sizeof(*h))
    {
	errno = EIO;
	return -1;
    }
    return 0;
}


static method_ty standard =
{
    sizeof(struct ar_hdr),
    standard_magic,
    standard_advance,
    standard_name_compare,
    standard_stat,
    standard_utime,
    0, /* close */
};

#endif /* SARMAG && !PORT5AR */
#ifdef AIAMAG

static int ai_magic _((archive_file_ty *));

static int
ai_magic(afp)
    archive_file_ty *afp;
{
    FL_HDR          fh;
    int             nbytes;

    trace(("ai_magic()\n{\n"));
    nbytes = read(afp->fd, &fh, FL_HSZ);
    if (nbytes != FL_HSZ || memcmp(fh.fl_magic, AIAMAG, SAIAMAG))
    {
	broken:
	errno = EINVAL;
	trace(("return -1;\n"));
	trace(("}\n"));
	return -1;
    }
    afp->start = number(fh.fl_fstmoff, sizeof(fh.fl_fstmoff));
    trace(("start = %08lX\n", afp->start));
    if (afp->start < 0)
	goto broken;
    afp->finish = number(fh.fl_lstmoff, sizeof(fh.fl_lstmoff));
    trace(("finish = %08lX\n", afp->finish));
    if (afp->finish < 0)
	goto broken;
    trace(("return 0;\n"));
    trace(("}\n"));
    return 0;
}


static int ai_advance _((archive_file_ty *));

static int
ai_advance(afp)
    archive_file_ty *afp;
{
    int             nbytes;
    struct ar_hdr   *h;
    int             h_len;
    char            *name;
    int             name_len;

    trace(("ai_advance()\n{\n"));
    if (afp->current > afp->finish)
    {
	errno = ENOENT;
	trace(("at finish\n"));
	trace(("return -1;\n"));
	trace(("}\n"));
	return -1;
    }

    h = afp->header;
    assert(h);
    name = (char *)(h + 1);

    h_len = AR_HSZ - sizeof(h->_ar_name);
    nbytes = read(afp->fd, h, h_len);
    if (nbytes < 0)
    {
	trace(("return -1;\n"));
	trace(("}\n"));
	return -1;
    }
    /* end of file not valid here */
    if (nbytes != h_len)
    {
	broken:
	errno = EINVAL;
	trace(("return -1;\n"));
	trace(("}\n"));
	return -1;
    }
    name_len = number(h->ar_namlen, sizeof(h->ar_namlen));
    if (name_len < 0 || name_len > 255)
	goto broken;

    nbytes = read(afp->fd, name, name_len);
    if (nbytes < 0)
    {
	trace(("return -1;\n"));
	trace(("}\n"));
	return -1;
    }
    if (nbytes != name_len)
	goto broken;
    name[name_len] = 0;

    afp->data = afp->current + h_len + name_len + 2;
    if (afp->data & 1)
	afp->data++;
    afp->size = number(h->ar_size, sizeof(h->ar_size));
    if (afp->size < 0)
	goto broken;
    afp->next = number(h->ar_nxtmem, sizeof(h->ar_nxtmem));
    trace(("afp->next = %08lX\n", afp->next));
    if (afp->next < 0)
	goto broken;
    trace(("return 0;\n"));
    trace(("}\n"));
    return 0;
}


static int ai_name_compare _((archive_file_ty *, string_ty *));

static int
ai_name_compare(afp, member)
    archive_file_ty *afp;
    string_ty       *member;
{
    struct ar_hdr   *h;
    char            *name;

    h = afp->header;
    assert(h);
    name = (char *)(h + 1);
    return !strcmp(member->str_text, name);
}


static int ai_stat _((archive_file_ty *, struct stat *));

static int
ai_stat(afp, st)
    archive_file_ty *afp;
    struct stat     *st;
{
    struct ar_hdr   *h;
    long            value;

    trace(("ai_stat()\n{\n"));
    h = afp->header;
    assert(h);
    value = number(h->ar_date, sizeof(h->ar_date));
    if (value < 0)
    {
	broken:
	errno = EINVAL;
	trace(("return -1;\n"));
	trace(("}\n"));
	return -1;
    }
    st->st_mtime = value;
    st->st_atime = value;
    st->st_ctime = value;

    value = number(h->ar_uid, sizeof(h->ar_uid));
    if (value < 0)
	goto broken;
    st->st_uid = value;

    value = number(h->ar_gid, sizeof(h->ar_gid));
    if (value < 0)
	goto broken;
    st->st_gid = value;

    value = octal(h->ar_mode, sizeof(h->ar_mode));
    if (value < 0)
	goto broken;
    st->st_mode = value;
    st->st_size = afp->size;
    trace(("return 0;\n"));
    trace(("}\n"));
    return 0;
}


static int ai_utime _((archive_file_ty *, struct utimbuf *));

static int
ai_utime(afp, ut)
    archive_file_ty *afp;
    struct utimbuf  *ut;
{
    struct ar_hdr   *h;
    int             h_len;
    int             nbytes;
    struct stat     st;
    FL_HDR          fh;

    h = afp->header;
    assert(h);
    numset(h->ar_date, sizeof(h->ar_date), ut->modtime);
    h_len = AR_HSZ - sizeof(h->_ar_name);
    nbytes = write(afp->fd, h, h_len);
    if (nbytes < 0)
	return -1;
    if (nbytes != h_len)
    {
	errno = EIO;
	return -1;
    }

    /*
     * success
     */
    return 0;
}


static method_ty ai =
{
    sizeof(struct ar_hdr) + 256,
    ai_magic,
    ai_advance,
    ai_name_compare,
    ai_stat,
    ai_utime,
    0, /* close */
};

#endif /* AIAMAG */
#if defined(ARMAG) && !defined(SARMAG)

static int old_magic _((archive_file_ty *));

static int
old_magic(afp)
    archive_file_ty *afp;
{
#ifndef M_XENIX
    int             magic;
#else
    unsigned short  magic;
#endif
    int             nbytes;

    nbytes = read(afp->fd, &magic, sizeof(magic));
    if (nbytes < 0)
	return -1;
    if (nbytes != sizeof(magic) || magic != ARMAG)
    {
	errno = EINVAL;
	return -1;
    }
    afp->start = sizeof(magic);
    return 0;
}


static int old_advance _((archive_file_ty *));

static int
old_advance(afp)
    archive_file_ty *afp;
{
    struct ar_hdr   *h;
    int             nbytes;

    h = afp->header;
    assert(h);
    nbytes = read(afp->fd, h, sizeof(*h));
    if (nbytes < 0)
	return -1;
    if (!nbytes)
    {
	errno = ENOENT;
	return -1;
    }
    if (nbytes != sizeof(*h))
    {
	errno = EINVAL;
	return -1;
    }

    /*
     * figure a few things
     */
    afp->size = h->ar_size;
    afp->data = afp->current + sizeof(*h);
    afp->next = afp->data + afp->size;
    if (afp->next & 1)
	afp->next++;

    /*
     * success
     */
    return 0;
}


static int old_name_compare _((archive_file_ty *, string_ty *));

static int
old_name_compare(afp, name)
    archive_file_ty *afp;
    string_ty       *name;
{
    struct ar_hdr   *h;

    h = afp->header;
    assert(h);
    return cmp_grope_name(afp, name, h->ar_name, sizeof(h->ar_name));
}


static int old_stat _((archive_file_ty *, struct stat *));

static int
old_stat(afp, st)
    archive_file_ty *afp;
    struct stat     *st;
{
    struct ar_hdr   *h;

    h = afp->header;
    assert(h);
    st->st_atime = h->ar_date;
    st->st_ctime = h->ar_date;
    st->st_gid = h->ar_gid;
    st->st_mode = h->ar_mode;
    st->st_mtime = h->ar_date;
    st->st_uid = h->ar_uid;
    return 0;
}


static int old_utime _((archive_file_ty *, struct utimbuf *));

static int
old_utime(afp, ut)
    archive_file_ty *afp;
    struct utimbuf  *ut;
{
    struct ar_hdr   *h;
    int             nbytes;

    h = afp->header;
    assert(h);
    h->ar_date = ut->modtime;
    nbytes = write(afp->fd, h, sizeof(*h));
    if (nbytes < 0)
	return -1;
    if (nbytes != sizeof(*h))
    {
	errno = EIO;
	return -1;
    }
    return 0;
}


static method_ty old =
{
    sizeof(struct ar_hdr),
    old_magic,
    old_advance,
    old_name_compare,
    old_stat,
    old_utime,
    0, /* close */
};

#endif /* !SARMAG && !AIAMAG */


/*
* This is a table of all archive formats understood
* by the <ar.h> system include file.
*/
static method_ty *table[] =
{
#if PORT5AR
    &port5,
#endif
#if defined(SARMAG) && !PORT5AR
    &standard,
#endif
#ifdef AIAMAG
    &ai,
#endif
#if defined(ARMAG) && !defined(SARMAG)
    &old,
#endif
    0
};

static archive_file_ty *archive_file_open _((string_ty *, int));

static archive_file_ty *
archive_file_open(path, mode)
    string_ty       *path;
    int             mode;
{
    int             fd;
    int             err;
    int             j;
    archive_file_ty *afp;

    /*
     * open the file as specified
     */
    trace(("archive_file_open(path = \"%s\", mode = %d)\n{\n"/*}*/,
	path->str_text, mode));
    assert(mode == (O_RDONLY | O_BINARY) || mode == (O_RDWR | O_BINARY));
    trace(("open\n"));
    fd = open(path->str_text, mode, 0666);
    if (fd < 0)
    {
	afp = 0;
	goto done;
    }

    trace(("alloc\n"));
    afp = mem_alloc(sizeof(archive_file_ty));
    afp->fd = fd;
    afp->start = 0;
    afp->finish = 0;
    afp->current = 0;
    afp->next = 0;
    afp->header = 0;
    afp->header_size = 0;
    afp->name_map = 0;
    afp->name_map_len = 0;

    /*
     * see if we can understand this format
     */
    for (j = 0; table[j]; ++j)
    {
	if (!table[j])
	{
	    errno = EINVAL;
	    bomb:
	    err = errno;
	    close(afp->fd);
	    if (afp->header)
		mem_free(afp->header);
	    mem_free(afp);
	    errno = err;
	    afp = 0;
	    goto done;
	}
	if (lseek(afp->fd, 0L, SEEK_SET) == -1)
    	    goto bomb;
	afp->method = table[j];
	if (afp->method->header_size > afp->header_size)
	{
    	    afp->header_size = afp->method->header_size;
    	    afp->header = mem_change_size(afp->header, afp->header_size);
	}
	err = afp->method->magic(afp);
	if (!err)
    	    break;
	if (err != EINVAL)
    	    goto bomb;
    }

    /*
     * here for all exits
     */
    done:
    trace(("return %08lX; /* errno = %s */\n", (long)afp, strerror(errno)));
    trace((/*{*/"}\n"));
    return afp;
}


static int archive_file_close _((archive_file_ty *));

static int
archive_file_close(afp)
    archive_file_ty *afp;
{
    int             err;

    trace(("archive_file_close(afp = %08lX)\n{\n"/*}*/, (long)afp));
    if (afp->method->close)
	afp->method->close(afp);
    if (afp->header)
	mem_free(afp->header);
    if (afp->name_map)
	mem_free(afp->name_map);
    err = close(afp->fd);
    mem_free(afp);
    trace(("return %d;\n", err));
    trace((/*{*/"}\n"));
    return err;
}


static int archive_file_stat _((archive_file_ty *, string_ty *, struct stat *));

static int
archive_file_stat(afp, member, st)
    archive_file_ty *afp;
    string_ty       *member;
    struct stat     *st;
{
    int             result;
    int             flag;

    /*
     * walk each entry
     */
    trace(("archive_file_stat(afp = %08lX, member = \"%s\")\n{\n"/*}*/,
	(long)afp, member->str_text));
    result = -1;
    afp->current = afp->start;
    for (;;)
    {
	/*
	 * read the next entry in the archive
	 */
	if (lseek(afp->fd, afp->current, SEEK_SET) == -1)
	    goto done;
	if (afp->method->advance(afp))
	    goto done;

	/*
	 * see if it is the one we want
	 */
	flag = afp->method->name_compare(afp, member);
	if (flag < 0)
	    goto done;
	if (flag)
	    break;

	/*
	 * advance to next entry
	 */
	afp->current = afp->next;
    }

    /*
     * set the file stats
     */
    trace(("found\n"));
    memset(st, 0, sizeof(*st));
    if (afp->method->do_stat(afp, st))
	goto done;

    /*
     * Because archive members are given the exact same mtime as
     * the input file, adjust this forward 1 second, so that the
     * archive member looks "younger" than the input file.
     */
    st->st_mtime++;
    trace(("mtime = %ld;\n", st->st_mtime));
    result = 0;

    /*
     * here for all exits
     */
    done:
    trace(("return %d;\n", result));
    trace((/*{*/"}\n"));
    return result;
}


static int archive_file_utime _((archive_file_ty *, string_ty *,
    struct utimbuf *));

static int
archive_file_utime(afp, member, utp)
    archive_file_ty *afp;
    string_ty       *member;
    struct utimbuf  *utp;
{
    int             result;
    int             flag;
    struct utimbuf  ut;

    /*
     * Because archive members are given the exact same mtime as
     * the input file, adjust this forward 1 second, so that the
     * archive member looks "younger" than the input file.
     */
    trace(("archive_file_utime(afp = %08lX, member = \"%s\")\n{\n"/*}*/,
	    (long)afp, member->str_text));
    ut.modtime = utp->modtime - 1;
    ut.actime = utp->actime;

    /*
     * walk each entry
     */
    result = -1;
    afp->current = afp->start;
    for (;;)
    {
	/*
	 * read the next entry in the archive
	 */
	if (lseek(afp->fd, afp->current, SEEK_SET) == -1)
	    goto done;
	if (afp->method->advance(afp))
	    goto done;

	/*
	 * find the length of the member name
	 */
	flag = afp->method->name_compare(afp, member);
	if (flag < 0)
	    goto done;
	if (flag)
	    break;

	/*
	 * advance to next entry
	 */
	afp->current = afp->next;
    }

    /*
     * set the date in the header
     * and write it back
     */
    if (lseek(afp->fd, afp->current, SEEK_SET) == -1)
	goto done;
    if (afp->method->utime(afp, &ut))
	goto done;
    result = 0;

    /*
     * here for all exits
     */
    done:
    trace(("return %d;\n", result));
    trace((/*{*/"}\n"));
    return result;
}


static int archive_file_fingerprint _((archive_file_ty *, string_ty *,
    fingerprint_ty *, char *));

static int
archive_file_fingerprint(afp, member, fp, buf)
    archive_file_ty *afp;
    string_ty       *member;
    fingerprint_ty  *fp;
    char            *buf;
{
    int             result;
    int             flag;
    long            size;

    /*
     * walk each entry
     */
    trace(("archive_file_fngrprnt(afp = %08lX, member = \"%s\")\n{\n"/*}*/,
	(long)afp, member->str_text));
    result = -1;
    afp->current = afp->start;
    for (;;)
    {
	/*
	 * read the next entry in the archive
	 */
	if (lseek(afp->fd, afp->current, SEEK_SET) == -1)
	    goto done;
	if (afp->method->advance(afp))
	    goto done;

	/*
	 * is it the one we want
	 */
	flag = afp->method->name_compare(afp, member);
	if (flag < 0)
	    goto done;
	if (flag)
	    break;

	/*
	 * advance to next entry
	 */
	afp->current = afp->next;
    }

    /*
     * read this portion of the file
     * and generate the fingerprint
     */
    size = afp->size;
    if (lseek(afp->fd, afp->data, SEEK_SET) == -1)
	goto done;
    while (size > 0)
    {
	unsigned char   ibuf[1024];
	int             len;
	int             nbytes;

	len = (size > sizeof(ibuf) ? sizeof(ibuf) : size);
	nbytes = read(afp->fd, ibuf, len);
	if (nbytes < 0)
	    goto done;
	if (nbytes == 0)
	{
	    errno = EINVAL;
	    goto done;
	}
	fingerprint_addn(fp, ibuf, nbytes);
	size -= nbytes;
    }
    fingerprint_sum(fp, buf);
    result = 0;

    /*
     * here for all exits
     */
    done:
    trace(("return %d;\n", result));
    trace((/*{*/"}\n"));
    return result;
}


static int archive_match _((string_ty *, string_ty **, string_ty **));

static int
archive_match(name, path_p, member_p)
    string_ty       *name;
    string_ty       **path_p;
    string_ty       **member_p;
{
    char            *s;
    char            *p;
    char            *mp;

    s = name->str_text;
    p = strchr(s, '('/*)*/);
    if
    (
	!p
    ||
	p == s
    ||
	s[name->str_length - 1] != /*(*/')'
    ||
	s[name->str_length - 2] == '/'
    ||
	(p - s) == name->str_length - 2
    )
	return 0;
    *path_p = str_n_from_c(s, p - s);
    mp = strrchr(p, '/');
    if (mp)
	++mp;
    else
	mp = p + 1;
    *member_p = str_n_from_c(mp, name->str_length - 1 - (mp - s));
    return 1;
}


int
archive_stat(name, st)
    string_ty       *name;
    struct stat     *st;
{
    string_ty       *path;
    string_ty       *member;
    archive_file_ty *afp;
    int             errno_hold;
    int             result;

    /*
     * extract path and member name
     */
    trace(("archive_stat(name = \"%s\")\n{\n"/*}*/, name->str_text));
    result = -1;
    path = 0;
    member = 0;
    if (!archive_match(name, &path, &member))
    {
	/*
	 * this gives the cleanest error handling
	 * in cook/os.c and cook/stat.cache.c
	 */
	errno = ENOENT;
	goto done;
    }
    assert(path);
    assert(member);

    /*
     * open the archive file
     */
    trace(("open\n"));
    afp = archive_file_open(path, O_RDONLY | O_BINARY);
    if (!afp)
    {
	str_free(member);
	str_free(path);
	goto done;
    }

    /*
     * read the relevant entry
     */
    trace(("stat\n"));
    if (archive_file_stat(afp, member, st))
    {
	errno_hold = errno;
	archive_file_close(afp);
	str_free(member);
	str_free(path);
	errno = errno_hold;
	goto done;
    }

    /*
     * close the archive file
     */
    trace(("close\n"));
    str_free(member);
    str_free(path);
    if (archive_file_close(afp))
	goto done;

    /*
     * success
     */
    trace(("success\n"));
    result = 0;

    /*
     * here for all exits
     */
    done:
#ifdef DEBUG
    errno_hold = errno;
    trace(("return %d; /* errno = %d */\n", result, errno_hold));
    trace((/*{*/"}\n"));
    errno = errno_hold;
#endif
    return result;
}


int
archive_utime(name, ut)
    string_ty       *name;
    struct utimbuf  *ut;
{
    string_ty       *path;
    string_ty       *member;
    archive_file_ty *afp;
    int             err;
    int             result;

    /*
     * extract path and member name
     */
    trace(("archive_utime(name = \"%s\")\n{\n"/*}*/, name->str_text));
    result = -1;
    path = 0;
    member = 0;
    if (!archive_match(name, &path, &member))
    {
	/*
	 * this gives the cleanest error handling
	 * in cook/os.c and cook/stat.cache.c
	 */
	errno = ENOENT;
	goto done;
    }
    assert(path);
    assert(member);

    /*
     * open the archive file
     */
    afp = archive_file_open(path, O_RDWR | O_BINARY);
    if (!afp)
    {
	str_free(member);
	str_free(path);
	goto done;
    }

    /*
     * read the relevant entry
     */
    if (archive_file_utime(afp, member, ut))
    {
	err = errno;
	archive_file_close(afp);
	str_free(member);
	str_free(path);
	errno = err;
	goto done;
    }

    /*
     * close the archive file
     */
    str_free(member);
    str_free(path);
    if (archive_file_close(afp))
	goto done;

    /*
     * success
     */
    trace(("success\n"));
    result = 0;

    /*
     * here for all exits
     */
    done:
    trace(("return %d;\n", result));
    trace((/*{*/"}\n"));
    return result;
}


int
archive_fingerprint(fp, name, buf)
    fingerprint_ty  *fp;
    string_ty       *name;
    char            *buf;
{
    string_ty       *path;
    string_ty       *member;
    archive_file_ty *afp;
    int             err;
    int             result;

    /*
     * extract path and member name
     */
    trace(("archive_fingerprint(name = \"%s\")\n{\n"/*}*/, name->str_text));
    result = -1;
    path = 0;
    member = 0;
    if (!archive_match(name, &path, &member))
    {
	/*
	 * this gives the cleanest error handling
	 * in cook/os.c and cook/stat.cache.c
	 */
	errno = ENOENT;
	goto done;
    }
    assert(path);
    assert(member);

    /*
     * open the archive file
     */
    afp = archive_file_open(path, O_RDONLY | O_BINARY);
    if (!afp)
    {
	str_free(member);
	str_free(path);
	goto done;
    }

    /*
     * read the relevant entry
     */
    if (archive_file_fingerprint(afp, member, fp, buf))
    {
	err = errno;
	archive_file_close(afp);
	str_free(member);
	str_free(path);
	errno = err;
	goto done;
    }

    /*
     * close the archive file
     */
    str_free(member);
    str_free(path);
    if (archive_file_close(afp))
	goto done;

    /*
     * success
     */
    trace(("success\n"));
    result = 0;

    /*
     * here for all exits
     */
    done:
    trace(("return %d;\n", result));
    trace((/*{*/"}\n"));
    return result;
}
