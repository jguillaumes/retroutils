/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)exec.h	1.2 (2.11BSD GTE) 10/31/93
 */

#ifndef _EXEC_
#define _EXEC_
/*
 * Header prepended to each a.out file.
 *
 * In the original 2.11BSD version the members of the exec structure
 * were declared as int. They have been changed to short to force
 * 16 bit integers.
 */
struct	exec {
	short	a_magic;	/* magic number */
unsigned short	a_text;		/* size of text segment */
unsigned short	a_data;		/* size of initialized data */
unsigned short	a_bss;		/* size of uninitialized data */
unsigned short	a_syms;		/* size of symbol table */
unsigned short	a_entry; 	/* entry point */
unsigned short	a_unused;	/* not used */
unsigned short	a_flag; 	/* relocation info stripped */
};

#define	NOVL	15		/* number of overlays */
struct	ovlhdr {
	short	max_ovl;	/* maximum overlay size */
unsigned short	ov_siz[NOVL];	/* size of i'th overlay */
};

/*
 * eXtended header definition for use with the new macros in a.out.h
*/
struct	xexec {
	struct	exec	e;
	struct	ovlhdr	o;
	};

#define	A_MAGIC1	0407	/* normal */
#define	A_MAGIC2	0410	/* read-only text */
#define	A_MAGIC3	0411	/* separated I&D */
#define	A_MAGIC4	0405	/* overlay */
#define	A_MAGIC5	0430	/* auto-overlay (nonseparate) */
#define	A_MAGIC6	0431	/* auto-overlay (separate) */

#endif
