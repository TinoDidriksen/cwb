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

/**
 * @file
 *
 * This file contains the API for the CWB "Corpus Library" (CL).
 *
 * If you are programming against the CL, you should #include ONLY this header file,
 * and make use of ONLY the functions declared here.
 *
 * Other functions in the CL should ONLY be used within the CWB itself by CWB developers.
 *
 * The header file is laid out in such a way as to semi-document the API, i.e. function
 * prototypes are given with brief notes on usage, parameters, and return values. You may
 * also wish to refer to CWB's automatically-generated HTML code documentation (created
 * using the Doxygen system; if you're reading this text in a web browser, then the
 * auto-generated documentation is almost certainly what you're looking at). However,
 * please note that the auto-generated documentation ALSO covers (a) functions internal to
 * the CL which should NOT be used when programming against it; (b) functions from the CWB
 * utilities and from the CQP program - neither of which are part of the CL. There is also
 * no distinction in that more extensive documentation between information that is
 * relevant to programming against the CL API and information that is relevant to
 * developers working on the CL itself. Caveat lector.
 *
 * Note that many functions have two names -- one that follows the standardised format
 * "cl_do_something()", and another that follows no particular pattern. The former
 * are the "new API" (in v3.0.0 or higher of CWB) and the latter are the "old-style" API
 * (depracated, but supported for backward compatibility). The old-style function names
 * SHOULD NOT be used in newly-written code. Such double names mostly exist for the core
 * data-access functions (i.e. for the Corpus and (especially) Attribute objects).
 *
 * In v3.0 and v3.1 of CWB, the new API was implemented as macros to the old API.
 * As of v3.2, the old API is implemented as macros to the new API.
 *
 * In a very few cases, the parameter list or return behaviour of a function also changed.
 * In this case, a function with the "old" parameter list is preserved (but depracated)
 * and has the same name as the new function but with the suffix "_oldstyle". The old
 * names are then re-implemented as macros to the _oldstyle functions. But, as should be
 * obvious, while these functions and the macros to them will remain in the public API
 * for backwards-compatibiltiy, they should not be used in new code, and are most
 * definitely depracated!
 *
 * The CL header is organised to reflect the conceptual structure of the library. While
 * it is not fully "object-oriented" in style most of the functions are organised around
 * a small number of data objects that represent real entities in a CWB-encoded corpus.
 * Each object is defined as an opaque type (usually a structure whose members are
 * PRIVATE and should only be accessed via the functions provided in the CL API).
 *
 * CONTENTS LIST FOR THIS HEADER FILE:
 *
 * SECTION 1          CL UTILITIES
 *
 *   1.1                ERROR HANDLING
 *
 *   1.2                MEMORY MANAGEMENT
 *
 *   1.3                DATA LIST CLASSES: cl_string_list AND cl_int_list
 *
 *   1.4                INTERNAL RANDOM NUMBER GENERATOR
 *
 *   1.5                SETTING CL CONFIG VARIABLES
 *
 *   1.6                MISCELLANEOUS UTILITIES
 *
 * SECTION 2          THE CORE CL LIBRARY (DATA ACCESS)
 *
 *   2.1                THE Corpus OBJECT
 *
 *   2.2                THE Attribute OBJECT
 *
 *   2.3                THE PositionStream OBJECT
 *
 * SECTION 3          SUPPORT CLASSES
 *
 *   3.1                THE CorpusProperty OBJECT
 *
 *   3.2                THE CorpusCharset OBJECT
 *
 *   3.3                THE CL_Regex OBJECT
 *
 *   3.4                THE cl_lexhash OBJECT
 *
 *   3.5                THE CL_BitVec OBJECT
 *
 * SECTION 4          THE OLD CL API
 *
 * (If you're looking at the auto-generated HTML documentation, this contents list,
 * which describes the structure of the actual "cl.h" header file, is wrong for you -
 * instead, use the index of links (above) to find the object or function
 * you are interested in.)
 *
 * We hope you enjoy using the CL!
 *
 * best regards from
 *
 * The CWB Development Team
 *
 * <a href="http://cwb.sourceforge.net">http://cwb.sourceforge.net</a>
 *
 */




/* The actual code of the header file begins here. */

#ifndef _cwb_cl_h
#define _cwb_cl_h

#include <strings.h>                /* for size_t */



/*
 *
 * SECTION 1 -- CL UTILITIES
 *
 */

/*
 *
 * SECTION 1.1 -- ERROR HANDLING
 *
 * (error values and related functions)
 *
 */

