#ifndef COOKER_H
# define COOKER_H

# include "common.h"

/* DEFINES */

/* GOBAL VARIABLE */
t_ipc   *g_cookers_ipc;
int     g_running;

/* PROTOTYPES */
int     main(int argc, char **argv);

#endif