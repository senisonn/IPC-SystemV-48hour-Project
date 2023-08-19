#ifndef WAITER_H
# define WAITER_H

# include "common.h"

t_ipc   *g_waiters_ipc;
int     g_running;

void    clean_exit(int status);
void    handle_sigint(int s);
int     waiter_queue();
void    signal_handler(int signal, void (*f)(int));
int     waiter_key();
void    handle_sigint(int s);


#endif