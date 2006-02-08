/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                           Ken Stevens, Steve McClure
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  ---
 *
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  xundump.c: Load back xdump output
 * 
 *  Known contributors to this file:
 *     Ron Koenderink, 2005
 *     Markus Armbruster, 2005
 */

/* FIXME normalize terminology: table/rows/columns or file/records/fields */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "file.h"
#include "match.h"
#include "nsc.h"
#include "optlist.h"
#include "prototypes.h"

static char *fname;
static int lineno;
static int human;
static int cur_type;
static void *cur_obj;
static int nxt_sel, nxt_idx;

static int gripe(char *, ...) ATTRIBUTE((format (printf, 1, 2)));
static int deffld(int, char *, int);
static void nxtfld(void);
static int setnum(int, double);
static int setstr(int, char *);
static int xunsymbol1(char *, struct symbol *, struct castr *, int);
static int setsym(int, char *);
static int mtsymset(int, long *);
static int add2symset(int, long *, char *);
static struct symbol *get_symtab(struct castr *);

static int
gripe(char *fmt, ...)
{
    va_list ap;

    fprintf(stderr, "%s:%d: ", fname, lineno);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    putc('\n', stderr);

    return -1;
}

static int
skipfs(FILE *fp)
{
    int ch;

    do {
	ch = getc(fp);
    } while (ch == ' ' || ch == '\t');

    if (ch == '#') {
	do {
	    ch = getc(fp);
	} while (ch != EOF && ch != '\n');
    }

    return ch;
}

static int
getid(FILE *fp, char *buf)
{
    int n;
    if (fscanf(fp, "%1023[^#() \t\n]%n", buf, &n) != 1 || !isalpha(buf[0]))
	return -1;
    return n;
}

static char *
xuesc(char *buf)
{
    char *src, *dst;
    int octal_chr, n;

    dst = buf;
    src = buf;
    while (*src) {
	if (*src == '\\') {
	    if (sscanf(++src, "%3o%n", &octal_chr, &n) != 1 || n != 3)
		return NULL;
	    *dst++ = (char)octal_chr;
	    src += 3;
	} else
	    *dst++ = *src++;
    }
    *dst = '\0';
    return buf;
}

static int
xufldname(FILE *fp, int i)
{
    int ch, idx;
    char buf[1024];

    ch = skipfs(fp);
    switch (ch) {
    case EOF:
	return gripe("Unexpected EOF");
    case '\n':
	if (nxt_sel >= 0)
	    return gripe("Header fields missing"); /* TODO which? */
	lineno++;
	return 0;
    default:
	ungetc(ch, fp);
	if (getid(fp, buf) < 0)
	    return gripe("Junk in header field %d", i + 1);
	ch = getc(fp);
	if (ch != '(') {
	    ungetc(ch, fp);
	    return deffld(i, buf, -1);
	}
	ch = getc(fp);
	ungetc(ch, fp);
	if (isdigit(ch) || ch == '-' || ch == '+') {
	    if (fscanf(fp, "%d", &idx) != 1) {
		return gripe("Malformed number in index of header field %d",
			     i + 1);
	    }
	} else {
	    if (getid(fp, buf) < 0)
		return gripe("Malformed index in header field %d", i + 1);
	    return gripe("Symbolic index in header field %d not yet implemented",
			 i + 1);
	}
	ch = getc(fp);
	if (ch != ')')
	    return gripe("Malformed index in header field %d", i + 1);
	return deffld(i, buf, idx);
    }
}

static int
xufld(FILE *fp, int i)
{
    int ch;
    char buf[1024];
    double dbl;
    long set;

    ch = skipfs(fp);
    switch (ch) {
    case EOF:
	return gripe("Unexpected EOF");
    case '\n':
	if (nxt_sel >= 0)
	    return gripe("Fields missing");
	lineno++;
	return 0;
    case '+': case '-': case '.':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
	ungetc(ch, fp);
	if (fscanf(fp, "%lg", &dbl) != 1)
	    return gripe("Malformed number in field %d", i + 1);
	return setnum(i, dbl);
    case '"':
	ch = getc(fp);
	if (ch == '"')
	    buf[0] = 0;
	else {
	    ungetc(ch, fp);
	    if (fscanf(fp, "%1023[^\"\n]", buf) != 1 || getc(fp) != '"')
		return gripe("Malformed string in field %d", i + 1);
	    if (!xuesc(buf))
		return gripe("Invalid escape sequence in field %d",
			     i + 1);
	}
	return setstr(i, buf);
    case '(':
	if (mtsymset(i, &set) < 0)
	    return -1;
	for (;;) {
	    ch = skipfs(fp);
	    if (ch == EOF || ch == '\n')
		return gripe("Unmatched '(' in field %d", i + 1);
	    if (ch == ')')
		break;
	    ungetc(ch, fp);
	    if (getid(fp, buf) < 0)
		return gripe("Junk in field %d", i + 1);
	    if (add2symset(i, &set, buf) < 0)
		return -1;
	}
	return setnum(i, set);
    default:
	ungetc(ch, fp);
	if (getid(fp, buf) < 0)
	    return gripe("Junk in field %d", i + 1);
	if (!strcmp(buf, "nil"))
	    return setstr(i, NULL);
	else
	    return setsym(i, buf);
    }
}

