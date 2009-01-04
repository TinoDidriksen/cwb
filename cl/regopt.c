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

#include "globals.h"
#include "regopt.h"

/* set of 'grains' (any matching string must contain one of these) */
char *cl_regopt_grain[MAX_GRAINS]; /* list of grains */
int cl_regopt_grain_len;        /* all grains have the same length */
int cl_regopt_grains;           /* number of grains */
int cl_regopt_anchor_start;     /* whether grains are anchored at beginning of string */
int cl_regopt_anchor_end;       /* whether grains are anchored at end of string */

/* jump table for Boyer-Moore search algorithm */
int cl_regopt_jumptable[256];    /* use _unsigned_ char as index */

/* when regex is parsed, grains for each segment are written to this intermediate buffer;
   if the new set of grains is better than the current one, it is copied to the global variables */
char *grain_buffer[MAX_GRAINS]; 
int   grain_buffer_grains = 0;

/* buffers for grain strings */
char  public_grain_data[MAX_LINE_LENGTH];
char   local_grain_data[MAX_LINE_LENGTH];

/* optimised = cl_regopt_analyse(regex_string);
 * try to extract set of grains from regular expression <regex_string>, which is used by the CL regex matcher
 * and cl_regex2id() for faster regular expression search; if successful, returns True and stores grains
 * in the global variables above (from which they should be copied to a CL_Regex struct) */
int cl_regopt_analyse(char *regex);
/* NB: the use of global variables and a fixed-size buffer for grains is partly due to historical reasons,
 * but it does also serve to reduce memory allocation overhead */ 

/*
 * interface functions
 */

char cl_regex_error[MAX_LINE_LENGTH];

/* create new CL regex buffer; regular expression is pre-processed according to flags 
 * (IGNORE_CASE and IGNORE_DIAC) and anchored to start and end of the string; then compiled and optimised;
 * currently, charset is ignored and assumed to be Latin1 */
CL_Regex cl_new_regex(char *regex, int flags, CorpusCharset charset) {
  char *iso_regex;              /* allocate dynamically to support very long regexps (from RE() operator) */
  char *anchored_regex;
  CL_Regex rx;
  int error_num, optimised, i, l;

  /* allocate temporary strings */
  l = strlen(regex);
  iso_regex = (char *) cl_malloc(l + 1);
  anchored_regex = (char *) cl_malloc(l + 5);

  /* allocate and initialise CL_Regex object */
  rx = (CL_Regex) cl_malloc(sizeof(struct _CL_Regex));
  rx->iso_string = NULL;
  rx->charset = charset;
  rx->flags = flags & (IGNORE_CASE | IGNORE_DIAC); /* mask unsupported flags */
  rx->grains = 0;               /* indicates no optimisation -> other opt. fields are invalid */

  /* pre-process regular expression (translate latex escapes and normalise) */
  cl_string_latex2iso(regex, iso_regex, l);
  cl_string_canonical(iso_regex, rx->flags);

  /* add start and end anchors to improve performance of regex matcher for expressions such as ".*ung" */
  sprintf(anchored_regex, "^(%s)$", iso_regex);

  /* compile regular expression with POSIX library function */ 
  error_num = regcomp(&rx->buffer, anchored_regex, REG_EXTENDED|REG_NOSUB);
  if (error_num != 0) {
    (void) regerror(error_num, NULL, cl_regex_error, MAX_LINE_LENGTH);
    fprintf(stderr, "Regex Compile Error: %s\n", cl_regex_error);
    cl_free(rx);
    cl_free(iso_regex);
    cl_free(anchored_regex);
    cderrno = CDA_EBADREGEX;
    return NULL;
  }

  /* allocate string buffer for cl_regex_match() function if flags are present */
  if (flags)
    rx->iso_string = (char *) cl_malloc(MAX_LINE_LENGTH);       /* this is for the string being matched, not the regex! */

  /* attempt to optimise regular expression */
  optimised = cl_regopt_analyse(iso_regex);
  if (optimised) {              /* copy optimiser data to CL_Regex object */
    rx->grains = cl_regopt_grains;
    rx->grain_len = cl_regopt_grain_len;
    rx->anchor_start = cl_regopt_anchor_start;
    rx->anchor_end = cl_regopt_anchor_end;
    for (i = 0; i < 256; i++)
      rx->jumptable[i] = cl_regopt_jumptable[i];
    for (i = 0; i < rx->grains; i++)
      rx->grain[i] = cl_strdup(cl_regopt_grain[i]);
    if (cl_debug) 
      fprintf(stderr, "CL: using %d grain(s) for optimised regex matching\n", rx->grains);
  }
  
  cl_free(iso_regex);
  cl_free(anchored_regex);
  cderrno = CDA_OK;
  return rx;
}

