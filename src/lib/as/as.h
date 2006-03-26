/*
 *  A* Search - A search library used in Empire to determine paths between
 *              objects.
 *  Copyright (C) 1990-1998 Phil Lapsley
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
 */
/*
 * A* definitions.
 *
 * @(#)as.h	1.9	11/13/90
 */
/*
 * 11/09/98 - Steve McClure
 *  Added path list caching structures
 */

#ifndef AS_H
#define AS_H

#include <stdio.h>		/* for FILE */
#include "misc.h"		/* for s_char */

/*
 * Coordinate.
 */
struct as_coord {
    int x, y;
};

/*
 * Path, made up of a linked list of coordinates.
 */
struct as_path {
    struct as_coord c;
    struct as_path *next;
};

/*
 * Basic node, used internally by A* algorithm.
 */
struct as_node {
    struct as_coord c;		/* our coordinate */
    double knowncost;		/* cost so far  */
    double lbcost;		/* lower bound on cost to dest */
    double inclbcost;		/* incremental lower bound cost */
    double seccost;		/* used to break ties */
    int step;
    int flags;
    struct as_node *back;
};
#define	AS_TRIED	1	/* we've tried this node before */

/*
 * Linked list of nodes, used internally by A* algorithm.
 */
struct as_queue {
    struct as_node *np;
    struct as_queue *next;
    struct as_queue *prev;
};

/*
 * Hash table entry, used to determine if we've seen a particular
 * coordinate before.
 */
struct as_hash {
    struct as_coord c;
    struct as_queue *qp;
    struct as_hash *next;
};

/*
 * User's handle on A*, returned by as_init().  Some of the data here is
 * used by A* internals.
 */
struct as_data {
    int maxneighbors;		/* max # of neighbors a cell can have */
    int hashsize;		/* size of internal hash table */

    int (*hash) (struct as_coord);	/* hash function (coord -> int) */
    int (*neighbor) (struct as_coord, struct as_coord *, s_char *);	/* function to find neighbors */
    double (*lbcost) (struct as_coord, struct as_coord, s_char *);	/* function to give lower bound cost */
    double (*realcost) (struct as_coord, struct as_coord, s_char *);	/* function to give real cost */
    double (*seccost) (struct as_coord, struct as_coord, s_char *);	/* function to secondary cost */
    char *userdata;		/* user's data, passed to callbacks */
    struct as_coord from;	/* from coordinate */
    struct as_coord to;		/* to coordinate */
    struct as_path *path;	/* solution */

    /* below are "private" to as_ routines */
    struct as_queue *head;
    struct as_queue *tried;
    struct as_hash **hashtab;
    struct as_queue *subsumed;
    struct as_coord *neighbor_coords;
    struct as_node **neighbor_nodes;
};

/* 
 * Added these for caching of paths as we stumble across them
 */

struct as_topath {
    coord x;
    struct as_path *path;	/* Path from holder of this list to here */
    struct as_topath *next;
};

struct as_frompath {
    coord x;
    struct as_topath **tolist;	/* List of nodes we have a path to */
    struct as_frompath *next;
};

/*
 * Some cheezy allocation macros.
 */
#define	AS_NEW_ARRAY(p, type, n, err) \
	(p) = (type *)calloc((n), sizeof(*(p))); \
	if ((p) == NULL) \
		return err; \

#define	AS_NEW(p, type, err) \
	AS_NEW_ARRAY((p), type, 1, err);

#define	AS_NEW_MALLOC(p, type, err) \
        (p) = (type *)malloc(sizeof(type)); \
	if ((p) == NULL) \
	       return err; \

/* Functions that the user can call.  */

extern struct as_data *as_init(int maxneighbors, int hashsize,
			       int (*hashfunc) (struct as_coord),
			       int (*neighborfunc) (struct as_coord,
						    struct as_coord *,
						    s_char *),
			       double (*lbcostfunc) (struct as_coord,
						     struct as_coord,
						     s_char *),
			       double (*realcostfunc) (struct as_coord,
						       struct as_coord,
						       s_char *),
			       double (*seccostfunc) (struct as_coord,
						      struct as_coord,
						      s_char *),
			       s_char *userdata);
extern int as_search(struct as_data *adp);
extern void as_delete(struct as_data *adp);
extern void as_reset(struct as_data *adp);
extern void as_stats(struct as_data *adp, FILE * fp);
extern struct as_path *as_find_cachepath(coord fx,
					 coord fy, coord tx, coord ty);

/* Functions that are "private" to algorithm */

extern void as_add_cachepath(struct as_data *adp);
extern void as_clear_cachepath(void);
extern void as_enable_cachepath(void);
extern void as_disable_cachepath(void);

extern void as_makepath(struct as_data *adp);
extern void as_free_path(struct as_path *pp);

extern int as_costcomp(const void *, const void *);
extern struct as_queue *as_extend(struct as_data *adp);
extern struct as_queue *as_merge(struct as_data *adp,
				 struct as_queue *head,
				 struct as_node **neighbors);
extern struct as_queue *as_iscinq(struct as_data *adp, struct as_coord c);
extern void as_setcinq(struct as_data *adp,
		       struct as_coord c, struct as_queue *qp);
extern void as_free_hashtab(struct as_data *adp);
extern int as_winnow(struct as_data *adp,
		     struct as_coord *coords, int ncoords);

#endif
