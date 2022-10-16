#include "editor.h"

static void editor_increase_buff(struct editor *p_editor) {
	++ p_editor->size;
	if (p_editor->size > p_editor->buff_size) {
		p_editor->buff_size *= 2;

		void *tmp = realloc(p_editor->buff, p_editor->buff_size);
		if (tmp == NULL) {
			free(p_editor->buff);
			assert(0 && "realloc fail");
		}

		p_editor->buff = (uint8_t*)tmp;
	}

	p_editor->buff[p_editor->size - 1] = 0;
}

static void editor_decrease_buff(struct editor *p_editor) {
	-- p_editor->size;
}

static void editor_calc_bytes_in_a_row(struct editor *p_editor) {
	int w = getmaxx(stdscr), h = getmaxy(stdscr);

	if (w < RULER_LENGTH + 4 || h < 4) {
		free(p_editor->buff);
		endwin();

		fatal("Window is too small");
	}

	p_editor->bytes_in_a_row = (w - RULER_LENGTH) / 3;
}

static void editor_calc_buff_pos(struct editor *p_editor) {
	p_editor->buff_pos = p_editor->cury * p_editor->bytes_in_a_row + p_editor->curx;
}

static void editor_save_file(struct editor *p_editor) {
	FILE *file = fopen(p_editor->path, "w");
	if (file == NULL) {
		free(p_editor->buff);
		endwin();

		fatal("Could not open file '%s' for writing", p_editor->path);
	}

	fwrite(p_editor->buff, p_editor->size, 1, file);

	fclose(file);
}

static void editor_render(struct editor *p_editor) {
	erase();

	curs_set(0);

	move(0, 0);
	printw("%s %s", p_editor->editing? "EDITING" : "READING", p_editor->path);

	int curx_render = 0, cury_render = 0;

	size_t   i = p_editor->scroll * p_editor->bytes_in_a_row;
	bool print = i < p_editor->size;

	if (print) {
		move(1, 1);
		printw("%014llX | ", (unsigned long long)i);
	}

	int x = 0, y = 0;
	for (; i < p_editor->size; ++ i) {
		bool is_selected = x == p_editor->curx && y == p_editor->cury - p_editor->scroll;
		if (is_selected) {
			cury_render = y + 1;
			curx_render = x * 3 + RULER_LENGTH;

			if (!p_editor->editing)
				attron(A_REVERSE);
		}

		printw("%02X", p_editor->buff[i]);

		if (is_selected && !p_editor->editing)
			attroff(A_REVERSE);

		addch(' ');

		if ((x + 1) % p_editor->bytes_in_a_row == 0) {
			if (i + 1 >= p_editor->size)
				continue;

			x = 0;
			++ y;

			move(y + 1, 1);
			printw("%014llX | ", (unsigned long long)i + 1);
		} else
			++ x;
	}

	move(y + 1 + (int)print, 1);
	printw("%014llX | ", (unsigned long long)p_editor->size);

	if (p_editor->editing) {
		move(cury_render, curx_render + (1 - p_editor->cur_ch));

		curs_set(1);
	}
}

static void editor_input(struct editor *p_editor) {
	int in = getch();

	/* General input */
	switch (in) {
	case KEY_RESIZE:
		editor_calc_bytes_in_a_row(p_editor);

		p_editor->curx = p_editor->buff_pos % p_editor->bytes_in_a_row;
		p_editor->cury = p_editor->buff_pos / p_editor->bytes_in_a_row;

		p_editor->scroll = 0;

		break;

	default: break;
	}

	if (p_editor->editing) { /* Editing mode input */
		switch (in) {
		case 'q': p_editor->editing = false; break;

		default:
			if (!isxdigit(in))
				break;

			char buff[2] = {in, 0};
			int  num     = strtol(buff, NULL, 16);

			int mult = (16 * p_editor->cur_ch);
			if (mult == 0)
				mult = 1;

			p_editor->buff[p_editor->buff_pos] += num * mult;

			-- p_editor->cur_ch;
			if (p_editor->cur_ch < 0)
				p_editor->editing = false;
		}
	} else { /* Reading mode input */
		switch (in) {
		case 'q': p_editor->quit = true;      break;
		case 's': editor_save_file(p_editor); break;
		case 'e':
			p_editor->editing = true;
			p_editor->cur_ch  = 1;
			p_editor->buff[p_editor->buff_pos] = 0;

			break;

		case 'i': editor_increase_buff(p_editor); break;
		case 'd': editor_decrease_buff(p_editor); break;

		case 'f':
			++ p_editor->scroll;
			size_t calc = p_editor->scroll * p_editor->bytes_in_a_row;
			if (calc > p_editor->size)
				-- p_editor->scroll;

			break;
		case 'r':
			if (p_editor->scroll > 0)
				-- p_editor->scroll;

			break;

		case KEY_UP:
			if (p_editor->cury > 0)
				-- p_editor->cury;
			else
				editor_calc_buff_pos(p_editor);

			break;

		case KEY_DOWN:
			++ p_editor->cury;
			editor_calc_buff_pos(p_editor);
			if (p_editor->buff_pos >= p_editor->size ||
			    p_editor->cury > (int)p_editor->size / p_editor->bytes_in_a_row)
				-- p_editor->cury;

			editor_calc_buff_pos(p_editor);

			break;

		case KEY_LEFT:
			if (p_editor->curx > 0)
				-- p_editor->curx;
			else
				editor_calc_buff_pos(p_editor);

			break;

		case KEY_RIGHT:
			++ p_editor->curx;
			editor_calc_buff_pos(p_editor);
			if (p_editor->buff_pos >= p_editor->size ||
			    p_editor->curx > p_editor->bytes_in_a_row - 1)
				-- p_editor->curx;

			editor_calc_buff_pos(p_editor);

			break;

		default: break;
		}
	}
}

static void editor_main_loop(struct editor *p_editor) {
	do {
		editor_render(p_editor);
		editor_input(p_editor);
	} while (!p_editor->quit);
}

void editor_edit(struct editor *p_editor, const char *p_path) {
	memset(p_editor, 0, sizeof(struct editor));
	p_editor->path = p_path;

	/* Read the file if it exists and set up the buffer */
	FILE *file = fopen(p_path, "r");
	if (file != NULL) {
		fseek(file, 0, SEEK_END);
		p_editor->size = ftell(file);
		rewind(file);
	} else
		p_editor->size = 1;

	p_editor->buff = (uint8_t*)malloc(p_editor->size);
	if (p_editor->buff == NULL)
		assert(0 && "malloc fail");

	if (p_editor->size == 0)
		p_editor->size = 1;
	else if (file != NULL) {
		size_t ret = fread(p_editor->buff, p_editor->size, 1, file);
		if (ret < 1)
			fatal("Error while reading '%s'", p_path);

		fclose(file);
	}

	p_editor->buff_size = p_editor->size;

	setlocale(LC_CTYPE, "");

	initscr();

	raw();
	noecho();
	keypad(stdscr, true);
	curs_set(0);

	editor_calc_bytes_in_a_row(p_editor);

	/* setting the terminal title */

	char path[32] = {0};
	strncpy(path, p_path, sizeof(path) - 1);

	/* if the path is longer than 15 characters, cut it and add ... at the end */
	if (strlen(p_path) > sizeof(path) - 1)
		memset(path + sizeof(path) - 4, '.', 3);

	char buff[64] = {0};
	snprintf(buff, sizeof(buff), "\033]0;binw (%s)\007\n", path);

	putp(buff);

	editor_main_loop(p_editor);

	free(p_editor->buff);

	curs_set(1);
	endwin();
}