/* show approximate level of optimisation, computed from grain length and number of grains (0 = not optimised) */
int cl_regex_optimised(CL_Regex rx) {
  if (rx->grains == 0) 
    return 0;                   /* not optimised */
  else {
    int level = (3 * rx->grain_len) / rx->grains;
    return((level >= 1) ? level+1 : 1); 
  }
}

/* match regular expression <rx> against string <str>, using the settings <rx> was created with */ 
int cl_regex_match(CL_Regex rx, char *str) {
  char *iso_string;             /* either the original string or a pointer to rx->iso_string */
  int optimised = (rx->grains > 0);
  int i, di, k, max_i, len, jump;
  int grain_match, result;

  if (rx->flags) {              /* normalise input string if necessary */
    iso_string = rx->iso_string;
    strcpy(iso_string, str);
    cl_string_canonical(iso_string, rx->flags);
  }
  else
    iso_string = str;

  grain_match = 0;
  if (optimised) {
    /* this 'optimised' matcher may look fairly bloated, but it's still way ahead of POSIX regexen */
    len = strlen(iso_string);
    /* string offset where first character of each grain would be */
    max_i = len - rx->grain_len; /* stop trying to match when i > max_i */
    if (rx->anchor_end) 
      i = (max_i >= 0) ? max_i : 0; /* if anchored at end, align grains with end of string */
    else
      i = 0;

    while (i <= max_i) {
      jump = rx->jumptable[(unsigned char) iso_string[i + rx->grain_len - 1]];
      if (jump > 0) {
        i += jump;              /* Boyer-Moore search */
      }
      else {
        for (k = 0; k < rx->grains; k++) {
          char *grain = rx->grain[k];
          di = 0;
          while ((di < rx->grain_len) && (grain[di] == iso_string[i+di]))
            di++;
          if (di >= rx->grain_len) {
            grain_match = 1;
            break;              /* we have found a grain match and can quit the loop */
          }
        }
        i++;
      }
      if (rx->anchor_start) break; /* if anchored at start, only the first iteration can match */
    }
  }
        
  if (optimised && !grain_match) /* enabled since version 2.2.b94 (14 Feb 2006) -- before: && cl_optimize */
    result = REG_NOMATCH; /* according to POSIX.2 */
  else
    result = regexec(&(rx->buffer), iso_string, 0, NULL, 0);

#if 0  /* debugging code used before version 2.2.b94 */
  /* critical: optimiser didn't accept candidate, but regex matched */
  if ((result == 0) && optimised && !grain_match) 
    fprintf(stderr, "CL ERROR: regex optimiser did not accept '%s' although it should have!\n", iso_string);
#endif

  return (result == 0);         /* return true if regular expression matched */
}


/* free CL_Regex object */
void cl_delete_regex(CL_Regex rx) {
  int i;
  regfree(&(rx->buffer));       /* free POSIX regex buffer */
  cl_free(rx->iso_string);      /* free string buffer if it was allocated */
  for (i = 0; i < rx->grains; i++)
    cl_free(rx->grain[i]);      /* free grain strings if regex was optimised */
  free(rx);
}



/*
 * helper functions (for optimiser)
 */

/* 'safe' symbol which will only match itself */
int
is_safe_char(unsigned char c) {
  if ((c >= 'A' && c <= 'Z') ||
      (c >= 'a' && c <= 'z') ||
      (c >= '0' && c <= '9') ||
      (c == '-' || c == '"' || c == '\'' || c == '%' || c == '&' || c == '/' || c == '_' || 
       c == '!' || c == ':' || c == ';'  || c == ',') ||
      (c >= 128 /* && c <= 255 */)) {  /* c <= 255 produces 'comparison is always true' compiler warning */
    return 1;
  }
  else {
    return 0;
  }
}

