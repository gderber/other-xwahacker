/*
 * XWAReplacer: replace temp.tie file for multiplayer with hyperspace jumps.
 * Copyright (C) 2012 Reimar Döffinger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#define usleep(a) Sleep(a/1000)
// replace both function and struct
#define stat _stat
#else
#include <termios.h>
#endif

static const int refstr_skip = 4;
static const char refstr[23][10] = {
  {0x12, 0, 0x0f, 0, 0, 0, 0, 0, 1, 0},
  "",
  "Neutral 1",
  "",
  "Neutral 2",
  "",
  "Neutral 3",
  "",
  "Neutral 4",
  "",
  "Region 1",
};

#define BLOCK_SIZE 4096
static void copy(FILE *to, FILE *from) {
  void *buf = malloc(BLOCK_SIZE);
  int got = 0;
  if (!buf)
    goto err_out;
  do {
    got = fread(buf, 1, BLOCK_SIZE, from);
    if (got < 0)
      goto err_out;
    if (got)
      got = fwrite(buf, 1, got, to);
  } while (got == BLOCK_SIZE);
err_out:
  free(buf);
}

static int check_and_replace(const char *replace) {
  struct stat statbuf;
  char buf[sizeof(refstr)] = {0};
  int res;
  FILE *file = NULL;
  FILE *src_file = NULL;

  // check for auto-generated multi-location temp.tie
  res = stat("SKIRMISH/temp.tie", &statbuf);
  if (res != 0)
    goto err_out;
  file = fopen("SKIRMISH/temp.tie", "rb");
  if (!file)
    goto err_out;
  res = fread(buf, 1, sizeof(buf), file);
  if (res != sizeof(buf) || memcmp(buf + refstr_skip, refstr[0] + refstr_skip, sizeof(refstr) - refstr_skip))
    goto err_out;

  // try to replace it
  src_file = fopen(replace, "rb");
  if (!src_file) {
    fprintf(stderr, "Could not open reference file %s\n", replace);
    goto err_out;
  }
  fclose(file);
  file = fopen("SKIRMISH/temp.tie", "wb");
  if (!file)
    goto err_out;
  copy(file, src_file);
  return 1;

err_out:
  if (file)
    fclose(file);
  if (src_file)
    fclose(src_file);
  return 0;
}

#define MAX_ENTRIES 128
#define MAX_LEN 128

typedef char NameEntry[MAX_LEN];

static NameEntry *read_list(const char *fname)
{
  static const char whitespace[] = " \r\n";
  int i;
  NameEntry *list = calloc(MAX_ENTRIES + 1, sizeof(*list));
  FILE *list_file = NULL;
  list_file = fopen(fname, "r");
  if (!list_file) {
    fprintf(stderr, "Could not open list file %s\n", fname);
    goto err_out;
  }
  for (i = 0; i < MAX_ENTRIES; i++) {
    int len;
    if (!fgets(list[i], sizeof(*list), list_file)) {
      list[i][0] = 0;
      break;
    }
    len = strlen(list[i]);
    // remove trailing newline
    while (len > 0 && strchr(whitespace, list[i][len - 1]))
      len--;
    list[i][len] = 0;
    // skip empty and comment lines
    if (len == 0 || list[i][0] == '#') {
      i--;
      continue;
    }
  }
  if (!list[0][0]) {
    fprintf(stderr, "List file %s does not contain any files\n", fname);
    goto err_out;
  }
  fclose(list_file);
  return list;
err_out:
  free(list);
  if (list_file)
    fclose(list_file);
  return NULL;
}

static void print_list(NameEntry *list, int pos) {
  int i;
  printf("\nNew position:\n");
  for (i = 0; list[i][0]; i++) {
    printf(i == pos ? "--> " : "    ");
    printf("%s", list[i]);
    printf(i == pos ? " <--" : "    ");
    printf("\n");
  }
  printf("Press 's' to move to next entry, 'w' to previous, 'q' to exit\n");
}

static void move_list(NameEntry *list, int *pos, int dir) {
  *pos += dir > 0;
  *pos -= dir < 0;
  if (*pos < 0) {
    int i = 0;
    while (list[i][0]) i++;
    *pos = i - 1;
  } else if (!list[*pos][0])
    *pos = 0;
}

#ifdef _WIN32
static void noblock_init(void) {}
static void noblock_uninit(void) {}
#else
static struct termios orig_term;

static void noblock_init(void) {
  struct termios new_term;
  tcgetattr(0, &orig_term);
  new_term = orig_term;
  new_term.c_lflag &= ~ICANON & ~ECHO;
  new_term.c_cc[VMIN] = 1;
  new_term.c_cc[VTIME] = 0;
  tcsetattr(0, TCSANOW, &new_term);
}

static void noblock_uninit(void) {
  tcsetattr(0, TCSANOW, &orig_term);
}
#endif

static int get_char_noblock(void) {
#ifdef _WIN32
  if (!_kbhit())
    return -1;
  return _getch();
#else
  char res;
  if (read(0, &res, 1) != 1)
    return -1;
  return res;
#endif
}

int main(int argc, char *argv[]) {
  int i;
  int list_pos = 0;
  const char *list_fname = argc == 2 ? argv[1] : "xwareplacer-list.txt";
  NameEntry *list = NULL;
  struct stat statbuf;
  if (stat("SKIRMISH", &statbuf) != 0) {
    fprintf(stderr, "Could not find SKIRMISH directory, started at wrong location?\n");
    goto err_out;
  }
  list = read_list(list_fname);
  if (!list)
    return 1;
  for (i = 0; list[i][0]; i++) {
    FILE *test = fopen(list[i], "rb");
    if (!test) {
      fprintf(stderr, "Could not open file %i from list: %s\n", i, list[i]);
      goto err_out;
    }
    fclose(test);
  }
  print_list(list, list_pos);
  noblock_init();
  do {
    int key;
    if (check_and_replace(list[list_pos])) {
      move_list(list, &list_pos, 1);
      print_list(list, list_pos);
    }

    // check input and allow moving up/down in list
    key = get_char_noblock();
    switch (key) {
    case 'w':
    case 's':
      move_list(list, &list_pos, key == 's' ? 1 : -1);
      print_list(list, list_pos);
    }
    if (key == 'q')
      break;

    usleep(0.01 * 1000 * 1000);
  } while (1);
  noblock_uninit();
  free(list);
  return 0;

err_out:
  fprintf(stderr, "Press enter to exit\n");
  getchar();
  free(list);
  return 1;
}
