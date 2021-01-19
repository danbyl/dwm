/* Key binding functions */
static void defaultgaps(const Arg *arg);
static void incrgaps(const Arg *arg);
static void togglegaps(const Arg *arg);

/* Layouts */
static void bstack(Monitor *m);
static void centeredmaster(Monitor *m);
static void centeredfloatingmaster(Monitor *m);
static void deck(Monitor *m);
static void dwindle(Monitor *m);
static void fibonacci(Monitor *m, int s);
static void spiral(Monitor *m);
static void tile(Monitor *);

/* Internals */
static void getgaps(Monitor *m, int *size, unsigned int *nc);
static void setgaps(int size);

static void
setgaps(int size)
{
	if (size < 0) size = 0;

	selmon->gapsize = size;
	arrange(selmon);
}

static void
togglegaps(const Arg *arg)
{
	enablegaps = !enablegaps;
	arrange(NULL);
}

static void
defaultgaps(const Arg *arg)
{
	setgaps(gapsize);
}

static void
incrgaps(const Arg *arg)
{
	setgaps(selmon->gapsize + arg->i);
}

static void
getgaps(Monitor *m, int *size, unsigned int *nc)
{
	unsigned int n, oe, ie;
	oe = ie = enablegaps;
	Client *c;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (smartgaps && n == 1) {
		oe = 0; // outer gaps disabled when only one client
	}

	*size = m->gapsize*oe*ie;
	*nc = n;            // number of clients
}

void
getfacts(Monitor *m, int msize, int ssize, float *mf, float *sf, int *mr, int *sr)
{
	unsigned int n;
	float mfacts, sfacts;
	int mtotal = 0, stotal = 0;
	Client *c;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	mfacts = MIN(n, m->nmaster);
	sfacts = n - m->nmaster;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++)
		if (n < m->nmaster)
			mtotal += msize / mfacts;
		else
			stotal += ssize / sfacts;

	*mf = mfacts; // total factor of master area
	*sf = sfacts; // total factor of stack area
	*mr = msize - mtotal; // the remainder (rest) of pixels after an even master split
	*sr = ssize - stotal; // the remainder (rest) of pixels after an even stack split
}

/***
 * Layouts
 */

/*
 * Bottomstack layout + gaps
 * https://dwm.suckless.org/patches/bottomstack/
 */

static void
bstack(Monitor *m)
{
	unsigned int i, n;
	int mx = 0, my = 0, mh = 0, mw = 0;
	int sx = 0, sy = 0, sh = 0, sw = 0;
	float mfacts, sfacts;
	int mrest, srest;
	Client *c;

	int size;
	getgaps(m, &size, &n);

	if (n == 0)
		return;

	sx = mx = m->wx + size;
	sy = my = m->wy + size;
	sh = mh = m->wh - 2*size;
	mw = m->ww - 2*size - size * (MIN(n, m->nmaster) - 1);
	sw = m->ww - 2*size - size * (n - m->nmaster - 1);

	if (m->nmaster && n > m->nmaster) {
		sh = (mh - size) * (1 - m->mfact);
		mh = (mh - size) * m->mfact;
		sx = mx;
		sy = my + mh + size;
	}

	getfacts(m, mw, sw, &mfacts, &sfacts, &mrest, &srest);

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
		if (i < m->nmaster) {
			resize(c, mx, my, (mw / mfacts) + (i < mrest ? 1 : 0) - (2*c->bw), mh - (2*c->bw), 0);
			mx += WIDTH(c) + size;
		} else {
			resize(c, sx, sy, (sw / sfacts) + ((i - m->nmaster) < srest ? 1 : 0) - (2*c->bw), sh - (2*c->bw), 0);
			sx += WIDTH(c) + size;
		}
	}
}

/*
 * Centred master layout + gaps
 * https://dwm.suckless.org/patches/centeredmaster/
 */