/* Error Codes. Note that "CDA" stands for "corpus data access". */
#define CDA_OK           0        /**< Error code: everything is fine; actual error values are all less than 0 */
#define CDA_ENULLATT    -1        /**< Error code: NULL passed as attribute argument */
#define CDA_EATTTYPE    -2        /**< Error code: function was called on illegal attribute */
#define CDA_EIDORNG     -3        /**< Error code: id out of range */
#define CDA_EPOSORNG    -4        /**< Error code: position out of range */
#define CDA_EIDXORNG    -5        /**< Error code: index out of range */
#define CDA_ENOSTRING   -6        /**< Error code: no such string encoded */
#define CDA_EPATTERN    -7        /**< Error code: illegal pattern */
#define CDA_ESTRUC      -8        /**< Error code: no structure at position */
#define CDA_EALIGN      -9        /**< Error code: no alignment at position */
#define CDA_EREMOTE     -10       /**< Error code: error in remote access */
#define CDA_ENODATA     -11       /**< Error code: can't load/create necessary data */
#define CDA_EARGS       -12       /**< Error code: error in arguments for dynamic call */
#define CDA_ENOMEM      -13       /**< Error code: memory fault [unused] */
#define CDA_EOTHER      -14       /**< Error code: other error */
#define CDA_ENYI        -15       /**< Error code: not yet implemented */
#define CDA_EBADREGEX   -16       /**< Error code: bad regular expression */
#define CDA_EFSETINV    -17       /**< Error code: invalid feature set format */
#define CDA_EBUFFER     -18       /**< Error code: buffer overflow (hard-coded internal buffer sizes) */
#define CDA_EINTERNAL   -19       /**< Error code: internal data consistency error (really bad) */

/* a global variable which will always be set to one of the above constants! */
extern int cl_errno;

/* error handling functions */
void cl_error(char *message);
char *cl_error_string(int error_num);




/*
 *
 * SECTION 1.2 -- MEMORY MANAGEMENT
 *
 */

/*
 * easy memory management functions
 *
 * use the following memory allocation functions instead of malloc(), calloc(), realloc(), strdup()
 * in your own programs to invoke the CL's memory manager when necessary
 */
void *cl_malloc(size_t bytes);
void *cl_calloc(size_t nr_of_elements, size_t element_size);
void *cl_realloc(void *block, size_t bytes);
char *cl_strdup(char *string);
/**
 * Safely frees memory.
 *
 * @see cl_malloc
 * @param p  Pointer to memory to be freed.
 */
#define cl_free(p) do { if ((p) != NULL) { free(p); p = NULL; } } while (0)
/* the do {...} while (0) should be safe in 'bare' if..then..else blocks */




/*
 *
 * SECTION 1.3 -- DATA LIST CLASSES: cl_string_list AND cl_int_list
 *
 */

/**
 * Automatically growing list of integers (just what you always need ...)
 */
typedef struct _cl_int_list    *cl_int_list;
/* the cl_int_list object API ...*/
cl_int_list cl_new_int_list(void);                           /* create int list object */
void cl_delete_int_list(cl_int_list l);                      /* delete int list object */
void cl_int_list_lumpsize(cl_int_list l, int s);             /* memory for the list is allocated in "lumps", default size is 64 entries */
int cl_int_list_size(cl_int_list l);                         /* current size of list */
int cl_int_list_get(cl_int_list l, int n);                   /* get value of n-th element in list (0 if out of range) */
void cl_int_list_set(cl_int_list l, int n, int val);         /* set n-th element (automatically extends list) */
void cl_int_list_append(cl_int_list l, int val);             /* append element to list */
void cl_int_list_qsort(cl_int_list l);                       /* sort list (ascending order) */

/**
 * Automatically growing list of strings (just what you always need ...)
 */
typedef struct _cl_string_list *cl_string_list;
/* the cl_string_list object API ...*/
cl_string_list cl_new_string_list(void);                     /* create string list object */
void cl_delete_string_list(cl_string_list l);                /* delete string list object */
void cl_free_string_list(cl_string_list l);                  /* free() all strings in list (use with care!) */
void cl_string_list_lumpsize(cl_string_list l, int s);       /* memory for the list is allocated in "lumps", default size is 64 entries */
int cl_string_list_size(cl_string_list l);                   /* current size of list */
char *cl_string_list_get(cl_string_list l, int n);           /* get value of n-th element in list (NULL if out of range) */
void cl_string_list_set(cl_string_list l, int n, char *val); /* set n-th element (does NOT make copy of string!) */
void cl_string_list_append(cl_string_list l, char *val);     /* append element to list */
void cl_string_list_qsort(cl_string_list l);                 /* sort list (using cl_strcmp()) */




/*
 *
 * SECTION 1.4 -- INTERNAL RANDOM NUMBER GENERATOR
 *
 */

/* built-in random number generator (RNG) */
void cl_set_seed(unsigned int seed);
void cl_randomize(void);
void cl_get_rng_state(unsigned int *i1, unsigned int *i2);
void cl_set_rng_state(unsigned int i1, unsigned int i2);
unsigned int cl_random(void);
double cl_runif(void);




/*
 *
 * SECTION 1.5 -- SETTING CL CONFIG VARIABLES
 *
 */

