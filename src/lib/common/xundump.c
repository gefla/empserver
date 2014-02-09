/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
 *     Markus Armbruster, 2005-2014
 */

/*
 * See doc/xdump!  And keep it up-to-date.
 *
 * Parsing of machine-readable xdump is not precise: it recognizes
 * comments, accepts whitespace in place of single space, and accepts
 * the full human-readable field syntax instead of its machine-
 * readable subset.
 *
 * FIXME:
 * - Normalize terminology: table/rows/columns or file/records/fields
 * - Loading tables with NSC_STRING elements more than once leaks memory
 * TODO:
 * - Symbolic array indexes
 * - Option to treat missing and unknown fields as warning, not error
 * TODO, but hardly worth the effort:
 * - Permit reordering of array elements
 */

#include <config.h>

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "file.h"
#include "match.h"
#include "nat.h"
#include "nsc.h"
#include "optlist.h"
#include "xdump.h"

static char *fname;		/* Name of file being read */
static int lineno;		/* Current line number */

static int cur_type;		/* Current table's file type */
static int partno;		/* Counts from 0..#parts-1 */
static int cur_id;		/* and its index in the table */
static int old_nelem;
static unsigned char *idgap;	/* idgap && idgap[ID] iff part#0 lacks ID */
static int idgap_len;		/* #elements in idgap[] */

static int human;		/* Reading human-readable syntax? */
static int ellipsis;		/* Header ended with ...? */
static int nflds;		/* #fields in input records */
static struct castr **fldca;	/* Map field number to selector */
static int *fldidx;		/* Map field number to index */
static struct valstr *fldval;	/* Map field number to value */
static int *caflds;		/* Map selector number to #fields seen */
static int *cafldspp;		/* ditto, in previous parts */

static int gripe(char *, ...) ATTRIBUTE((format (printf, 1, 2)));
static int deffld(int, char *, int);
static int chkflds(void);
static int setnum(int, double);
static int setstr(int, char *);
static int setsym(int, char *);
static int mtsymset(int, long *);
static int add2symset(int, long *, char *);
static int xubody(FILE *);
static int xutail(FILE *, struct castr *);

/*
 * Does the code hardcode indexes for table TYPE?
 */
static int
have_hardcoded_indexes(int type)
{
    return type == EF_ITEM || type == EF_SECTOR_CHR
	|| type == EF_INFRASTRUCTURE;
}

/*
 * Okay to truncate table TYPE?
 */
static int
may_truncate(int type)
{
    return empfile[type].nent < 0 && !have_hardcoded_indexes(type);
}

/*
 * Is TYPE's 0-th selector a usable ID?
 */
static int
ca0_is_id(int type)
{
    struct castr *ca = ef_cadef(type);

    return ca[0].ca_table == type && !(ca[0].ca_flags & NSC_EXTRA);
}

/*
 * Can we fill in gaps in table TYPE?
 */
static int
can_fill_gaps(int type)
{
    return (ca0_is_id(type) || type == EF_SECTOR || type == EF_REALM)
	&& !have_hardcoded_indexes(type);
}

/*
 * Gripe about the current line to stderr, return -1.
 */
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

/* Make TYPE the current table.  */
static void
tbl_start(int type)
{
    cur_type = type;
    partno = 0;
    cur_id = -1;
    old_nelem = type == EF_BAD ? 0 : ef_nelem(type);
    idgap = NULL;
    idgap_len = 0;
}

/* End the current table.  */
static void
tbl_end(void)
{
    free(idgap);
    tbl_start(EF_BAD);
}

/*
 * Seek to current table's ID-th object.
 * Extend the table if necessary.
 * Save ID in cur_id.
 * Return the object on success, NULL on failure.
 */
static void *
tbl_seek(int id)
{
    struct empfile *ep = &empfile[cur_type];
    void *obj;

    if (id >= ef_nelem(cur_type)) {
	if (!ef_ensure_space(cur_type, id, 1)) {
	    gripe("Can't put ID %d into table %s", id, ep->name);
	    return NULL;
	}
    }

    obj = ef_ptr(cur_type, id);
    if (CANT_HAPPEN(!obj))
	return NULL;
    cur_id = id;
    return obj;
}

