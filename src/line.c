/* Line-oriented editing functions

   Copyright (c) 1997-2011 Free Software Foundation, Inc.

   This file is part of GNU Zile.

   GNU Zile is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Zile is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Zile; see the file COPYING.  If not, write to the
   Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
   MA 02111-1301, USA.  */

#include <config.h>

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "extern.h"


/*
 * Structure
 */
struct Line
{
#define FIELD(ty, name) ty name;
#include "line.h"
#undef FIELD
};

#undef GETTER
#undef SETTER

#define GETTER(Obj, name, ty, field)            \
  ty                                            \
  get_ ## name ## _ ## field (const Obj *p)     \
  {                                             \
    return p->field;                            \
  }                                             \

#define SETTER(Obj, name, ty, field)            \
  void                                          \
  set_ ## name ## _ ## field (Obj *p, ty field) \
  {                                             \
    ((Line *)p)->field = field;                 \
  }

#define FIELD(ty, field)                        \
  GETTER (const Line, line, ty, field)          \
  SETTER (const Line, line, ty, field)

#include "line.h"
#undef FIELD

/*
 * Circular doubly-linked lists
 */

/* Create an empty list, returning a pointer to the list */
Line *
line_new (void)
{
  Line *l = XZALLOC (Line);
  *l = (Line) {.next = l, .prev = l, .text = astr_new ()};
  return l;
}

/* Insert a line into list after the given point, returning the new line */
const Line *
line_insert (const Line *lp, astr as)
{
  Line *n = XZALLOC (Line);
  *n = (Line) {.next = lp->next, .prev = lp, .text = as};
  set_line_prev (get_line_next (lp), n);
  set_line_next (lp, n);

  return n;
}


/*
 * Adjust markers (including point) when line at point is split, or
 * next line is joined on, or where a line is edited.
 *   newlp is the line to which characters were moved, oldlp the line
 *    moved from (if dir == 0, newlp == oldlp)
 *   pointo is point at which oldlp was split (to make newlp) or
 *     joined to newlp
 *   dir is 1 for split, -1 for join or 0 for line edit (newlp == oldlp)
 *   if dir == 0, delta gives the number of characters inserted (>0) or
 *     deleted (<0)
 */
static void
adjust_markers (const Line * newlp, const Line * oldlp, size_t pointo, int dir, ptrdiff_t delta)
{
  Marker *m_pt = point_marker ();

  assert (dir >= -1 && dir <= 1);

  for (Marker *m = get_buffer_markers (cur_bp); m != NULL; m = get_marker_next (m))
    {
      Point pt = get_marker_pt (m);

      if (pt.p == oldlp && (dir == -1 || pt.o > pointo))
        {
          pt.p = newlp;
          pt.o += delta - (pointo * dir);
          pt.n += dir;
        }
      else if (pt.n > get_buffer_pt (cur_bp).n)
        pt.n += dir;

      set_marker_pt (m, pt);
    }

  /* This marker has been updated to new position. */
  goto_point (get_marker_pt (m_pt));
  unchain_marker (m_pt);
}

/*
 * Check the case of a string.
 * Returns 2 if it is all upper case, 1 if just the first letter is,
 * and 0 otherwise.
 */
static int
check_case (const char *s, size_t len)
{
  size_t i;

  for (i = 0; i < len && isupper ((int) s[i]); i++)
    ;
  if (i == len)
    return 2;
  else if (i == 1)
    for (; i < len && !isupper ((int) s[i]); i++)
      ;
  return i == len;
}

/* Insert the character at the current position and move the text at its right
 * whatever the insert/overwrite mode is.
 * This function doesn't change the current position of the pointer.
 */
static int
intercalate_char (int c)
{
  if (warn_if_readonly_buffer ())
    return false;

  undo_save (UNDO_REPLACE_BLOCK, get_buffer_pt (cur_bp), 0, 1);
  astr_insert_char (get_buffer_pt (cur_bp).p->text, get_buffer_pt (cur_bp).o, c);
  set_buffer_modified (cur_bp, true);

  return true;
}

/*
 * Insert the character `c' at the current point position
 * into the current buffer.
 */