/*
 *  Functions for setting global CL configuration options
 */
void cl_set_debug_level(int level);       /* 0 = none (default), 1 = some, 2 = all */
void cl_set_optimize(int state);          /* 0 = off, 1 = on */
void cl_set_memory_limit(int megabytes);  /* 0 or less turns limit off */




/*
 *
 * SECTION 1.6 -- MISCELLANEOUS UTILITIES
 *
 */

/*
 *  misc CL utility functions
 */

int cl_strcmp(char *s1, char *s2);

char *cl_string_latex2iso(char *str, char *result, int target_len);
/* <result> points to buffer of appropriate size; auto-allocated if NULL;
   str == result is explicitly allowed; conveniently returns <result> */
extern int cl_allow_latex2iso; /* cl_string_latex2iso will only change a string if this is true, it is false by default*/

char *cl_xml_entity_decode(char *s); /* removes the four default XML entities from the string, in situ */
/**
 * For a given character, say whether it is legal for an XML name.
 *
 * TODO: Currently, anything in the upper half of the 8-bit range is
 * allowed (in the old Latin1 days this was anything from 0xa0 to
 * 0xff). This will work with anyt non-ascii character set, but
 * is almost certainly too lax.
 *
 * @param c  Character to check. (It is expected to be a char,
 *           so is typecase to unsigned char for comparison with
 *           upper-128 hex values.)
 */
#define cl_xml_is_name_char(c)  ( ( c >= 'A'  && c <= 'Z')  ||       \
                                  ( c >= 'a'  && c <= 'z')  ||       \
                                  ( c >= '0'  && c <= '9')  ||       \
                                  (    (unsigned char) c >= 0x80     \
                                    && (unsigned char) c <= 0xff     \
                                  ) ||                               \
                                  ( c == '-') ||                     \
                                  ( c == '_')                        \
                                 )

void cl_path_adjust_os(char *path);  /* normalises a path to Windowslike or Unixlike, depending on the build;
                                        string changed in place. */
void cl_path_adjust_independent(char *path); /* makes a path Unixlike, regardless of the OS; string changed in place. */

char *cl_path_registry_quote(char *path); /* adds registry-format quotes and slashes to a path where necessary;
                                             a newly-allocated string is returned. */

/* validate and manipulate strings that are (sub)corpus identifiers */
int cl_id_validate(char *s);
void cl_id_toupper(char *s);
void cl_id_tolower(char *s);

/* built-in support for handling feature set attributes */
char *cl_make_set(char *s, int split);
int cl_set_size(char *s);
int cl_set_intersection(char *result, const char *s1, const char *s2);




/*
 *
 * SECTION 2 -- THE CORE CL LIBRARY (DATA ACCESS)
 *
 * These are the central CL corpus and attribute 'methods'.
 *
 */

/*
 *
 * SECTION 2.1 -- THE Corpus OBJECT
 *
 */

/**
 * The Corpus object: contains information on a loaded corpus,
 * including all its attributes.
 */
typedef struct TCorpus Corpus;

/* corpus access functions */
Corpus *cl_new_corpus(char *registry_dir, char *registry_name);
int cl_delete_corpus(Corpus *corpus);
char *cl_standard_registry();
cl_string_list cl_corpus_list_attributes(Corpus *corpus, int attribute_type);




/*
 *
 * SECTION 2.2 -- THE Attribute OBJECT
 *
 */

/* TODO ... wouldn't it be nice if the Attribute methods returned int/string list objects instead of
 * char ** and int *?  But that would involve re-engineering EVERYTHING.  */
/**
 * The Attribute object: an entire segment of a corpus, such as an
 * annotation field, an XML structure, or a set
 *
 * The attribute can be of any flavour (s, p etc); this information
 * is specified internally.
 *
 * Note that each Attribute object is associated with a particular
 * corpus. They aren't abstract, i.e. every corpus has a "word"
 * p-attribute but any Attribute object for a "word" refers to the
 * "word" of a specific corpus, not to "word" attributes in general.
 */
typedef union _Attribute Attribute;

/* constants indicating attribute types */

/** No type of attribute */
#define ATT_NONE       0
/** Positional attributes, ie streams of word tokens, word tags - any "column" that has a value at every corpus position. */
#define ATT_POS        (1<<0)
/** Structural attributes, ie a set of SGML/XML-ish "regions" in the corpus delimited by the same SGML/XML tag */
#define ATT_STRUC      (1<<1)
/** Alignment attributes, ie a set of zones of alignment between a source and target corpus */
#define ATT_ALIGN      (1<<2)
/** Dynamic attributes, ?? */
#define ATT_DYN        (1<<6)

/** shorthand for "any / all types of attribute" */
/* TODO shouldn't ATT_ALL and ATT_REAL have brackets round their expansions?? in case of higher-precedence operator */
#define ATT_ALL        ATT_POS   | ATT_STRUC | ATT_ALIGN | ATT_DYN
/** shorthand for "any / all types of attribute except dynamic" */
#define ATT_REAL       ATT_POS   | ATT_STRUC | ATT_ALIGN


