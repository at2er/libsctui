/** Simple C terminal user interface
 *
 * Put 'SCTUI_IMPL' to one source file to compile it and use it.
 *
 * Some function will immediately write the control sequence to stdout.
 *
 * Usage: See function declarations.
 *
 * Version:
 *     0.1.1: fix(sctui.h): backspace code.
 *     0.1.0
 *
 * MIT License
 *
 * Copyright (c) 2026 at2er <xb0515@outlook.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef LIBSCTUI_H
#define LIBSCTUI_H
#include <stddef.h>
#include <stdint.h>
#include <termios.h>

#define KBS 127  /* 127 backspace    (But it is DEL in 'man ascii') */
#define KCR '\r' /* 13  carriage ret */
#define KESC 27  /* 27  escape       */

#define KCTRL(K) ((K) & 0x1f)
#define SCTUI_KEYBUF_SIZ 3

struct sctui {
	struct termios cur, orig;
	int cursor_x, cursor_y, w, h;

	char *buf;
	size_t buf_size, buf_used;
};

/* It immediately write the control sequence to terminal. */
extern void sctui_clear(void);

/* Commit draw buffer to terminal by write(). */
extern void sctui_commit(struct sctui *sctui);

/* Move cursor to a position. */
extern void sctui_cursor(struct sctui *sctui, int x, int y);

/* Reset terminal to origin. */
extern void sctui_fini(void);

/* Get terminal window size to 'sctui->{w,h}' */
extern void sctui_get_win(struct sctui *sctui);

/* Get input to keybuf */
extern void sctui_grab_key(int keybuf[SCTUI_KEYBUF_SIZ]);

extern void sctui_hide_cursor(struct sctui *sctui);

/* You must init sctui by this before calling
 * any function needed 'sctui'. */
extern void sctui_init(struct sctui *sctui);

/**
 * Handle '\n' to space(' ') and fill white to space.
 * @param buf: char[`w` or more]
 */
extern void sctui_prepare_text(char *buf,
		int offset, int w,
		const char *text);

extern void sctui_show_cursor(struct sctui *sctui);

/**
 * Write some text to draw buffer (sctui->buf).
 */
extern void sctui_text(struct sctui *sctui,
		int x, int y,
		const char *text);

#endif

#ifdef SCTUI_IMPL
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define ESC_CLEAR_SCREEN     "\x1b[2J"

#define ESC_CLOSE_ALT_SCREEN "\x1b[?1049l"
#define ESC_OPEN_ALT_SCREEN  "\x1b[?1049h"

#define ESC_HIDE_CURSOR      "\x1b[?25l"
#define ESC_SHOW_CURSOR      "\x1b[?25h"

static void
_sctui_buf_append(struct sctui *sctui, const char *text)
{
	size_t siz = strlen(text);
	assert(siz <= BUFSIZ); //FIXME
	if (sctui->buf_used + siz > sctui->buf_size)
		sctui_commit(sctui);
	stpcpy(&sctui->buf[sctui->buf_used], text);
	sctui->buf_used += siz;
}

static void
_sctui_die(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);

	exit(1);
}

static void *
_sctui_ecalloc(size_t nmenb, size_t size)
{
	void *p = calloc(nmenb, size);
	if (!p)
		_sctui_die("failed to calloc\n");
	return p;
}

static struct sctui *global_sctui;

void
sctui_clear(void)
{
	write(STDOUT_FILENO, ESC_CLEAR_SCREEN, 4);
}

void
sctui_commit(struct sctui *sctui)
{
	write(STDOUT_FILENO, sctui->buf, sctui->buf_used);
	sctui->buf_used = 0;
}

void
sctui_cursor(struct sctui *sctui, int x, int y)
{
	char b[32];
	if (sctui->cursor_x == x && sctui->cursor_y == y)
		return;
	sprintf(b, "\x1b[%u;%uH", y, x);
	_sctui_buf_append(sctui, b);
	sctui->cursor_x = x;
	sctui->cursor_y = y;
}

void
sctui_fini(void)
{
	assert(global_sctui);
	write(STDOUT_FILENO, ESC_CLOSE_ALT_SCREEN, 8);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &global_sctui->orig);
}

void
sctui_get_win(struct sctui *sctui)
{
	struct winsize ws;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		sctui_fini();
		_sctui_die("[sctui]: failed to get winsize\n");
	}
	sctui->w = ws.ws_col ? ws.ws_col : 80;
	sctui->h = ws.ws_row ? ws.ws_row : 24;
}

void
sctui_grab_key(int keybuf[SCTUI_KEYBUF_SIZ])
{
	read(STDIN_FILENO, keybuf, 1);
}

void
sctui_hide_cursor(struct sctui *sctui)
{
	_sctui_buf_append(sctui, ESC_HIDE_CURSOR);
}

void
sctui_init(struct sctui *sctui)
{
	if (global_sctui)
		_sctui_die("[sctui]: initialized\n");

	tcgetattr(STDIN_FILENO, &sctui->orig);
	global_sctui = sctui;
	sctui->cur = sctui->orig;
	sctui->cur.c_cflag |= CS8;
	sctui->cur.c_iflag &= ~(IXON | ICRNL);
	sctui->cur.c_lflag &= ~(ECHO | ICANON | ISIG);
	sctui->cur.c_oflag &= ~OPOST;
	sctui->cur.c_cc[VMIN] = 0;
	sctui->cur.c_cc[VTIME] = 1;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &sctui->cur);
	write(STDOUT_FILENO, ESC_OPEN_ALT_SCREEN, 8);
	sctui_get_win(sctui);
	sctui->cursor_x = sctui->cursor_y = 1;
	sctui->buf = _sctui_ecalloc(BUFSIZ, sizeof(char));
	sctui->buf_size = BUFSIZ;
	sctui->buf_used = 0;
	write(STDOUT_FILENO, ESC_CLEAR_SCREEN, strlen(ESC_CLEAR_SCREEN));
}

void
sctui_prepare_text(char *buf,
		int offset, int w,
		const char *text)
{
	int tlen = strlen(text), len;
	int i;
	len = w - offset;
	if (len > tlen)
		len = tlen;
	strncpy(&buf[offset], text, len);
	for (i = 0; i < offset; i++)
		buf[i] = ' ';
	for (i = offset; i < w; i++) {
		if (i < offset + len && buf[i] != '\n')
			continue;
		buf[i] = ' ';
	}
	buf[i] = '\0';
}

void
sctui_show_cursor(struct sctui *sctui)
{
	_sctui_buf_append(sctui, ESC_SHOW_CURSOR);
}

void
sctui_text(struct sctui *sctui,
		int x, int y,
		const char *text)
{
	int ox = sctui->cursor_x, oy = sctui->cursor_y;

	sctui_cursor(sctui, x, y);
	_sctui_buf_append(sctui, text);
	sctui_cursor(sctui, ox, oy);
}
#endif /* SCTUI_IMPL */