/*
 * Omit ID1..ID2-1.
 * Reset the omitted objects to default state.
 */
static void
omit_ids(int id1, int id2)
{
    int i;

    if (id1 >= id2)
	return;

    idgap = realloc(idgap, id2 * sizeof(*idgap));
    for (i = idgap_len; i < id1; i++)
	idgap[i] = 0;
    for (i = id1; i < id2; i++) {
	ef_blank(cur_type, i, ef_ptr(cur_type, i));
	idgap[i] = 1;
    }
    idgap_len = id2;
}

/*
 * Return the smallest non-omitted ID in ID1..ID2-1 if any, else -1.
 */
static int
expected_id(int id1, int id2)
{
    int i;

    for (i = id1; i < id2; i++) {
	if (i >= idgap_len || !idgap[i])
	    return i;
    }
    return -1;
}

/*
 * Finish table part.
 * If the table has variable length, truncate it.
 * Else ensure we're omitting the same objects as the previous parts.
 * Reset any omitted objects to default state.
 * Return 0 on success, -1 on failure.
 */
static int
tbl_part_done(void)
{
    struct empfile *ep = &empfile[cur_type];

    if (cur_id + 1 < ep->fids) {
	if (partno == 0) {
	    if (may_truncate(cur_type)) {
		if (!ef_truncate(cur_type, cur_id + 1))
		    return -1;
	    } else {
		if (!can_fill_gaps(cur_type))
		    return gripe("Expected %d more rows",
				 ep->fids - (cur_id + 1));
		omit_ids(cur_id + 1, ep->fids);
	    }
	} else {
	    if (expected_id(cur_id + 1, ep->fids) >= 0)
		return gripe("Table's first part has more rows");
	}
    }

    partno++;
    cur_id = -1;
    return 0;
}

/*
 * Get selector for field FLDNO.
 * Assign the field's selector index to *IDX, unless it is null.
 * Return the selector on success, null pointer on error.
 */
static struct castr *
getfld(int fldno, int *idx)
{
    if (fldno >= nflds) {
	gripe("Too many fields, expected only %d", nflds);
	return NULL;
    }
    if (CANT_HAPPEN(fldno < 0))
	return NULL;
    if (idx)
	*idx = fldidx[fldno];
    return fldca[fldno];
}

/*
 * Find the field for selector CA with index IDX.
 * Return the field number if it exists, else -1.
 */
static int
fld_find(struct castr *ca, int idx)
{
    int i;

    for (i = 0; i < nflds; i++) {
	if (fldca[i] == ca && fldidx[i] == idx)
	    return i;
    }
    return -1;
}

/*
 * Get the current row's ID.
 * Current table's 0-th selector must be a usable ID.
 * Return ID on success, -1 on failure.
 */
static int
rowid(void)
{
    struct castr *ca = ef_cadef(cur_type);
    int fldno, id, max_id;

    if (CANT_HAPPEN(partno != 0 || !ca0_is_id(cur_type)))
	return -1;

    fldno = fld_find(ca, 0);
    if (fldno < 0)
	return cur_id + 1;	/* ID not specified */
    /*
     * Field values not representable as int will be rejected by
     * putnum() or putstr().  Leave the error reporting to them, and
     * simply pick the next ID here.
     */
    if (fldval[fldno].val_type != NSC_DOUBLE)
	return cur_id + 1;
    id = fldval[fldno].val_as.dbl;
    if (id != fldval[fldno].val_as.dbl)
	return cur_id + 1;

    if (id != cur_id + 1 && !can_fill_gaps(cur_type))
	return gripe("Expected %d in field %d",
		     cur_id + 1, fldno + 1);
    if (id <= cur_id)
	return gripe("Field %d must be > %d", fldno + 1, cur_id);
    max_id = ef_id_limit(cur_type);
    if (id > max_id)
	return gripe("Field %d must be <= %d", fldno + 1, max_id);

    return id;
}

/*
 * Find the field NAME with index IDX and value representable as long.
 * Return the field number if it exists, else -1.
 */