void
centeredmaster(Monitor *m)
{
	unsigned int i, n;
	int mx = 0, my = 0, mh = 0, mw = 0;
	int lx = 0, ly = 0, lw = 0, lh = 0;
	int rx = 0, ry = 0, rw = 0, rh = 0;
	float mfacts = 0, lfacts = 0, rfacts = 0;
	int mtotal = 0, ltotal = 0, rtotal = 0;
	int mrest = 0, lrest = 0, rrest = 0;
	Client *c;

	int size;
	getgaps(m, &size, &n);

	if (n == 0)
		return;

	/* initialize areas */
	mx = m->wx + size;
	my = m->wy + size;
	mh = m->wh - 2*size - size * ((!m->nmaster ? n : MIN(n, m->nmaster)) - 1);
	mw = m->ww - 2*size;
	lh = m->wh - 2*size - size * (((n - m->nmaster) / 2) - 1);
	rh = m->wh - 2*size - size * (((n - m->nmaster) / 2) - ((n - m->nmaster) % 2 ? 0 : 1));

	if (m->nmaster && n > m->nmaster) {
		/* go mfact box in the center if more than nmaster clients */
		if (n - m->nmaster > 1) {
			/* ||<-S->|<---M--->|<-S->|| */
			mw = (m->ww - 2*size - 2*size) * m->mfact;
			lw = (m->ww - mw - 2*size - 2*size) / 2;
			mx += lw + size;
		} else {
			/* ||<---M--->|<-S->|| */
			mw = (mw - size) * m->mfact;
			lw = m->ww - mw - size - 2*size;
		}
		rw = lw;
		lx = m->wx + size;
		ly = m->wy + size;
		rx = mx + mw + size;
		ry = m->wy + size;
	}

	/* calculate facts */
	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++) {
		if (!m->nmaster || n < m->nmaster)
			mfacts += 1;
		else if ((n - m->nmaster) % 2)
			lfacts += 1; // total factor of left hand stack area
		else
			rfacts += 1; // total factor of right hand stack area
	}

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++)
		if (!m->nmaster || n < m->nmaster)
			mtotal += mh / mfacts;
		else if ((n - m->nmaster) % 2)
			ltotal += lh / lfacts;
		else
			rtotal += rh / rfacts;

	mrest = mh - mtotal;
	lrest = lh - ltotal;
	rrest = rh - rtotal;

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
		if (!m->nmaster || i < m->nmaster) {
			/* nmaster clients are stacked vertically, in the center of the screen */
			resize(c, mx, my, mw - (2*c->bw), (mh / mfacts) + (i < mrest ? 1 : 0) - (2*c->bw), 0);
			my += HEIGHT(c) + size;
		} else {
			/* stack clients are stacked vertically */
			if ((i - m->nmaster) % 2 ) {
				resize(c, lx, ly, lw - (2*c->bw), (lh / lfacts) + ((i - 2*m->nmaster) < 2*lrest ? 1 : 0) - (2*c->bw), 0);
				ly += HEIGHT(c) + size;
			} else {
				resize(c, rx, ry, rw - (2*c->bw), (rh / rfacts) + ((i - 2*m->nmaster) < 2*rrest ? 1 : 0) - (2*c->bw), 0);
				ry += HEIGHT(c) + size;
			}
		}
	}
}

void
centeredfloatingmaster(Monitor *m)
{
	unsigned int i, n;
	float mfacts, sfacts;
	int mrest, srest;
	int mx = 0, my = 0, mh = 0, mw = 0;
	int sx = 0, sy = 0, sh = 0, sw = 0;
	Client *c;

	float msizef = 1.0; // master inner vertical gap factor
	int size;
	getgaps(m, &size, &n);

	if (n == 0)
		return;

	sx = mx = m->wx + size;
	sy = my = m->wy + size;
	sh = mh = m->wh - 2*size;
	mw = m->ww - 2*size - size*(n - 1);
	sw = m->ww - 2*size - size*(n - m->nmaster - 1);

	if (m->nmaster && n > m->nmaster) {
		msizef = 0.8;
		/* go mfact box in the center if more than nmaster clients */
		if (m->ww > m->wh) {
			mw = m->ww * m->mfact - size*msizef*(MIN(n, m->nmaster) - 1);
			mh = m->wh * 0.9;
		} else {
			mw = m->ww * 0.9 - size*msizef*(MIN(n, m->nmaster) - 1);
			mh = m->wh * m->mfact;
		}
		mx = m->wx + (m->ww - mw) / 2;
		my = m->wy + (m->wh - mh - 2*size) / 2;

		sx = m->wx + size;
		sy = m->wy + size;
		sh = m->wh - 2*size;
	}

	getfacts(m, mw, sw, &mfacts, &sfacts, &mrest, &srest);

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			/* nmaster clients are stacked horizontally, in the center of the screen */
			resize(c, mx, my, (mw / mfacts) + (i < mrest ? 1 : 0) - (2*c->bw), mh - (2*c->bw), 0);
			mx += WIDTH(c) + size*msizef;
		} else {
			/* stack clients are stacked horizontally */
			resize(c, sx, sy, (sw / sfacts) + ((i - m->nmaster) < srest ? 1 : 0) - (2*c->bw), sh - (2*c->bw), 0);
			sx += WIDTH(c) + size;
		}
}

