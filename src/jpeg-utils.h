#ifndef JPEG_UTILS_H_
# define JPEG_UTILS_H_

# include <stdio.h>
# include <jpeglib.h>

# include "options.h"

int should_recompress(s_compress_options *opts, int quality);
int compute_new_quality(s_compress_options *opts, int quality);

// Get the quality of the given JPEG from its quantization table.
// This function was taken directly from ImageMagick (JPEGSetImageQuality).
int JPEGGetImageQuality(struct jpeg_decompress_struct *jpeg_info);

#endif // !JPEG_UTILS_H_
