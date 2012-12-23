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
  do {
    got = fread(buf, 1, BLOCK_SIZE, from);
    if (got < 0)
      return;
    if (got)
      got = fwrite(buf, 1, got, to);
  } while (got == BLOCK_SIZE);
}

static int check_and_replace(const char *replace) {
  struct stat statbuf;
  char buf[sizeof(refstr)] = {0};
  int res;
  FILE *file = NULL;
  FILE *src_file = NULL;

  // check for auto-generated multi-location temp.tie
  res = stat("SKIRMISH/temp.tie", &statbuf);
  if (res != 0 || statbuf.st_size != 157648)
    goto err_out;
  file = fopen("SKIRMISH/temp.tie", "rb");
  if (!file)
    goto err_out;
  res = fread(buf, 1, sizeof(buf), file);
  if (res != sizeof(buf) || memcmp(buf, refstr, sizeof(refstr)))
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

int main(int argc, char *argv[]) {
  struct stat statbuf;
  if (stat("SKIRMISH", &statbuf) != 0) {
    fprintf(stderr, "Could not find SKIRMISH directory, started at wrong location?\n");
    return 1;
  }
  // TODO: read in list of replacement files
  // TODO: check we can open all files in list
  // TODO: print out list?
  do {
    if (check_and_replace("test.tie")) {
      // TODO: move on to next in list
      // TODO: print out list with updated position
    }
    // TODO: check input and allow moving up/down in list
    usleep(0.1 * 1000 * 1000);
  } while (1);
}