/* there are a huge number of Attribute "methods" accessing different
 * kinds of Attribute in different ways... */

/* attribute access functions: general Attribute methods */

/**
 * Finds an attribute that matches the specified parameters, if one exists.
 *
 * Note that although this is a cl_new_* function, and it is the canonical way
 * that we get an Attribute to call Attribute-functions on, it doesn't actually
 * create any kind of object. The Attribute exists already as one of the dependents
 * of the Corpus object; this function simply locates it and returns a pointer
 * to it.
 *
 * This "function" is implemented as a macro wrapped round the depracated function,
 * making the means of calling it more in line with the rest of the CL.
 *
 * @see                   cl_new_attribute_oldstyle
 *
 * @param corpus          The corpus in which to search for the attribute.
 * @param attribute_name  The name of the attribute (i.e. the handle it has in the registry file)
 * @param type            Type of attribute to be searched for.
 *
 * @return                Pointer to Attribute object, or NULL if not found.
 */
#define cl_new_attribute(c, name, type) cl_new_attribute_oldstyle(c, name, type, NULL)

/* depracated */
Attribute *cl_new_attribute_oldstyle(Corpus *corpus,
                                     char *attribute_name,
                                     int type,
                                     char *data);        /* *** UNUSED *** */
int cl_delete_attribute(Attribute *attribute);
int cl_sequence_compressed(Attribute *attribute);
int cl_index_compressed(Attribute *attribute);

/* attribute access functions: lexicon access (positional attributes) */
char *cl_id2str(Attribute *attribute, int id);
int cl_str2id(Attribute *attribute, char *id_string);
int cl_id2strlen(Attribute *attribute, int id);
int cl_sort2id(Attribute *attribute, int sort_index_position);
int cl_id2sort(Attribute *attribute, int id);

/* attribute access functions: size (positional attributes) */
int cl_max_cpos(Attribute *attribute);
int cl_max_id(Attribute *attribute);

/* attribute access functions: token sequence & index (positional attributes) */
int cl_id2freq(Attribute *attribute, int id);

/**
 * Gets all the corpus positions where the specified item is found on the given P-attribute.
 * @see         cl_id2cpos_oldstyle
 * @param a     The P-attribute to look on.
 * @param id    The id of the item to look for.
 * @param freq  The frequency of the specified item is written here. This will be 0 in the
 *              case of errors.
 */
#define cl_id2cpos(a, id, freq) cl_id2cpos_oldstyle(a, id, freq, NULL, 0)

/* depracated */
int *cl_id2cpos_oldstyle(Attribute *attribute,
                         int id,
                         int *freq,
                         int *restrictor_list,
                         int restrictor_list_size);
int cl_cpos2id(Attribute *attribute, int position);
char *cl_cpos2str(Attribute *attribute, int position);

/* ========== some high-level constructs */

char *cl_id2all(Attribute *attribute,
                  int index, int *freq, int *slen);

int *cl_regex2id(Attribute *attribute,
                 char *pattern,
                 int flags,
                 int *number_of_matches);

int cl_idlist2freq(Attribute *attribute,
                   int *ids,
                   int number_of_ids);

/**
 * Gets a list of corpus positions matching a list of ids.
 * @see cl_idlist2cpos_oldstyle
 * @param a            The P-attribute we are looking in
 * @param idlist       A list of item ids (i.e. id codes for items on this attribute).
 * @param idlist_size  The length of this list.
 * @param sort         boolean: return sorted list?
 * @param size         The size of the allocated table will be placed here.
 */
#define cl_idlist2cpos(a, idlist, idlist_size, sort, size) cl_idlist2cpos_oldstyle(a, idlist, idlist_size, sort, size, NULL, 0)

/* depracated */
int *cl_idlist2cpos_oldstyle(Attribute *attribute,
                             int *ids,
                             int number_of_ids,
                             int sort,
                             int *size_of_table,
                             int *restrictor_list,
                             int restrictor_list_size);

/* attribute access functions: structural attributes */
/* note that "struc", in these function names, abbreviates "number identifiying
 * one structure instance on this s-attribute" */

int cl_cpos2struc2cpos(Attribute *attribute,
                       int position,
                       int *struc_start,
                       int *struc_end);
int cl_cpos2struc(Attribute *a, int cpos);

/* depracated */
int cl_cpos2struc_oldstyle(Attribute *attribute,
                           int position,
                           int *struc_num);

