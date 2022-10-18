/* See LICENSE file for copyright and license details. */

/* appearance */
static unsigned int borderpx = 2;        /* border pixel of windows */
static unsigned int snap     = 12;       /* snap pixel */
static unsigned int gapsize  = 15;       /* gap size */
static int enablegaps        = 0;
static int swallowing        = 0;        /* swallowing enabled/disabled */
static int swallowfloating   = 0;        /* 1 means swallow floating windows by default */
static int smartgaps         = 0;        /* 1 means no outer gap when there is only one window */
static const unsigned int systraypinning = 2;     /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft  = 1;     /* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayleftpadding = 4; /* space between the systray and the status text if systray is on left */
static const unsigned int systrayspacing = 2;     /* systray spacing */
static const int systraypinningfailfirst = 1;     /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static int showsystray = 1;        /* 0 means no systray */
static int showbar     = 1;        /* 0 means no bar */
static int topbar      = 0;        /* 0 means bottom bar */
#define ICONSIZE 16   /* icon size */
#define ICONSPACING 5 /* space between icon and title */
static void (*attach)(Client *c) = attachmaster; /* attachmaster, attachabove, attachbelow, attachtop, attachbottom */
static const char *fonts[] = {
	"monospace:size=12",
	"Noto Color Emoji:pixelsize=12:antialias=true:autohint=true",
};
static const char dmenufont[] = "monospace:size=12";
static const char dmenufontlarge[] = "monospace:size=16";
static char normbgcolor[]     = "#111111";
static char normbordercolor[] = "#222222";
static char normfgcolor[]     = "#bbbbbb";
static char selfgcolor[]      = "#eeeeee";
static char selbordercolor[]  = "#116688";
static char selbgcolor[]      = "#005577";
static char hidfgcolor[]      = "#ff0000";
static char hidbordercolor[]  = "#080808";
static char hidbgcolor[]      = "#080808";
static char matchselbgcolor[] = "#005577";
static char dmenuselbgcolor[] = "#222222";
static char *colors[][3] = {
	/*               fg           bg           border   */
	[SchemeNorm] = { normfgcolor, normbgcolor, normbordercolor },
	[SchemeSel]  = { selfgcolor,  selbgcolor,  selbordercolor  },
	[SchemeHid]  = { hidfgcolor,  hidbgcolor,  hidbordercolor  },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static Rule defaultrules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class                     instance    title       tags mask     isfloating   isterminal noswallow monitor borderless */
	{ { .pat.str = "St" },       { 0 },      { 0 },      0,            0,           1,         0,        -1,     0 },
};
static char *rulespath = "~/.config/dwm/rules.yml";

/* layout(s) */
static float mfact              = 0.50; /* factor of master area size [0.05..0.95] */
static int nmaster              = 1;    /* number of clients in master area */
static int resizehints          = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1;    /* 1 will force focus on the fullscreen window */

#define FORCE_VSPLIT 1  /* nrowgrid layout: force two clients to always split vertically */
#include "vanitygaps.c"
static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* Default: Master on left, slaves on right */
	{ "TTT",      bstack },  /* Master on top, slaves on bottom */

	{ "[M]",      monocle }, /* All windows on top of eachother */
	{ "H[]",      deck },    /* Master on left, slaves in monocle-like mode on right */

	{ "|M|",      centeredmaster },         /* Master in middle, slaves on sides */
	{ ">M>",      centeredfloatingmaster }, /* Same but master floats */

	{ "><>",      NULL },    /* no layout function means floating behavior */

	/* { "[@]",      spiral },  /1* Fibonacci spiral *1/ */
	/* { "[\\]",     dwindle }, /1* Decreasing in size right and leftward *1/ */
	{ NULL,       NULL },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

#define STATUSBAR "dwmstatus"

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufontlarge, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", dmenuselbgcolor, "-sf", selfgcolor, "-shb", matchselbgcolor, NULL };
static const char *termcmd[]  = { "st", NULL };

#include "dmenuaction.h"

/*
 * Xresources preferences to load at startup
 */
