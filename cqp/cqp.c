/*
 *  IMS Open Corpus Workbench (CWB)
 *  Copyright (C) 1993-2006 by IMS, University of Stuttgart
 *  Copyright (C) 2007-     by the respective contributers (see file AUTHORS)
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2, or (at your option) any later
 *  version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 *  Public License for more details (in the file "COPYING", or available via
 *  WWW at http://www.gnu.org/copyleft/gpl.html).
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include "../cl/globals.h"
#include "../cl/macros.h"

#include "cqp.h"
#include "options.h"
#include "output.h"
#include "symtab.h"
#include "tree.h"
#include "eval.h"
#include "corpmanag.h"
#include "regex2dfa.h"
#include "ranges.h"
#include "macro.h"
#include "targets.h"

#include "parser.tab.h"


extern FILE *yyin;
extern int yyparse (void);
extern void yyrestart(FILE *input_file);


FILE *cqp_files[MAXCQPFILES];
int cqp_file_p;

int reading_cqprc = 0;

/* ======================================== Query Buffer Interface */

char QueryBuffer[QUERY_BUFFER_SIZE];
int QueryBufferP = 0;
int QueryBufferOverflow = 0;


/* =============================================================== */

static void
sigINT_signal_handler(int signum)
{
  if (EvaluationIsRunning) {
    fprintf(stderr, "** Aborting evaluation ... (press Ctrl-C again to exit CQP)\n");
    EvaluationIsRunning = 0;
  }
  signal_handler_is_installed = 0;
}

void
install_signal_handler(void) {
  signal_handler_is_installed = 1; /* pretend it's installed even if it wasn't done properly, so CQP won't keep trying */
  if (signal(SIGINT, sigINT_signal_handler) == SIG_ERR) {
    cqpmessage(Warning, "Can't install interrupt handler.\n");
    signal(SIGINT, SIG_IGN);
  }
}

/* =============================================================== */

void
cqp_randomize(void) {
  cl_randomize();  /* randomize internal RNG */
}

/* =============================================================== */

int initialize_cqp(int argc, char **argv)
{
  char *home = NULL;
  char init_file_fullname[256];

  FILE *cqprc;

  extern int yydebug;

  /*
   * initialize global variables
   */

  exit_cqp = 0;
  cqp_file_p = 0;

  corpuslist = NULL;

  eep = -1;

  /*
   * intialise built-in random number generator
   */

  cl_randomize();

  /*
   * initialise macro database
   */

  init_macros();

  /*
   * parse program options
   */

  parse_options(argc, argv);

  /* let's always run stdout unbuffered */
  /*  if (batchmode || rangeoutput || insecure || !isatty(fileno(stdout))) */
  if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
    perror("unbuffer stdout");

  yydebug = parser_debug;

  /*
   * read initialization file
   */

  if (cqp_init_file ||
      (!child_process && (!batchmode || (batchfd == NULL)) && !(which_app == cqpserver))
      ) {

    /*
     * Read init file specified with -I <file>
     *   if no init file was specified, and we're not in batchmode, child mode, or cqpserver,
     *   looks for ~/.cqprc
     * Same with macro init file (-M <file> or ~/.cqpmacros), but ONLY if macros are enabled.
     */

    /*
     * allow interactive commands during processing of initialization file ???
     * (I don't think this is the case!!)
     */

    init_file_fullname[0] = '\0';

    /* read init file specified with -I , otherwise look for ~/.cqprc */
    if (cqp_init_file)
      sprintf(init_file_fullname, "%s", cqp_init_file);
    else if ((home = (char *)getenv("HOME")) != NULL)
      sprintf(init_file_fullname, "%s/%s", home, CQPRC_NAME);

    if (init_file_fullname[0] != '\0') {
      if ((cqprc = fopen(init_file_fullname, "r")) != NULL) {

	reading_cqprc = 1;	/* not good for very much, really */
	if (!cqp_parse_file(cqprc, 1)) {
	  fprintf(stderr, "Parse errors while reading %s, exitting.\n",
		  init_file_fullname);
	  exit(1);
	}
	reading_cqprc = 0;

	/* fclose(cqprc);  was already closed by cqp_parse_file!! */
      }
      else if (cqp_init_file) {
	fprintf(stderr, "Can't read initialization file %s\n",
		init_file_fullname);
	exit(1);
      }
    }
  }

  if (!enable_macros && macro_init_file)
    cqpmessage(Warning, "Macros not enabled. Ignoring macro init file %s.", macro_init_file);

  if (enable_macros &&
      (macro_init_file ||
       (!child_process && (!batchmode || (batchfd == NULL)) && !(which_app == cqpserver))
       )
      ) {

    init_file_fullname[0] = '\0';

    /* read macro init file specified with -M , otherwise look for ~/.cqpmacros */
    if (macro_init_file)
      sprintf(init_file_fullname, "%s", macro_init_file);
    else if ((home = (char *)getenv("HOME")) != NULL)
      sprintf(init_file_fullname, "%s/%s", home, CQPMACRORC_NAME);

    if (init_file_fullname[0] != '\0') {
      if ((cqprc = fopen(init_file_fullname, "r")) != NULL) {

	reading_cqprc = 1;	/* not good for very much, really */
	if (!cqp_parse_file(cqprc, 1)) {
	  fprintf(stderr, "Parse errors while reading %s, exitting.\n",
		  init_file_fullname);
	  exit(1);
	}
	reading_cqprc = 0;

	/* fclose(cqprc);  was already closed by cqp_parse_file!! */
      }
      else if (macro_init_file) {
	fprintf(stderr, "Can't read macro initialization file %s\n",
		init_file_fullname);
	exit(1);
      }
    }
  } /* ends if (!child_process || (batchmode ... ) ... ) */

  check_available_corpora(UNDEF);

  /* load the default corpus. */
  if ((default_corpus) && !set_current_corpus_name(default_corpus, 0)) {
    fprintf(stderr, "Can't set current corpus to default corpus %s, exiting.\n",
	    default_corpus);
    exit(1);
  }

  if (signal(SIGPIPE, SIG_IGN) == SIG_IGN) {
    /* fprintf(stderr, "Couldn't install SIG_IGN for SIGPIPE signal\n"); */
    /* -- be silent about not being able to ignore the SIGPIPE signal, which often happens in slave mode */
    signal(SIGPIPE, SIG_DFL);
  }

  return 1;
}

