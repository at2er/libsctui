/* This file is part of libsctui.
   SPDX-License-Identifier: LGPL-3.0-or-later
*/
#ifndef LIBSCTUI_H
#define LIBSCTUI_H
#include <stddef.h>
#include <stdint.h>
#include <termios.h>

#define SCTUI_KEYBUF_SIZ 3

struct sctui {
	struct termios cur, orig;
	int cursor_x, cursor_y, w, h;

	char *buf;
	size_t buf_size, buf_used;
};

void sctui_clear(void);
void sctui_commit(struct sctui *sctui);
void sctui_cursor(struct sctui *sctui, int x, int y);
void sctui_fini(void);
void sctui_get_win(struct sctui *sctui);
void sctui_grab_key(char keybuf[SCTUI_KEYBUF_SIZ]);
void sctui_hide_cursor(struct sctui *sctui);
void sctui_init(struct sctui *sctui);

/**
 * Handle '\n' to space(' ') and fill white to space.
 * @param buf: char[`w` or more]
 */
void sctui_prepare_text(char *buf,
		int offset, int w,
		const char *text);

void sctui_show_cursor(struct sctui *sctui);

/**
 * Write some text to draw buffer (sctui->buf).
 */
void sctui_text(struct sctui *sctui,
		int x, int y,
		const char *text);

#endif
