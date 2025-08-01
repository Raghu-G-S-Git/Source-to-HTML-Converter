
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "s2html_event.h"

#define SIZE_OF_SYMBOLS (sizeof(symbols))
#define SIZE_OF_OPERATORS (sizeof(operators))
#define WORD_BUFF_SIZE 100

/********** Internal states and event of parser **********/
typedef enum
{
	PSTATE_IDLE,
	PSTATE_PREPROCESSOR_DIRECTIVE,
	PSTATE_SUB_PREPROCESSOR_MAIN,
	PSTATE_SUB_PREPROCESSOR_RESERVE_KEYWORD,
	PSTATE_SUB_PREPROCESSOR_ASCII_CHAR,
	PSTATE_HEADER_FILE,
	PSTATE_RESERVE_KEYWORD,
	PSTATE_NUMERIC_CONSTANT,
	PSTATE_STRING,
	PSTATE_SINGLE_LINE_COMMENT,
	PSTATE_MULTI_LINE_COMMENT,
	PSTATE_ASCII_CHAR
} pstate_e;

/********** global variables **********/

/* parser state variable */
static pstate_e state = PSTATE_IDLE;

/* sub state is used only in preprocessor state */
static pstate_e state_sub = PSTATE_SUB_PREPROCESSOR_MAIN;

/* event variable to store event and related properties */
static pevent_t pevent_data;
static int event_data_idx = 0;

static char word[WORD_BUFF_SIZE];
static int word_idx = 0;

static char *res_kwords_data[] = {"const", "volatile", "extern", "auto", "register",
								  "static", "signed", "unsigned", "short", "long",
								  "double", "char", "int", "float", "struct",
								  "union", "enum", "void", "typedef", ""};

static char *res_kwords_non_data[] = {"goto", "return", "continue", "break",
									  "if", "else", "for", "while", "do",
									  "switch", "case", "default", "sizeof", ""};

static char operators[] = {'/', '+', '*', '-', '%', '=', '<', '>', '~', '&', ',', '!', '^', '|'};
static char symbols[] = {'(', ')', '{', '}', '[', ']', ':'};

/********** state handlers **********/
pevent_t *pstate_idle_handler(FILE *fd, int ch);
pevent_t *pstate_single_line_comment_handler(FILE *fd, int ch);
pevent_t *pstate_multi_line_comment_handler(FILE *fd, int ch);
pevent_t *pstate_numeric_constant_handler(FILE *fd, int ch);
pevent_t *pstate_string_handler(FILE *fd, int ch);
pevent_t *pstate_header_file_handler(FILE *fd, int ch);
pevent_t *pstate_ascii_char_handler(FILE *fd, int ch);
pevent_t *pstate_reserve_keyword_handler(FILE *fd, int ch);
pevent_t *pstate_preprocessor_directive_handler(FILE *fd, int ch);
pevent_t *pstate_sub_preprocessor_main_handler(FILE *fd, int ch);

/********** Utility functions **********/

/* function to check if given word is reserved key word */
static int is_reserved_keyword(char *word)
{
	int idx = 0;

	/* search for data type reserved keyword */
	while (*res_kwords_data[idx])
	{
		if (strcmp(res_kwords_data[idx++], word) == 0)
			return RES_KEYWORD_DATA;
	}

	idx = 0; // reset index
	/* search for non data type reserved key word */
	while (*res_kwords_non_data[idx])
	{
		if (strcmp(res_kwords_non_data[idx++], word) == 0)
			return RES_KEYWORD_NON_DATA;
	}

	return 0; // word did not match, return false
}

/* function to check symbols */
static int is_symbol(char c)
{
	int idx;
	for (idx = 0; idx < SIZE_OF_SYMBOLS; idx++)
	{
		if (symbols[idx] == c)
			return 1;
	}

	return 0;
}

/* function to check operator */
static int is_operator(char c)
{
	int idx;
	for (idx = 0; idx < SIZE_OF_OPERATORS; idx++)
	{
		if (operators[idx] == c)
			return 1;
	}

	return 0;
}

/* to set parser event */
static void set_parser_event(pstate_e s, pevent_e e)
{
	pevent_data.data[event_data_idx] = '\0';
	pevent_data.length = event_data_idx;
	event_data_idx = 0;
	state = s;
	pevent_data.type = e;
}

/************ Event functions **********/

/* This function parses the source file and generate
 * event based on parsed characters and string
 */
