#include "message.h"
#include "logging.h"
#include "utils.h"

#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

int ctl_pipe[2];

int send_ctl_message(int type, void *data, size_t len)
{
    uint64_t msg = TO64(type, len);

    if (write(ctl_pipe[1], &msg, sizeof(msg)) < 0)
    {
        printlog(ERROR, "write() failed: %s", strerror(errno));
        return -1;
    }

    if (write(ctl_pipe[1], data, len) < 0)
    {
        printlog(ERROR, "write() failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}

int close_ctl_pipe()
{
    close(ctl_pipe[0]);
    close(ctl_pipe[1]);

    return 0;
}