/* find longest grain (i.e. string of safe symbols not followed by ?, *, or {..}) starting at <mark>; 
   any backslash-escaped are allowed but the backslashes must be stripped by the caller;
   returns pointer to first character after grain (<mark> if none is found) */
char *
read_grain(char *mark) {
  char *point = mark;
  int last_char_escaped = 0, glen;

  glen = 0;                     /* effective length of grain */
  while (is_safe_char(*point) || ((*point == '\\') && (point[1]))) {
    if (*point == '\\') {       /* skip backslash and escaped character (but not at end of string)*/
      point++; 
      last_char_escaped = 1;
    }
    else {
      last_char_escaped = 0;
    }
    point++;
    glen++;
  }
  if (point > mark) {           /* if followed by ?, *, or {..}, shrink grain by one char */
    if (*point == '?' || *point == '*' || *point == '{') {
      point--;
      glen--;
      if (last_char_escaped)    /* if last character was escaped, make sure to remove the backslash as well */
        point--;
    }
  }
  if (glen >= 2)
    return point;
  else
    return mark;
}

/* read in matchall, any safe character, or a reasonably safe-looking character class */
char *
read_matchall(char *mark) {
  if (*mark == '.') {
    return mark + 1;
  }
  else if (*mark == '[') {
    char *point = mark + 1;
    /* according to the POSIX standard, \ does not have a special meaning in a character class;
       we won't skip it or any other special characters with possibly messy results;
       we just accept | as a special optimisation for the matches and contains operators in CQP */
    while (*point != ']' &&  *point != '\\' && *point != '[' && *point != '\0') {
      point++;
    }
    return (*point == ']') ? point + 1 : mark;
  }
  else if (is_safe_char(*mark)) {
    return mark + 1;
  }
  else if (*mark == '\\') {     /* outside a character class, \ always escapes to literal meaning */
    return mark + 2;
  }
  else {
    return mark;
  }
}

/* read in Kleene star, ?, +, or general repetition {<n>,<m>};
   returns pointer to first character after Kleene star */
char *
read_kleene(char *mark) {
  char *point = mark;
  if (*point == '?' || *point == '*' || *point == '+') {
    return point + 1;
  }
  else if (*point == '{') {
    point++;
    while ((*point >= '0' && *point <= '9') || (*point == ',')) {
      point++;
    }
    return (*point == '}') ? point + 1 : mark;
  }
  else {
    return mark;
  }
}

/* read in wildcard segment matching arbitrary substring (but without '|' symbol);
   returns pointer to first character after wildcard segment */
char *
read_wildcard(char *mark) {
  char *point;
  point = read_matchall(mark);
  if (point > mark) {
    return read_kleene(point);
  }
  else {
    return mark;
  }
}

/* find grains in disjunction group, which are stored in grain_buffer; <mark> must point to '(' at beginning of disjunction group;
   returns pointer to first character after disjunction group iff parse succeeded, <mark> otherwise; 
   grain_buffer_grains > 0 iff a grain could be found in every alternative;
   align_start and align_end are true if the grains from _all_ alternatives are anchored at the
   start or end of the disjunction group, respectively */