pevent_t *get_parser_event(FILE *fd)
{
	int ch, pre_ch;
	pevent_t *evptr = NULL; /*    NULL    */
	/* Read char by char */
	while ((ch = fgetc(fd)) != EOF)
	{
#ifdef DEBUG
		//	putchar(ch);
#endif
		switch (state)
		{
		case PSTATE_IDLE:
			if ((evptr = pstate_idle_handler(fd, ch)) != NULL)
				return evptr;
			break;
		case PSTATE_SINGLE_LINE_COMMENT:
			if ((evptr = pstate_single_line_comment_handler(fd, ch)) != NULL)
				return evptr;
			break;
		case PSTATE_MULTI_LINE_COMMENT:
			if ((evptr = pstate_multi_line_comment_handler(fd, ch)) != NULL)
				return evptr;
			break;
		case PSTATE_PREPROCESSOR_DIRECTIVE:
			if ((evptr = pstate_preprocessor_directive_handler(fd, ch)) != NULL)
				return evptr;
			break;
		case PSTATE_RESERVE_KEYWORD:
			if ((evptr = pstate_reserve_keyword_handler(fd, ch)) != NULL)
				return evptr;
			break;
		case PSTATE_NUMERIC_CONSTANT:
			if ((evptr = pstate_numeric_constant_handler(fd, ch)) != NULL)
				return evptr;
			break;
		case PSTATE_STRING:
			if ((evptr = pstate_string_handler(fd, ch)) != NULL)
				return evptr;
			break;
		case PSTATE_HEADER_FILE:
			if ((evptr = pstate_header_file_handler(fd, ch)) != NULL)
				return evptr;
			break;
		case PSTATE_ASCII_CHAR:
			if ((evptr = pstate_ascii_char_handler(fd, ch)) != NULL)
				return evptr;
			break;
		default:
			printf("unknown state\n");
			state = PSTATE_IDLE;
			break;
		}
	}

	/* end of file is reached, move back to idle state and set EOF event */
	set_parser_event(PSTATE_IDLE, PEVENT_EOF);

	return &pevent_data; // return final event
}

/********** IDLE state Handler **********
 * Idle state handler identifies
 ****************************************/

pevent_t *pstate_idle_handler(FILE *fd, int ch)
{
	int pre_ch;
	switch (ch)
	{
	case '\'': // begining of ASCII char
		if (event_data_idx)
		{
			fseek(fd, -1L, SEEK_CUR); // unget chars
			set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
			return &pevent_data;
		}
		else
		{
			state = PSTATE_ASCII_CHAR;
			pevent_data.data[event_data_idx++] = ch;
		}
		break;
	case '/':
		pre_ch = ch;
		if ((ch = fgetc(fd)) == '*') // multi line comment
		{
			if (event_data_idx) // we have regular exp in buffer first process that
			{
				fseek(fd, -2L, SEEK_CUR); // unget chars
				set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
				return &pevent_data;
			}
			else //	multi line comment begin
			{
#ifdef DEBUG
				printf("Multi line comment Begin : /*\n");
#endif
				state = PSTATE_MULTI_LINE_COMMENT;
				pevent_data.data[event_data_idx++] = pre_ch;
				pevent_data.data[event_data_idx++] = ch;
			}
		}
		else if (ch == '/') // single line comment
		{
			if (event_data_idx) // we have regular exp in buffer first process that
			{
				fseek(fd, -2L, SEEK_CUR); // unget chars
				set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
				return &pevent_data;
			}
			else //	single line comment begin
			{
#ifdef DEBUG
				printf("Single line comment Begin : //\n");
#endif
				state = PSTATE_SINGLE_LINE_COMMENT;
				pevent_data.data[event_data_idx++] = pre_ch;
				pevent_data.data[event_data_idx++] = ch;
			}
		}
		else // it is regular exp
		{
			fseek(fd, -1L, SEEK_CUR); // unget chars
			pevent_data.data[event_data_idx++] = pre_ch;
			// pevent_data.data[event_data_idx++] = ch;
		}
		break;
	case '#':
		if (event_data_idx) // we have regular exp in buffer first process that
		{
			fseek(fd, -1L, SEEK_CUR); // unget chars
			set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
			return &pevent_data;
		}
		else
		{
			state = PSTATE_PREPROCESSOR_DIRECTIVE;
			state_sub = PSTATE_SUB_PREPROCESSOR_MAIN;
			pevent_data.data[event_data_idx++] = ch;
		}
		break;
	case '\"':
		if (event_data_idx) // we have regular exp in buffer first process that
		{
			fseek(fd, -1L, SEEK_CUR); // unget chars
			set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
			return &pevent_data;
		}
		else //	String begins
		{
			state = PSTATE_STRING;
			pevent_data.data[event_data_idx++] = ch;
		}
		break;

	case '0' ... '9': // detect numeric constant
		if (event_data_idx)
		{
			fseek(fd, -1L, SEEK_CUR);
			set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
			return &pevent_data;
		}
		else
		{
			state = PSTATE_NUMERIC_CONSTANT;
			pevent_data.data[event_data_idx++] = ch;
		}
		break;

	case 'a' ... 'z': // could be reserved key word
		if (event_data_idx)
		{
			fseek(fd, -1L, SEEK_CUR);
			set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
			return &pevent_data;
		}
		else
		{
			state = PSTATE_RESERVE_KEYWORD;
			pevent_data.data[event_data_idx++] = ch;
		}
		break;

	default: // Assuming common text starts by default.
		pevent_data.data[event_data_idx++] = ch;
		// if(is_operator(ch) == 1)
		// {

		// 	return &pevent_data;
		// }
		break;
	}

	return NULL;
}

