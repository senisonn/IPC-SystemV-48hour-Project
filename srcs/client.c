#include "client.h"

void    clean_exit(int status)
{
    free(g_clients_ipc);
    raise(SIGINT);
    exit(status);
}
void	handle_sigusr1(int status)
{
    clean_exit(EXIT_SUCCESS);
}

void    print_semaphores()
{
    for (int sem_idx = 0; sem_idx < g_clients_ipc->nb_semaphores; sem_idx++)
    {
        printf("(client) - sem_clients[%d]: %d,\n", sem_idx, get_semaphore_value(g_clients_ipc, sem_idx));
    }
}

void    send_order_to_waiter(int waiter_id, int client_pid, int specialty_idx)
{
    t_client_order_to_waiter_packet     snd_packet;
    snd_packet.waiter_id = waiter_id + 1;
    snd_packet.client_pid = client_pid;
    snd_packet.specialty_idx = specialty_idx;

    couleur(BLEU);
    printf("(client) pid: %d | Send order to: %d for: %d\n", client_pid, waiter_id, specialty_idx);
    msgsnd(g_clients_ipc->msg_queue, &snd_packet, sizeof(t_client_order_to_waiter_packet), 0);
}
void    send_payment(int waiter_id, int client_pid)
{
    t_waiter_receive_payment_from_client    snd_packet;
    snd_packet.waiter_id = waiter_id + 1;

    couleur(BLEU);
    printf("(client) pid: %d | Send payment to: %d\n", client_pid, waiter_id);
    msgsnd(g_clients_ipc->msg_queue, &snd_packet, sizeof(t_waiter_receive_payment_from_client), 0);
}

int main(int argc, char *argv[])
{
    int     nb_specialties = atoi(argv[1]);
    int     nb_waiters = (int) argv[2][0];
    int     pid = getpid();

    signal_handler(SIGUSR1, handle_sigusr1);
	srand(time(NULL));
    g_clients_ipc = NULL;

    g_clients_ipc = get_clients_ipc(nb_waiters);
    if (g_clients_ipc == NULL)
    {
        perror("cannot get clients ipc");
        clean_exit(EXIT_FAILURE);
    }

    couleur(BLEU);
    printf("(client) pid: %d | New client\n", pid);

    int least_busy_waiter_idx = -1;
    int least_busy_waiter_value = 2147483647;
    for (int waiter_idx = 0; waiter_idx < nb_waiters; waiter_idx++)
    {
        int value = get_semaphore_value(g_clients_ipc, nb_waiters + waiter_idx);
        if (value < least_busy_waiter_value)
        {
            least_busy_waiter_idx = waiter_idx;
            least_busy_waiter_value = value;
        }

    }
    
    int specialty_to_order = rand() % nb_specialties;

    /* Take WAITER */
    increment_semaphore(g_clients_ipc, nb_waiters + least_busy_waiter_idx, 1);
    decrement_semaphore(g_clients_ipc, least_busy_waiter_idx, 1);
    
    /* Send order */
    send_order_to_waiter(least_busy_waiter_idx, pid, specialty_to_order);

    /* Receive specialty */
    t_waiter_to_client_packet   rcv_specialty_packet;
    msgrcv(g_clients_ipc->msg_queue, &rcv_specialty_packet, sizeof(t_waiter_to_client_packet), pid, 0);
    couleur(BLEU);
    printf("(client) pid: %d | Received my ordered specialty from waiter: %d (specialty: %d)\n", pid, least_busy_waiter_idx, specialty_to_order);

    /* Receive check */
    t_waiter_check_to_client_packet     rcv_check_packet;
    msgrcv(g_clients_ipc->msg_queue, &rcv_check_packet, sizeof(t_waiter_check_to_client_packet), pid, 0);
    couleur(BLEU);
    printf("(client) pid: %d | Received a check from waiter: %d\n", pid, least_busy_waiter_idx);

    /* Send payment */
    send_payment(least_busy_waiter_idx, pid);

    /* Release WAITER */
    decrement_semaphore(g_clients_ipc, nb_waiters + least_busy_waiter_idx, 1);
    increment_semaphore(g_clients_ipc, least_busy_waiter_idx, 1);

    couleur(BLEU);
    printf("(client) pid: %d | Exit (il s'est bien rassasié | PS : j'ai pas beaucoup dormi)\n", pid);

    clean_exit(EXIT_SUCCESS);
}