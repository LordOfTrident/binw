#include <string.h> /* memset, strncpy, strlen */
#include <locale.h> /* setlocale */
#include <stdio.h>  /* snprintf, FILE, fopen, fclose, fread */
#include <stdlib.h> /* size_t, malloc, free */
#include <stdint.h> /* uint8_t */
#include <assert.h> /* assert */
#include <ctype.h>  /* isxdigit */

#include <ncurses.h> /* terminal i/o */

#include "utils.h"

#define RULER_LENGTH 18

struct editor {
	const char *path;

	uint8_t *buff;
	size_t   size, buff_size;
	size_t   buff_pos;

	int curx, cury;
	int scroll;

	int bytes_in_a_row;

	int  cur_ch;
	bool quit, editing;
};

void editor_edit(struct editor *p_editor, const char *p_path);