pevent_t *pstate_preprocessor_directive_handler(FILE *fd, int ch)
{

	int tch = ch;
	// state_sub = PSTATE_SUB_PREPROCESSOR_MAIN;

	// while ((tch = fgetc(fd)) != EOF)
	while (ch != EOF)
	{
		switch (state_sub)
		{
		case PSTATE_SUB_PREPROCESSOR_MAIN:
			return pstate_sub_preprocessor_main_handler(fd, tch);
		case PSTATE_HEADER_FILE:
			return pstate_header_file_handler(fd, tch);
		// case PSTATE_SUB_PREPROCESSOR_RESERVE_KEYWORD:
		// 	return pstate_reserve_keyword_handler(fd, tch);
		case PSTATE_SUB_PREPROCESSOR_ASCII_CHAR:
			return pstate_ascii_char_handler(fd, tch);
		default:
			printf("Unknown state\n");
			state_sub = PSTATE_IDLE;
			state = PSTATE_IDLE; // Reset to IDLE on error
			set_parser_event(PSTATE_IDLE, PEVENT_PREPROCESSOR_DIRECTIVE);
			return &pevent_data;
		}
	}

	return NULL; // Return NULL if EOF is reached

	// switch (state_sub)
	// {
	// case PSTATE_SUB_PREPROCESSOR_MAIN:
	// 	return pstate_sub_preprocessor_main_handler(fd, ch);
	// case PSTATE_SUB_PREPROCESSOR_RESERVE_KEYWORD:
	// 	return pstate_reserve_keyword_handler(fd, ch);
	// case PSTATE_SUB_PREPROCESSOR_ASCII_CHAR:
	// 	return pstate_ascii_char_handler(fd, ch);
	// default:
	// 	printf("unknown state\n");
	// 	state = PSTATE_IDLE;
	// }

	// return NULL;
}

