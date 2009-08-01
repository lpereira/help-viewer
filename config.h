#ifndef __CONFIG_H__
#define __CONFIG_H__

#define DEBUG(...)
#define XDEBUG(msg,...) fprintf(stderr, "*** %s:%d (%s) *** " msg "\n", \
                               __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#endif	/* __CONFIG_H__ */
