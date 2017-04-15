/* Common header file for Macintosh STDWIN implementation */

#include <Types.h>
#include <Quickdraw.h>
#include <Controls.h>
#include <Events.h>
#include <Windows.h>

#ifndef HAVE_UNIVERSAL_HEADERS
#if defined(USESROUTINEDESCRIPTORS) || defined(GENERATINGCFM)
#define HAVE_UNIVERSAL_HEADERS 1
#endif
#endif

#include "stdwconf.h"	/* Sets compiler dependent feature test macros */


#ifdef MPW

#include <Strings.h>

/* Quickdraw globals aren't really global in MPW */
#define QD(var) (qd.var)

/* MPW allows 'static' in forward declarations */
#define STATIC static

/* MPW passes Points by address */
#define PASSPOINT &

/* MPW glue converts most strings on the fly.
   (XXX I believe you can turn this off now?) */
#define CLEVERGLUE

#endif /* MPW */


#ifdef THINK_C

/* Pascal-to-C and back string conversion routines have different names */
#include <pascal.h>

/* In Think, QD globals aren't really global any more either... */
#define QD(var) (qd.var)

/* THINK C allows 'static' in forward declarations */
#define STATIC static

/* THINK C passes Points by value */
#define PASSPOINT /**/

/* THINK C supports console I/O through <console.h> */
#define CONSOLE_IO

#endif /* THINK_C */


#ifdef __MWERKS__

#include <Strings.h>

/* Quickdraw globals aren't really global in Mwerks */
#define QD(var) (qd.var)

/* MW allows 'static' in forward declarations */
#define STATIC static

/* MW passes Points by value */
#define PASSPOINT /**/

/* No CLEVERGLUE, No CONSOLE_IO */
#undef CLEVERGLUE
#undef CONSOLE_IO

#define CtoPstr C2PStr
#define PtoCstr P2CStr

#endif /* __MWERKS__ */


/* Private include files: */

#include "tools.h"
#include "stdwin.h"
#include "menu.h"

/* Resolve conflict between <TextEdit.h> and "stdwin.h" */
#undef teclick


#ifdef CLEVERGLUE
/* MPW converts C to Pascal strings in the glue */
#define PSTRING(str) (str)
#else
/* THINK C needs a real function to do this (see "pstring.c").
   This is different from CtoPstr since it does not do it inline */
extern unsigned char *PSTRING _ARGS((char *));
#endif


/* Window struct. */

struct _window {
	short tag;		/* Window tag, usable as document id */
	void (*drawproc)();	/* Draw procedure */
	WindowPtr w;		/* Mac Window */
	int hcaret, vcaret;	/* Caret position, document coordinates */
	bool caret_on;		/* Set if caret currently visible */
	TEXTATTR attr;		/* Text attributes */
	ControlHandle hbar, vbar;	/* Scroll bars */
	int docwidth, docheight;	/* Document size */
	int orgh, orgv;		/* Window origin, document coordinates */
	struct menubar mbar;	/* List of attached local menus */
	unsigned long timer;	/* Tick count for timer event */
	CURSOR *cursor;		/* Cursor if not default */
	COLOR fgcolor, bgcolor;	/* Default colors for this window */
};

extern TEXTATTR wattr;		/* Current text attributes */

#define TX_INVERSE	0x80	/* Or-ed into style bits */

/* Peculiarities of the Macintosh: */

#define TICKSPERSECOND	60	/* Clock ticks at 60 Hz (everywhere) */

#define MENUBARHEIGHT	20	/* Height of menu bar */
#define TITLEBARHEIGHT	18	/* Height of window title bar */
#define BAR		15	/* Scroll bar width, minus one pixel */

/* ASCII codes generated by special keys: */
#define ENTER_KEY	0x03

#define LEFT_ARROW	0x1c
#define RIGHT_ARROW	0x1d
#define UP_ARROW	0x1e
#define DOWN_ARROW	0x1f


/* Miscellaneous definitions. */

#define CLICK_DIST	5	/* Max mouse move within a click */

/* Text drawn in the very left or right margin doesn't look nice.
   Therefore, we have a little margin on each side.
   Its width is determined here: */
#define LSLOP	4	/* Pixels in left margin */
#define RSLOP	4	/* Pixels in right margin */

/* Global data: */
extern GrafPtr screen;		/* Window Manager's GrafPort */
extern WINDOW *active;		/* Active window, if any */
extern bool _wmenuhilite;	/* Set if menu item highlighted */
extern bool _wm_down;		/* Set if mouse down (in appl. area) */
extern COLOR _w_fgcolor;	/* Current foreground color */
extern COLOR _w_bgcolor;	/* Current background color */

/* Function prototypes: */

void dprintf _ARGS((char *fmt, ...));

void wsetstyle _ARGS((Style face));

WINDOW *whichwin _ARGS((WindowPtr w));

void makerect _ARGS((WINDOW *win, Rect *pr,
	int left, int top, int right, int bottom));
void getwinrect _ARGS((WINDOW *win, Rect *pr));

void set_arrow _ARGS((void));
void set_applcursor _ARGS((void));
void set_watch _ARGS((void));
void set_ibeam _ARGS((void));
void set_hand _ARGS((void));

void makescrollbars _ARGS((WINDOW *win, /*bool*/int hor, /*bool*/int ver));
void movescrollbars _ARGS((WINDOW *win));
void hidescrollbars _ARGS((WINDOW *win));
void showscrollbars _ARGS((WINDOW *win));
void _wgrowicon _ARGS((WINDOW *win));
void _wfixorigin _ARGS((WINDOW *));

void scrollby _ARGS((WINDOW *win, Rect *pr, int dh, int dv));
void do_scroll _ARGS((Point *pwhere,
	WINDOW *win, ControlHandle bar, int pcode));
void dragscroll _ARGS((WINDOW *win, int h, int v, int constrained));

void initwattr _ARGS((void));

void inval_border _ARGS((WindowPtr w));
void valid_border _ARGS((WindowPtr w));

void rmlocalmenus _ARGS((WINDOW *win));
void addlocalmenus _ARGS((WINDOW *win));
void initmbar _ARGS((struct menubar *mp));
void killmbar _ARGS((struct menubar *mp));
void setup_menus _ARGS((void));

void do_about _ARGS((void));
void getargcargv _ARGS((int *pargc, char ***pargv));
void fullpath _ARGS((char *buf, int wdrefnum, char *file));
char *getdirname _ARGS((int wdrefnum));


void rmcaret _ARGS((WINDOW *win));
void showcaret _ARGS((WINDOW *win));
void blinkcaret _ARGS((WINDOW *win));
void _wresetmouse _ARGS((void));

void _wfreeclip _ARGS((void));
bool checktimer _ARGS((EVENT *ep));
void autoscroll _ARGS((WINDOW *active, int h, int v));
void _wdo_menu _ARGS((EVENT *ep, long menu_item));

void _w_usefgcolor _ARGS((COLOR color));
void _w_usebgcolor _ARGS((COLOR color));

/* SetRect is much faster this way... */
#define SetRect(pr, l, t, r, b) ((pr)->left = (l), (pr)->top = (t), \
				(pr)->right = (r), (pr)->bottom = (b))