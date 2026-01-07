/* This file is part of libsctui.
   SPDX-License-Identifier: LGPL-3.0-or-later
*/
#include "sctui.h"
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

static inline char *buf_cur(struct sctui *sctui)
{
	return &sctui->buf[sctui->buf_used];
}

static inline size_t buf_rem(struct sctui *sctui)
{
	return sctui->buf_size - sctui->buf_used;
}

static void die(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);

	exit(1);
}

static void *ecalloc(size_t nmenb, size_t size)
{
	void *p = calloc(nmenb, size);
	if (!p)
		die("failed to calloc\n");
	return p;
}

static struct sctui *global_sctui;

void sctui_clear(void)
{
	write(STDOUT_FILENO, ESC_CLEAR_SCREEN, 4);
}

void sctui_commit(struct sctui *sctui)
{
	write(STDOUT_FILENO, sctui->buf, sctui->buf_used);
	sctui->buf_used = 0;
}

void sctui_cursor(struct sctui *sctui, int x, int y)
{
	sctui->buf_used += snprintf(buf_cur(sctui), buf_rem(sctui),
			"\x1b[%u;%uH", y, x);
	sctui->cursor_x = x;
	sctui->cursor_y = y;
}

void sctui_fini(void)
{
	assert(global_sctui);
	write(STDOUT_FILENO, ESC_CLOSE_ALT_SCREEN, 8);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &global_sctui->orig);
}

void sctui_get_win(struct sctui *sctui)
{
	struct winsize ws;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		sctui_fini();
		die("[sctui]: failed to get winsize\n");
	}
	sctui->w = ws.ws_col ? ws.ws_col : 80;
	sctui->h = ws.ws_row ? ws.ws_row : 24;
}

void sctui_grab_key(int keybuf[SCTUI_KEYBUF_SIZ])
{
	read(STDIN_FILENO, keybuf, 1);
}

void sctui_hide_cursor(struct sctui *sctui)
{
	sctui->buf_used += snprintf(buf_cur(sctui), buf_rem(sctui),
			ESC_HIDE_CURSOR);
}

void sctui_init(struct sctui *sctui)
{
	if (global_sctui)
		die("[sctui]: initialized\n");

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
	sctui->buf = ecalloc(BUFSIZ, sizeof(char));
	sctui->buf_size = BUFSIZ;
	sctui->buf_used = 0;
	write(STDOUT_FILENO, ESC_CLEAR_SCREEN, strlen(ESC_CLEAR_SCREEN));
}

void sctui_prepare_text(char *buf,
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

void sctui_show_cursor(struct sctui *sctui)
{
	sctui->buf_used += snprintf(buf_cur(sctui), buf_rem(sctui),
			ESC_SHOW_CURSOR);
}

void sctui_text(struct sctui *sctui,
		int x, int y,
		const char *text)
{
	int ox = sctui->cursor_x, oy = sctui->cursor_y;
	sctui_cursor(sctui, x, y);
	sctui->buf_used += snprintf(buf_cur(sctui), buf_rem(sctui),
			"%s", text);
	sctui_cursor(sctui, ox, oy);
}
