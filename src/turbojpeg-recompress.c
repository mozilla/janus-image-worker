#include "turbojpeg-recompress.h"

#include <stdlib.h>
#include <stdio.h>

#include <jpeglib.h>

#include "utils.h"
#include "jpeg-utils.h"

void recompress_turbojpeg(s_compress_options *compress_opts)
{
  struct jpeg_decompress_struct cinfo_in;
  struct jpeg_compress_struct   cinfo_out;
  struct jpeg_error_mgr         jerr;
  JSAMPARRAY                    buffer;
  int                           quality;

  // default jpeg error handler, exit on error.
  cinfo_in.err = jpeg_std_error(&jerr);
  cinfo_out.err = jpeg_std_error(&jerr);

  jpeg_create_compress(&cinfo_out);
  jpeg_stdio_dest(&cinfo_out, stdout);

  jpeg_create_decompress(&cinfo_in);
  jpeg_stdio_src(&cinfo_in, stdin);

  jpeg_read_header(&cinfo_in, 1);

  quality = JPEGGetImageQuality(&cinfo_in);
  LOG_DEBUG("Quality: %d\n", quality);

  if (!should_recompress(compress_opts, quality)) {
    LOG_INFO("Image will not be recompressed.");
    fseek(stdin, 0, SEEK_SET);
    go_through();
  }

  jpeg_start_decompress(&cinfo_in);

  cinfo_out.image_width = cinfo_in.output_width;
  cinfo_out.image_height = cinfo_in.output_height;
  cinfo_out.input_components = 3;
  cinfo_out.in_color_space = cinfo_in.out_color_space;
  cinfo_out.scan_info = NULL;
  cinfo_out.num_scans = 0;

  int new_quality = compute_new_quality(compress_opts, quality);

  jpeg_set_defaults(&cinfo_out);
  jpeg_set_quality(&cinfo_out, new_quality, TRUE);
  jpeg_start_compress(&cinfo_out, 1);

  int row_stride = cinfo_in.output_width * cinfo_in.output_components;
  buffer = (*cinfo_in.mem->alloc_sarray)
    ((j_common_ptr) &cinfo_in, JPOOL_IMAGE, row_stride, 1);

  if (!buffer) {
    LOG_ERROR("Unable to allocate line buffer.\n");
    exit(2);
  }

  while (cinfo_out.next_scanline < cinfo_out.image_height) {
    (void) jpeg_read_scanlines(&cinfo_in, buffer, 1);
    (void) jpeg_write_scanlines(&cinfo_out, buffer, 1);
  }

  jpeg_finish_decompress(&cinfo_in);
  jpeg_finish_compress(&cinfo_out);
}