static int
fld_find_long_by_name(char *name, int idx)
{
    int i;

    for (i = 0; i < nflds; i++) {
	if (!strcmp(fldca[i]->ca_name, name) && fldidx[i] == idx)
	    break;
    }

    if (i == nflds || fldval[i].val_type != NSC_DOUBLE
	|| (long)fldval[i].val_as.dbl != fldval[i].val_as.dbl)
	return -1;
    return i;
}

/*
 * Get the current row's ID.
 * Current table's type must be EF_SECTOR.
 * Return ID on success, -1 on failure.
 */
static int
rowid_sect(void)
{
    int fldno_x, fldno_y, id;
    coord x, y;

    if (CANT_HAPPEN(partno != 0 || cur_type != EF_SECTOR))
	return -1;

    fldno_x = fld_find_long_by_name("xloc", 0);
    fldno_y = fld_find_long_by_name("yloc", 0);
    if (fldno_x < 0 || fldno_y < 0)
	return cur_id + 1;

    id = sctoff((long)fldval[fldno_x].val_as.dbl,
		(long)fldval[fldno_y].val_as.dbl);
    /* Note: reporting values out of range left to putnum() */
    if (id <= cur_id) {
	sctoff2xy(&x, &y, cur_id);
	return gripe("Coordinates in fields %d,%d must be > %d,%d",
		     fldno_x + 1, fldno_y + 1, x, y);
    }
    return id;
}

/*
 * Get the current row's ID.
 * Current table's type must be EF_REALM.
 * Return ID on success, -1 on failure.
 */
static int
rowid_realm(void)
{
    int fldno_cnum, fldno_realm, id;
    long realm, cnum;

    if (CANT_HAPPEN(partno != 0 || cur_type != EF_REALM))
	return -1;

    fldno_cnum = fld_find_long_by_name("cnum", 0);
    fldno_realm = fld_find_long_by_name("realm", 0);
    if (fldno_cnum < 0 || fldno_realm < 0)
	return cur_id + 1;

    realm = (long)fldval[fldno_realm].val_as.dbl;
    cnum = (long)fldval[fldno_cnum].val_as.dbl;
    if (cnum < 0 || cnum >= MAXNOC)
	return gripe("Field %d must be between 0 and %d",
		     fldno_cnum, MAXNOC);
    if (realm < 0 || realm >= MAXNOR)
	return gripe("Field %d must be between 0 and %d",
		     fldno_realm, MAXNOR);
    id = realm + cnum * MAXNOR;
    if (id <= cur_id)
	return gripe("Fields %d,%d must be > (%d,%d)",
		     fldno_cnum + 1, fldno_realm + 1,
		     cur_id / MAXNOR, cur_id % MAXNOR);
    return id;
}
/*
 * Get the current row's object.
 * Extend the table if necessary.
 * Save ID in cur_id.
 * Return the object on success, NULL on failure.
 */
static void *
rowobj(void)
{
    int last_id = cur_id;
    int id;
    void *obj;

    if (partno) {
	id = expected_id(cur_id + 1, empfile[cur_type].fids);
	if (id < 0) {
	    gripe("Table's first part doesn't have this row");
	    return NULL;
	}
    } else if (ca0_is_id(cur_type)) {
	id = rowid();
	if (id < 0)
	    return NULL;
    } else if (cur_type == EF_SECTOR) {
	id = rowid_sect();
	if (id < 0)
	    return NULL;
    } else if (cur_type == EF_REALM) {
	id = rowid_realm();
	if (id < 0)
	    return NULL;
    } else
	id = last_id + 1;
    if (id > ef_id_limit(cur_type)) {
	gripe("Too many rows");
	return NULL;
    }

    obj = tbl_seek(id);
    if (obj && !partno)
	omit_ids(last_id + 1, id);
    return obj;
}

/*
 * Is a new value for field FLDNO required to match the old one?
 */
static int
fldval_must_match(int fldno)
{
    struct castr *ca = ef_cadef(cur_type);
    int i = fldca[fldno] - ca;

    /*
     * Value must match if:
     * it's for a const selector, unless the object is still blank, or
     * it was already given in a previous part of a split table.
     */
    return (cur_id < old_nelem && (fldca[fldno]->ca_flags & NSC_CONST))
	|| fldidx[fldno] < cafldspp[i];
}

