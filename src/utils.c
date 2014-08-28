#include "utils.h"

#include <unistd.h>
#include <stdlib.h>

void go_through(void)
{
  char buf[BUF_SIZE];
  size_t len;

  while ((len = read(STDIN_FILENO, buf, BUF_SIZE))) {
    write(STDOUT_FILENO, buf, len);
  }

  exit(0);
}
