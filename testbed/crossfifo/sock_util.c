#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>
#include "log.h"


int create_server_sock(const char * ip, unsigned short * port) {
    int fd;
    int yes;
    socklen_t sock_len;
    struct sockaddr_in bind_addr;

    assert(port);

    do {

        // create
        fd = socket(AF_INET, SOCK_STREAM, 0);
        yes = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, 
                sizeof(yes)) < 0) {
            cff_debug_err(("setsockopt failed"));
            break;
        }

        // fill addr
        bind_addr.sin_family = AF_INET;
        bind_addr.sin_port = htons(*port);
        if (!ip || !ip[0])
            bind_addr.sin_addr.s_addr = INADDR_ANY;
        else {
            if (!inet_aton(ip, &(bind_addr.sin_addr))) {
                cff_debug(("invalid ip"));
                break;
            }
        }
        bzero(&(bind_addr.sin_zero), sizeof(bind_addr.sin_zero));

        // bind
        if (bind(fd, (struct sockaddr *)&bind_addr, 
                sizeof(struct sockaddr)) < 0) {
            cff_debug_err(("bind failed"));
            break;
        }

        // get real bound addr
        sock_len = sizeof(struct sockaddr_in);
        if (getsockname(fd, (struct sockaddr *)&bind_addr,
               &sock_len) < 0) {
            cff_debug_err(("getsockname failed"));
            break;
        }
        *port = ntohs(bind_addr.sin_port);

        return fd;

    } while (0);

    close(fd);
    return -1;
}