/*
 * Set OBJ's field FLDNO to DBL.
 * Return 0 on success, -1 on error.
 */
static int
putnum(void *obj, int fldno, double dbl)
{
    struct castr *ca = fldca[fldno];
    int idx = fldidx[fldno];
    char *memb_ptr;
    double old, new;

    memb_ptr = (char *)obj + ca->ca_off;

    switch (ca->ca_type) {
    case NSC_CHAR:
	old = ((signed char *)memb_ptr)[idx];
	new = ((signed char *)memb_ptr)[idx] = (signed char)dbl;
	break;
    case NSC_UCHAR:
	old = ((unsigned char *)memb_ptr)[idx];
	new = ((unsigned char *)memb_ptr)[idx] = (unsigned char)dbl;
	break;
    case NSC_SHORT:
	old = ((short *)memb_ptr)[idx];
	new = ((short *)memb_ptr)[idx] = (short)dbl;
	break;
    case NSC_USHORT:
	old = ((unsigned short *)memb_ptr)[idx];
	new = ((unsigned short *)memb_ptr)[idx] = (unsigned short)dbl;
	break;
    case NSC_INT:
	old = ((int *)memb_ptr)[idx];
	new = ((int *)memb_ptr)[idx] = (int)dbl;
	break;
    case NSC_LONG:
	old = ((long *)memb_ptr)[idx];
	new = ((long *)memb_ptr)[idx] = (long)dbl;
	break;
    case NSC_XCOORD:
	old = ((coord *)memb_ptr)[idx];
	/* FIXME use variant of xrel() that takes orig instead of nation */
	if (old >= WORLD_X / 2)
	    old -= WORLD_X;
	new = ((coord *)memb_ptr)[idx] = XNORM((coord)dbl);
	if (new >= WORLD_X / 2)
	    new -= WORLD_X;
	break;
    case NSC_YCOORD:
	old = ((coord *)memb_ptr)[idx];
	/* FIXME use variant of yrel() that takes orig instead of nation */
	if (old >= WORLD_Y / 2)
	    old -= WORLD_Y;
	new = ((coord *)memb_ptr)[idx] = YNORM((coord)dbl);
	if (new >= WORLD_Y / 2)
	    new -= WORLD_Y;
	break;
    case NSC_FLOAT:
	old = ((float *)memb_ptr)[idx];
	((float *)memb_ptr)[idx] = (float)dbl;
	new = dbl;		/* suppress new != dbl check */
	break;
    case NSC_DOUBLE:
	old = ((double *)memb_ptr)[idx];
	((double *)memb_ptr)[idx] = dbl;
	new = dbl;		/* suppress new != dbl check */
	break;
    case NSC_TIME:
	old = ((time_t *)memb_ptr)[idx];
	new = ((time_t *)memb_ptr)[idx] = (time_t)dbl;
	break;
    default:
	return gripe("Field %d doesn't take numbers", fldno + 1);
    }

    if (fldval_must_match(fldno) && old != dbl)
	return gripe("Value for field %d must be %g", fldno + 1, old);
    if (new != dbl)
	return gripe("Field %d can't hold this value", fldno + 1);

    return 0;
}

/*
 * Set obj's field FLDNO to STR.
 * Return 0 on success, -1 on error.
 */
static int
putstr(void *obj, int fldno, char *str)
{
    struct castr *ca = fldca[fldno];
    int idx = fldidx[fldno];
    int must_match, mismatch;
    size_t sz, len;
    char *memb_ptr, *old;

    memb_ptr = (char *)obj + ca->ca_off;
    must_match = fldval_must_match(fldno);
    mismatch = 0;

    switch (ca->ca_type) {
    case NSC_STRING:
	old = ((char **)memb_ptr)[idx];
	if (must_match)
	    mismatch = old ? !str || strcmp(old, str) : !!str;
	else
	    /* FIXME may leak old value */
	    ((char **)memb_ptr)[idx] = str ? strdup(str) : NULL;
	len = -1;		/* unlimited */
	break;
    case NSC_STRINGY:
	if (CANT_HAPPEN(idx))
	    return -1;
	if (!str)
	    return gripe("Field %d doesn't take nil", fldno + 1);
	/* Wart: if ca_len <= 1, the terminating null may be omitted */
	sz = ca->ca_len;
	len = sz > 1 ? sz - 1 : sz;
	if (strlen(str) > len)
	    return gripe("Field %d takes at most %d characters",
			 fldno + 1, (int)len);
	old = memb_ptr;
	if (must_match)
	    mismatch = !str || strncmp(old, str, len);
	else
	    strncpy(memb_ptr, str, sz);
	break;
    default:
	return gripe("Field %d doesn't take strings", fldno + 1);
    }

    if (mismatch) {
	if (old)
	    return gripe("Value for field %d must be \"%.*s\"",
			 fldno + 1, (int)len, old);
	else
	    return gripe("Value for field %d must be nil", fldno + 1);
    }

    return 0;
}

