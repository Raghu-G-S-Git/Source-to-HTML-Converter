/*
	Name	 : Raghu G S

	Project	 : Source to HTML

	Objective: The objective of the Source-to-HTML Converter project is to develop a C-based tool that reads 
		a source code file (typically written in C or similar languages) and converts it into a well-formatted, 
		syntax-highlighted HTML file for easy viewing in a web browser. This is especially useful for documentation, 
		code reviews, presentations, or educational content, where code readability and structure are crucial. 
		The project aims to enhance the visual representation of code by preserving indentation, highlighting keywords, 
		and ensuring that special characters are rendered properly in HTML. By doing so, it bridges the gap between 
		raw source code and web-friendly documentation.
*/


#include <stdio.h>
#include "s2html_event.h"
#include "s2html_conv.h"

/********** main **********/

int main (int argc, char *argv[])
{
	FILE * sfp, *dfp; // source and destination file descriptors 
	pevent_t *event;
	char dest_file[100];

	if(argc < 2)
	{
		printf("\nINVALID ARGUMENTS !\n");
		printf("USEAGE: <executable> <file name> \n");
		printf("Example : ./a.out abc.txt\n\n");
		return 1;
	}
#ifdef DEBUG
	printf("File to be opened : %s\n", argv[1]);
#endif

	/* opnen the file */

	if(NULL == (sfp = fopen(argv[1], "r")))
	{
		printf("Error! File %s could not be opened\n", argv[1]);
		return 2;
	}
	/* Check for output file */
	if (argc > 2)
	{
		sprintf(dest_file, "%s.html", argv[2]);
	}
	else
	{
		sprintf(dest_file, "%s.html", argv[1]);
	}
	/* open dest file */
	if (NULL == (dfp = fopen(dest_file, "w")))
	{
		printf("Error! could not create %s output file\n", dest_file);
		return 3;
	}

	/* write HTML starting Tags */
	html_begin(dfp, HTML_OPEN);

	/* Read from src file convert into html and write to dest file */

	do
	{
		event = get_parser_event(sfp);
		/* call sourc_to_html */
		source_to_html(dfp, event);
//		printf("In main : Event = %d\n", event);
	} while (event->type != PEVENT_EOF);

	/* Call start_or_end_conv function for ending the convertation */
	html_end(dfp, HTML_CLOSE);
	
	printf("\nOutput file %s generated\n", dest_file);
/* close file */
	fclose(sfp);
	fclose(dfp);

	return 0;
}