/* flags set in return values of cl_cpos2boundary() function */
#define STRUC_INSIDE 1  /**< cl_cpos2boundary() return flag: specified position is WITHIN a region of this s-attribute */
#define STRUC_LBOUND 2  /**< cl_cpos2boundary() return flag: specified position is AT THE START BOUNDARY OF a region of this s-attribute */
#define STRUC_RBOUND 4  /**< cl_cpos2boundary() return flag: specified position is AT THE END BOUNDARY OF a region of this s-attribute */
int cl_cpos2boundary(Attribute *a, int cpos);  /* convenience function: within region or at boundary? */

int cl_struc2cpos(Attribute *attribute,
                  int struc_num,
                  int *struc_start,
                  int *struc_end);
int cl_max_struc(Attribute *a);
int cl_max_struc_oldstyle(Attribute *attribute, int *nr_strucs);         /* depracated */
int cl_struc_values(Attribute *attribute);
char *cl_struc2str(Attribute *attribute, int struc_num);
char *cl_cpos2struc2str(Attribute *attribute, int position);

/* attribute access functions: extended alignment attributes (with fallback to old alignment) */
int cl_has_extended_alignment(Attribute *attribute);
int cl_max_alg(Attribute *attribute);
int cl_cpos2alg(Attribute *attribute, int cpos);
int cl_alg2cpos(Attribute *attribute, int alg,
                int *source_region_start, int *source_region_end,
                int *target_region_start, int *target_region_end);

/* attribute access functions: alignment attributes (old style) -- DEPRACATED */
int cl_cpos2alg2cpos_oldstyle(Attribute *attribute,
                              int position,
                              int *aligned_start,
                              int *aligned_end,
                              int *aligned_start2,
                              int *aligned_end2);

/* attribute access functions: dynamic attributes (N/A)
 *
 * NOTE that dynamic attributes are not currently supported.
 * Most of the code has been thrown out.
 *
 * Before we can prototype these, we need the DynCallResult datatype
 * This is properly an object on its own, but it is not separate
 * enough from the Attribute to merit its own heading.*/

/**
 *  maximum size of 'dynamic' strings
 */
#define CL_DYN_STRING_SIZE 2048

/**
 *  The DynCallResult object (needed to allocate space for dynamic function arguments)
 */
typedef struct _DCR {
  int type;              /**< Type of DynCallResult, indicated by one of the  ATTAT_x macro constants*/
  union {
    int intres;
    char *charres;
    double floatres;
    struct {
      Attribute *attr;
      int token_id;
    } parefres;
  } value;               /**< value of the result: can be int, string, float, or p-attribute reference */
  /**
   * buffer for dynamic strings returned by function calls
   * NB: this imposes a hard limit on the size of dynamic strings !!
   * @see CL_DYN_STRING_SIZE
   */
  char dynamic_string_buffer[CL_DYN_STRING_SIZE];
} DynCallResult;

/* result and argument types of dynamic attributes; ATTAT = attribute argument type */
#define ATTAT_NONE    0                /**< Dynamic att argument type: none */
#define ATTAT_POS     1                /**< Dynamic att argument type: ?? */
#define ATTAT_STRING  2                /**< Dynamic att argument type: string */
#define ATTAT_INT     3                /**< Dynamic att argument type: integer */
#define ATTAT_VAR     4                /**< Dynamic att argument type: variable number of string arguments (only in arglist) */
#define ATTAT_FLOAT   5                /**< Dynamic att argument type: floating point */
#define ATTAT_PAREF   6                /**< Dynamic att argument type: ?? */

/* and now the functions:
 *
 * ...: parameters (of *int or *char) and structure
 * which gets the result (*int or *char)
 */
int cl_dynamic_call(Attribute *attribute,
                    DynCallResult *dcr,
                    DynCallResult *args,
                    int nr_args);
int cl_dynamic_numargs(Attribute *attribute);




/*
 *
 * SECTION 2.3 -- THE PositionStream OBJECT
 *
 */

/**
 * The PositionStream object: gives stream-like reading of an Attribute.
 */
typedef struct _position_stream_rec_ *PositionStream;

/* Functions for attribute access using a position stream */
PositionStream cl_new_stream(Attribute *attribute, int id);
int cl_delete_stream(PositionStream *ps);
int cl_read_stream(PositionStream ps,
                   int *buffer,
                   int buffer_size);




/*
 *
 * SECTION 3 -- SUPPORT CLASSES
 *
 */

/*
 *
 * SECTION 3.1 -- THE CorpusProperty OBJECT
 *
 */

/**
 * The CorpusProperty object.
 *
 * The underlying structure takes the form of a linked-list entry.
 *
 * Note that unlike most CL objects, the underlying structure is
 * exposed in the public API.
 *
 * Each Corpus object has, as one of its members, the head entry
 * on a list of CorpusProperties.
 */
typedef struct TCorpusProperty {
  /** A string specifying the property in question. */
  char *property;
  /** A string containing the value of the property in question. */
  char *value;
  /** Pointer to the next entry in the linked list. */
  struct TCorpusProperty *next;
} *CorpusProperty;

