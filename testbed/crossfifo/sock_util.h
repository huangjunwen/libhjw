#ifndef _CFF_SOCK_UTIL_H_
#define _CFF_SOCK_UTIL_H_

#include <sys/types.h>

/* ip: NULL or "" for INADDR_ANY
 * port: in/out param, if *port == 0, then after this call it is
 *  filled with the actual bound port
 */
int create_server_sock(const char * ip, unsigned short * port);


#endif // _CFF_SOCK_UTIL_H_
