#ifndef __COMMAND_H
#define __COMMAND_H

#include "default.h"

C_API_BEGIN

#define CONSOLE_BUFFER_LEN  512
#define ARGVS_MAX   16

struct cmd_table
{
    const char *name;
    int (*cmd)(struct cmd_table *,int argc,char *const argv[]);
};
typedef struct cmd_table cmd_tbl_t;

void main_loop();

C_API_END


#endif // __COMMAND_H