int
insert_char (int c)
{
  size_t t = tab_width (cur_bp);

  if (warn_if_readonly_buffer ())
    return false;

  if (get_buffer_overwrite (cur_bp))
    {
      Point pt = get_buffer_pt (cur_bp);
      /* Current character isn't the end of line or a \t
         || current char is a \t && we are on the tab limit.  */
      if ((pt.o < astr_len (pt.p->text)) && ((astr_get (pt.p->text, pt.o) != '\t')
           ||
           ((astr_get (pt.p->text, pt.o) == '\t') && ((get_goalc () % t) == t))))
        {
          /* Replace the character.  */
          char ch = (char) c;
          undo_save (UNDO_REPLACE_BLOCK, pt, 1, 1);
          astr_nreplace_cstr (pt.p->text, pt.o, 1, &ch, 1);
          ++pt.o;
          goto_point (pt);
          set_buffer_modified (cur_bp, true);

          return true;
        }
      /*
       * Fall through the "insertion" mode of a character at the end
       * of the line, since it is the same as "overwrite" mode.
       */
    }

  intercalate_char (c);
  forward_char ();
  adjust_markers (get_buffer_pt (cur_bp).p, get_buffer_pt (cur_bp).p, get_buffer_pt (cur_bp).o, 0, 1);

  return true;
}

/*
 * Insert a newline at the current position without moving the cursor.
 * Update markers after point in the split line.
 */
static bool
intercalate_newline (void)
{
  if (warn_if_readonly_buffer ())
    return false;

  undo_save (UNDO_REPLACE_BLOCK, get_buffer_pt (cur_bp), 0, 1);

  /* Move the text after the point into a new line. */
  line_insert (get_buffer_pt (cur_bp).p,
               astr_substr (get_buffer_pt (cur_bp).p->text, get_buffer_pt (cur_bp).o,
                            astr_len (get_buffer_pt (cur_bp).p->text) - get_buffer_pt (cur_bp).o));
  set_buffer_last_line (cur_bp, get_buffer_last_line (cur_bp) + 1);
  astr_truncate (get_buffer_pt (cur_bp).p->text, get_buffer_pt (cur_bp).o);
  adjust_markers (get_buffer_pt (cur_bp).p->next, get_buffer_pt (cur_bp).p, get_buffer_pt (cur_bp).o, 1, 0);

  set_buffer_modified (cur_bp, true);
  thisflag |= FLAG_NEED_RESYNC;

  return true;
}

bool
delete_char (void)
{
  deactivate_mark ();

  if (eobp ())
    {
      minibuf_error ("End of buffer");
      return false;
    }

  if (warn_if_readonly_buffer ())
    return false;

  undo_save (UNDO_REPLACE_BLOCK, get_buffer_pt (cur_bp), 1, 0);

  if (eolp ())
    {
      size_t oldlen = astr_len (get_buffer_pt (cur_bp).p->text);
      const Line *oldlp = get_buffer_pt (cur_bp).p->next;

      /* Join the lines. */
      astr_cat (get_buffer_pt (cur_bp).p->text, oldlp->text);
      set_line_next (get_line_prev (oldlp), get_line_next (oldlp));
      set_line_prev (get_line_next (oldlp), get_line_prev (oldlp));

      adjust_markers (get_buffer_pt (cur_bp).p, oldlp, oldlen, -1, 0);
      set_buffer_last_line (cur_bp, get_buffer_last_line (cur_bp) - 1);
      thisflag |= FLAG_NEED_RESYNC;
    }
  else
    {
      astr_remove (get_buffer_pt (cur_bp).p->text, get_buffer_pt (cur_bp).o, 1);
      adjust_markers (get_buffer_pt (cur_bp).p, get_buffer_pt (cur_bp).p, get_buffer_pt (cur_bp).o, 0, -1);
    }

  set_buffer_modified (cur_bp, true);

  return true;
}

/*
 * Replace text in the line `lp' with `newtext'. If `replace_case' is
 * true then the new characters will be the same case as the old if
 * `case-replace' is true.
 */