char *
read_disjunction(char *mark, int *align_start, int *align_end) {
  char *point, *p2, *q, *buf;
  int grain, failed;  

  if (*mark == '(') {
    point = mark + 1;
    buf = local_grain_data;
    grain_buffer_grains = 0;
    grain = 0;
    failed = 0;

    /* if we can extend the disjunction parser to allow parentheses around the initial segment of 
       an alternative, then regexen created by the matches operator will also be optimised! */
    *align_start = *align_end = 1;
    while (1) {                 /* loop over alternatives in disjunction */
      for (p2 = read_grain(point); p2 == point; p2 = read_grain(point)) {
        p2 = read_wildcard(point); /* try skipping data until grain is found */
        if (p2 > point) {
          point = p2;           /* advance point and look for grain again */
          *align_start = 0;     /* grain in this alternative can't be aligned at start */
        }
        else 
          break;                /* didn't find grain and can't skip any further, so give up */
      }
      if (p2 == point) {
        failed = 1;             /* no grain found in this alternative -> return failure */
        break;          
      }
      if (grain < MAX_GRAINS) {
        grain_buffer[grain] = buf; /* copy grain into local grain buffer */
        for (q = point; q < p2; q++) { 
          if (*q == '\\') q++;  /* skip backslash used as escape character */
          *buf++ = *q;
        }
        *buf++ = '\0';
      }
      grain++;
      point = p2;
      while (*point != '|') {
        p2 = read_wildcard(point); /* try skipping data up to next | or ) */
        if (p2 > point) {
          point = p2;
          *align_end = 0;       /* grain in this alternative can't be aligned at end */
        }
        else
          break;
      }
      if (*point == '|') 
        point++;                /* continue with next alternative */
      else 
        break;                  /* abort scanning */
    }
    
    if (*point == ')' && !failed) { /* if point is at ) character, we've successfully read the entire disjunction*/
      grain_buffer_grains = (grain > MAX_GRAINS) ? 0 : grain;
      return point + 1;         /* continue parsing after disjunction */
    }
    else {
      return mark;              /* failed to parse disjunction and identify grains */
    }
  }
  else {
    return mark;
  }
}

/* copy local grains to public buffer if they are better than the current set */
/* if the flag <front_aligned> is set, grain strings are aligned on the left when they are reduced to equal lengths;
   if <anchored> is set, the grains are anchored at beginning or end of string, depending on <front_aligned> */
void
update_grain_buffer(int front_aligned, int anchored) {
  char *buf = public_grain_data;
  int i, len, N;

  N = grain_buffer_grains;
  if (N > 0) {
    len = MAX_LINE_LENGTH;
    for (i = 0; i < N; i++) {
      int l = strlen(grain_buffer[i]);
      if (l < len) len = l;
    }
    if (len >= 2) {             /* minimum grain length is 2 */
      /* we make a heuristics decision whether the new set of grains is better than the current one;
         based on grain legth and the number of grains */
      if ((len > (cl_regopt_grain_len + 1)) ||
          ((len == (cl_regopt_grain_len + 1)) && (N <= (3 * cl_regopt_grains))) ||
          ((len == cl_regopt_grain_len) && (N < cl_regopt_grains)) ||
          ((len == (cl_regopt_grain_len - 1)) && ((3 * N) < cl_regopt_grains))) {
        /* the new set of grains is better, copy them to the output buffer */
        for (i = 0; i < N; i++) {
          int l, diff;
          strcpy(buf, grain_buffer[i]);
          cl_regopt_grain[i] = buf;
          l = strlen(buf);
          assert((l >= len) && "Sorry, I messed up grain lengths while optimising a regex.");
          if (l > len) {        /* reduce grains to common length */
            diff = l - len;
            if (front_aligned) {
              buf[len+1] = '\0'; /* chop off tail */
            }
            else {
              cl_regopt_grain[i] += diff; /* chop off head, i.e. advance string pointer */
            }
          }
          buf += l + 1;
        }
        cl_regopt_grains = N;
        cl_regopt_grain_len = len;
        cl_regopt_anchor_start = cl_regopt_anchor_end = 0;
        if (anchored) {
          if (front_aligned) cl_regopt_anchor_start = 1; else cl_regopt_anchor_end = 1;
        }
      }
    }
  }
}

/* compute jump table for Boyer-Moore search; 
   unlike the textbook version, this jumptable includes the last character of each grain
   (in order to avoid running the string comparing loops every time) */