/* ... and ... the CorpusProperty API */
CorpusProperty cl_first_corpus_property(Corpus *corpus);
CorpusProperty cl_next_corpus_property(CorpusProperty p);
char *cl_corpus_property(Corpus *corpus, char *property);




/*
 *
 * SECTION 3.2 -- THE CorpusCharset OBJECT
 *
 */

/**
 * The CorpusCharset object:
 * an identifier for one of the character sets supported by CWB.
 *
 * (Note on adding new character sets: add them immediately before
 * unknown_charset. Do not change the order of existing charsets.
 * Remember to update the special-chars module if you do so.)
 */
typedef enum ECorpusCharset {
  ascii = 0,

  /* planned support for ISO-8859-* charsets; as of v3.2.x, only latin1, ascii and utf8 are supported */

  /* latin1 = 8859-1, latin2 = 8859-2, latin3 = 8859-3, latin4 = 8859-4, cyrillic = 8859-5,
     arabic = 8859-6, greek = 8859-7, hebrew = 8859-8, latin5 = 8859-9, latin6 = 8859-10,
     latin7 = 8859-13, latin8 = 8859-14, latin9 = 8859-15 */
  latin1, latin2, latin3, latin4, cyrillic,
  arabic, greek,  hebrew, latin5, latin6,
  latin7, latin8, latin9,
  utf8,
  /* everything else is 'unknown' .. client apps should check the corresponding property value */
  unknown_charset
} CorpusCharset;

/* ... and related functions */
CorpusCharset cl_corpus_charset(Corpus *corpus);
char *cl_charset_name(CorpusCharset id);
CorpusCharset cl_charset_from_name(char *name);
char *cl_charset_name_canonical(char *name_to_check);

/* the main functions for which CorpusCharset "matters" are the following... */

/* the case/diacritic string normalization features used by CL regexes and CQP (modify input string!) */
void cl_string_canonical(char *s, CorpusCharset charset, int flags);
/* modifies string <s> in place; flags are IGNORE_CASE and IGNORE_DIAC */

/* boolean function, returns is string valid? */
int cl_string_validate_encoding(const char *s, CorpusCharset charset);

/* various functions related to sorting/grouping... */
char *cl_string_reverse(const char *s, CorpusCharset charset); /* creates a new string */

int cl_string_qsort_compare(const char *s1,
                            const char *s2,
                            CorpusCharset charset,
                            int flags,
                            int reverse);

/**
 * "Dummy" charset macro for calling cl_string_canonical
 *
 * We have a problem - CorpusCharsets are attached to corpora. So what charset do we use with
 * cl_string_canonical if we are calling it on a string that does not (yet) have a corpus?
 *
 * The answer: CHARSET_FOR_IDENTIFIERS. This should only be used as the 2nd argument to
 * cl_string_canonical when the string is an identifier for a corpus, attribute, or whatever.
 *
 * Note it is Ascii in v3.2.x+, breaking backwards compatibility with 2.2.x where Latin1 was
 * allowed for identifiers.
 */
#define CHARSET_FOR_IDENTIFIERS ascii





/*
 *
 * SECTION 3.3 -- THE CL_Regex OBJECT
 *
 */

/**
 * The CL_Regex object: an optimised regular expression.
 *
 * The CL regex engine wraps around another regex library (v3.1.x: POSIX, will be PCRE
 * in v3.2.0+) to implement CL semantics. These are: (a) the engine always
 * matches the entire string; (b) there is support for case-/diacritic-insensitive matching;
 * (c) certain optimisations are implemented.
 *
 * Associated with the CL regular expression engine are macros for three flags: IGNORE_CASE,
 * IGNORE_DIAC and IGNORE_REGEX. All three are used by the related cl_regex2id(), but only
 * the first two are used by the CL_Regex object.
 *
 * @see cl_regex2id
 */
typedef struct _CL_Regex *CL_Regex;

/** Flag ignore-case in regular expression engine. @see cl_regex2id */
#define IGNORE_CASE 1
/** Flag ignore-diacritics in regular expression engine. @see cl_regex2id */
#define IGNORE_DIAC 2
/** Flag for: don't use regular expression engine - match as a literal string. @see cl_regex2id */
#define IGNORE_REGEX 4

/* ... and the regex API ... */
CL_Regex cl_new_regex(char *regex, int flags, CorpusCharset charset /*ignored before v3.2*/);
int cl_regex_optimised(CL_Regex rx); /* 0 = not optimised; otherwise, value indicates level of optimisation */
int cl_regex_match(CL_Regex rx, char *str); /* automatically uses normalisation flags from constructor;
                                               returns True when regex matches */
void cl_delete_regex(CL_Regex rx);
extern char cl_regex_error[];

/* two functions interface the optimiser system's reporting capabilities */
void cl_regopt_count_reset(void);
int cl_regopt_count_get(void);




/*
 *
 * SECTION 3.4 -- THE cl_lexhash OBJECT
 *
 */