static int
xuflds(FILE *fp, int (*parse)(FILE *, int))
{
    int i, ch, res;

    for (i = 0; ; i++) {
	res = parse(fp, i);
	if (res < 0)
	    return -1;
	if (res == 0)
	    return i;
	ch = getc(fp);
	if (ch == '\n')
	    ungetc(ch, fp);
	else if (ch != ' ' && ch != '\t')
	    return gripe("Bad field separator after field %d", i + 1);
    }
}

static int
deffld(int fldidx, char *name, int idx)
{
    struct castr *ca = ef_cadef(cur_type);
    int res;

    if (nxt_sel < 0)
	return gripe("Too many fields, expected only %d", fldidx);

    res = stmtch(name, ca, offsetof(struct castr, ca_name),
		     sizeof(struct castr));
    if (ca[nxt_sel].ca_type != NSC_STRINGY && ca[nxt_sel].ca_len != 0) {
	if (res != nxt_sel || idx != nxt_idx)
	    return gripe("Expected %s(%d) in field %d",
			 ca[nxt_sel].ca_name, nxt_idx, fldidx + 1);
    } else {
	if (res != nxt_sel || idx >= 0)
	    return gripe("Expected %s in field %d",
			 ca[nxt_sel].ca_name, fldidx + 1);
    }

    nxtfld();
    return 1;
}

static void
nxtfld(void)
{
    struct castr *ca = ef_cadef(cur_type);
    unsigned len = ca[nxt_sel].ca_type == NSC_STRINGY ? 0 : ca[nxt_sel].ca_len;	/* FIXME ugly */

    nxt_idx++;
    if ((unsigned)nxt_idx >= len) {
	nxt_idx = 0;
	for (;;) {
	    nxt_sel++;
	    if (!ca[nxt_sel].ca_name) {
		nxt_sel = -1;
		break;
	    }
	    if (!(ca[nxt_sel].ca_flags & NSC_EXTRA))
		break;
	}
    }
}

static struct castr *
ca4fld(int fldidx)
{
    struct castr *ca = ef_cadef(cur_type);
    unsigned len = ca[nxt_sel].ca_type == NSC_STRINGY ? 0 : ca[nxt_sel].ca_len;	/* FIXME ugly */

    if (nxt_sel < 0 || CANT_HAPPEN(nxt_idx && (unsigned)nxt_idx >= len)) {
	gripe("Too many fields, expected only %d", fldidx);
	return NULL;
    }
    return &ca[nxt_sel];
}