void
line_replace_text (const Line * lp, size_t offset, size_t oldlen,
                   const char *newtext, int replace_case)
{
  astr as = astr_new_cstr (newtext);

  replace_case = replace_case && get_variable_bool ("case-replace");

  if (replace_case)
    {
      int case_type = check_case (astr_cstr (lp->text) + offset, oldlen);

      if (case_type != 0)
          astr_recase (as, case_type == 1 ? case_capitalized : case_upper);
    }

  set_buffer_modified (cur_bp, true);
  astr_replace (lp->text, offset, oldlen, as);
  adjust_markers (lp, lp, offset, 0, (ptrdiff_t) (astr_len (as) - oldlen));
}

/*
 * Insert a character at the current position in insert mode
 * whatever the current insert mode is.
 */
int
insert_char_in_insert_mode (int c)
{
  bool old_overwrite = get_buffer_overwrite (cur_bp);

  set_buffer_overwrite (cur_bp, false);
  int ret = insert_char (c);
  set_buffer_overwrite (cur_bp, old_overwrite);

  return ret;
}

static void
insert_expanded_tab (int (*inschr) (int chr))
{
  undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);

  int c = get_goalc ();
  int t = tab_width (cur_bp);
  for (c = t - c % t; c > 0; --c)
    (*inschr) (' ');

  undo_save (UNDO_END_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
}

static bool
insert_tab (void)
{
  if (warn_if_readonly_buffer ())
    return false;

  if (get_variable_bool ("indent-tabs-mode"))
    insert_char_in_insert_mode ('\t');
  else
    insert_expanded_tab (insert_char_in_insert_mode);

  return true;
}

DEFUN ("tab-to-tab-stop", tab_to_tab_stop)
/*+
Insert a tabulation at the current point position into the current
buffer.
+*/
{
  ok = execute_with_uniarg (true, uniarg, insert_tab, NULL);
}
END_DEFUN

bool
insert_newline (void)
{
  return intercalate_newline () && forward_char ();
}

/*
 * If point is greater than fill-column, then split the line at the
 * right-most space character at or before fill-column, if there is
 * one, or at the left-most at or after fill-column, if not. If the
 * line contains no spaces, no break is made.
 *
 * Return flag indicating whether break was made.
 */
bool
fill_break_line (void)
{
  size_t fillcol = get_variable_number ("fill-column");
  bool break_made = false;

  /* Only break if we're beyond fill-column. */
  if (get_goalc () > fillcol)
    {
      size_t break_col = 0;

      /* Save point. */
      Marker *m = point_marker ();

      /* Move cursor back to fill column */
      size_t old_col = get_buffer_pt (cur_bp).o;
      while (get_goalc () > fillcol + 1)
        {
          Point pt = get_buffer_pt (cur_bp);
          pt.o--;
          goto_point (pt);
        }

      /* Find break point moving left from fill-column. */
      for (size_t i = get_buffer_pt (cur_bp).o; i > 0; i--)
        {
          int c = astr_get (get_line_text (get_buffer_pt (cur_bp).p), i - 1);
          if (isspace (c))
            {
              break_col = i;
              break;
            }
        }

      /* If no break point moving left from fill-column, find first
         possible moving right. */
      if (break_col == 0)
        {
          for (size_t i = get_buffer_pt (cur_bp).o + 1;
               i < astr_len (get_line_text (get_buffer_pt (cur_bp).p));
               i++)
            {
              int c = astr_get (get_line_text (get_buffer_pt (cur_bp).p), i - 1);
              if (isspace (c))
                {
                  break_col = i;
                  break;
                }
            }
        }

      if (break_col >= 1) /* Break line. */
        {
          Point pt = get_buffer_pt (cur_bp);
          pt.o = break_col;
          goto_point (pt);
          FUNCALL (delete_horizontal_space);
          insert_newline ();
          goto_point (get_marker_pt (m));
          break_made = true;
        }
      else /* Undo fiddling with point. */
        {
          Point pt = get_buffer_pt (cur_bp);
          pt.o = old_col;
          goto_point (pt);
        }

      unchain_marker (m);
    }

  return break_made;
}

