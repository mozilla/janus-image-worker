#include "mozjpeg-recompress.h"

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include "mozjpeg/jpeglib.h"

#include "utils.h"
#include "jpeg-utils.h"

struct jpeg_fcts {
  struct jpeg_error_mgr *(*jpeg_std_error)();
  void        (*jpeg_CreateCompress)();
  void        (*jpeg_CreateDecompress)();
  void        (*jpeg_read_header)();
  void        (*jpeg_stdio_dest)();
  void        (*jpeg_stdio_src)();
  void        (*jpeg_set_defaults)();
  void        (*jpeg_set_quality)();
  void        (*jpeg_start_compress)();
  void        (*jpeg_start_decompress)();
  JDIMENSION  (*jpeg_write_scanlines)();
  JDIMENSION  (*jpeg_read_scanlines)();
  void        (*jpeg_finish_compress)();
  void        (*jpeg_finish_decompress)();
};

struct jpeg_fcts mozjpeg_fcts;

static void load_mozjpeg_fcts(void)
{
  void *handle;

#ifdef __APPLE__
  handle = dlopen("libmozjpeg.dylib", RTLD_LAZY);
#else
  handle = dlopen("libmozjpeg.so", RTLD_LAZY);
#endif

  if (!handle) {
    LOG_ERROR("Unable to load mozjpeg library.");
    go_through();
  }

  mozjpeg_fcts.jpeg_std_error = (struct jpeg_error_mgr *(*)())dlsym(handle, "jpeg_std_error");
  mozjpeg_fcts.jpeg_CreateCompress = (void (*)())dlsym(handle, "jpeg_CreateCompress");
  mozjpeg_fcts.jpeg_CreateDecompress = (void (*)())dlsym(handle, "jpeg_CreateDecompress");
  mozjpeg_fcts.jpeg_set_defaults = (void (*)())dlsym(handle, "jpeg_set_defaults");
  mozjpeg_fcts.jpeg_read_header = (void (*)())dlsym(handle, "jpeg_read_header");
  mozjpeg_fcts.jpeg_stdio_dest = (void (*)())dlsym(handle, "jpeg_stdio_dest");
  mozjpeg_fcts.jpeg_stdio_src = (void (*)())dlsym(handle, "jpeg_stdio_src");
  mozjpeg_fcts.jpeg_set_quality = (void (*)())dlsym(handle, "jpeg_set_quality");
  mozjpeg_fcts.jpeg_start_compress = (void (*)())dlsym(handle, "jpeg_start_compress");
  mozjpeg_fcts.jpeg_start_decompress = (void (*)())dlsym(handle, "jpeg_start_decompress");
  mozjpeg_fcts.jpeg_read_scanlines = (JDIMENSION (*)())dlsym(handle, "jpeg_read_scanlines");
  mozjpeg_fcts.jpeg_write_scanlines = (JDIMENSION (*)())dlsym(handle, "jpeg_write_scanlines");
  mozjpeg_fcts.jpeg_finish_compress = (void (*)())dlsym(handle, "jpeg_finish_compress");
  mozjpeg_fcts.jpeg_finish_decompress = (void (*)())dlsym(handle, "jpeg_finish_decompress");
}

void recompress_mozjpeg(s_compress_options *compress_opts)
{
  struct jpeg_decompress_struct cinfo_in;
  struct jpeg_compress_struct   cinfo_out;
  struct jpeg_error_mgr         jerr;
  JSAMPARRAY                    buffer;
  int                           quality;

  load_mozjpeg_fcts();

  // default jpeg error handler, exit on error.
  cinfo_in.err = mozjpeg_fcts.jpeg_std_error(&jerr);
  cinfo_out.err = mozjpeg_fcts.jpeg_std_error(&jerr);

  mozjpeg_fcts.jpeg_CreateCompress(&cinfo_out, JPEG_LIB_VERSION, sizeof(struct jpeg_compress_struct));
  mozjpeg_fcts.jpeg_stdio_dest(&cinfo_out, stdout);

  mozjpeg_fcts.jpeg_CreateDecompress(&cinfo_in, JPEG_LIB_VERSION, sizeof(struct jpeg_decompress_struct));
  mozjpeg_fcts.jpeg_stdio_src(&cinfo_in, stdin);

  mozjpeg_fcts.jpeg_read_header(&cinfo_in, 1);

  quality = JPEGGetImageQuality(&cinfo_in);
  LOG_DEBUG("Source quality: %d\n", quality);

  if (!should_recompress(compress_opts, quality)) {
    LOG_INFO("Image will not be recompressed.");
    fseek(stdin, 0, SEEK_SET);
    go_through();
  }

  mozjpeg_fcts.jpeg_start_decompress(&cinfo_in);

  cinfo_out.image_width = cinfo_in.output_width;
  cinfo_out.image_height = cinfo_in.output_height;
  cinfo_out.input_components = 3;
  cinfo_out.in_color_space = cinfo_in.out_color_space;
  cinfo_out.scan_info = NULL;
  cinfo_out.num_scans = 0;

  cinfo_out.use_moz_defaults = 1;
  cinfo_out.optimize_scans = 0;

  int new_quality = compute_new_quality(compress_opts, quality);

  mozjpeg_fcts.jpeg_set_defaults(&cinfo_out);
  mozjpeg_fcts.jpeg_set_quality(&cinfo_out, new_quality, TRUE);
  mozjpeg_fcts.jpeg_start_compress(&cinfo_out, 1);

  int row_stride = cinfo_in.output_width * cinfo_in.output_components;
  buffer = (*cinfo_in.mem->alloc_sarray)
    ((j_common_ptr) &cinfo_in, JPOOL_IMAGE, row_stride, 1);

  if (!buffer) {
    LOG_ERROR("Unable to allocate line buffer.\n");
    exit(2);
  }

  while (cinfo_out.next_scanline < cinfo_out.image_height) {
    (void) mozjpeg_fcts.jpeg_read_scanlines(&cinfo_in, buffer, 1);
    (void) mozjpeg_fcts.jpeg_write_scanlines(&cinfo_out, buffer, 1);
  }

  mozjpeg_fcts.jpeg_finish_decompress(&cinfo_in);
  mozjpeg_fcts.jpeg_finish_compress(&cinfo_out);
}