static int
setnum(int fldidx, double dbl)
{
    struct castr *ca = ca4fld(fldidx);
    char *memb_ptr;
    double old;

    if (!ca)
	return -1;

    memb_ptr = cur_obj;
    memb_ptr += ca->ca_off;
    switch (ca->ca_type) {
    case NSC_CHAR:
    case NSC_TYPEID:
	old = ((signed char *)memb_ptr)[nxt_idx];
	((signed char *)memb_ptr)[nxt_idx] = (signed char)dbl;
	break;
    case NSC_UCHAR:
	old = ((unsigned char *)memb_ptr)[nxt_idx];
	((unsigned char *)memb_ptr)[nxt_idx] = (unsigned char)dbl;
	break;
    case NSC_SHORT:
	old = ((short *)memb_ptr)[nxt_idx];
	((short *)memb_ptr)[nxt_idx] = (short)dbl;
	break;
    case NSC_USHORT:
	old = ((unsigned short *)memb_ptr)[nxt_idx];
	((unsigned short *)memb_ptr)[nxt_idx] = (unsigned short)dbl;
	break;
    case NSC_INT:
	old = ((int *)memb_ptr)[nxt_idx];
	((int *)memb_ptr)[nxt_idx] = (int)dbl;
	break;
    case NSC_LONG:
	old = ((long *)memb_ptr)[nxt_idx];
	((long *)memb_ptr)[nxt_idx] = (long)dbl;
	break;
    case NSC_XCOORD:
	old = ((coord *)memb_ptr)[nxt_idx];
	((coord *)memb_ptr)[nxt_idx] = XNORM((coord)dbl);
	break;
    case NSC_YCOORD:
	old = ((coord *)memb_ptr)[nxt_idx];
	((coord *)memb_ptr)[nxt_idx] = YNORM((coord)dbl);
	break;
    case NSC_FLOAT:
	old = ((float *)memb_ptr)[nxt_idx];
	((float *)memb_ptr)[nxt_idx] = (float)dbl;
	break;
    case NSC_DOUBLE:
	old = ((double *)memb_ptr)[nxt_idx];
	((double *)memb_ptr)[nxt_idx] = dbl;
	break;
    case NSC_TIME:
	old = ((time_t *)memb_ptr)[nxt_idx];
	((time_t *)memb_ptr)[nxt_idx] = (time_t)dbl;
	break;
    default:
	return gripe("Field %d doesn't take numbers", fldidx + 1);
    }

    if ((ca->ca_flags & NSC_CONST) && old != dbl)
	return gripe("Value for field %d must be %g", fldidx + 1, old);

    nxtfld();
    return 1;
}

static int
setstr(int fldidx, char *str)
{
    struct castr *ca = ca4fld(fldidx);
    char *memb_ptr, *old;

    if (!ca)
	return -1;

    memb_ptr = cur_obj;
    memb_ptr += ca->ca_off;
    switch (ca->ca_type) {
    case NSC_STRING:
	old = ((char **)memb_ptr)[nxt_idx];
	if (!(ca->ca_flags & NSC_CONST))
	    ((char **)memb_ptr)[nxt_idx] = str ? strdup(str) : NULL;
	break;
    case NSC_STRINGY:
	CANT_HAPPEN(nxt_idx);
	if (!str)
	    return gripe("Field doesn't take nil");
	if (strlen(str) > ca->ca_len)
	    return gripe("Field %d takes at most %d characters",
			 fldidx + 1, ca->ca_len);
	old = memb_ptr;
	if (!(ca->ca_flags & NSC_CONST))
	    strncpy(memb_ptr, str, ca->ca_len);
	break;
    default:
	return gripe("Field %d doesn't take strings", fldidx + 1);
    }

    if ((ca->ca_flags & NSC_CONST) && strcmp(old, str))
	    return gripe("Value for field %d must be \"%s\"", fldidx + 1, old);

    nxtfld();
    return 1;
}

static int
xunsymbol1(char *id, struct symbol *symtab, struct castr *ca, int n)
{
    int i = stmtch(id, symtab, offsetof(struct symbol, name),
		   sizeof(struct symbol));
    if (i < 0)
	return gripe("%s %s symbol `%s' in field %d",
		     i == M_NOTUNIQUE ? "Ambiguous" : "Unknown",
		     ca->ca_name, id, n);
    return i;
}

static int
setsym(int fldidx, char *sym)
{
    struct castr *ca = ca4fld(fldidx);
    struct symbol *symtab;
    int i;

    if (!ca)
	return -1;

    symtab = get_symtab(ca);
    if (!symtab || (ca->ca_flags & NSC_BITS))
	return gripe("Field %d doesn't take symbols", fldidx + 1);

    i = xunsymbol1(sym, symtab, ca, fldidx);
    if (i < 0)
	return -1;
    return setnum(fldidx, symtab[i].value);
}

static int
has_const(struct castr ca[])
{
    int i;

    for (i = 0; ca[i].ca_name; i++) {
	if (ca[i].ca_flags & NSC_CONST)
	    return 1;
    }
    return 0;
}

static int
mtsymset(int fldidx, long *set)
{
    struct castr *ca = ca4fld(fldidx);
    struct symbol *symtab;
    int i;

    if (!ca)
	return -1;

    symtab = get_symtab(ca);
    if (!symtab || !(ca->ca_flags & NSC_BITS)) {
	return gripe("Field %d doesn't take symbol sets", fldidx + 1);
    }
    *set = 0;
    return 0;
}