pevent_t *pstate_sub_preprocessor_main_handler(FILE *fd, int ch)
{
	/* write a switch case here to detect several events here
	 * This state is similar to Idle state with slight difference
	 * in state transition.
	 * return event data at the end of event
	 * else return NULL
	 */
	switch (ch)
	{
	case '<': // Start of a header file
	case '\"':
		if (event_data_idx)
		{
			fseek(fd, -1L, SEEK_CUR);
			set_parser_event(PSTATE_PREPROCESSOR_DIRECTIVE, PEVENT_PREPROCESSOR_DIRECTIVE);
			// set_parser_event(PSTATE_PREPROCESSOR_DIRECTIVE, PEVENT_HEADER_FILE);
			// state_sub = PSTATE_HEADER_FILE;
			return &pevent_data;
		}

		// pevent_data.data[event_data_idx++] = ch;
		// ch = fgetc(fd);
		// if (ch == 'l')
		// 	// pevent_data.data[event_data_idx++] = ch;
		// ch = fgetc(fd);
		// if (ch == 't')
		state_sub = PSTATE_HEADER_FILE;
		// pevent_data.data[event_data_idx++] = ch;
		break;

		// case 'a' ... 'z': // Part of a keyword or identifier
		// case 'A' ... 'Z':
		// case '_':
		// 	state_sub = PSTATE_SUB_PREPROCESSOR_RESERVE_KEYWORD;
		// 	pevent_data.data[event_data_idx++] = ch;
		// 	break;

	case '\'': // ASCII character in directive
		if (event_data_idx)
		{
			fseek(fd, -1L, SEEK_CUR);
			set_parser_event(PSTATE_PREPROCESSOR_DIRECTIVE, PEVENT_PREPROCESSOR_DIRECTIVE);
			return &pevent_data;
		}
		state_sub = PSTATE_SUB_PREPROCESSOR_ASCII_CHAR;
		pevent_data.data[event_data_idx++] = ch;
		break;

	case ' ':
		if(event_data_idx)
		{
			fseek(fd, -1L, SEEK_CUR);
			set_parser_event(PSTATE_PREPROCESSOR_DIRECTIVE, PEVENT_PREPROCESSOR_DIRECTIVE);
			return &pevent_data;
		}
		else
		{
			pevent_data.data[event_data_idx++] = ch;
			while(ch = fgetc(fd))
			{
				if(ch != ' ')
				{
					break;
				}
				pevent_data.data[event_data_idx++] = ch;
			}
			if(ch == '\"' || ch == '<')
			{
				fseek(fd, -1L, SEEK_CUR);
				set_parser_event(PSTATE_PREPROCESSOR_DIRECTIVE, PEVENT_PREPROCESSOR_DIRECTIVE);
				return &pevent_data;
			}
			else
			{
				fseek(fd, -1L, SEEK_CUR);
				set_parser_event(PSTATE_IDLE, PEVENT_PREPROCESSOR_DIRECTIVE);
				return &pevent_data;
			}
		}
	// case '0' ... '9': // detect numeric constant
	// 	if (event_data_idx)
	// 	{
	// 		fseek(fd, -1L, SEEK_CUR);
	// 		set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
	// 		return &pevent_data;
	// 	}
	// 	else
	// 	{
	// 		state = PSTATE_NUMERIC_CONSTANT;
	// 		pevent_data.data[event_data_idx++] = ch;
	// 	}
	// 	break;
	
	case '\n': // End of directive
		pevent_data.data[event_data_idx++] = ch;
		set_parser_event(PSTATE_IDLE, PEVENT_PREPROCESSOR_DIRECTIVE);
		// set_parser_event(PSTATE_PREPROCESSOR_DIRECTIVE, PEVENT_REGULAR_EXP);
		return &pevent_data;

	default: // Collect other characters as part of directive
		pevent_data.data[event_data_idx++] = ch;
		break;
	}

	return NULL;
}

pevent_t *pstate_header_file_handler(FILE *fd, int ch)
{
	/* write a switch case here to store header file name
	 * return event data at the end of event
	 * else return NULL
	 */
	while (ch != EOF)
	{
		switch (ch)
		{
		// case '&': // End of header file
		// 	ch = fgetc(fd);
		// 	if (ch == 'g')
		// 	ch = fgetc(fd);
		// 	if (ch == 't')
		case '>':
			state_sub = PSTATE_HEADER_FILE;

			set_parser_event(PSTATE_IDLE, PEVENT_HEADER_FILE);
			pevent_data.property = STD_HEADER_FILE;
			return &pevent_data;
		case '\"':
			state_sub = PSTATE_HEADER_FILE;
			set_parser_event(PSTATE_IDLE, PEVENT_HEADER_FILE);
			pevent_data.property = USER_HEADER_FILE;
			return &pevent_data;
		case '\n': // Unexpected newline, return as-is
			pevent_data.data[event_data_idx++] = ch;
			set_parser_event(PSTATE_IDLE, PEVENT_HEADER_FILE);
			pevent_data.property = STD_HEADER_FILE;
			return &pevent_data;

		default: // Collect header file characters
			pevent_data.data[event_data_idx++] = ch;
			break;
		}

		ch = fgetc(fd); // Fetch the next character
	}

	return NULL;
}

