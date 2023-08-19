#include "initial.h"

/* DEBUG */
void    ft_putchar(char c)
{
	write(1, &c, 1);
}
void    ft_putstr(char *s)
{
	size_t arg_idx;

	if (!s)
		return ;
	arg_idx = 0;
	while (s[arg_idx])
	{
		ft_putchar(s[arg_idx]);
		arg_idx++;
	}
}
void	ft_putnbr(int n)
{
	long nb;

	nb = (long) n;
	if (n < 0)
	{
		nb *= -1;
		ft_putchar('-');
	}
	if (nb / 10 > 0)
		ft_putnbr(nb / 10);
	ft_putchar(nb % 10 + 48);
}
void	print_bytes_in_binary(char *to_print, int32_t size, int human_print)
{
	char	current_bit;

	if (!human_print)
	{
		for (int32_t index = 0; index < size; index++)
		{
			for (int8_t bit_shifting = 7; bit_shifting >= 0; bit_shifting--)
			{
				current_bit = (to_print[index] >> bit_shifting) & 0x01;
				printf("%u", current_bit);
			}
			if (index + 1 < size)
				printf(" ");
		}
	}
	else
	{
		for (int32_t index = size - 1; index >= 0; index--)
		{
			for (int8_t bit_shifting = 7; bit_shifting >= 0; bit_shifting--)
			{
				current_bit = (to_print[index] >> bit_shifting) & 0x01;
				printf("%u", current_bit);
			}
			if (index - 1 >= 0)
				printf(" ");
		}
	}
	printf("\n");
}
void    debug()
{
    ft_putstr("g_fastfood:\n{\n");
    ft_putstr("  argc: "); ft_putnbr(g_fastfood->argc); ft_putchar('\n');
    ft_putstr("  argv: ["); ft_putchar('\n');
    for (int arg_idx = 0; arg_idx < g_fastfood->argc; arg_idx++)
    {
        ft_putstr("    ");
        ft_putnbr(arg_idx);
        ft_putstr(": ");
        ft_putstr(g_fastfood->argv[arg_idx]);
        ft_putstr(",\n");
    }
    ft_putstr("  ]\n");
    ft_putstr("  nb_waiters: "); ft_putnbr(g_fastfood->nb_waiters); ft_putchar('\n');
    ft_putstr("  nb_cookers: "); ft_putnbr(g_fastfood->nb_cookers); ft_putchar('\n');
    ft_putstr("  nb_terminals: "); ft_putnbr(g_fastfood->nb_terminals); ft_putchar('\n');
    ft_putstr("  nb_specialties: "); ft_putnbr(g_fastfood->nb_specialties); ft_putchar('\n');
    ft_putstr("  nb_utensils: "); ft_putnbr(g_fastfood->nb_utensils); ft_putchar('\n');
    ft_putstr("  utensils: ["); ft_putchar('\n');
    for (int utensil_idx = 0; utensil_idx < g_fastfood->nb_utensils; utensil_idx++)
    {
        ft_putstr("    ");
        ft_putnbr(utensil_idx);
        ft_putstr(": ");
        ft_putnbr(g_fastfood->utensils[utensil_idx]);
        ft_putstr(",\n");
    }
    ft_putstr("  specialties: ["); ft_putchar('\n');
    for (int specialty_idx = 0; specialty_idx < g_fastfood->nb_specialties; specialty_idx++)
    {
        ft_putstr("    ");
        ft_putnbr(specialty_idx);
        ft_putstr(": ");
        ft_putnbr(g_fastfood->cookers_ipc->shm_menu[specialty_idx]);
        ft_putstr(",\n");
    }
    ft_putstr("  ]\n");
    ft_putstr("}\n");
}
/* DEBUG */