/**
 *  The cl_lexhash class (lexicon hashes, with IDs and frequency counts)
 *
 *  A "lexicon hash" links strings to integers. Each cl_lexhash object
 *  represents an entire table of such things; individual string-to-int
 *  links are represented by cl_lexhash_entry objects.
 *
 *  Within the cl_lexhash, the entries are grouped into buckets. A
 *  bucket is the term for a "slot" on the hash table. The linked-list
 *  in a given bucket represent all the different string-keys that map
 *  to one particular index value.
 *
 *  Each entry contains the key itself (for search-and-retrieval),
 *  the frequency of that type (incremented when a token is added that
 *  is already in the lexhash), an ID integer, plus a bundle of "data"
 *  associated with that string.
 *
 *  These lexicon hashes are used, notably, in the encoding of corpora
 *  to CWB-index-format.
 */
typedef struct _cl_lexhash *cl_lexhash;
/**
 * Underlying structure for the cl_lexhash_entry class.
 * Unlike most underlying structures, this is public in the CL API.
 */
typedef struct _cl_lexhash_entry {
  char *key;                        /**< hash key == token */
  unsigned int freq;                /**< frequency of this type */
  int id;                           /**< the id code of this type */
  /**
   * This entry's data fields.
   * Use as entry->data.integer, entry->data.numeric, ...
   */
  struct _cl_lexhash_entry_data {
    int integer;
    double numeric;
    void *pointer;
  } data;
  struct _cl_lexhash_entry *next;   /**< next entry on the linked-list (ie in the bucket) */
} *cl_lexhash_entry;

/*
 * ... and ... its API!!
 */
cl_lexhash cl_new_lexhash(int buckets);
void cl_delete_lexhash(cl_lexhash lh);
void cl_lexhash_set_cleanup_function(cl_lexhash lh, void (*func)(cl_lexhash_entry));
void cl_lexhash_auto_grow(cl_lexhash lh, int flag);
cl_lexhash_entry cl_lexhash_add(cl_lexhash lh, char *token);
cl_lexhash_entry cl_lexhash_find(cl_lexhash lh, char *token);
int cl_lexhash_id(cl_lexhash lh, char *token);
int cl_lexhash_freq(cl_lexhash lh, char *token);
int cl_lexhash_del(cl_lexhash lh, char *token);
int cl_lexhash_size(cl_lexhash lh);




/*
 *
 * SECTION 3.5 -- THE CL_BitVec OBJECT
 *
 */
typedef struct _CL_BitVec *CL_BitVec;                 /**< The CL_BitVec object: doesn't seem to exist {???-- AH}. */




/*
 * SECTION 4 -- THE OLD CL API
 *
 * compatibility macros : old names #defined to new names...
 *
 */

/* The old-style names are being phased out in CWB itself; but these macros
 * will be preserved for backwards-compatibility with software programmed
 * against earlier versions of the CL API. They cover most, but not all, of
 * the core corpus-data-access functionality in the Corpus and Attribute
 * classes.
 *
 * The macros are given here in order of old name.
 *
 * As noted in the file intro, all these old function names are DEPRACATED.
 */
#define ClosePositionStream(ps) cl_delete_stream(ps)
#define OpenPositionStream(a, id) cl_new_stream(a, id)
#define ReadPositionStream(ps, buf, size) cl_read_stream(ps, buf, size)
#define attr_drop_attribute(a) cl_delete_attribute(a)
#define call_dynamic_attribute(a, dcr, args, nr_args) cl_dynamic_call(a, dcr, args, nr_args)
#define cderrno cl_errno
#define cdperror(message) cl_error(message)
#define cdperror_string(no) cl_error_string(no)
#define central_corpus_directory() cl_standard_registry()
#define collect_matches(a, idlist, idlist_size, sort, size, rl, rls) cl_idlist2cpos_oldstyle(a, idlist, idlist_size, sort, size, rl, rls)
#define collect_matching_ids(a, re, flags, size) cl_regex2id(a, re, flags, size)
#define cumulative_id_frequency(a, list, size) cl_idlist2freq(a, list, size)
#define drop_corpus(c) cl_delete_corpus(c)
#define find_attribute(c, name, type, data) cl_new_attribute_oldstyle(c, name, type, data)
#define get_alg_attribute(a, p, start1, end1, start2, end2) cl_cpos2alg2cpos_oldstyle(a, p, start1, end1, start2, end2)
#define get_attribute_size(a) cl_max_cpos(a)
#define get_bounds_of_nth_struc(a, struc, start, end) cl_struc2cpos(a, struc, start, end)
#define get_id_at_position(a, cpos) cl_cpos2id(a, cpos)
#define get_id_of_string(a, str) cl_str2id(a, str)
#define get_id_frequency(a, id) cl_id2freq(a, id)
#define get_id_from_sortidx(a, sid) cl_sort2id(a, sid)
#define get_id_info(a, sid, freq, len) cl_id2all(a, sid, freq, len)
#define get_id_range(a) cl_max_id(a)
#define get_id_string_len(a, id) cl_id2strlen(a, id)
#define get_nr_of_strucs(a, nr) cl_max_struc_oldstyle(a, nr)
#define get_num_of_struc(a, p, num) cl_cpos2struc_oldstyle(a, p, num)
#define get_positions(a, id, freq, rl, rls) cl_id2cpos_oldstyle(a, id, freq, rl, rls)
#define get_sortidxpos_of_id(a, id) cl_id2sort(a, id)
#define get_string_at_position(a, cpos) cl_cpos2str(a, cpos)
#define get_string_of_id(a, id) cl_id2str(a, id)
#define get_struc_attribute(a, cpos, start, end) cl_cpos2struc2cpos(a, cpos, start, end)
#define inverted_file_is_compressed(a) cl_index_compressed(a)
#define item_sequence_is_compressed(a) cl_sequence_compressed(a)
#define nr_of_arguments(a) cl_dynamic_numargs(a)
#define setup_corpus(reg, name) cl_new_corpus(reg, name)
#define structure_has_values(a) cl_struc_values(a)
#define structure_value(a, struc) cl_struc2str(a, struc)
#define structure_value_at_position(a, cpos) cl_cpos2struc2str(a, cpos)

