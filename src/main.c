#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "utils.h"
#include "mozjpeg-recompress.h"
#include "turbojpeg-recompress.h"
#include "png-recompress.h"

#define VERSION 0
#define SUBVERSION 1

static void display_usage(void)
{
  fprintf(stderr, "Janus Image Worker, v%d.%d\n", VERSION, SUBVERSION);
  fprintf(stderr, "image-worker [options]\n");
  fprintf(stderr, "\t--help Display this help\n");
  fprintf(stderr, "\t--small Try to produce the smallest file\n");
  fprintf(stderr, "\t--fast Try to reduce the file size as fast as possible\n");
}

static void parse_opts(int argc, char **argv, s_compress_options *opts)
{
  for (int i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i], "--small"))
      opts->mode = SMALLEST;
    else if (!strcmp(argv[i], "--fast"))
      opts->mode = FASTEST;
    else if (!strcmp(argv[i], "--help"))
    {
      display_usage();
      exit(0);
    }
    else
    {
      LOG_ERROR("Invalid argument `%s'.", argv[i]);
      display_usage();
      exit(2);
    }
  }
}

int main(int argc, char **argv)
{
  unsigned char       magic[2];
  char                buf[STDIN_BUFFER_SIZE];
  s_compress_options  opts  =
  {
    .mode = DEFAULT
  };

  parse_opts(argc, argv, &opts);

  // We set our own stdin buffer so we are sure it's big enough for images
  // operations. For example this is useful for sending the original JPEG if
  // the quality is already low (by seeking to the beginning of the buffer).
  setbuffer(stdin, buf, STDIN_BUFFER_SIZE);

  // We read the beginning of the magic number and put it back in the stream so
  // that magic number checks from the libraries don't fail.
  if (fread(magic, 1, 2, stdin) != 2)
  {
    LOG_ERROR("Unable to read magic number.");
    go_through();
  }

  ungetc(magic[1], stdin);
  ungetc(magic[0], stdin);

  if (magic[0] == 255 && magic[1] == 216)
  {
    LOG_INFO("Recompressing JPEG...");
    if (opts.mode == SMALLEST)
      recompress_mozjpeg(&opts);
    else
      recompress_turbojpeg(&opts);
  }
  else if (magic[0] == 137 && magic[1] == 'P') {
    LOG_INFO("Recompressing PNG...");
    recompress_png(&opts);
  }
  else
  {
    LOG_ERROR("Unsupported file format. Input not modified.");
    go_through();
  }

  return 0;
}