void print_usage()
{
    fprintf(stderr, "Program must contains at least 5 arguments\n");
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\t./fastfood <nb_waiters> <nb_cookers> <nb_terminals> <nb_specialties> <nb_1 ... nb_k>\n");
}
void clean_exit(int status)
{
    if (DEBUG)
    {
        debug();
        ft_putstr("Exit status: "); ft_putnbr(status); ft_putchar('\n');
    }

    if (g_fastfood != NULL)
    {
        /* Pids arrays */
        if (g_fastfood->clients_pids != NULL)
            for (int client_idx = 0; client_idx < MAX_CLIENTS; client_idx++)
                if (g_fastfood->clients_pids[client_idx] != -1)
                    kill(g_fastfood->clients_pids[client_idx], SIGUSR1);
        if (g_fastfood->cookers_pids != NULL)
            for (int cooker_idx = 0; cooker_idx < g_fastfood->nb_cookers; cooker_idx++)
                if (g_fastfood->cookers_pids[cooker_idx] != -1)
                    kill(g_fastfood->cookers_pids[cooker_idx], SIGUSR1);
        if (g_fastfood->waiters_pids != NULL)
            for (int waiter_idx = 0; waiter_idx < g_fastfood->nb_waiters; waiter_idx++)
                if (g_fastfood->waiters_pids[waiter_idx] != -1)
                    kill(g_fastfood->waiters_pids[waiter_idx], SIGUSR1);
        free(g_fastfood->clients_pids);
        free(g_fastfood->cookers_pids);
        free(g_fastfood->waiters_pids);

        /* IPCs */
        destroy_ipc(g_fastfood->clients_ipc);
        destroy_ipc(g_fastfood->cookers_ipc);
        destroy_ipc(g_fastfood->waiters_ipc);

        /* Delete utensils */
        free(g_fastfood->utensils);
    }
    free(g_fastfood);
    
    exit(status);
}

/* Signals */
void handle_sigint(int s)
{
    clean_exit(EXIT_SUCCESS);
}

/* Utils */
int     get_utensil_idx_in_shm_menu(int specialty_idx, int utensil_idx)
{
    return specialty_idx * g_fastfood->nb_utensils + utensil_idx;
}