/*
 * Save the current row's fields in its object.
 * Return 0 on success, -1 on failure.
 */
static int
putrow(void)
{
    int i, ret = 0;
    void *obj;

    obj = rowobj();
    if (!obj)
	return -1;

    for (i = 0; i < nflds; i++) {
	switch (fldval[i].val_type) {
	case NSC_DOUBLE:
	    ret |= putnum(obj, i, fldval[i].val_as.dbl);
	    break;
	case NSC_STRING:
	    ret |= putstr(obj, i, fldval[i].val_as.str.base);
	    free(fldval[i].val_as.str.base);
	    break;
	default:
	    CANT_REACH();
	    ret = -1;
	}
    }

    return ret;
}

/*
 * Read and ignore field separators from FP.
 * Return first character that is not a field separator.
 */
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

/*
 * Decode escape sequences in BUF.
 * Return BUF on success, null pointer on failure.
 */
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

/*
 * Read an identifier from FP into BUF.
 * BUF must have space for 1024 characters.
 * Return number of characters read on success, -1 on failure.
 */
static int
getid(FILE *fp, char *buf)
{
    int n;
    if (fscanf(fp, "%1023[^\"#()<>= \t\n]%n", buf, &n) != 1
	|| !isalpha(buf[0]))
	return -1;
    xuesc(buf);
    return n;
}

/*
 * Try to read a field name from FP.
 * I is the field number, counting from zero.
 * If a name is read, set fldca[I] and fldidx[I] for it, and update
 * caflds[].
 * Return 1 if a name or ... was read, 0 on end of line, -1 on error.
 */
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
	nflds = i - (ellipsis != 0);
	if (chkflds() < 0)
	    return -1;
	lineno++;
	return 0;
    case '.':
	if (getc(fp) != '.' || getc(fp) != '.')
	    return gripe("Junk in header field %d", i + 1);
	if (i == 0)
	    return gripe("Header fields expected");
	ellipsis = 1;
	ch = skipfs(fp);
	if (ch != EOF && ch != '\n')
	    return gripe("Junk after ...");
	ungetc(ch, fp);
	return 1;
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
	    if (fscanf(fp, "%d", &idx) != 1)
		return gripe("Malformed number in index of header field %d",
			     i + 1);
	    if (idx < 0)
		return gripe("Index must not be negative in header field %d",
			     i + 1);
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

/*
 * Try to read a field value from FP.
 * I is the field number, counting from zero.
 * Return 1 if a value was read, 0 on end of line, -1 on error.
 */