static ResourcePref resources[] = {
	{ "normbgcolor",     STRING,  &normbgcolor },
	{ "normbordercolor", STRING,  &normbordercolor },
	{ "normfgcolor",     STRING,  &normfgcolor },
	{ "selbgcolor",      STRING,  &selbgcolor },
	{ "selbordercolor",  STRING,  &selbordercolor },
	{ "selfgcolor",      STRING,  &selfgcolor },
	{ "hidbgcolor",      STRING,  &hidbgcolor },
	{ "hidbordercolor",  STRING,  &hidbordercolor },
	{ "hidfgcolor",      STRING,  &hidfgcolor },
	{ "matchselbgcolor", STRING,  &matchselbgcolor },
	{ "dmenuselbgcolor", STRING,  &dmenuselbgcolor },
	{ "borderpx",        INTEGER, &borderpx },
	{ "snap",            INTEGER, &snap },
	{ "showsystray",     INTEGER, &showsystray },
	{ "showbar",         INTEGER, &showbar },
	{ "topbar",          INTEGER, &topbar },
	{ "nmaster",         INTEGER, &nmaster },
	{ "resizehints",     INTEGER, &resizehints },
	{ "mfact",           FLOAT,   &mfact },
	{ "gapsize",         INTEGER, &gapsize },
	{ "enablegaps",      INTEGER, &enablegaps },
	{ "swallowing",      INTEGER, &swallowing },
	{ "swallowfloating", INTEGER, &swallowfloating },
	{ "smartgaps",       INTEGER, &smartgaps },
};

#include "movestack.c"
static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_d,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_s,      spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_j,      movestack,      {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_k,      movestack,      {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY|ShiftMask,             XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_g,      togglegaps,     {0} },
	{ MODKEY|ShiftMask,             XK_g,      toggleborders,  {0} },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_i,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_Tab,    shiftview,      {.i = +1} },
	{ MODKEY|ShiftMask,             XK_Tab,    shiftview,      {.i = -1} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY|ShiftMask,             XK_t,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_y,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY|ShiftMask,             XK_y,      setlayout,      {.v = &layouts[3]} },
	{ MODKEY,                       XK_u,      setlayout,      {.v = &layouts[4]} },
	{ MODKEY|ShiftMask,             XK_u,      setlayout,      {.v = &layouts[5]} },
	{ MODKEY,                       XK_space,  zoom,           {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY|ShiftMask,             XK_b,      togglebar,      {0} },
	{ MODKEY|ShiftMask,             XK_s,      toggleswallow,  {0} },
	{ MODKEY|ShiftMask,             XK_f,      togglefullscr,  {0} },
	{ MODKEY,                       XK_F5,     dmenuaction,    {.v = &actionarg } },
	{ MODKEY,                       XK_a,      setattach,      {.ui = AttachMaster } },
	{ MODKEY|ShiftMask,             XK_a,      setattach,      {.ui = AttachBottom } },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_Left,   focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_Right,  focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_Left,   tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_Right,  tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      killclient,     {0} },
	{ MODKEY|ShiftMask|ControlMask, XK_q,      quit,           {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function         argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,       {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,       {.v = &layouts[6]} },
	{ ClkWinTitle,          0,              Button1,        togglewinfocus,  {0} },
	{ ClkWinTitle,          0,              Button2,        zoom,            {0} },
	{ ClkWinTitle,          0,              Button3,        togglewinhidden, {0} },
	{ ClkStatusText,        0,              Button1,        sigstatusbar,    {.i = 1} },
	{ ClkStatusText,        0,              Button2,        sigstatusbar,    {.i = 2} },
	{ ClkStatusText,        0,              Button3,        sigstatusbar,    {.i = 3} },
	{ ClkStatusText,        ShiftMask,      Button1,        sigstatusbar,    {.i = 6} },
	{ ClkStatusText,        ShiftMask,      Button3,        spawn,           SHCMD("st -e nvim ~/src/dwmstatus/config.h") },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,       {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating,  {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,     {0} },
	{ ClkTagBar,            0,              Button1,        view,            {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,      {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,             {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,       {0} },
};