void
make_jump_table(void) {
  int j, k, jump;
  unsigned int   ch;
  unsigned char *grain;         /* want unsigned char to compare with unsigned int ch */

  for (ch = 0; ch < 256; ch++)
    cl_regopt_jumptable[ch] = 0;
  if (cl_regopt_grains > 0) {
    /* compute smallest jump distance for each character (0 -> matches last character of one or more grains) */
    for (ch = 32; ch < 256; ch++) {
      jump = cl_regopt_grain_len; /* if character isn't contained in any of the grains, jump by grain length */
      for (k = 0; k < cl_regopt_grains; k++) {
        grain = (unsigned char *) cl_regopt_grain[k] + cl_regopt_grain_len - 1; /* pointer to last character in grain */
        for (j = 0; j < cl_regopt_grain_len; j++, grain--) {
          if (*grain == ch) {
            if (j < jump) jump = j;
            break;              /* can't find shorter jump dist. for this grain */
          }
        }
      }
      cl_regopt_jumptable[ch] = jump;
    }
    if (cl_debug) {
      fprintf(stderr, "CL: cl_regopt_jumptable for Boyer-Moore search is\n");
      for (k = 32; k < 256; k += 16) {
        fprintf(stderr, "CL: ");
        for (j = 0; j < 15; j++) {
          ch = k + j;
          fprintf(stderr, "|%2d %c ", cl_regopt_jumptable[ch], ch);
        }
        fprintf(stderr, "\n");
      }
    }
  }
}


/*
 *  regexp optimiser: try to analyse regular expression and find best set of grains
 */
int
cl_regopt_analyse(char *regex) {
  char *point, *mark, *q, *buf;
  int i, ok, at_start, at_end, align_start, align_end, anchored;

  mark = regex;
  if (cl_debug) {
    fprintf(stderr, "CL: cl_regopt_analyse('%s')\n", regex);
  }
  cl_regopt_grains = 0;
  cl_regopt_grain_len = 0;
  cl_regopt_anchor_start = cl_regopt_anchor_end = 0;

  ok = 1;
  while (ok) {
    at_start = (mark == regex);
    point = read_grain(mark);
    if (point > mark) {         /* found single grain segment -> copy to local buffer */
      buf = local_grain_data;
      for (q = mark; q < point; q++) {
        if (*q == '\\') q++;    /* skip backslash used as escape character */
        *buf++ = *q;
      }
      *buf++ = '\0';
      grain_buffer[0] = local_grain_data;
      grain_buffer_grains = 1;
      mark = point;
      /* update public grain set */
      at_end = (*mark == '\0');
      anchored = (at_start || at_end);
      update_grain_buffer(at_start, anchored);
      if (*mark == '+') mark++; /* last character of grain may be repeated -> skip the '+' */
    }
    else {
      point = read_disjunction(mark, &align_start, &align_end);
      if (point > mark) {       /* found disjunction group, which is automatically stored in the local grain buffer */
        mark = point;
        /* can't accept grain set if disjunction could be optional: (..)?, (..)*, (..){0,} */
        if ((*mark == '?') || (*mark == '*') || (*mark == '{')) {
          mark = read_kleene(mark); /* accept as wildcard segment */
        }
        else {                  
          /* update public grain set */
          at_end = (*mark == '\0');
          at_start = (at_start && align_start); /* check that grains within disjunction are aligned, too */
          at_end = (at_end && align_end);
          anchored = (at_start || at_end);
          update_grain_buffer(at_start, anchored);
          if (*mark == '+') mark++;
        }
      }
      else {
        point = read_wildcard(mark);
        if (point > mark) {     /* found segment matching some substring -> skip */
          mark = point;
        }
        else {
          ok = 0;               /* no recognised segment starting at mark */
        }
      }
    }
    /* accept if we're at end of string */
    if (*mark == '\0') {
      ok =  (cl_regopt_grains > 0) ? 1 : 0;
      if (cl_debug && ok) {
        fprintf(stderr, "CL: Regex optimised, %d grain(s) of length %d\n", cl_regopt_grains, cl_regopt_grain_len);
        fprintf(stderr, "CL: grain set is");
        for (i = 0; i < cl_regopt_grains; i++) {
          fprintf(stderr, " [%s]", cl_regopt_grain[i]);
        }
        if (cl_regopt_anchor_start)
          fprintf(stderr, " (anchored at beginning of string)");
        if (cl_regopt_anchor_end)
          fprintf(stderr, " (anchored at end of string)");
        fprintf(stderr, "\n");
      }
      if (ok)
        make_jump_table();      /* compute jump table for Boyer-Moore search */
      return ok;
    }
  }

  /* couldn't analyse regexp -> no optimisation */
  return 0;
}