/*
 * Some "old" functions have gone altogether; they are not just depracated, but vanished!
 *
 * (So some compatibility with the pre-3.2.0 CL has been broken in a minor way. If you
 * REALLY need these functions, you can get them back by #including other headers from
 * the CL module - but as these are now "private methods" they are not guaranteed to
 * stay the same, or even to exist at all, in future revisions of the CL.)
 *
 *    cl_string_maptable()
 *
 * This is because it didn't make sense for v3.2.0 or higher, where UTF8 strings are not
 * only possible but likely and we need to be a bit more sophisticated all around about
 * how we deal with inter-character mappings.
 *
 *    describe_corpus()
 *
 * This was not really "at home" in a low-level API; it's been hidden away, and may later
 * move out of the CL altogether and into the CWB utilities, where it fits better.
 *
 *    find_corpus()
 *
 * This should never have been a public function in the first place - it's an internal
 * function called by cl_new_corpus().
 *
 */

/* new style function names implemented as macros mapping to the old names : retained as a record of how it was done pre-v3.2.0

#define cl_new_corpus(reg, name) setup_corpus(reg, name)
#define cl_delete_corpus(c) drop_corpus(c)
#define cl_standard_registry() central_corpus_directory()
#define cl_new_attribute(c, name, type) find_attribute(c, name, type, NULL)
#define cl_delete_attribute(a) attr_drop_attribute(a)
#define cl_sequence_compressed(a) item_sequence_is_compressed(a)
#define cl_index_compressed(a) inverted_file_is_compressed(a)
#define cl_new_stream(a, id) OpenPositionStream(a, id)
#define cl_delete_stream(ps) ClosePositionStream(ps)
#define cl_read_stream(ps, buf, size) ReadPositionStream(ps, buf, size)
#define cl_id2str(a, id) get_string_of_id(a, id)
#define cl_str2id(a, str) get_id_of_string(a, str)
#define cl_id2strlen(a, id) get_id_string_len(a, id)
#define cl_sort2id(a, sid) get_id_from_sortidx(a, sid)
#define cl_id2sort(a, id) get_sortidxpos_of_id(a, id)
#define cl_max_cpos(a) get_attribute_size(a)
#define cl_max_id(a) get_id_range(a)
#define cl_id2freq(a, id) get_id_frequency(a, id)
#define cl_id2cpos(a, id, freq) get_positions(a, id, freq, NULL, 0)
#define cl_cpos2id(a, cpos) get_id_at_position(a, cpos)
#define cl_cpos2str(a, cpos) get_string_at_position(a, cpos)
#define cl_id2all(a, sid, freq, len) get_id_info(a, sid, freq, len)
#define cl_regex2id(a, re, flags, size) collect_matching_ids(a, re, flags, size)
#define cl_idlist2freq(a, list, size) cumulative_id_frequency(a, list, size)
#define cl_idlist2cpos(a, idlist, idlist_size, sort, size) collect_matches(a, idlist, idlist_size, sort, size, NULL, 0)
#define cl_cpos2struc2cpos(a, cpos, start, end) get_struc_attribute(a, cpos, start, end)
#define cl_struc2cpos(a, struc, start, end) get_bounds_of_nth_struc(a, struc, start, end)
#define cl_struc_values(a) structure_has_values(a)
#define cl_struc2str(a, struc) structure_value(a, struc)
#define cl_cpos2struc2str(a, cpos) structure_value_at_position(a, cpos)
*/

#endif /* ifndef _cwb_cl_h_ */