static int
add2symset(int fldidx, long *set, char *sym)
{
    struct castr *ca = ca4fld(fldidx);
    struct symbol *symtab;
    int i;

    if (!ca)
	return -1;

    symtab = get_symtab(ca);
    i = xunsymbol1(sym, symtab, ca, fldidx);
    if (i < 0)
	return -1;
    *set |= symtab[i].value;
    return 0;
}

static struct symbol *
get_symtab(struct castr *ca)
{
    int symtype = ca->ca_table;
    struct symbol *symtab;

    if (symtype == EF_BAD || ef_cadef(symtype) != symbol_ca)
	return NULL;

    symtab = ef_ptr(symtype, 0);
    CANT_HAPPEN(!symtab);
    return symtab;
}

static int
xuheader(FILE *fp, int expected_table)
{
    char name[64];
    int res, ch;
    int type;

    while ((ch = skipfs(fp)) == '\n')
	lineno++;
    if (ch == EOF && expected_table == EF_BAD)
	return -1;
    ungetc(ch, fp);

    human = ch == 'c';
    res = -1;
    if ((human
         ? fscanf(fp, "config%*[ \t]%63[^ \t#\n]%n", name, &res) != 1
         : fscanf(fp, "XDUMP%*[ \t]%63[^ \t#\n]%*[ \t]%*[^ \t#\n]%n",
                  name, &res) != 1) || res < 0)
	return gripe("Expected xdump header");

    type = ef_byname(name);
    if (type < 0)
	return gripe("Unknown table `%s'", name);
    if (expected_table != EF_BAD && expected_table != type)
	return gripe("Expected table `%s', not `%s'",
		     ef_nameof(expected_table), name);

    if (CANT_HAPPEN(!(ef_flags(type) & EFF_MEM)))
	return -1;

    if (skipfs(fp) != '\n')
	return gripe("Junk after xdump header");
    lineno++;

    return type;
}

static int
xutrailer(FILE *fp, int type, int row)
{
    int rows, ch, res;

    res = -1;
    if (human) {
	if (fscanf(fp, "config%n",  &res) != 0 || res < 0)
	    return gripe("Malformed table footer");
    } else {
	if (fscanf(fp, "%d", &rows) != 1)
	    return gripe("Malformed table footer");
	if (row != rows)
	    return gripe("Read %d rows, which doesn't match footer "
			 "%d rows", row, rows);
    }
    if (skipfs(fp) != '\n')
	return gripe("Junk after table footer");
    lineno++;

    return 0;
}

int
xundump(FILE *fp, char *file, int expected_table)
{
    struct empfile *ep;
    int type, fixed_rows, need_sentinel;
    int row, res, ch;

    if (fname != file) {
        fname = file;
	lineno = 1;
    }

    if ((type = xuheader(fp, expected_table)) < 0)
	return -1;

    ep = &empfile[type];
    fixed_rows = has_const(ef_cadef(type));
    need_sentinel = !EF_IS_GAME_STATE(type);

    cur_type = type;
    if (human) {
	nxt_sel = nxt_idx = 0;
	if (xuflds(fp, xufldname) < 0)
	    return -1;
    }

    for (row = 0;; ++row) {
	ch = skipfs(fp);
	if (ch == '/')
	    break;
	ungetc(ch, fp);
	/* TODO ability to skip records */
	if (!fixed_rows) {
	    if (row >= ep->csize - !!need_sentinel)
		/* TODO grow cache unless EFF_STATIC */
		return gripe("Too many rows for table %s", ef_nameof(type));
	    if (row >= ep->cids)
		/* TODO grow file */
		ep->cids = ep->fids = row + 1;
	}
	cur_obj = ef_ptr(type, row);
	if (!cur_obj)
	    return gripe("Too many rows for table %s", ef_nameof(type));
	nxt_sel = nxt_idx = 0;
	res = xuflds(fp, xufld);
	if (res < 0)
	    return -1;
    }
    if (row != ep->fids) {
	if (fixed_rows)
	    return gripe("Table %s requires %d rows, got %d",
			 ef_nameof(type), ep->fids, row);
	else {
	    ep->cids = ep->fids = row;
	    if (EF_IS_GAME_STATE(type) && row != ep->csize)
		/* TODO truncate file */
		gripe("Warning: should resize table %s from %d to %d, not implemented",
		      ef_nameof(type), ep->csize, row);
	}
    }

    if (need_sentinel) {
	if (CANT_HAPPEN(row >= ep->csize))
	    return gripe("No space for sentinel");
	memset(ep->cache + ep->size * row, 0, ep->size);
    }

    if (xutrailer(fp, type, row) < 0)
	return -1;
    
    return type;
}
