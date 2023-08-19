#ifndef INITIAL_H
#define INITIAL_H

# include "common.h"

/* DEFINES */
#define DEBUG           0

#define MIN_EMPLOYEES   1
#define MAX_EMPLOYEES   100
#define MAX_CLIENTS     10

typedef struct  s_fastfood {
    /* Arguments */
    int     argc;
    char    **argv;

    /* Parameters */
    int     nb_waiters;
    int     nb_cookers;
    int     nb_terminals;
    int     nb_specialties;
    
    /* Utensils */
    int     nb_utensils;
    int     *utensils;

    /* IPC */
    /*waiters will use a message queue and get a semaphore to get a termial*/
    t_ipc   *waiters_ipc;
    /*cookers will use a SMP to get the menu and a semaphore to access to utensils*/
    t_ipc   *cookers_ipc;
    /*clients will use a semaphore to call waiters*/
    t_ipc   *clients_ipc;

    /* Pids arrays */
    pid_t   *waiters_pids;
    pid_t   *cookers_pids;

    int     nb_clients;
    pid_t   *clients_pids;
}   t_fastfood;

/* GOBAL VARIABLE */
t_fastfood  *g_fastfood;

/* PROTOTYPES */

void    print_usage();
void    clean_exit(int status);
int     get_utensil_idx_in_shm_menu(int specialty_idx, int utensil_idx);
void    handle_sigint(int s);
void    signal_handler(int signal, void (*f)(int));

int     main(int argc, char **argv);
int     init_parameters();
int     init_utensils();
int     init_ipcs();
int     init_shm_menu();
int     init_utensils_semaphores();
int     init_pids_arrays();
int     init_cookers_processes();

#endif