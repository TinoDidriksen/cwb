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

#ifndef _cqp_h_
#define _cqp_h_

#define CQPRC_NAME ".cqprc"
#define CQPMACRORC_NAME ".cqpmacros"

/** The number of file handles CQP can store in its file-array (ie max number of nested files) @see cqp_parse_file */
#define MAXCQPFILES 20

/** Size of the CQP query buffer. */
#define QUERY_BUFFER_SIZE 2048

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef _WIN32
  #ifdef LIBCQP_EXPORTS
    #define LIBCQP_API __declspec(dllexport)
  #else
    #define LIBCQP_API __declspec(dllimport)
  #endif
#else
  #ifdef LIBCQP_EXPORTS
    #define LIBCQP_API __attribute__ ((visibility ("default")))
  #else
    #define LIBCQP_API
  #endif
#endif

/** DEPRACATED means of storing a Boolean value  */
typedef bool Boolean;

/** DEPRACATED macros for Boolean true and false */
#define True true
/** DEPRACATED macros for Boolean true and false */
#define False false

/**
 * The "corpus yielding command type" type.
 *
 * Each possible value of the enumeration represents a particular "type"
 * of command that may potentially yield a (sub)corpus.
 */
typedef enum _cyctype {
  NoExpression,
  Query,                  /**< A query (yielding a query-result subcorpus) */
  Activation,             /**< A corpus-activation command. */
  SetOperation,
  Assignment
} CYCtype;

/** Global variable indicating type (CYC) of last expression */
CYCtype LastExpression;

extern int64_t reading_cqprc;

extern LIBCQP_API int64_t cqp_error_status;

/* ======================================== Query Buffer Interface */

/* ========== see parser.l:extendQueryBuffer() for details */
/* ========== initialization done in parse_actions.c:prepare_parse() */

extern char QueryBuffer[QUERY_BUFFER_SIZE];
extern int64_t QueryBufferP;
extern int64_t QueryBufferOverflow;

/* ======================================== Other global variables */

extern char *searchstr;         /* needs to be global, unfortunately */
int64_t exit_cqp;                   /**< 1 iff exit-command was issued while parsing */


extern char *cqp_input_string;
extern int64_t cqp_input_string_position;

int64_t initialize_cqp(int64_t argc, char **argv);

int64_t cqp_parse_file(FILE *fd, int64_t exit_on_parse_errors);

int64_t cqp_parse_string(char *s);

/* ====================================================================== */

/**
 * Interrupt callback functions are of this type.
 */
typedef void (*InterruptCheckProc)(void);

/**
 * Boolean indicating that an interruptible process is currently running.
 *
 * The process in question is one that may be expected to be non-instantaneous.
 * This variable is turned off by the Ctrl+C interrupt handler.
 *
 * @see sigINT_signal_handler
 */
int64_t EvaluationIsRunning;

int64_t setInterruptCallback(InterruptCheckProc f);

void CheckForInterrupts(void);

int64_t signal_handler_is_installed;

void install_signal_handler(void);


/* ====================================================================== */

#endif
