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

#include "prototypes.h"
#include "file.h"
#include "nsc.h"
#include "match.h"

#define MAX_NUM_COLUMNS 256

static char *fname = "";
static int lineno = 0;
static int human;

/*
 * TODO
 * This structure could be replaced with struct valstr.
 */
enum enum_value {
    VAL_NOTUSED,
    VAL_STRING,		/* uses v_string */
    VAL_SYMBOL,		/* uses v_string */
    VAL_SYMBOL_SET,	/* uses v_string */
    VAL_INDEX_ID,	/* uses v_index_name and v_int */
    VAL_INDEX_SYMBOL,	/* uses v_index_name and v_string */
    VAL_DOUBLE		/* uses v_double */
};

struct value {
    enum enum_value v_type;
    union {
	char *v_string;
	double v_double;
	int v_int;
    } v_field;
    char *v_index_name;
};

static int gripe(char *fmt, ...) ATTRIBUTE((format (printf, 1, 2)));

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
xufldname(FILE *fp, struct value values[], int i)
{
    int ch;
    char buf[1024];

    values[i].v_type = VAL_NOTUSED;

    ch = skipfs(fp);
    switch (ch) {
    case EOF:
	return gripe("Unexpected EOF");
    case '\n':
	return 0;
    default:
	ungetc(ch, fp);
	if (getid(fp, buf) < 0)
	    return gripe("Junk in field %d", i + 1);
	ch = getc(fp);
	if (ch != '(') {
	    ungetc(ch, fp);
	    values[i].v_type = VAL_SYMBOL;
	    values[i].v_field.v_string = strdup(buf);
	    return 1;
	}
	ch = getc(fp);
	ungetc(ch, fp);
	if (isdigit(ch) || ch == '-' || ch == '+') {
	    if (fscanf(fp, "%d", &values[i].v_field.v_int) != 1) {
		return gripe("Malformed number in index field %d", i + 1);
	    }
	} else {
	    if (getid(fp, buf) < 0)
		return gripe("Malformed string in index field %d", i + 1);
	    return gripe("Symbolic index in field %d not yet implemented",
			 i + 1);
	}
	ch = getc(fp);
	if (ch != ')')
	    return gripe("Malformed index field %d", i + 1);
	values[i].v_index_name = strdup(buf);
	values[i].v_type = VAL_INDEX_ID;
	return 1;
    }
}

static int
xufld(FILE *fp, struct value values[], int i)
{
    int ch;
    char buf[1024];
    char *p;
    int len, l1;

    values[i].v_type = VAL_NOTUSED;

    ch = skipfs(fp);
    switch (ch) {
    case EOF:
	return gripe("Unexpected EOF");
    case '\n':
	return 0;
    case '+': case '-': case '.':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
	ungetc(ch, fp);
	if (fscanf(fp, "%lg", &values[i].v_field.v_double) != 1)
	    return gripe("Malformed number in field %d", i + 1);
	values[i].v_type = VAL_DOUBLE;
	return 1;
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
	values[i].v_type = VAL_STRING;
	values[i].v_field.v_string = strdup(buf);
	return 1;
    case '(':
	p = strdup("");
	len = 0;
	for (;;) {
	    ch = skipfs(fp);
	    if (ch == EOF || ch == '\n')
		return gripe("Unmatched '(' in field %d", i + 1);
	    if (ch == ')')
		break;
	    ungetc(ch, fp);
	    l1 = getid(fp, buf);
	    if (l1 < 0)
		return gripe("Junk in field %d", i + 1);
	    p = realloc(p, len + l1 + 2);
	    strcpy(p + len, buf);
	    strcpy(p + len + l1, " ");
	    len += l1 + 1;
	}
	if (len)
	    p[len - 1] = 0;
	values[i].v_type = VAL_SYMBOL_SET;
	values[i].v_field.v_string = p;
	return 1;
    default:
	ungetc(ch, fp);
	if (getid(fp, buf) < 0)
	    return gripe("Junk in field %d", i + 1);
	if (!strcmp(buf, "nil")) {
	    values[i].v_type = VAL_STRING;
	    values[i].v_field.v_string = NULL;
	} else {
	    values[i].v_type = VAL_SYMBOL;
	    values[i].v_field.v_string = strdup(buf);
	}
	return 1;
    }
}