/*
 * Deck layout + gaps
 * https://dwm.suckless.org/patches/deck/
 */

static void
deck(Monitor *m)
{
	unsigned int i, n;
	int mx = 0, my = 0, mh = 0, mw = 0;
	int sx = 0, sy = 0, sh = 0, sw = 0;
	float mfacts, sfacts;
	int mrest, srest;
	Client *c;

	int size;
	getgaps(m, &size, &n);

	if (n == 0)
		return;

	sx = mx = m->wx + size;
	sy = my = m->wy + size;
	sh = mh = m->wh - 2*size - size * (MIN(n, m->nmaster) - 1);
	sw = mw = m->ww - 2*size;

	if (m->nmaster && n > m->nmaster) {
		sw = (mw - size) * (1 - m->mfact);
		mw = (mw - size) * m->mfact;
		sx = mx + mw + size;
		sh = m->wh - 2*size;
	}

	getfacts(m, mh, sh, &mfacts, &sfacts, &mrest, &srest);

	if (n - m->nmaster > 0) /* sizeerride layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "D %d", n - m->nmaster);

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			resize(c, mx, my, mw - (2*c->bw), (mh / mfacts) + (i < mrest ? 1 : 0) - (2*c->bw), 0);
			my += HEIGHT(c) + size;
		} else {
			resize(c, sx, sy, sw - (2*c->bw), sh - (2*c->bw), 0);
		}
}

/*
 * Fibonacci layout + gaps
 * https://dwm.suckless.org/patches/fibonacci/
 */

static void
fibonacci(Monitor *m, int s)
{
	unsigned int i, n;
	int nx, ny, nw, nh;
	int size;
	Client *c;

	getgaps(m, &size, &n);

	if (n == 0)
		return;

	nx = m->wx + size;
	ny = size;
	nw = m->ww - 2*size;
	nh = m->wh - 2*size;

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next)) {
		if ((i % 2 && nh / 2 > 2*c->bw)
		   || (!(i % 2) && nw / 2 > 2*c->bw)) {
			if (i < n - 1) {
				if (i % 2)
					nh = (nh - size) / 2;
				else
					nw = (nw - size) / 2;

				if ((i % 4) == 2 && !s)
					nx += nw + size;
				else if ((i % 4) == 3 && !s)
					ny += nh + size;
			}
			if ((i % 4) == 0) {
				if (s)
					ny += nh + size;
				else
					ny -= nh + size;
			}
			else if ((i % 4) == 1)
				nx += nw + size;
			else if ((i % 4) == 2)
				ny += nh + size;
			else if ((i % 4) == 3) {
				if (s)
					nx += nw + size;
				else
					nx -= nw + size;
			}
			if (i == 0)	{
				if (n != 1)
					nw = (m->ww - 2*size - size) * m->mfact;
				ny = m->wy + size;
			}
			else if (i == 1)
				nw = m->ww - nw - size - 2*size;
			i++;
		}

		resize(c, nx, ny, nw - (2*c->bw), nh - (2*c->bw), False);
	}
}

static void
dwindle(Monitor *m)
{
	fibonacci(m, 1);
}

static void
spiral(Monitor *m)
{
	fibonacci(m, 0);
}

/*
 * Default tile layout + gaps
 */

static void
tile(Monitor *m)
{
	unsigned int i, n;
	int mx = 0, my = 0, mh = 0, mw = 0;
	int sx = 0, sy = 0, sh = 0, sw = 0;
	float mfacts, sfacts;
	int mrest, srest;
	Client *c;


	int size;
	getgaps(m, &size, &n);

	if (n == 0)
		return;

	sx = mx = m->wx + size;
	sy = my = m->wy + size;
	mh = m->wh - 2*size - size * (MIN(n, m->nmaster) - 1);
	sh = m->wh - 2*size - size * (n - m->nmaster - 1);
	sw = mw = m->ww - 2*size;

	if (m->nmaster && n > m->nmaster) {
		sw = (mw - size) * (1 - m->mfact);
		mw = (mw - size) * m->mfact;
		sx = mx + mw + size;
	}

	getfacts(m, mh, sh, &mfacts, &sfacts, &mrest, &srest);

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			resize(c, mx, my, mw - (2*c->bw), (mh / mfacts) + (i < mrest ? 1 : 0) - (2*c->bw), 0);
			my += HEIGHT(c) + size;
		} else {
			resize(c, sx, sy, sw - (2*c->bw), (sh / sfacts) + ((i - m->nmaster) < srest ? 1 : 0) - (2*c->bw), 0);
			sy += HEIGHT(c) + size;
		}
}