int cqp_parse_file(FILE *fd, int exit_on_parse_errors)
{
  int ok, quiet;
  int cqp_status;

  ok = 1;
  quiet = silent || (fd != stdin);

  if (cqp_file_p < MAXCQPFILES) {

    cqp_files[cqp_file_p] = yyin;
    cqp_file_p++;

    yyin = fd;
    yyrestart(yyin);

    while (ok && !feof(fd) && !exit_cqp) {
      if (child_process && ferror(fd)) {
        /* in child mode, abort on read errors (to avoid hang-up when parent has died etc.) */
        fprintf(stderr, "READ ERROR -- aborting CQP session\n");
        break;
      }

      if (!quiet) {
	if (current_corpus != NULL)
	  if (STREQ(current_corpus->name, current_corpus->mother_name))
	    printf("%s> ", current_corpus->name);
	  else
	    printf("%s:%s[%d]> ",
		   current_corpus->mother_name,
		   current_corpus->name,
		   current_corpus->size);
	else
	  printf("[no corpus]> ");
      }

      cqp_status = yyparse();
      if ((cqp_status != 0) && exit_on_parse_errors)
	ok = 0;

      if (child_process && (cqp_status != 0) && !reading_cqprc) {
        fprintf(stderr, "PARSE ERROR\n"); /*  */
      }
      if (child_process && !reading_cqprc) {
#if 0
        /* empty lines after commands in child mode have been disabled as of version 2.2.b94 */
        printf("\n");		/* print empty line as separator in child mode */
#endif
        fflush(stdout);
        fflush(stderr);
      }

    } /* endwhile */

    if (fd != stdin)
      fclose(fd);

    cqp_file_p--;
    yyin = cqp_files[cqp_file_p];

    if (save_on_exit)
      save_unsaved_subcorpora();

    return ok;
  } /* endif (cqp_file_p < MAXCQPFILES) */
  else {
    fprintf(stderr, "CQP: too many nested files (%d)\n", cqp_file_p);
    return 0;
  }
}

int cqp_parse_string(char *s)
{
  int ok, len, abort;
  int cqp_status;

  ok = 1;
  abort = 0;
  len = strlen(s);

  cqp_input_string_position = 0;
  cqp_input_string = s;

  while (ok && (cqp_input_string_position < len) && !exit_cqp) {

    if (abort) {        /* trying to parse a second command -> abort with error */
      cqpmessage(Error, "Multiple commands on a single line not allowed in CQPserver mode.");
      ok = 0;
      break;
    }

    cqp_status = yyparse();
    if (cqp_status != 0)
      ok = 0;

    if (which_app == cqpserver)
      abort = 1;        /* only one command per line in CQPserver (security reasons) */

  } /* endwhile */

  cqp_input_string_position = 0;
  cqp_input_string = NULL;

  return ok;
}

/* ============================================================ */

InterruptCheckProc interruptCallbackHook = NULL;

int setInterruptCallback(InterruptCheckProc f)
{
  interruptCallbackHook = f;
  return 1;
}

void CheckForInterrupts()
{
  if (interruptCallbackHook)
    interruptCallbackHook();
}

/* ============================================================ */
