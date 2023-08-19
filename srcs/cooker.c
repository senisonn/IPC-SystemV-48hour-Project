#include "cooker.h"

void	handle_sigusr1(int status)
{
	g_running = 0;
	raise(SIGINT);
}

void    send_specialty_to_waiter(int id, int waiter_id)
{
    t_cooker_to_waiter_packet    snd_packet;
    snd_packet.waiter_id = waiter_id + 1;

	couleur(ROUGE);
	printf("(cooker) id: %d | Send specialty back to waiter: %d\n", id, waiter_id);
    msgsnd(g_cookers_ipc->msg_queue, &snd_packet, sizeof(t_cooker_to_waiter_packet), 0);
}

int main(int argc, char **argv)
{
	int		id = (int) argv[1][0];
	int		nb_specialties = atoi(argv[2]);
	int		nb_utensils = (int) argv[3][0];
	
	g_running = 1;
	signal_handler(SIGUSR1, handle_sigusr1);
	srand(time(NULL));
	
	g_cookers_ipc = get_cookers_ipc(nb_specialties, nb_utensils);
	if (g_cookers_ipc == NULL)
	{
		perror("cannot get cookers ipc");
		exit(EXIT_FAILURE);
	}

	/* printf("(cooker) id: %d | New cooker\n", id);
    for (int sem_idx = 0; sem_idx < nb_utensils; sem_idx++)
    {
        printf("(cooker) - sem_cookers[%d]: %d,\n", sem_idx, get_semaphore_value(g_cookers_ipc, sem_idx));
    } */

	t_waiter_to_cooker_packet	rcv_packet;
	while (g_running)
	{
		/* Waiting to receive an order from a waiter */
		msgrcv(g_cookers_ipc->msg_queue, &rcv_packet, sizeof(t_waiter_to_cooker_packet), WAITER_TO_COOKER_FILTER, 0);
		couleur(ROUGE);

		int specialty_idx = rcv_packet.specialty_idx;
		/* If speciality index is wrong */
		if (specialty_idx >= nb_specialties)
			continue;
		printf("(cooker) id: %d | Received specialty: %d from waiter %d\n", id, specialty_idx, rcv_packet.waiter_id);
		
		for (int utensils_idx = 0; utensils_idx < nb_utensils; utensils_idx++)
			decrement_semaphore(g_cookers_ipc, utensils_idx, g_cookers_ipc->shm_menu[specialty_idx * nb_utensils + utensils_idx]);

		sleep(rand() % 5 + 1);

		for (int utensils_idx = 0; utensils_idx < nb_utensils; utensils_idx++)
			increment_semaphore(g_cookers_ipc, utensils_idx, g_cookers_ipc->shm_menu[specialty_idx * nb_utensils + utensils_idx]);
		
		send_specialty_to_waiter(id, rcv_packet.waiter_id);
	}

	couleur(ROUGE);
	printf("(cooker) id: %d | Exit\n", id);

	free(g_cookers_ipc);
    exit(EXIT_SUCCESS);
}