static bool
newline (void)
{
  if (get_buffer_autofill (cur_bp) &&
      get_goalc () > (size_t) get_variable_number ("fill-column"))
    fill_break_line ();
  return insert_newline ();
}

DEFUN ("newline", newline)
/*+
Insert a newline at the current point position into
the current buffer.
+*/
{
  ok = execute_with_uniarg (true, uniarg, newline, NULL);
}
END_DEFUN

DEFUN ("open-line", open_line)
/*+
Insert a newline and leave point before it.
+*/
{
  ok = execute_with_uniarg (true, uniarg, intercalate_newline, NULL);
}
END_DEFUN

void
insert_nstring (const char *s, size_t len)
{
  undo_save (UNDO_REPLACE_BLOCK, get_buffer_pt (cur_bp), 0, len);
  undo_nosave = true;
  for (size_t i = 0; i < len; i++)
    {
      if (s[i] == '\n')
        insert_newline ();
      else
        insert_char_in_insert_mode (s[i]);
    }
  undo_nosave = false;
}

DEFUN_NONINTERACTIVE_ARGS ("insert", insert,
                           STR_ARG (arg))
/*+
Insert the argument at point.
+*/
{
  STR_INIT (arg);
  insert_nstring (arg, strlen (arg));
}
END_DEFUN

void
insert_astr (castr as)
{
  insert_nstring (astr_cstr (as), astr_len (as));
}

void
bprintf (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  insert_astr (astr_vfmt (fmt, ap));
  va_end (ap);
}

static bool
backward_delete_char (void)
{
  deactivate_mark ();

  if (!backward_char ())
    {
      minibuf_error ("Beginning of buffer");
      return false;
    }

  delete_char ();
  return true;
}

static bool
backward_delete_char_overwrite (void)
{
  if (bolp () || eolp ())
    return backward_delete_char ();

  deactivate_mark ();

  if (warn_if_readonly_buffer ())
    return false;

  backward_char ();
  if (following_char () == '\t')
    insert_expanded_tab (insert_char);
  else
    insert_char (' ');
  backward_char ();

  return true;
}

DEFUN_ARGS ("delete-char", delete_char,
            INT_OR_UNIARG (n))
/*+
Delete the following @i{n} characters (previous if @i{n} is negative).
+*/
{
  INT_OR_UNIARG_INIT (n);
  ok = execute_with_uniarg (true, n, delete_char, backward_delete_char);
}
END_DEFUN

DEFUN_ARGS ("backward-delete-char", backward_delete_char,
            INT_OR_UNIARG (n))
/*+
Delete the previous @i{n} characters (following if @i{n} is negative).
+*/
{
  bool (*forward) (void) = get_buffer_overwrite (cur_bp) ?
    backward_delete_char_overwrite : backward_delete_char;
  INT_OR_UNIARG_INIT (n);
  ok = execute_with_uniarg (true, n, forward, delete_char);
}
END_DEFUN

DEFUN ("delete-horizontal-space", delete_horizontal_space)
/*+
Delete all spaces and tabs around point.
+*/
{
  undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);

  while (!eolp () && isspace (following_char ()))
    delete_char ();

  while (!bolp () && isspace (preceding_char ()))
    backward_delete_char ();

  undo_save (UNDO_END_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
}
END_DEFUN