static int
xufld(FILE *fp, int i)
{
    int ch, j;
    char buf[1024];
    double dbl;
    long set;

    ch = skipfs(fp);
    switch (ch) {
    case EOF:
	return gripe("Unexpected EOF");
    case '\n':
	CANT_HAPPEN(i > nflds);
	for (j = i; j < nflds; j++) {
	    if (CA_IS_ARRAY(fldca[j]))
		gripe("Field %s(%d) missing",
		      fldca[j]->ca_name, fldidx[j]);
	    else
		gripe("Field %s missing", fldca[j]->ca_name);
	}
	if (i != nflds || putrow() < 0)
	    return -1;
	lineno++;
	return i < nflds ? -1 : 0;
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

/*
 * Read fields from FP.
 * Use PARSE() to read each field.
 * Return number of fields read on success, -1 on error.
 */
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

/*
 * Define the FLDNO-th field.
 * If IDX is negative, define as selector NAME, else as NAME(IDX).
 * Set fldca[FLDNO] and fldidx[FLDNO] accordingly.
 * Update caflds[].
 * Return 1 on success, -1 on error.
 */
static int
deffld(int fldno, char *name, int idx)
{
    struct castr *ca = ef_cadef(cur_type);
    int res;

    res = stmtch(name, ca, offsetof(struct castr, ca_name),
		 sizeof(struct castr));
    if (res < 0)
	return gripe("Header %s of field %d is %s", name, fldno + 1,
		     res == M_NOTUNIQUE ? "ambiguous" : "unknown");
    if ((ca[res].ca_flags & NSC_EXTRA) || CANT_HAPPEN(ca[res].ca_get))
	return gripe("Extraneous header %s in field %d", name, fldno + 1);
    if (CA_IS_ARRAY(&ca[res])) {
	if (idx < 0)
	    return gripe("Header %s requires an index in field %d",
			 ca[res].ca_name, fldno + 1);
	if (idx >= ca[res].ca_len)
	    return gripe("Header %s(%d) index out of bounds in field %d",
			 ca[res].ca_name, idx, fldno + 1);
	if (idx < caflds[res])
	    return gripe("Duplicate header %s(%d) in field %d",
			 ca[res].ca_name, idx, fldno + 1);
	if (idx > caflds[res])
	    return gripe("Expected header %s(%d) in field %d",
			 ca[res].ca_name, caflds[res], fldno + 1);
    } else {
	if (idx >= 0)
	    return gripe("Header %s doesn't take an index in field %d",
			 ca[res].ca_name, fldno + 1);
	idx = 0;
	if (caflds[res])
	    return gripe("Duplicate header %s in field %d",
			 ca[res].ca_name, fldno + 1);
    }
    fldca[fldno] = &ca[res];
    fldidx[fldno] = idx;
    caflds[res]++;
    return 1;
}

/*
 * Check fields in xdump are sane.
 * Return 0 on success, -1 on error.
 */
static int
chkflds(void)
{
    struct castr *ca = ef_cadef(cur_type);
    int i, len, cafldsmax, res = 0;

    if (ellipsis)
	return res;		/* table is split, another part expected */

    /* Check for missing fields */
    for (i = 0; ca[i].ca_name; i++) {
	cafldsmax = MAX(caflds[i], cafldspp[i]);
	if (ca[i].ca_flags & NSC_EXTRA)
	    continue;
	len = CA_ARRAY_LEN(&ca[i]);
	if (!len && !cafldsmax)
	    res = gripe("Header field %s missing", ca[i].ca_name);
	else if (len && cafldsmax == len - 1)
	    res = gripe("Header field %s(%d) missing",
			ca[i].ca_name, len - 1);
	else if (len && cafldsmax < len - 1)
	    res = gripe("Header fields %s(%d) ... %s(%d) missing",
			ca[i].ca_name, cafldsmax, ca[i].ca_name, len - 1);
    }

    return res;
}

/*
 * Set value of field FLDNO in current row to DBL.
 * Return 1 on success, -1 on error.
 */
static int
setnum(int fldno, double dbl)
{
    if (!getfld(fldno, NULL))
	return -1;
    fldval[fldno].val_cat = NSC_VAL;
    fldval[fldno].val_type = NSC_DOUBLE;
    fldval[fldno].val_as.dbl = dbl;
    return 1;
}

/*
 * Set value of field FLDNO in current row to STR.
 * Return 1 on success, -1 on error.
 */
static int
setstr(int fldno, char *str)
{
    if (!getfld(fldno, NULL))
	return -1;
    fldval[fldno].val_cat = NSC_VAL;
    fldval[fldno].val_type = NSC_STRING;
    fldval[fldno].val_as.str.base = str ? strdup(str) : NULL;
    fldval[fldno].val_as.str.maxsz = INT_MAX;
				/* really SIZE_MAX, but that's C99 */
    return 1;
}

/*
 * Resolve symbol name ID in table referred to by CA.
 * Use field number N for error messages.
 * Return index in referred table on success, -1 on failure.
 */
static int
xunsymbol(char *id, struct castr *ca, int n)
{
    int i = ef_elt_byname(ca->ca_table, id);
    if (i < 0)
	return gripe("%s %s symbol `%s' in field %d",
		     i == M_NOTUNIQUE ? "Ambiguous" : "Unknown",
		     ca->ca_name, id, n + 1);
    return i;
}

/*
 * Map symbol index to symbol value.
 * CA is the table, and I is the index in it.
 */
static int
symval(struct castr *ca, int i)
{
    int type = ca->ca_table;

    if (type != EF_BAD && ef_cadef(type) == symbol_ca)
	/* symbol table, value is in the table */
	return ((struct symbol *)ef_ptr(type, i))->value;
    /* value is the table index */
    return i;
}

/*
 * Set value of field FLDNO in current object to value of symbol SYM.
 * Return 1 on success, -1 on error.
 */
static int
setsym(int fldno, char *sym)
{
    struct castr *ca;
    int i;

    ca = getfld(fldno, NULL);
    if (!ca)
	return -1;

    if (ca->ca_table == EF_BAD || (ca->ca_flags & NSC_BITS))
	return gripe("Field %d doesn't take symbols", fldno + 1);

    i = xunsymbol(sym, ca, fldno);
    if (i < 0)
	return -1;
    return setnum(fldno, symval(ca, i));
}

/*
 * Create an empty symbol set for field FLDNO in *SET.
 * Return 1 on success, -1 on error.
 */
static int
mtsymset(int fldno, long *set)
{
    struct castr *ca;

    ca = getfld(fldno, NULL);
    if (!ca)
	return -1;

    if (ca->ca_table == EF_BAD || ef_cadef(ca->ca_table) != symbol_ca
	|| !(ca->ca_flags & NSC_BITS))
	return gripe("Field %d doesn't take symbol sets", fldno + 1);
    *set = 0;
    return 0;
}

/*
 * Add a symbol to a symbol set for field FLDNO in *SET.
 * SYM is the name of the symbol to add.
 * Return 1 on success, -1 on error.
 */
static int
add2symset(int fldno, long *set, char *sym)
{
    struct castr *ca;
    int i;

    ca = getfld(fldno, NULL);
    if (!ca)
	return -1;

    i = xunsymbol(sym, ca, fldno);
    if (i < 0)
	return -1;
    *set |= symval(ca, i);
    return 0;
}

/*
 * Read an xdump table header line from FP.
 * Expect header for EXPECTED_TABLE, unless it is EF_BAD.
 * Recognize header for machine- and human-readable syntax, and set
 * human accordingly.
 * Return table type on success, -2 on EOF before header, -1 on failure.
 */
static int
xuheader(FILE *fp, int expected_table)
{
    char name[64];
    int res, ch;
    int type;

    while ((ch = skipfs(fp)) == '\n')
	lineno++;
    if (ch == EOF && expected_table == EF_BAD)
	return -2;
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

    if (!empfile[type].file
	|| !ef_cadef(type) || !(ef_flags(type) & EFF_MEM)) {
	CANT_HAPPEN(expected_table != EF_BAD);
	return gripe("Table `%s' is not permitted here", name);
    }

    if (skipfs(fp) != '\n')
	return gripe("Junk after xdump header");
    lineno++;

    return type;
}

/*
 * Find fields in this xdump.
 * If reading human-readable syntax, read a field header line from FP.
 * Else take fields from the table's selectors in CA[].
 * Set ellipsis, nflds, fldca[], fldidx[] and caflds[] accordingly.
 * Return 0 on success, -1 on failure.
 */
static int
xufldhdr(FILE *fp, struct castr ca[])
{
    struct castr **fca;
    int *fidx;
    int ch, i, j, n;

    for (i = 0; ca[i].ca_name; i++)
	caflds[i] = 0;
    ellipsis = 0;

    if (human) {
	while ((ch = skipfs(fp)) == '\n')
	    lineno++;
	ungetc(ch, fp);
	if (xuflds(fp, xufldname) < 0)
	    return -1;
    } else {
	fca = fldca;
	fidx = fldidx;

	for (i = 0; ca[i].ca_name; i++) {
	    if ((ca[i].ca_flags & NSC_EXTRA))
		continue;
	    n = CA_ARRAY_LEN(&ca[i]);
	    j = 0;
	    do {
		*fca++ = &ca[i];
		*fidx++ = j;
	    } while (++j < n);
	}

	nflds = fidx - fldidx;
    }

    return 0;
}

/*
 * Read xdump footer from FP.
 * CA[] contains the table's selectors.
 * The body had RECS records.
 * Update cafldspp[] from caflds[].
 * Return 0 on success, -1 on failure.
 */
static int
xufooter(FILE *fp, struct castr ca[], int recs)
{
    int res, n, i;

    res = -1;
    if (human) {
	if (fscanf(fp, "config%n", &res) != 0 || res < 0)
	    return gripe("Malformed table footer");
    } else {
	if (fscanf(fp, "%d", &n) != 1)
	    return gripe("Malformed table footer");
	if (recs != n)
	    return gripe("Read %d rows, which doesn't match footer "
			 "%d rows", recs, n);
    }
    if (skipfs(fp) != '\n')
	return gripe("Junk after table footer");
    if (tbl_part_done() < 0)
	return -1;
    lineno++;

    for (i = 0; ca[i].ca_name; i++) {
	if (cafldspp[i] < caflds[i])
	    cafldspp[i] = caflds[i];
    }

    return 0;
}

/*
 * Read an xdump table from FP.
 * Both machine- and human-readable xdump syntax are recognized.
 * Expect table EXPECTED_TABLE, unless it is EF_BAD.
 * Report errors to stderr.
 * Messages assume FP starts in the file FILE at line *PLNO.
 * Update *PLNO to reflect lines read from FP.
 * Return table type on success, -2 on EOF before header, -1 on failure.
 */
int
xundump(FILE *fp, char *file, int *plno, int expected_table)
{
    struct castr *ca;
    int type, nca, nf, i, ch;

    fname = file;
    lineno = *plno;

    if ((type = xuheader(fp, expected_table)) < 0)
	return type;

    ca = ef_cadef(type);
    if (CANT_HAPPEN(!ca))
	return -1;

    nca = nf = 0;
    for (i = 0; ca[i].ca_name; i++) {
	nca++;
	if (!(ca[i].ca_flags & NSC_EXTRA))
	    nf += MAX(1, CA_ARRAY_LEN(&ca[i]));
    }
    fldca = malloc(nf * sizeof(*fldca));
    fldidx = malloc(nf * sizeof(*fldidx));
    fldval = malloc(nf * sizeof(*fldval));
    caflds = malloc(nca * sizeof(*caflds));
    cafldspp = calloc(nca, sizeof(*cafldspp));

    tbl_start(type);
    if (xutail(fp, ca) < 0)
	type = EF_BAD;
    tbl_end();

    free(cafldspp);
    free(caflds);
    free(fldval);
    free(fldidx);
    free(fldca);

    /* Skip empty lines so that callers can easily check for EOF */
    while ((ch = skipfs(fp)) == '\n')
	lineno++;
    ungetc(ch, fp);

    *plno = lineno;
    return type;
}

/*
 * Read the remainder of an xdump after the table header line from FP.
 * CA[] contains the table's selectors.
 * Return 0 on success, -1 on failure.
 */
static int
xutail(FILE *fp, struct castr *ca)
{
    int recs;

    for (;;) {
	if (xufldhdr(fp, ca) < 0)
	    return -1;
	if ((recs = xubody(fp)) < 0)
	    return -1;
	if (xufooter(fp, ca, recs) < 0)
	    return -1;
	if (!ellipsis)
	    return 0;
	if (xuheader(fp, cur_type) < 0)
	    return -1;
    }
}

/*
 * Read the body of an xdump table from FP.
 * Return number of rows read on success, -1 on failure.
 */
static int
xubody(FILE *fp)
{
    int i, ch;

    for (i = 0;; ++i) {
	while ((ch = skipfs(fp)) == '\n')
	    lineno++;
	if (ch == '/')
	    break;
	ungetc(ch, fp);
	if (xuflds(fp, xufld) < 0)
	    return -1;
    }
    return i;
}
