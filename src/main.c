#include "main.h"

void usage(void) {
	puts("Usage: "APP_NAME" [FILE] [OPTIONS]\n"
	     "Options:\n"
	     "  -h, --help     Show this message\n"
	     "  -v, --version  Print the version\n"
	     "\nControls:\n"
	     "  q       Quit the program or editing mode\n"
	     "  s       Save the file\n"
	     "  e       Go to editing mode\n"
	     "  f       Scroll down\n"
	     "  r       Scroll up\n"
	     "  i       Increase the buffer by 1\n"
	     "  d       Decrease the buffer by 1\n"
	     "  arrows  Move the cursor\n"
	     "\nWhen in editing mode, you are typing the byte digits in hexadecimal");

	exit(EXIT_SUCCESS);
}

void version(void) {
	printf(APP_NAME" %i.%i.%i\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

	exit(EXIT_SUCCESS);
}

int main(int p_argc, char **p_argv) {
	const char *path = NULL;

	for (int i = 1; i < p_argc; ++ i) {
		if (strcmp(p_argv[i], "-h") == 0 || strcmp(p_argv[i], "--help") == 0)
			usage();
		else if (strcmp(p_argv[i], "-v") == 0 || strcmp(p_argv[i], "--version") == 0)
			version();
		else if (path != NULL) {
			fatal("Unexpected argument '%s'", p_argv[i]);

			exit(EXIT_FAILURE);
		} else
			path = p_argv[i];
	}

	if (path == NULL)
		fatal("No file path specified");

	struct editor editor;
	editor_edit(&editor, path);

	return EXIT_SUCCESS;
}