DEFUN ("just-one-space", just_one_space)
/*+
Delete all spaces and tabs around point, leaving one space.
+*/
{
  undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
  FUNCALL (delete_horizontal_space);
  insert_char_in_insert_mode (' ');
  undo_save (UNDO_END_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
}
END_DEFUN

/***********************************************************************
                         Indentation command
***********************************************************************/
/*
 * Go to cur_goalc () in the previous non-blank line.
 */
static void
previous_nonblank_goalc (void)
{
  size_t cur_goalc = get_goalc ();

  /* Find previous non-blank line. */
  while (FUNCALL_ARG (forward_line, -1) == leT && is_blank_line ());

  /* Go to `cur_goalc' in that non-blank line. */
  while (!eolp () && get_goalc () < cur_goalc)
    forward_char ();
}

DEFUN ("indent-relative", indent_relative)
/*+
Space out to under next indent point in previous nonblank line.
An indent point is a non-whitespace character following whitespace.
The following line shows the indentation points in this line.
    ^         ^    ^     ^   ^           ^      ^  ^    ^
If the previous nonblank line has no indent points beyond the
column point starts at, `tab-to-tab-stop' is done instead, unless
this command is invoked with a numeric argument, in which case it
does nothing.
+*/
{
  size_t target_goalc = 0, cur_goalc = get_goalc ();
  size_t t = tab_width (cur_bp);

  ok = leNIL;

  if (warn_if_readonly_buffer ())
    return leNIL;

  deactivate_mark ();

  /* If we're on first line, set target to 0. */
  if (get_buffer_pt (cur_bp).p->prev == get_buffer_lines (cur_bp))
    target_goalc = 0;
  else
    { /* Find goalc in previous non-blank line. */
      Marker *m = point_marker ();

      previous_nonblank_goalc ();

      /* Now find the next blank char. */
      if (!(preceding_char () == '\t' && get_goalc () > cur_goalc))
        while (!eolp () && (!isspace (following_char ())))
          forward_char ();

      /* Find next non-blank char. */
      while (!eolp () && (isspace (following_char ())))
        forward_char ();

      /* Target column. */
      if (!eolp ())
        target_goalc = get_goalc ();

      goto_point (get_marker_pt (m));
      unchain_marker (m);
    }

  /* Insert indentation.  */
  undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
  if (target_goalc > 0)
    {
      /* If not at EOL on target line, insert spaces & tabs up to
         target_goalc; if already at EOL on target line, insert a tab.
       */
      cur_goalc = get_goalc ();
      if (cur_goalc < target_goalc)
        {
          do
            {
              if (cur_goalc % t == 0 && cur_goalc + t <= target_goalc)
                ok = bool_to_lisp (insert_tab ());
              else
                ok = bool_to_lisp (insert_char_in_insert_mode (' '));
            }
          while (ok == leT && (cur_goalc = get_goalc ()) < target_goalc);
        }
      else
        ok = bool_to_lisp (insert_tab ());
    }
  else
    ok = bool_to_lisp (insert_tab ());
  undo_save (UNDO_END_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
}
END_DEFUN

static size_t
previous_line_indent (void)
{
  size_t cur_indent;
  Marker *m = point_marker ();

  FUNCALL (previous_line);
  FUNCALL (beginning_of_line);

  /* Find first non-blank char. */
  while (!eolp () && (isspace (following_char ())))
    forward_char ();

  cur_indent = get_goalc ();

  /* Restore point. */
  goto_point (get_marker_pt (m));
  unchain_marker (m);

  return cur_indent;
}

DEFUN ("indent-for-tab-command", indent_for_tab_command)
/*+
Indent line or insert a tab.
Depending on `tab-always-indent', either insert a tab or indent.
If initial point was within line's indentation, position after
the indentation.  Else stay at same point in text.
+*/
{
  if (get_variable_bool ("tab-always-indent"))
    return bool_to_lisp (insert_tab ());
  else if (get_goalc () < previous_line_indent ())
    return FUNCALL (indent_relative);
}
END_DEFUN

DEFUN ("newline-and-indent", newline_and_indent)
/*+
Insert a newline, then indent.
Indentation is done using the `indent-for-tab-command' function.
+*/
{
  ok = leNIL;

  if (warn_if_readonly_buffer ())
    return leNIL;

  deactivate_mark ();

  undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
  if (insert_newline ())
    {
      Marker *m = point_marker ();

      /* Check where last non-blank goalc is. */
      previous_nonblank_goalc ();
      size_t pos = get_goalc ();
      int indent = pos > 0 || (!eolp () && isspace (following_char ()));
      goto_point (get_marker_pt (m));
      unchain_marker (m);
      /* Only indent if we're in column > 0 or we're in column 0 and
         there is a space character there in the last non-blank line. */
      if (indent)
        FUNCALL (indent_for_tab_command);
      ok = leT;
    }
  undo_save (UNDO_END_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
}
END_DEFUN
