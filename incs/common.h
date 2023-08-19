#ifndef COMMON_H
# define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <strings.h>
#include <time.h>
#include <signal.h>

/* Defines */
# define KEY_FILE           "ipc_key_file"

# define KEY_CODE_MAIN      'm'
# define KEY_CODE_COOKER    'c'
# define KEY_CODE_WAITER    'w'
# define KEY_CODE_CLIENT    'k'

#define couleur(param) fprintf(stdout,"\033[%sm",param)

#define NOIR  "30"
#define ROUGE "31"
#define VERT  "32"
#define JAUNE "33"
#define BLEU  "34"
#define CYAN  "36"
#define BLANC "37"
#define REINIT "0"

#define WAITER_TO_COOKER_FILTER    256

/* WAITERS <=> COOKERS */
typedef struct  s_waiter_to_cooker_packet
{
    long    filter;
    pid_t   waiter_id;
    int     specialty_idx;
}   t_waiter_to_cooker_packet;
typedef struct  s_cooker_to_waiter_packet
{
    long    waiter_id;
}   t_cooker_to_waiter_packet;

/* CLIENTS <=> WAITERS */
typedef struct s_client_order_to_waiter_packet
{
    long    waiter_id;
    pid_t   client_pid;
    int     specialty_idx;
}   t_client_order_to_waiter_packet;
typedef struct s_waiter_check_to_client_packet
{
    long    client_pid;
}   t_waiter_check_to_client_packet;
typedef struct s_waiter_receive_payment_from_client
{
    long    waiter_id;
}   t_waiter_receive_payment_from_client;
typedef struct s_waiter_to_client_packet
{
    long    client_pid;
}   t_waiter_to_client_packet;
/* IPC */
typedef struct s_ipc
{
    key_t       main_key;
    key_t       custom_key;

    int         shm;
    /*smp for the cookers*/
    int         *shm_menu;
    
    int         nb_semaphores;
    int         semaphores;
    
    int         msg_queue;
}   t_ipc;

/* Prototypes */
/* Signals */
void    signal_handler(int signal, void (*f) (int));

void    *destroy_ipc(t_ipc *ipc);

/* Semaphores */
void    increment_semaphore(t_ipc *ipc, int idx, int value);
void    decrement_semaphore(t_ipc *ipc, int idx, int value);
int     get_semaphore_value(t_ipc *ipc, int idx);

/* IPCs */
t_ipc   *get_waiters_ipc();
t_ipc   *get_cookers_ipc(int nb_specialties, int nb_utensils);
t_ipc   *get_clients_ipc(int nb_waiters);


#endif