pevent_t *pstate_reserve_keyword_handler(FILE *fd, int ch)
{
	/* write a switch case here to store words
	 * return event data at the end of event
	 * else return NULL
	 */
	switch (ch)
	{
	case 'a' ... 'z':
		pevent_data.data[event_data_idx++] = ch;
		break;
	case '_':
		pevent_data.data[event_data_idx++] = ch;
		break;
	case 'A' ... 'Z':
		pevent_data.data[event_data_idx++] = ch;
		break;
	case '0' ... '9':
		pevent_data.data[event_data_idx++] = ch;
		break;
	default:
		fseek(fd, -1L, SEEK_CUR);
		pevent_data.data[event_data_idx++] = '\0';

		int res = is_reserved_keyword(pevent_data.data);

		if (res == RES_KEYWORD_DATA)
		{
			set_parser_event(PSTATE_IDLE, PEVENT_RESERVE_KEYWORD);
			pevent_data.property = RES_KEYWORD_DATA;
			return &pevent_data;
		}
		else if (res == RES_KEYWORD_NON_DATA)
		{
			set_parser_event(PSTATE_IDLE, PEVENT_RESERVE_KEYWORD);
			pevent_data.property = RES_KEYWORD_NON_DATA;
			return &pevent_data;
		}
		else
		{
			set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
			return &pevent_data;
		}
		break;
	}

	return NULL;
}
pevent_t *pstate_numeric_constant_handler(FILE *fd, int ch)
{
	/* write a switch case here to store digits
	 * return event data at the end of event
	 * else return NULL
	 */
	switch (ch)
	{
	case '0' ... '9':
		pevent_data.data[event_data_idx++] = ch;
		break;
	case '\n':
		pevent_data.data[event_data_idx++] = ch;
		break;
	case '.':
		pevent_data.data[event_data_idx++] = ch;
		break;
	default:
		fseek(fd, -1L, SEEK_CUR);
		set_parser_event(PSTATE_IDLE, PEVENT_NUMERIC_CONSTANT);
		return &pevent_data;
		break;
	}
	return NULL;
}
pevent_t *pstate_string_handler(FILE *fd, int ch)
{
	/* write a switch case here to store string
	 * return event data at the end of event
	 * else return NULL
	 */
	switch (ch)
	{
	case '\"': // string ends here
		pevent_data.data[event_data_idx++] = ch;
		set_parser_event(PSTATE_IDLE, PEVENT_STRING);
		return &pevent_data;
	case '\n':
		pevent_data.data[event_data_idx++] = ch;
		set_parser_event(PSTATE_IDLE, PEVENT_STRING);
		return &pevent_data;
	default:
		pevent_data.data[event_data_idx++] = ch;
		break;
	}
	return NULL;
}

pevent_t *pstate_single_line_comment_handler(FILE *fd, int ch)
{
	int pre_ch;
	switch (ch)
	{
	case '\n': /* single line comment ends here */
#ifdef DEBUG
		printf("\nSingle line comment end\n");
#endif
		pre_ch = ch;
		pevent_data.data[event_data_idx++] = ch;
		set_parser_event(PSTATE_IDLE, PEVENT_SINGLE_LINE_COMMENT);
		return &pevent_data;
	default: // collect single line comment chars
		pevent_data.data[event_data_idx++] = ch;
		break;
	}

	return NULL;
}
pevent_t *pstate_multi_line_comment_handler(FILE *fd, int ch)
{
	int pre_ch;
	switch (ch)
	{
	case '*': /* comment might end here */
		pre_ch = ch;
		pevent_data.data[event_data_idx++] = ch;
		if ((ch = fgetc(fd)) == '/')
		{
#ifdef DEBUG
			printf("\nMulti line comment End : */\n");
#endif
			pre_ch = ch;
			pevent_data.data[event_data_idx++] = ch;
			set_parser_event(PSTATE_IDLE, PEVENT_MULTI_LINE_COMMENT);
			return &pevent_data;
		}
		else // multi line comment string still continued
		{
			pevent_data.data[event_data_idx++] = ch;
		}
		break;
	case '/':
		/* go back by two steps and read previous char */
		fseek(fd, -2L, SEEK_CUR); // move two steps back
		pre_ch = fgetc(fd);		  // read a char
		fgetc(fd);				  // to come back to current offset

		pevent_data.data[event_data_idx++] = ch;
		if (pre_ch == '*')
		{
			set_parser_event(PSTATE_IDLE, PEVENT_MULTI_LINE_COMMENT);
			return &pevent_data;
		}
		break;
	default: // collect multi-line comment chars
		pevent_data.data[event_data_idx++] = ch;
		break;
	}

	return NULL;
}
pevent_t *pstate_ascii_char_handler(FILE *fd, int ch)
{
	/* write a switch case here to store ASCII chars
	 * return event data at the end of event
	 * else return NULL
	 */
	switch (ch)
	{
	case '\'': // ASCII ends here
		pevent_data.data[event_data_idx++] = ch;
		set_parser_event(PSTATE_IDLE, PEVENT_ASCII_CHAR);
		return &pevent_data;
		break;
	case '\n': // ASCII ends here
		pevent_data.data[event_data_idx++] = ch;
		set_parser_event(PSTATE_IDLE, PEVENT_ASCII_CHAR);
		return &pevent_data;
		break;
	default: // collect ASCII Chars
		pevent_data.data[event_data_idx++] = ch;
		break;
	}

	return NULL;
}
/**** End of file ****/