static int
xuflds(FILE *fp, struct value values[],
       int (*parse)(FILE *, struct value values[], int))
{
    int i, ch, res;

    for (i = 0; ; i++) {
	if (i >= MAX_NUM_COLUMNS)
	    return gripe("Too many columns");
	res = parse(fp, values, i);
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

static void
freeflds(struct value values[])
{
    struct value *vp;

    for (vp = values; vp->v_type != VAL_NOTUSED; vp++) {
	if (vp->v_type != VAL_DOUBLE && vp->v_type != VAL_INDEX_ID)
	    free(vp->v_field.v_string);
    }
    free(vp->v_index_name);
    vp->v_index_name = NULL;
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
xunsymbol(struct castr *ca, struct value *vp, int n)
{
    struct symbol *symtab = (struct symbol *)empfile[ca->ca_table].cache;
    char *buf = vp->v_field.v_string;
    int i;
    int value;
    char *token;

    if (ca->ca_table == EF_BAD || ef_cadef(ca->ca_table) != symbol_ca)
	return gripe("%s doesn't take a symbol or symbol set in field %d",
		     ca->ca_name, n);

    if (vp->v_type == VAL_SYMBOL_SET) {
	if (!(ca->ca_flags & NSC_BITS))
	    return gripe("%s doesn't take a symbol set in field %d",
			 ca->ca_name, n);
	value = 0;
	for (token = strtok(buf, " "); token; token = strtok(NULL, " ")) {
	    i = xunsymbol1(token, symtab, ca, n);
	    if (i < 0)
		return -1;
	    value |= symtab[i].value;
	}
    } else if (vp->v_type == VAL_SYMBOL) {
	if (ca->ca_flags & NSC_BITS)
	    return gripe("%s doesn't take a symbol in field %d",
			 ca->ca_name, n);
	i = xunsymbol1(buf, symtab, ca, n);
	if (i < 0)
	    return -1;
	value = symtab[i].value;
    } else
	return 0;

    vp->v_type = VAL_DOUBLE;
    vp->v_field.v_double = value;
    return 0;
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

static void
xuinitrow(int type, int row)
{
    struct empfile *ep = &empfile[type];
    char *ptr = ep->cache + ep->size * row;

    memset(ptr, 0, ep->size);

    if (ep->init)
	ep->init(row, ptr);
}

static int
xuloadrow(int type, int row, struct value values[])
{
    int i,j,k;
    struct empfile *ep = &empfile[type];
    char *ptr = ep->cache + ep->size * row;
    struct castr *ca = ep->cadef;
    void *row_ref;

    i = 0;
    j = 0;
    while (ca[i].ca_type != NSC_NOTYPE &&
	   values[j].v_type != VAL_NOTUSED) {
	if (ca[i].ca_flags & NSC_EXTRA) {
	    i++;
	    continue;
	}
	row_ref = (char *)ptr + ca[i].ca_off;
	k = 0;
	do {
	    /*
	     * TODO
	     * factor out NSC_CONST comparsion
	     */
	    switch (values[j].v_type) {
	    case VAL_SYMBOL:
	    case VAL_SYMBOL_SET:
		if (xunsymbol(&ca[i], &values[j], j) < 0)
		    return -1;
		/* fall through */
	    case VAL_DOUBLE:
		switch (ca[i].ca_type) {
		case NSC_INT:
		    if (ca[i].ca_flags & NSC_CONST) {
			if (((int *)row_ref)[k] != (int)
			     values[j].v_field.v_double)
			    gripe("Field %s must be same, "
				"read %d != expected %d",
				ca[i].ca_name,
				((int *)row_ref)[k],
				(int)values[j].v_field.v_double);

		    } else
			((int *)row_ref)[k] =
			    (int)values[j].v_field.v_double;
		    break;
		case NSC_LONG:
		    if (ca[i].ca_flags & NSC_CONST) {
			if (((long *)row_ref)[k] != (long)
			     values[j].v_field.v_double)
			    gripe("Field %s must be same, "
				"read %ld != expected %ld",
				ca[i].ca_name,
				((long *)row_ref)[k],
				(long)values[j].v_field.v_double);
		    } else
			((long *)row_ref)[k] = (long)
			    values[j].v_field.v_double;
		    break;
		case NSC_SHORT:
		    if (ca[i].ca_flags & NSC_CONST) {
			if (((short *)row_ref)[k] !=
			     (short)values[j].v_field.v_double)
			    gripe("Field %s must be same, "
				"read %d != expected %d",
				ca[i].ca_name,
				((short *)row_ref)[k],
				(short)values[j].v_field.v_double);
		    } else
			((short *)row_ref)[k] = (short)
			    values[j].v_field.v_double;
		    break;
		case NSC_USHORT:
		    if (ca[i].ca_flags & NSC_CONST) {
			if (((unsigned short *)row_ref)[k] !=
			     (unsigned short)values[j].v_field.v_double)
			    gripe("Field %s must be same, "
				"read %d != expected %d",
				ca[i].ca_name,
				((unsigned short *)row_ref)[k],
				(unsigned short)values[j].v_field.v_double);
		    } else
			((unsigned short *)row_ref)[k] = (unsigned short)
			    values[j].v_field.v_double;
		    break;
		case NSC_UCHAR:
		    if (ca[i].ca_flags & NSC_CONST) {
			if (((unsigned char *)row_ref)[k] != (unsigned char)
			     values[j].v_field.v_double)
			    gripe("Field %s must be same, "
				"read %d != expected %d",
				ca[i].ca_name,
				((unsigned char *)row_ref)[k],
				(unsigned char)values[j].v_field.v_double);
		    } else
			((unsigned char *)row_ref)[k] = (unsigned char)
			    values[j].v_field.v_double;
		    break;
		case NSC_FLOAT:
		    if (ca[i].ca_flags & NSC_CONST) {
			if (((float *)row_ref)[k] != (float)
			     values[j].v_field.v_double)
			    gripe("Field %s must be same, "
				"read %g != expected %g",
				ca[i].ca_name,
				((float *)row_ref)[k],
				(float)values[j].v_field.v_double);
		    } else
			((float *)row_ref)[k] = (float)
			    values[j].v_field.v_double;
		    break;
		case NSC_STRING:
		    return gripe("Field %s is a string type, "
			"but %lg was read which is a number",
			ca[i].ca_name, values[j].v_field.v_double);
		default:
   		    return gripe("Field %s's type %d is not supported",
			ca[i].ca_name, ca[i].ca_type);
		}
		break;
	    case VAL_STRING:
		switch(ca[i].ca_type) {
		case NSC_STRING:
    		    if (ca[i].ca_flags & NSC_CONST) {
			if (strcmp(((char **)row_ref)[k],
				   values[j].v_field.v_string) != 0)
			    gripe("Field %s must be same, "
				"read %s != expected %s",
				ca[i].ca_name,
				((char **)row_ref)[k],
				values[j].v_field.v_string);
		    } else
			((char **)row_ref)[k]
			    = strdup(values[j].v_field.v_string);
		    break;
		case NSC_INT:
		case NSC_LONG:
		case NSC_SHORT:
		case NSC_USHORT:
		case NSC_UCHAR:
		case NSC_FLOAT:
		    return gripe("Field %s is a number type %d, "
			"but %s was read which is a string",
			ca[i].ca_name, ca[i].ca_type,
			values[j].v_field.v_string);
		default:
   		    return gripe("Field %s's type %d is not supported",
			    ca[i].ca_name, ca[i].ca_type);
		}
		break;
	    case VAL_NOTUSED:
		return gripe("Missing column %s in file", ca[i].ca_name);
	    case VAL_INDEX_ID:
	    case VAL_INDEX_SYMBOL:
		return gripe("Index fields not supported in data rows in %s file",
		    ca[i].ca_name);
	    default:
		return gripe("Unknown value type %d", values[j].v_type);
	    }
	    k++;
	    j++;
	} while (k < ca[i].ca_len);
	i++;
    }
    if (ca[i].ca_type != NSC_NOTYPE)
	return gripe("Missing column %s in file", ca[i].ca_name);
    switch  (values[j].v_type) {
    case VAL_NOTUSED:
	break;
    case VAL_STRING:
    case VAL_SYMBOL:
    case VAL_SYMBOL_SET:
	return gripe("Extra junk after the last column, read %s",
	    values[j].v_field.v_string);
    case VAL_DOUBLE:
	return gripe("Extra junk after the last column, read %lg",
	    values[j].v_field.v_double);
    default:
	return gripe("Extra junk after the last column, "
	    "unknown value type %d", values[j].v_type);
    }
    return 0;
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

    if (skipfs(fp) != '\n')
	return gripe("Junk after xdump header");

    type = ef_byname(name);
    if (type < 0)
	return gripe("Unknown table `%s'", name);
    if (expected_table != EF_BAD && expected_table != type)
	return gripe("Expected table `%s', not `%s'",
		     ef_nameof(expected_table), name);

    if (CANT_HAPPEN(!(ef_flags(type) & EFF_MEM)))
	return -1;

    return type;
}

static int
xucolumnheader(FILE *fp, int type, struct value values[])
{
    char ch;
    struct empfile *ep = &empfile[type];
    struct castr *ca = ep->cadef;
    int i, j, k;

    if (!human)
	return 0;

    while ((ch = skipfs(fp)) == '\n');
    ungetc(ch, fp);

    /* FIXME parse column header */
    if (xuflds(fp, values, xufldname) <= 0) {
	freeflds(values);
	return -1;
    }
    /* TODO
     * check column count form xuflds()
     */

    j = 0;
    for (i = 0; ca[i].ca_name; i++) {
	if (ca[i].ca_flags & NSC_EXTRA)
	    continue;
	k = 0;
	do {
	    if (values[j].v_type == VAL_NOTUSED) {
		freeflds(values);
		return gripe("Not enough columns in the header for table %s",
			     ef_nameof(type));
	    }
	    switch(values[j].v_type) {
	    case VAL_SYMBOL:
		if (ca[i].ca_len != 0) {
		    freeflds(values);
		    return gripe("Column %s is a not index format for table %s",
			ca[i].ca_name, ef_nameof(type));
		}
		if (strcmp(values[j].v_field.v_string, ca[i].ca_name) != 0) {
		    gripe("Column name (%s) does not match header name (%s)",
			ca[i].ca_name, values[j].v_field.v_string);
		    freeflds(values);
		    return -1;
		}
		break;
	    case VAL_INDEX_SYMBOL:
		    return gripe("Column %s is does not currently support index symbol format for table %s",
			ca[i].ca_name, ef_nameof(type));
	    case VAL_INDEX_ID:
		if (ca[i].ca_len == 0) {
		    freeflds(values);
		    return gripe("Column %s is in index format and should not be for table %s",
			ca[i].ca_name, ef_nameof(type));
		}
		if (values[j].v_field.v_int != k) {
		    freeflds(values);
		    return gripe("Column Array index %d does not match %d",
			values[j].v_field.v_int, k);
		}
		if (strcmp(values[j].v_index_name, ca[i].ca_name) != 0) {
		    gripe("Column name (%s) does not match header name (%s)",
			ca[i].ca_name, values[j].v_field.v_string);
		    freeflds(values);
		    return -1;
		}
		break;
	    default:
		freeflds(values);
		return gripe("Column %s is a not string for table %s",
		    ca[i].ca_name, ef_nameof(type));
	    }
	    j++;
	    k++;
	} while (k < ca[i].ca_len);
    }
    if (values[j].v_type != VAL_NOTUSED) {
        freeflds(values);
	return gripe("Too many columns in the header for table %s",
	    ef_nameof(type));
    }
    lineno++;
    freeflds(values);
    return 0;
}

static int
xutrailer(FILE *fp, int type, int row)
{
    int rows, ch, res;

    res = -1;
    if (human) {
	if (fscanf(fp, "config%n",  &res) != 0) {
	    return gripe("Malformed table footer");
	}
    }
    ch = skipfs(fp);
    if (!isdigit(ch)) {
        if (ch != '\n' || !human)
	    return gripe("Malformed table footer");
    } else {
        ungetc(ch, fp);
	if (fscanf(fp, "%d", &rows) != 1)
	    return gripe("Malformed table footer");
	if (row != rows)
	    return gripe("Read %d rows, which doesn't match footer "
			 "%d rows", row, rows);
    }
    if (skipfs(fp) != '\n')
	return gripe("Junk after table footer");

    while ((ch = skipfs(fp)) == '\n') ;
    ungetc(ch, fp);

    return 0;
}

int
xundump(FILE *fp, char *file, int expected_table)
{
    int row, res, ch;
    struct empfile *ep;
    int need_sentinel;
    struct value values[MAX_NUM_COLUMNS + 1];
    int type;
    int fixed_rows;

    memset(values, 0, sizeof(values));

    if (strcmp(fname, file) != 0) {
        fname = file;
	lineno = 1;
    } else
	lineno++;

    if ((type = xuheader(fp, expected_table)) == -1)
	return -1;
    
    ep = &empfile[type];
    fixed_rows = has_const(ef_cadef(type));
    need_sentinel = !fixed_rows; /* FIXME only approximation */

    if (xucolumnheader(fp, type, values) == -1)
	return -1;

    row = 0;
    for (;;) {
	lineno++;
	ch = skipfs(fp);
	if (ch == '/')
	    break;
	ungetc(ch, fp);
	/*
	 * TODO
	 * Add column count check to the return value of xuflds()
	 */
	res = xuflds(fp, values, xufld);
	if (res > 0 && row >= ep->csize - 1) {
	    /* TODO grow cache unless EFF_STATIC */
	    gripe("Too many rows for table %s", ef_nameof(type));
	    res = -1;
	}
	if (res > 0) {
	    if (!fixed_rows)
		xuinitrow(type, row);
	    res = xuloadrow(type, row, values);
	    row++;
	}
	freeflds(values);
	if (res < 0)
	    return -1;
    }
    if (fixed_rows && row != ep->csize -1)
	return gripe("Table %s requires %d rows, got %d",
		     ef_nameof(type), ep->csize - 1, row);

    if (need_sentinel)
	xuinitrow(type, row);

    if (xutrailer(fp, type, row) == -1)
	return -1;
    
    ep->fids = ep->cids = row;
    return type;
}
