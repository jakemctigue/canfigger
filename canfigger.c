/*
This file is part of canfigger<https://github.com/andy5995/canfigger>

Copyright (C) 2021  Andy Alt (andy400-dev@yahoo.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include "canfigger.h"

#define MAX_LINE 2048

void
canfigger_free (st_canfigger_node * node)
{
  if (node != NULL)
  {
    canfigger_free (node->next);
    free (node);
  }
  return;
}


/*!
 * If haystack begins with 'needle', returns a pointer to the first occurrence
 * in the string after 'needle'.
 * ex1: char *ptr = del_char_shift ('/', string);
 * '*string' will still point to 'string[0]'
 *
 * ex2: string = del_char_shift ('/', string);
 * '*string' may change
 */
static char *
del_char_shift_left (const char needle, char *haystack)
{
  char *ptr = haystack;
  if (*ptr != needle)
    return ptr;

  while (*ptr == needle)
    ptr++;

  return ptr;
}


/*!
 * Removes trailing white space from a string (including newlines, formfeeds,
 * tabs, etc
 * @param[out] str The string to be altered
 * @return void
 */
static void
trim_whitespace (char *str)
{
  if (str == NULL)
  {
#ifndef TEST_LIB
    exit (EXIT_FAILURE);
#else
    errno = 1;
    return;
#endif
  }

  char *pos_0 = str;
  /* Advance pointer until NULL terminator is found */
  while (*str != '\0')
    str++;

  /* set pointer to segment preceding NULL terminator */
  if (str != pos_0)
    str--;
  else
    return;

  while (isspace (*str))
  {
    *str = '\0';
    if (str != pos_0)
      str--;
    else
      break;
  }

  return;
}

static void
grab_str_segment (const char *begin, const char *end, char *str)
{
  do
  {
    *str++ = *begin++;
  }
  while (begin < end);

  *str = '\0';
  return;
}


st_canfigger_list *
canfigger_parse_file (const char *file, const char delimiter)
{
  static st_canfigger_node *root = NULL;
  st_canfigger_list *list = NULL;

  FILE *fd = fopen (file, "r");
  if (fd == NULL)
    return NULL;

  char fd_line[MAX_LINE];
  while (fgets (fd_line, sizeof fd_line, fd) != NULL)
  {
    trim_whitespace (fd_line);
    char *line_ptr = fd_line;
    line_ptr = del_char_shift_left (' ', line_ptr);
    switch (*line_ptr)
    {
    case '#':
      continue;
    case '\0':
      continue;
    }

    st_canfigger_node *tmp_node = malloc (sizeof (struct st_canfigger_node));
    if (tmp_node != NULL)
    {
      if (root != NULL)
        list->next = tmp_node;

      char *tmp_key = strchr (line_ptr, '=');
      if (tmp_key == NULL)
      {
        trim_whitespace (line_ptr);
        strcpy (tmp_node->key, line_ptr);
        *tmp_node->value = '\0';
        *tmp_node->attribute = '\0';
      }
      else
      {
        char key[MAX_LINE];
        grab_str_segment (line_ptr, tmp_key, key);
        trim_whitespace (key);
        strcpy (tmp_node->key, key);
        tmp_key++;
        tmp_key = del_char_shift_left (' ', tmp_key);
        char *tmp_value = strchr (tmp_key, delimiter);
        if (tmp_value == NULL)
        {
          strcpy (tmp_node->value, tmp_key);
          trim_whitespace (tmp_key);
          *tmp_node->attribute = '\0';
        }
        else
        {
          char value[MAX_LINE];
          grab_str_segment (tmp_key, tmp_value, value);
          strcpy (tmp_node->value, value);
          tmp_value++;
          tmp_value = del_char_shift_left (' ', tmp_value);
          char *tmp_attribute = tmp_value;
          strcpy (tmp_node->attribute, tmp_attribute);
        }
      }
      tmp_node->next = NULL;
      list = tmp_node;

      if (root == NULL)
        root = list;
    }
    else
    {
      if (root != NULL)
        canfigger_free (root);
      errno = ENOMEM;
      return NULL;
    }
  }

  int r = fclose (fd);
  if (r != 0)
  {
    return NULL;
  }

  list = root;
  return list;
}
