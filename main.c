#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <argp.h>

#include <qlibc/extensions/qconfig.h>

#define MAX_SIZE 42

static char doc[] = "Change brightness by percentage";
static char args_doc[] = "PERCENTAGE";

typedef enum { SET, INC, DEC } op_type;

struct arguments
{
	int value;
	op_type operation;
	char *file;
};


bool is_integer(const char* s) {
	int i = 0, size = strlen(s);
	bool is_integer = true;

	while (i<size && is_integer) {
		if (!isdigit(s[i])) {
			is_integer = false;
		}
		i++;
	}

	return is_integer;
}

char* read_file(const char* filename) {
	FILE *file_pointer = fopen(filename, "r");
	//Check if the file is opened
	if (file_pointer == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	//Get file size
	fseek(file_pointer, 0, SEEK_END);  
	long file_size = ftell(file_pointer);  

	//Allocate some memory to store the file data
	char *file_content = malloc(file_size);  

	//Go to start and read all the file content
	rewind(file_pointer);
	fread(file_content, 1, file_size, file_pointer);  
	fclose(file_pointer);  

	return file_content;
}

int parse_value(const char* val) {
	//TODO trim vals
	if (is_integer(val)) {
		return atoi(val);
	} else if(access(val, F_OK) != -1) {
		char *file_content = read_file(val);

		//Convert to integer and return
		int val = atoi(file_content);
		free(file_content);
		return val;

	} else {
		return -1;
	}
}

void change_brightness(const char* filename, int new_value) {
	char brightness[MAX_SIZE];
	snprintf(brightness, MAX_SIZE, "%d", new_value);

	FILE *file_pointer = fopen(filename, "w");
	//Check if the file is opened
	if (file_pointer == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	fwrite(brightness, strlen(brightness), 1, file_pointer);
	fclose(file_pointer);
}

int parse_options(int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;

	switch (key) {
		case 's':
			arguments->operation = SET;
			break;

		case 'i':
			arguments->operation = INC;
			break;

		case 'd':
			arguments->operation = DEC;
			break;

		case 'c':
			arguments->file = arg;
			break;

		case ARGP_KEY_ARG:
			/* Too many arguments.  */
			if (state->arg_num > 0) {
				argp_usage(state);
			}
			if (is_integer(arg)) {
				arguments->value = atoi(arg);
			}
			break;


		case ARGP_KEY_END:
			/* Not enough arguments.  */
			if (state->arg_num < 1) {
				argp_usage(state);
			}
			break;

		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

int main(int argc __attribute__((unused)), char **argv __attribute__((unused))) {
	struct arguments arguments;

	/* Default values.  */
	arguments.value = -1;
	arguments.operation = INC;
	arguments.file = "config.ini";

	struct argp_option options[] =
	{
		{ "set", 's', 0, 0, "Set brightness to PERCENTAGE %" },
		{ "increase", 'i', 0, 0, "Increase brightness by PERCENTAGE %" },
		{ "decrease", 'd', 0, 0, "Decrease brightness by PERCENTAGE %" },
		{ "config", 'c', "FILE", 0, "Use FILE instead of default (config.ini)" },
		{ 0 }
	};

	struct argp argp = { options, parse_options, args_doc, doc };
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	qlisttbl_t *tbl = qconfig_parse_file(NULL, arguments.file, '=');
	if (tbl) {
		int cur_bright = 50000, max_bright = 100000;

		const char* cur_bright_file = (char*) qlisttbl_get(tbl, "brightness_file", NULL, true);
		if (cur_bright_file) {
			cur_bright = parse_value(cur_bright_file);
		}

		const char* max_bright_val = (char*) qlisttbl_get(tbl, "max_brightness", NULL, true);
		if (max_bright_val) {
			max_bright = parse_value(max_bright_val);
		}

		if (cur_bright_file) {
			if (arguments.value != -1) {
				int new_bright;
				switch(arguments.operation) {
					case SET:
						new_bright = ((arguments.value*max_bright)/100) % max_bright;
						break;

					case INC:
						new_bright = (cur_bright + (arguments.value*max_bright)/100) % max_bright;
						break;

					case DEC:
						new_bright = (cur_bright - (arguments.value*max_bright)/100) % max_bright;
						break;
				}
				//Get new percentage
				printf("New brigthness : %d\n", new_bright);
				change_brightness(cur_bright_file, new_bright);
			} else {
				puts("oups");
			}

		}

	} else {
		perror("Could not parse config.ini\n");
		exit(EXIT_FAILURE);
		return 0;
	}
}
