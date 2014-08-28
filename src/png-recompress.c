#include "png-recompress.h"

#include "pngquant/pngquant.h"

void recompress_png(s_compress_options *compress_opts)
{
  struct pngquant_options opts = {
    .fast_compression = (compress_opts->mode == FASTEST),
    .floyd = 1.f,
    .force = 0,
    .ie_mode = 0,
    .liq = liq_attr_create(),
    .min_quality_limit = 0,
    .skip_if_larger = 1,
    .using_stdin = 1,
    .verbose = 0,
  };

  pngquant_file("stdin", NULL, &opts);
}
