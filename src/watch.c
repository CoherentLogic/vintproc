/*
 * watch.c
 *  Repeats a command at regular intervals.
 *  
 * Part of the vintproc tools, a procps substitute
 * for old UNIX systems on which procps will not 
 * compile or run due to missing library support.
 *
 * Inspired by watch(1) from the procps package.
 *
 * Copyright (C) 2015 Coherent Logic Development LLC
 * 
 * Author: John Willis <jpw@coherent-logic.com>
 * Date:   12 Apr 2015
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>

#define VERSION_STRING "0.0.1"

size_t command_string_length;
size_t time_string_length;
char *command_string;

int interval;

int screen_width;
int screen_height;

int tflg;
int bflg;
int hflg;
int eflg;
int vflg;

run_command()
{
	FILE *pipe;
	int cmd_err;

	char c;

	if((pipe = popen(command_string, "r")) == NULL) {
		fatal_error("Could not open pipe.");
		prog_exit(1);
	}

	while((c = getc(pipe)) != EOF) {
		putchar(c);
	}

	cmd_err = pclose(pipe);
	
	if(cmd_err && bflg) beep_term();
	if(cmd_err && eflg) exit(cmd_err);
}

beep_term()
{
	putchar('\a');
}

init_term()
{
	screen_width = 80;
	screen_height = 24;

	char *tmp;
	
	struct winsize w;
	if(ioctl(2, TIOCGWINSZ, &w) > -1) {
		screen_width = w.ws_col;
		screen_height = w.ws_row;
	}

	redraw_screen();
}

redraw_screen()
{
	clear_screen();
	if(tflg == 0) print_header();
}

clear_screen()
{
	system("clear"); /* this hack needs to go away. later... */
}

print_header()
{

	char *interval_string;
	size_t interval_string_length = 15;
	int i;

	interval_string = calloc(interval_string_length, sizeof(char));

	snprintf(interval_string, interval_string_length, "Every %2ds: ", interval);

	time_t t = time(NULL);

	char *time_string = ctime(&t);
	time_string_length = strlen(time_string);

	char *padding_string;
	size_t padding_string_length;

	padding_string_length = screen_width - interval_string_length - time_string_length - command_string_length;
	padding_string_length += 5;
	padding_string = calloc(padding_string_length, sizeof(char));
	snprintf(padding_string, padding_string_length, "% *s", padding_string_length, " ");

	size_t header_length;
	char *header;

	header_length = screen_width;
	header = calloc(header_length, sizeof(char));

	snprintf(header, header_length, "%s%s%s%s", interval_string, command_string, padding_string, time_string);  
	fprintf(stderr, "%s\n", header);
	
	for(i = 0; i < screen_width; i++) {
		putc('_', stderr);
	}
	fprintf(stderr, "\n\n");
}

fatal_error(msg)
	char *msg;
{
	fprintf(stderr, "%s\n", msg);

	prog_exit(1);
}

sig_handler()
{
	prog_exit(1);
}

prog_exit(code)
	int code;
{
	exit(code);
}

int main(argc, argv)
	int argc;
	char *argv[];
{
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;

	command_string_length = 0;
	interval = 2;

	tflg = 0;
	bflg = 0;
	hflg = 0;
	eflg = 0;
	vflg = 0;

	while((c = getopt(argc, argv, "tbehvn:")) != EOF) {
		switch(c) {
		case 'n':
			interval = atoi(optarg);
			break;
		case 't':
			tflg++;
			break;
		case 'b':
			bflg++;
			break;
		case 'e':
			eflg++;
			break;
		case 'h':
			hflg++;
			errflg++;
			break;
		case 'v':
			vflg++;
			break;
		case '?':
			errflg++;
			break;
		}	
	}

	if(errflg) {
		fprintf(stderr, "usage:  watch [-tbehv] [-n <interval>] command\n");
		prog_exit(1);
	}

	if(vflg) {
		fprintf(stderr, "%s\n", VERSION_STRING);
		prog_exit(0);
	}

	int original_optind = optind;
	for(; optind < argc; optind++) {
		command_string_length += strlen(argv[optind]);
		command_string_length++;
	}
	
	optind = original_optind;

	command_string = calloc(command_string_length, sizeof(char));
	
	for(; optind < argc; optind++) {
		strcat(command_string, argv[optind]);
		strcat(command_string, " ");
	}

	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGHUP, sig_handler);

	init_term();

	while(1) {
		redraw_screen();
		run_command();
		sleep(interval);
	}

	prog_exit(0);
}

