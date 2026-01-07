/** Simple Key binding
 * A simple header only library to handle key press with `sctui`
 * in terminal user interface program.
 *
 * @important: `get_keys_table` must be declared before include this file.
 * `const struct key *get_keys_table(void)`
 *
 * Keys:
 *   - Control (ctrl):
 *     Starting with '^' means 'ctrl + X', such as "^c" means 'ctrl + c'.
 *     ( see **Special Characters** )
 *
 *   - Shift:
 *     * Visible ascii characters: shift changes their certain key.
 *     ( see **Special Characters** )
 *
 *   - Special Characters:
 *     * Backspace: "/b"
 *     * '^': "^^"
 *     * '/': "//"
 *
 * Option macros: #bool(defined: true, undefined: false)
 *   SKB_MAX_KEYCOMBO -> int
 *
 *   SKB_HAS_IMPL -> bool:
 *     Pass this macro with compiler flag '-DSKB_HAS_IMPL',
 *     and define `SKB_IMPL` in a file without '-DSKB_HAS_IMPL'.
 *
 *   SKB_IMPL -> bool:
 *     Put implment to a file and other files need `SKB_HAS_IMPL`
 *     to use this library.
 *
 *   SKB_REDEFINE -> bool:
 *     Redefine struct and union of `skb`.
 *     The default content is defined in `SKB_DEFINE_ARG` and `SKB_DEFINE_KEY`.
 *     After redefine, you must undefine this macro:
 *     `#undef SKB_REDEFINE`
 *     And define these macros: `SKB_DEFINED_ARG` and `SKB_DEFINED_KEY`.
 *
 *   SKB_DEFINED_ARG, SKB_DEFINED_KEY -> bool:
 *     The `union arg` or `struct key` is defined by yourself.
 *
 * MIT License
 *
 * Copyright (c) 2025 at2er <xb0515@outlook.com>
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
#define SKB_DEFINE_ARG \
	int i; \
	const char *s; \
	unsigned int ui; \
	void *v;
#define SKB_DEFINE_KEY \
	const char *keys; \
	void (*func)(const union arg *arg); \
	const union arg arg;

#ifndef SKB_REDEFINE
#ifndef SKB_H
#define SKB_H
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include "sctui.h"

#ifndef SKB_MAX_KEYCOMBO
#define SKB_MAX_KEYCOMBO 5
#endif

enum _SKB_APPLY_KEY_RESULT {
	_SKB_APPLY_KEY_NOT_FOUND,
	_SKB_APPLY_KEY_SUCCESS,
	_SKB_APPLY_KEY_USABLE_KEY
};

#ifndef SKB_DEFINED_ARG
union arg { SKB_DEFINE_ARG };
#endif

#ifndef SKB_DEFINED_KEY
struct key { SKB_DEFINE_KEY };
#endif

#ifdef SKB_HAS_IMPL
#define EXP extern
#else
#define EXP static
#endif
EXP int  key_buf[SCTUI_KEYBUF_SIZ];
EXP char key_combo[SKB_MAX_KEYCOMBO];
EXP int  key_combo_count;
#undef EXP

/* functions */
#ifdef SKB_HAS_IMPL
#define FDECLARE ;
#define FEXP extern
#else
#define FDECLARE
#define FEXP static
#endif

static int
_skb_compare_key(const char *key, int pressed)
{
#define SPECIAL_CHR(C) (pressed == (C) && key[0] == (C) && key[1] == (C))
	if (pressed == KBACKSPACE) {
		if (strncmp(key, "/b", 2) == 0)
			return 2;
		return 0;
	} else if (iscntrl(pressed)) {
		if (key[0] != '^')
			return 0;
		if (KCTRL(key[1]) == KCTRL(pressed))
			return 2;
	} else if (SPECIAL_CHR('/')) {
		return 2;
	} else if (SPECIAL_CHR('^')) {
		return 2;
	}
#undef SPECIAL_CHR
	if (pressed != key[0])
		return 0;
	return 1;
}

static enum _SKB_APPLY_KEY_RESULT
_skb_apply_key(const struct key *key)
{
	int r;
	for (int i = 0, ki = 0; i < key_combo_count; i++) {
		r = _skb_compare_key(&key->keys[ki], key_combo[i]);
		if (r < 1)
			break;
		ki += r;
		if (i != key_combo_count - 1)
			continue;
		if (key->keys[ki] != '\0')
			return _SKB_APPLY_KEY_USABLE_KEY;
		key->func(&key->arg);
		key_combo_count = 0;
		return _SKB_APPLY_KEY_SUCCESS;
	}
	return _SKB_APPLY_KEY_NOT_FOUND;
}

FEXP bool
skb_handle_key(void) FDECLARE
#ifndef SKB_HAS_IMPL
{
	const struct key *keys;
	enum _SKB_APPLY_KEY_RESULT ret;
	bool usable_key = false;
	key_combo[key_combo_count++] = key_buf[0];
	keys = get_keys_table();
	for (int i = 0; keys[i].keys != NULL; i++) {
		ret = _skb_apply_key(&keys[i]);
		if (ret == _SKB_APPLY_KEY_SUCCESS)
			return true;
		if (ret == _SKB_APPLY_KEY_USABLE_KEY)
			usable_key = true;
	}
	return usable_key;
}
#endif

#undef FEXP /* functions */

#endif /* SKB_H */

#endif /* SKB_REDEFINE */
