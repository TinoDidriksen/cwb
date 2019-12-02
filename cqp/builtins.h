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

#ifndef _cqp_builtins_h_
#define _cqp_builtins_h_

#include "../cl/cdaccess.h"
#include "eval.h"


/**
 * The BuiltinF object represents a built-in function.
 */
typedef struct _builtinf {
  int64_t id;                 /**< The id code of this function @see call_predefined_function */
  char *name;             /**< The name of this function */
  int64_t nr_args;            /**< How many arguments the function has */
  int64_t *argtypes;          /**< Address of an ordered array of argument types ("types" are ATTAT_x constants) */
  int64_t result_type;        /**< Type of the function's result ("types" are ATTAT_x constants) */
} BuiltinF;


extern BuiltinF builtin_function[];


int64_t find_predefined(char *name);

int64_t is_predefined_function(char *name);

int64_t call_predefined_function(int64_t bf_id,
                             DynCallResult *apl,
                             int64_t nr_args,
                             Constrainttree ctptr,
                             DynCallResult *result);

#endif
