#include "sjf.h"

#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include <unistd.h>

/**
 * @brief First-In-First-Out (FIFO) scheduling algorithm.
 *
 * This function implements the FIFO scheduling algorithm. If the CPU is not idle it
 * checks if the application is ready and frees the CPU.
 * If the CPU is idle, it selects the next task to run based on the order they were added
 * to the ready queue. The task that has been in the queue the longest is selected to run next.
 *
 * @param current_time_ms The current time in milliseconds.
 * @param rq Pointer to the ready queue containing tasks that are ready to run.
 * @param cpu_task Double pointer to the currently running task. This will be updated
 *                 to point to the next task to run.
 */
void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;      // Add to the running time of the application/task
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Task finished
            // Send msg to application
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            // Application finished and can be removed (this is FIFO after all)
            free((*cpu_task));
            (*cpu_task) = NULL;
        }
    }
    if (*cpu_task == NULL && rq->head != NULL) {
        queue_elem_t *curr = rq->head;
        queue_elem_t *shortest_elem = rq->head;
        queue_elem_t *prev = NULL, *shortest_prev = NULL;

        while (curr) {
            if (curr->pcb->time_ms < shortest_elem->pcb->time_ms) {
                shortest_elem = curr;
                shortest_prev = prev;
            }
            prev = curr;
            curr = curr->next;
        }

        // Remove o nó encontrado
        queue_elem_t *removed = remove_queue_elem(rq, shortest_elem);

        // Passa o PCB para o CPU
        *cpu_task = removed->pcb;

        // Liberta só o nó da lista (não o PCB)
        free(removed);
    }
}