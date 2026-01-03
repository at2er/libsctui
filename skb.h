/**
 * A simple header only library to handle key press with `sctui`
 * in terminal user interface program.
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
 *   SKB_HANDLE_UNKNOWN_KEY -> function(key_buf, key_combo, key_combo_count):
 *     The unknown keys handler.
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
#include "sctui.h"

#ifndef SKB_HANDLE_UNKNOWN_KEY
#define SKB_HANDLE_UNKNOWN_KEY(KEY_BUF, KEY_COMBO, KEY_COMBO_COUNT)
#endif

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
EXP char key_buf[SCTUI_KEYBUF_SIZ];
EXP char key_combo[SKB_MAX_KEYCOMBO];
EXP int  key_combo_count;
#undef EXP

/* functions */
#ifdef SKB_HAS_IMPL
#define FN(SIGN, ...) extern SIGN;
#else
#define FN(SIGN, ...) static SIGN __VA_ARGS__
#endif

static enum _SKB_APPLY_KEY_RESULT
_skb_apply_key(const struct key *key)
{
	for (int i = 0; key_combo[i] == key->keys[i]
			&& i < key_combo_count;
			i++) {
		if (i != key_combo_count - 1)
			continue;
		if (key->keys[i + 1] != '\0')
			return _SKB_APPLY_KEY_USABLE_KEY;
		key->func(&key->arg);
		key_combo_count = 0;
		return _SKB_APPLY_KEY_SUCCESS;
	}
	return _SKB_APPLY_KEY_NOT_FOUND;
}

FN(void
skb_handle_key(void),
{
	const struct key *keys;
	enum _SKB_APPLY_KEY_RESULT ret;
	int usable_key = 0;
	key_combo[key_combo_count++] = key_buf[0];
	keys = get_keys_table();
	for (int i = 0; keys[i].keys != NULL; i++) {
		ret = _skb_apply_key(&keys[i]);
		if (ret == _SKB_APPLY_KEY_SUCCESS)
			return;
		if (ret == _SKB_APPLY_KEY_USABLE_KEY)
			usable_key = 1;
	}
	if (!usable_key) {
		key_combo_count = 0;
		return;
	}

	if (usable_key)
		return;
	SKB_HANDLE_UNKNOWN_KEY(key_buf, key_combo, key_combo_count)
	key_combo_count = 0;
})

#undef FEXP /* functions */

#endif /* SKB_H */

#endif /* SKB_REDEFINE */
