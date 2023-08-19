#include "waiter.h"

void handle_sigusr1(int status)
{
    g_running = 0;
    raise(SIGINT);
}

void    send_check_to_client(int client_pid)
{
    t_waiter_check_to_client_packet snd_packet;
    snd_packet.client_pid = client_pid;
    
    msgsnd(g_waiters_ipc->msg_queue, &snd_packet, sizeof(t_waiter_check_to_client_packet), 0);
}
void    send_order_to_cookers(int id, int specialty_idx)
{
    t_waiter_to_cooker_packet    snd_packet;
    snd_packet.filter = WAITER_TO_COOKER_FILTER;
    snd_packet.waiter_id = id;
    snd_packet.specialty_idx = specialty_idx;

    msgsnd(g_waiters_ipc->msg_queue, &snd_packet, sizeof(t_waiter_to_cooker_packet), 0);
}
void    send_specialty_to_client(int client_pid)
{
    t_waiter_to_client_packet    snd_packet;
    snd_packet.client_pid = client_pid;

    msgsnd(g_waiters_ipc->msg_queue, &snd_packet, sizeof(t_waiter_to_client_packet), 0);
}

int main(int argc, char * argv[])
{
    int id = argv[1][0];
    int pid = getpid();

    g_running = 1;
    signal_handler(SIGUSR1, handle_sigusr1);
    srand(time(NULL));

    g_waiters_ipc = get_waiters_ipc();
    if (g_waiters_ipc == NULL)
    {
        perror("cannot get waiters icp");
        exit(EXIT_FAILURE);
    }

    couleur(JAUNE);
    printf("(waiter) id: %d | pid: %d | New waiter\n", id, pid);

    t_client_order_to_waiter_packet                 rcv_client_order_packet;
    t_waiter_receive_payment_from_client            rcv_payment_packet;
    t_cooker_to_waiter_packet                       rcv_waiter_specialty_packet;
    while (g_running)
    {
        /* 1. Receive client order */
        msgrcv(g_waiters_ipc->msg_queue, &rcv_client_order_packet, sizeof(t_client_order_to_waiter_packet), id + 1, 0);
        couleur(JAUNE);
        printf("(waiter) id: %d | pid: %d | Received an order from client: %d for: %d\n", id, pid, rcv_client_order_packet.client_pid, rcv_client_order_packet.specialty_idx);

        /* 2. Sending order to cookers */
        couleur(JAUNE);
        printf("(waiter) id: %d | pid: %d | Send order to cookers for client: %d for specialty: %d\n", id, pid, rcv_client_order_packet.client_pid, rcv_client_order_packet.specialty_idx);
        send_order_to_cookers(id, rcv_client_order_packet.specialty_idx);

        /* 3. Receive specialty from cooker */
        msgrcv(g_waiters_ipc->msg_queue, &rcv_waiter_specialty_packet, sizeof(t_cooker_to_waiter_packet), id + 1, 0);
        couleur(JAUNE);
        printf("(waiter) id: %d | pid: %d | Received specialty from cookers for client: %d\n", id, pid, rcv_client_order_packet.client_pid);

        /* 4. Sending specialty to client */
        couleur(JAUNE);
        printf("(waiter) id: %d | pid: %d | Send specialty to client: %d (specialty: %d)\n", id, pid, rcv_client_order_packet.client_pid, rcv_client_order_packet.specialty_idx);
        send_specialty_to_client(rcv_client_order_packet.client_pid);

        /* 5. Take a terminal */
        decrement_semaphore(g_waiters_ipc, 0, 1);
        couleur(JAUNE);
        printf("(waiter) id: %d | pid: %d | Take a terminal\n", id, pid);
        
        /* 6. Send check to client */
        send_check_to_client(rcv_client_order_packet.client_pid);
        couleur(JAUNE);
        printf("(waiter) id: %d | pid: %d | Send check to client: %d\n", id, pid, rcv_client_order_packet.client_pid);
        
        /* 7. Waiter receive payment */
        msgrcv(g_waiters_ipc->msg_queue, &rcv_payment_packet, sizeof(t_waiter_receive_payment_from_client), id + 1, 0);
        couleur(JAUNE);
        printf("(waiter) id: %d | pid: %d | Received payment from client: %d\n", id, pid, rcv_client_order_packet.client_pid);
        
        /* 8. Waiter release the terminal */
        increment_semaphore(g_waiters_ipc, 0, 1);
        couleur(JAUNE);
        printf("(waiter) id: %d | pid: %d | Release a terminal\n", id, pid);
        
    }
    
    couleur(JAUNE);
    printf("(waiter) id: %d | pid: %d | Exit\n", id, pid);

    free(g_waiters_ipc);
    exit(EXIT_SUCCESS);
}