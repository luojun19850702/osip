#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "stdtypes.h"
#include <ifaddrs.h>
#include <semaphore.h>
#include "cdua1int.h"
#include "cdua1ugt.h"
#include "unitdefinition.h"
#include <rtp.h>
#include <libdua.h>
#include <libduasync.h>
#include <voice.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <linux/prctl.h>

#include <semaphore.h>
#include "media_extensions.h"
#include "app_alsa_utility.h"
#include "libdtls.h"

#define PIPE_READ	0
#define PIPE_WRITE	1

#endif 