/* Init */
int init_parameters()
{
    g_fastfood->nb_waiters = atoi(g_fastfood->argv[1]);
    g_fastfood->nb_cookers = atoi(g_fastfood->argv[2]);
    g_fastfood->nb_terminals = atoi(g_fastfood->argv[3]);
    g_fastfood->nb_specialties = atoi(g_fastfood->argv[4]);

    if (g_fastfood->nb_waiters <= 0
        || g_fastfood->nb_cookers <= 0
        || g_fastfood->nb_terminals <= 0
        || g_fastfood->nb_specialties <= 0)
    {
        fprintf(stderr, "Error: one or more arguments are negative or overlap\n");
        return 0;
    }

    if (g_fastfood->nb_waiters < MIN_EMPLOYEES
        || g_fastfood->nb_waiters > MAX_EMPLOYEES
        || g_fastfood->nb_cookers < MIN_EMPLOYEES
        || g_fastfood->nb_cookers > MAX_EMPLOYEES)
    {
        fprintf(stderr, "Error: <nb_waiters> or <nb_cookers> value belongs to [2, 100]\n");
        return 0;
    }
    
    if (g_fastfood->nb_waiters <= g_fastfood->nb_terminals)
    {
        fprintf(stderr, "Error: the number of terminals must be smaller than the number of waiters\n");
        return 0;
    }

    return 1;
}
int init_utensils()
{
    g_fastfood->nb_utensils = g_fastfood->argc - 5;

    if (g_fastfood->nb_utensils > 100)
    {
        fprintf(stderr, "Error: nomber utensils: cannot have more than an hundred of utensils\n");
        return 0;
    }
    
    g_fastfood->utensils = malloc(g_fastfood->nb_utensils * sizeof(int));
    if (g_fastfood->utensils == NULL)
    {
        fprintf(stderr, "Error: insufficient memory: cannot malloc utensils array\n");
        return 0;
    }

    for (int arg_idx = 5; arg_idx < g_fastfood->argc; arg_idx++)
    {
        g_fastfood->utensils[arg_idx - 5] = atoi(g_fastfood->argv[arg_idx]);
        if (g_fastfood->utensils[arg_idx - 5] < 0)
        {
            fprintf(stderr, "Error: utensil argument (idx: %d) is negative\n", arg_idx);
            return 0;
        }
    }
    return 1;
}
int init_ipcs()
{
    /* Waiters ipc */
    g_fastfood->waiters_ipc = get_waiters_ipc();
    if (g_fastfood->waiters_ipc == NULL)
        return 0;
    if (semctl(g_fastfood->waiters_ipc->semaphores, 0, SETVAL, g_fastfood->nb_terminals) == -1)
    {
        fprintf(stderr, "Error: failed to set nb_terminals in waiters semaphore\n");
        return 0;
    }
    
    /* Cookers ipc */
    g_fastfood->cookers_ipc = get_cookers_ipc(g_fastfood->nb_specialties, g_fastfood->nb_utensils);
    if (g_fastfood->cookers_ipc == NULL)
        return 0;

    /* Clients ipc */
    g_fastfood->clients_ipc = get_clients_ipc(g_fastfood->nb_waiters);
    if (g_fastfood->clients_ipc == NULL)
        return 0;

    return 1;
}
int init_shm_menu()
{
    int check_sum;

    for (int specialty_idx = 0; specialty_idx < g_fastfood->nb_specialties; specialty_idx++)
    {
        check_sum = 0;
        for (int utensil_idx = 0; utensil_idx < g_fastfood->nb_utensils; utensil_idx++)
        {
            int value = (int) (rand() % g_fastfood->utensils[utensil_idx]);
            g_fastfood->cookers_ipc->shm_menu[get_utensil_idx_in_shm_menu(specialty_idx, utensil_idx)] = value;
            check_sum += value;
        }
        if (check_sum == 0)
        {
            fprintf(stderr, "Error: a specialty needs to use at least 1 utensil (idx: %d)\n", specialty_idx);
            return 0;
        }
    }
    
    return 1;
}
int init_terminals_semaphore()
{
    union semun { int val; } sem_val;
    sem_val.val = g_fastfood->nb_terminals;

    if (semctl(g_fastfood->waiters_ipc->semaphores, 0, SETVAL, sem_val) == -1)
    {
        fprintf(stderr, "Error: terminals semaphore cannot be initialized\n");
        return 0;
    }
    
    // printf("initial - sem_waiters[0]: %d\n", get_semaphore_value(g_fastfood->waiters_ipc, 0));
    
    return 1;
}
int init_utensils_semaphores()
{
    for (int sem_idx = 0; sem_idx < g_fastfood->nb_utensils; sem_idx++)
    {
        union semun { int val; } sem_val;
        sem_val.val = g_fastfood->utensils[sem_idx];

        if (semctl(g_fastfood->cookers_ipc->semaphores, sem_idx, SETVAL, sem_val) == -1)
        {
            fprintf(stderr, "Error: utensils semaphores cannot be initialized\n");
            return 0;
        }
        // printf("initial - sem_cookers[%d]: %d\n", sem_idx, get_semaphore_value(g_fastfood->cookers_ipc, sem_idx));
    }

    return 1;
}
int init_waiters_semaphores()
{
    /*semaphore size is doubled first part of the semaphore countains just the state of waiter 1(not busy) and the second part is to check how many client are waiting for a waiter*/
    for (int sem_idx = 0; sem_idx < g_fastfood->nb_waiters * 2; sem_idx++)
    {
        union semun { int val; } sem_val;
        /*filling the semaphore*/
        if (sem_idx < g_fastfood->nb_waiters)
            sem_val.val = 1;
        else
            sem_val.val = 0;
        /*checking if the semaphore[index] value is set*/
        if (semctl(g_fastfood->clients_ipc->semaphores, sem_idx, SETVAL, sem_val) == -1)
        {
            fprintf(stderr, "Error: waiters semaphores cannot be initialized\n");
            return 0;
        }
        // printf("initial - sem_clients[%d]: %d\n", sem_idx, get_semaphore_value(g_fastfood->clients_ipc, sem_idx));
    }
    return 1;
}
int init_pids_arrays()
{
    g_fastfood->waiters_pids = (int *) malloc(g_fastfood->nb_waiters * sizeof(int));
    if (g_fastfood->waiters_pids == NULL)
    {
        fprintf(stderr, "Error: insufficient memory: cannot malloc waiters_pids array\n");
        return 0;
    }
    for (int pid_idx = 0; pid_idx < g_fastfood->nb_waiters; pid_idx++)
        g_fastfood->waiters_pids[pid_idx] = -1;

    g_fastfood->cookers_pids = (int *) malloc(g_fastfood->nb_cookers * sizeof(int));
    if (g_fastfood->cookers_pids == NULL)
    {
        fprintf(stderr, "Error: insufficient memory: cannot malloc waiters_pids array\n");
        return 0;
    }
    for (int pid_idx = 0; pid_idx < g_fastfood->nb_cookers; pid_idx++)
        g_fastfood->cookers_pids[pid_idx] = -1;
    
    g_fastfood->clients_pids = (int *) malloc(MAX_CLIENTS * sizeof(int));
    if (g_fastfood->clients_pids == NULL)
    {
        fprintf(stderr, "Error: insufficient memory: cannot malloc clients_pids array\n");
        return 0;
    }
    for (int pid_idx = 0; pid_idx < MAX_CLIENTS; pid_idx++)
        g_fastfood->clients_pids[pid_idx] = -1;

    return 1;
}
int init_cookers_processes()
{
    int     pid;
    char    *cooker_argv[5] = { "cooker", NULL, g_fastfood->argv[4], NULL, NULL};

    char str_nb_utensils[2] = { g_fastfood->nb_utensils, 0 };
    cooker_argv[3] = str_nb_utensils;
    
    for (int cooker_idx = 0; cooker_idx < g_fastfood->nb_cookers; cooker_idx++)
    {
        pid = fork();
        
        char str_cooker_idx[2] = { cooker_idx, 0 };
        cooker_argv[1] = str_cooker_idx;

        if (pid == -1)
        {
            fprintf(stderr, "Error: fork cooker error\n");
            return 0;
        }
        if (pid == 0)
        {
            g_fastfood->cookers_pids[cooker_idx] = getpid();
            execve("cooker", cooker_argv, NULL);
            exit(-1);
        }
    }
    return 1;
} 
int init_waiters_processes()
{
    int     pid;
    char    *waiter_argv[3] = { "waiter", NULL, NULL};

    for (int waiter_idx = 0; waiter_idx < g_fastfood->nb_waiters; waiter_idx++)
    {
        pid = fork();
        
        char str_waiter_idx[2] = { waiter_idx, 0 };
        waiter_argv[1] = str_waiter_idx;

        if (pid == -1)
        {
            fprintf(stderr, "Error: fork waiter error\n");
            return 0;
        }
        if (pid == 0)
        {
            g_fastfood->waiters_pids[waiter_idx] = getpid();
            execve("waiter", waiter_argv, NULL);
            exit(-1);
        }
    }
    return 1;
}
int init_clients_processes()
{
    int     pid;
    char    *client_argv[4] = { "client", g_fastfood->argv[4], NULL, NULL};

    g_fastfood->nb_clients = 0;
    while (1)
    {
        if (g_fastfood->nb_clients < MAX_CLIENTS)
        {
            pid = fork();
        
            char str_nb_waiters[2] = { g_fastfood->nb_waiters, 0 };
            client_argv[2] = str_nb_waiters;

            if (pid == -1)
            {
                fprintf(stderr, "Error: fork client error\n");
                return 0;
            }
            g_fastfood->nb_clients++;
            if (pid == 0)
            {
                for (int client_idx = 0; client_idx < MAX_CLIENTS; client_idx++)
                {
                    if (g_fastfood->clients_pids[client_idx] == -1)
                    {
                        g_fastfood->clients_pids[client_idx] = getpid();
                        break;
                    }
                }
                execve("client", client_argv, NULL);
                exit(-1);
            }
        }

        for (int client_idx = 0; client_idx < MAX_CLIENTS; client_idx++)
        {
            int client_pid = g_fastfood->clients_pids[client_idx];
            if (client_pid != -1)
            {
                /* Non-blocking WAITPID */
                int result = waitpid(client_pid, NULL, WNOHANG);
                if (result != 0)
                    g_fastfood->clients_pids[client_idx] = -1;
            }
        }
    }

    return 1;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    g_fastfood = NULL;
    
    /* Check number of arguments */
    if (argc < 6) 
    {
        print_usage();
        clean_exit(EXIT_FAILURE);
    }
    
    /* Init fastfood main structure */
    g_fastfood = malloc(1 * sizeof(t_fastfood));
    if (g_fastfood == NULL)
    {
        fprintf(stderr, "Error: insufficient memory: cannot malloc fastfood structure\n");
        clean_exit(EXIT_FAILURE);
    }
    g_fastfood->argc = argc;
    g_fastfood->argv = argv;
    g_fastfood->utensils = NULL;
    g_fastfood->waiters_ipc = NULL;
    g_fastfood->cookers_ipc = NULL;
    g_fastfood->clients_ipc = NULL;
    g_fastfood->waiters_pids = NULL;
    g_fastfood->cookers_pids = NULL;
    g_fastfood->clients_pids = NULL;

    signal_handler(SIGINT, handle_sigint);

    if (!init_parameters())
        clean_exit(EXIT_FAILURE);
    if (!init_utensils())
        clean_exit(EXIT_FAILURE);
    
    if (!init_ipcs())
        clean_exit(EXIT_FAILURE);
    if (!init_shm_menu())
        clean_exit(EXIT_FAILURE); 
    if (!init_terminals_semaphore())
        clean_exit(EXIT_FAILURE);
    if (!init_utensils_semaphores())
        clean_exit(EXIT_FAILURE);
    if (!init_waiters_semaphores())
        clean_exit(EXIT_FAILURE);
    
    if (!init_pids_arrays())
        clean_exit(EXIT_FAILURE);
    
    if (!init_cookers_processes())
        clean_exit(EXIT_FAILURE);
    if (!init_waiters_processes())
        clean_exit(EXIT_FAILURE);
    if (!init_clients_processes())
        clean_exit(EXIT_FAILURE);
    
    clean_exit(EXIT_SUCCESS);
}
