#include "common.h"

/* Signals */
void    signal_handler(int signal, void (*f) (int))
{
    struct sigaction action;

    action.sa_handler = f;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(signal, &action, NULL);
}

/* Init */
int     get_key(t_ipc *ipc, char key_code)
{
    int         key;
    struct stat st;

    if ((stat(KEY_FILE, &st) == -1) && (open(KEY_FILE, O_RDONLY | O_CREAT | O_EXCL, 0660) == -1))
    {
        perror("stat/open main_key file failed");
        return 0;
    }
    
    key = ftok(KEY_FILE, key_code);
    if (key == -1)
    {
        perror("ftok main_key failed");
	    return 0;
    }

    return key;
}

int     init_ipc_shm(t_ipc *ipc, int nb_specialties, int nb_utensils) 
{
    /*Creating a SMP whitch contains an array of 1 dimension  */
    ipc->shm = shmget(ipc->main_key, nb_specialties * nb_utensils * sizeof(int), IPC_CREAT | IPC_EXCL | 0660);
    if (ipc->shm == -1)
    {
        if (errno == EEXIST)
        {
            ipc->shm = shmget(ipc->main_key, nb_specialties * nb_utensils * sizeof(int), 0);
            if (ipc->shm == -1)
            {
                perror("shmget failed");
                return 0;
            }
        }
        else
        {
            perror("shmget failed");
            return 0;
        }
    }
    /*linking the SMP to a adress*/
    ipc->shm_menu = shmat(ipc->shm, NULL, 0);
    if (ipc->shm_menu == (int *) -1)
    {
        perror("shmat failed");
        return 0;
    }

    return 1;
}
int     init_ipc_semaphores(t_ipc *ipc)
{
    ipc->semaphores = semget(ipc->custom_key, ipc->nb_semaphores, IPC_CREAT | IPC_EXCL | 0660);
    if (ipc->semaphores == -1)
    {
        if (errno == EEXIST)
        {
            // printf("init_ipc_semaphores - semaphores already exists\n");
            ipc->semaphores = semget(ipc->custom_key, ipc->nb_semaphores, 0);
            if (ipc->semaphores == -1)
            {
                perror("semget failed");
                return 0;
            }
        }
        else
        {
            perror("semget failed");
            return 0;
        }
    }
    /* else
    {
        printf("init_ipc_semaphores - created semaphores\n");
    } */

    return 1;
}
int     init_ipc_message_queue(t_ipc *ipc)
{
    /*Create a message queue*/
    ipc->msg_queue = msgget(ipc->main_key, 0777 | IPC_CREAT | IPC_EXCL);
    if (ipc->msg_queue == -1)
    {
        if (errno == EEXIST)
        {
            ipc->msg_queue = msgget(ipc->main_key, 0);
            if (ipc->msg_queue == -1)
            {
                perror("msgget failed");
                return 0;
            }
        }
        else
        {
            perror("msgget failed");
            return 0;
        }
    }
    
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGINT);
    sigprocmask(SIG_SETMASK,&mask,NULL);
    return 1;
}
/* Destroy */
void    *destroy_ipc(t_ipc *ipc)
{
    /*destroying ipc struct and ipcs*/
    if (ipc != NULL)
    {
        if (ipc->semaphores != 0)
            semctl(ipc->semaphores, 0, IPC_RMID);
        if (ipc->shm != 0)
            shmctl(ipc->shm, IPC_RMID, NULL);
    }
    free(ipc);
    
    // printf("destroy_ipc - ipc destroyed\n");
    
    return NULL;
}

/* Semaphores */
void    increment_semaphore(t_ipc *ipc, int idx, int value)
{
    // TODO Replace 0 by SEM_UNDO
    struct sembuf V = { idx, value, 0 };
    semop(ipc->semaphores, &V, 1);
}
void    decrement_semaphore(t_ipc *ipc, int idx, int value)
{
    // TODO Replace 0 by SEM_UNDO
    struct sembuf P = { idx, -value, 0 };
    semop(ipc->semaphores, &P, 1);
}
int    get_semaphore_value(t_ipc *ipc, int idx)
{
    return semctl(ipc->semaphores, idx, GETVAL);
}

t_ipc   *get_ipc()
{
    t_ipc       *ipc;
    
    ipc = malloc(1 * sizeof(t_ipc));
    if (ipc == NULL)
    {
        perror("malloc ipc failed");
        return NULL;
    }
    bzero(ipc, sizeof(t_ipc));

    ipc->main_key = get_key(ipc, KEY_CODE_MAIN);
    if (!ipc->main_key)
        return destroy_ipc(ipc);

    if (!init_ipc_message_queue(ipc))
        return destroy_ipc(ipc);

    return ipc;
}
t_ipc   *get_waiters_ipc()
{
    t_ipc   *ipc;

    ipc = get_ipc();
    if (ipc == NULL)
        return NULL;

    ipc->custom_key = get_key(ipc, KEY_CODE_WAITER);
    if (!ipc->custom_key)
        return destroy_ipc(ipc);
    
    /* USE CUSTOM KEY */
    ipc->nb_semaphores = 1;
    if (!init_ipc_semaphores(ipc))
        return destroy_ipc(ipc);

    return ipc;
}
t_ipc   *get_cookers_ipc(int nb_specialties, int nb_utensils)
{
    t_ipc   *ipc;

    ipc = get_ipc();
    if (ipc == NULL)
        return NULL;

    ipc->custom_key = get_key(ipc, KEY_CODE_COOKER);
    if (!ipc->custom_key)
        return destroy_ipc(ipc);
    
    // printf("get_cookers_ipc - main_key: %d - custom_key: %d\n", ipc->main_key, ipc->custom_key);

    /* USE MAIN KEY */
    if (!init_ipc_shm(ipc, nb_specialties, nb_utensils))
        return destroy_ipc(ipc);

    /* USE CUSTOM KEY */
    ipc->nb_semaphores = nb_utensils;
    if (!init_ipc_semaphores(ipc))
        return destroy_ipc(ipc);
    
    return ipc;
}
t_ipc   *get_clients_ipc(int nb_waiters)
{
    t_ipc   *ipc;

    ipc = get_ipc();
    if (ipc == NULL)
        return NULL;
    
    ipc->custom_key = get_key(ipc, KEY_CODE_CLIENT);
    if (!ipc->custom_key)
        return destroy_ipc(ipc);
    
    /* USE CUSTOM KEY */
    ipc->nb_semaphores = nb_waiters * 2;
    if (!init_ipc_semaphores(ipc))
        return destroy_ipc(ipc);
    
    return ipc;
}
