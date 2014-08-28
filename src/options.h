#ifndef OPTIONS_H_
# define OPTIONS_H_

typedef enum {
  DEFAULT,
  FASTEST,
  SMALLEST
} e_mode;

typedef struct {
  e_mode  mode;
} s_compress_options;

#endif // OPTIONS_H_
