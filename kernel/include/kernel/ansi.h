#ifndef _KERNEL_ANSI_H
#define _KERNEL_ANSI_H

#include <stdint.h>

// from https://github.com/64/cansid

// Copyright 2017 64
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

struct ansi_state {
	enum {
		ANSI_ESC,
		ANSI_BRACKET,
		ANSI_PARSE,
		ANSI_BGCOLOR,
		ANSI_FGCOLOR,
		ANSI_EQUALS,
		ANSI_ENDVAL,
	} state;
	uint8_t style;
	uint8_t next_style;
};

struct color_char {
	uint8_t style;
	uint8_t ascii;
};

struct ansi_state ansi_init(void);
struct color_char ansi_process(struct ansi_state *state, char x);

#endif