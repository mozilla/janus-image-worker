#ifndef UTILS_H_
# define UTILS_H_

# include <stdio.h>

# define BUF_SIZE 8192
# define STDIN_BUFFER_SIZE 65536

# define LOG_ERROR(MESSAGE, ...) \
  fprintf(stderr, "[ERROR] " MESSAGE "\n", ##__VA_ARGS__)
# define LOG_INFO(MESSAGE, ...) \
  fprintf(stderr, "[INFO] " MESSAGE "\n", ##__VA_ARGS__)

# ifdef DEBUG
#  define LOG_DEBUG(MESSAGE, ...) \
  fprintf(stderr, "[DEBUG] " MESSAGE "\n", ##__VA_ARGS__)
# else
#  define LOG_DEBUG(...) ;
# endif

// Write stdin on stdout without modification.
void go_through(void);

#endif // UTILS_H_
