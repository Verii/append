/*
    Copyright 2015 James Smith <james@apertum.org>

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void
usage(void)
{
  fprintf(stderr, "Copyright 2015 James Smith <james@apertum.org>\n");
  fprintf(stderr,
          "append: %s\n\t"
          "[-s source] Read from source\n\t"
          " read from stdin by default\n\t"
          "[-v] Print which files are being written to\n\t"
          "[-h] This help\n\t"
          "\nEverything after -s will be interpreted as a file name."
          "\nThat is, use the program like this: prepend -v tofile1 tofile2"
          "\nOr like: prepend -s fromfile tofile1 tofile2"
          "\nBut not like this: prepend -v -s fromfile1 -h tofile1 tofile2"
          "\n\nProject webpage: <https://github.com/Verii/append>\n",
          VERSION);
  exit(1);
}

int
append(const char* str, size_t str_len, const char* path, size_t path_len)
{
  struct stat ss;
  memset(&ss, 0, sizeof(ss));
  lstat(path, &ss);

  if (!S_ISREG(ss.st_mode)) {
    fprintf(stderr, "\"%.*s\" not a file or does not exist.\n", (int)path_len,
            path);
    return 1;
  }

  FILE* fappend = fopen(path, "a");
  if (!fappend) {
    perror(path);
    return 1;
  }

  fwrite(str, 1, str_len, fappend);
  fclose(fappend);

  return 0;
}

/* Reads in the source file at path to buffer buf with length buf_len */
int
in_source(FILE* fin, char** buf, int* buf_len)
{
  char* stdin_buf;
  size_t stdin_buflen = 4096, stdin_len = 0;
  stdin_buf = malloc(stdin_buflen);
  if (!stdin_buf) {
    perror("Cannot get memory");
    return 1;
  }

  /* Read in file to buffer 4k at a time */
  int tmp_len = 0;
  while ((tmp_len = fread(&stdin_buf[stdin_len], 1, stdin_buflen - stdin_len,
                          fin)) > 0) {
    stdin_len += tmp_len;

    if (stdin_len >= stdin_buflen) {
      void* np = realloc(stdin_buf, stdin_buflen * 2);
      if (np) {
        stdin_buf = np;
        stdin_buflen *= 2;
      }
    }
  }

  if (stdin_len < 1) {
    fprintf(stderr, "No data in file. Quitting.\n");
    return 1;
  }

  *buf = stdin_buf;
  *buf_len = stdin_len;
  fclose(fin);

  return 0;
}

int
main(int argc, char** argv)
{
  FILE* fin = NULL;

  char verbose_opt = 0;

  int opt;
  while ((opt = getopt(argc, argv, "+vhcs:")) != -1) {
    switch (opt) {
      case 's': /* Use file as read in buffer */
        fin = fopen(optarg, "r");
        if (!fin) {
          perror(optarg);
          exit(2);
        }
        break;
      case 'v':
        verbose_opt = 1;
        break;
      case 'h': /* command line help: FALLTHORUGH */
      default:
        usage();
        /* NOT REACHED */
    }
  }

  /* Default to stdin if source file not specified */
  if (!fin) {
    fin = stdin;
    fprintf(stderr, "Reading from stdin, press ^D to quit.\n");
  }

  char* in_buf = NULL;
  int in_buflen = 0;

  if (in_source(fin, &in_buf, &in_buflen) != 0 || !in_buf || in_buflen <= 0) {
    return 1;
  }

  /* Append buffer to each file */
  while (optind < argc) {
    if (verbose_opt)
      fprintf(stderr, "Writing to file: %s\n", argv[optind]);
    append(in_buf, in_buflen, argv[optind], strlen(argv[optind]));

    optind++;
  }

  free(in_buf);

  return 0;
}
