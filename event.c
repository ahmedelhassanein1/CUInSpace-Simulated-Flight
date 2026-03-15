/***************************************************************
 * event.c 
 * Contains functionality for events and event queues.
 * Event queues pop from the head and push into priority order, highest priority at the beginning.
 ***************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "defs.h"

/**
 * Initializes an `Event` structure.
 *
 * Sets up an `Event` with the provided system, resource, status, priority, and amount.
 *
 * @param[out] event     Pointer to the `Event` to initialize.
 * @param[in]  system    Pointer to the `System` that generated the event.
 * @param[in]  resource  Pointer to the `Resource` associated with the event.
 * @param[in]  status    Status code representing the event type.
 */
void event_init(Event *event, System *system, Resource *resource, int status) {
    event->system   = system;
    event->resource = resource;
    event->status   = status;
    event->priority = status & 0xFF00; // Extract priority bits from status
    
}

/**
 * Initializes an `EventQueue` structure.
 *
 * @param[out] queue     Pointer to the `EventQueue` to initialize.
 */
void event_queue_init(EventQueue *queue) {
    queue->head = NULL;
    sem_init(&queue->mutex,0,1);


}
    
/**
 * Cleans up the `EventQueue` by freeing all nodes.
 *
 * @param[in,out] queue  Pointer to the `EventQueue` to clean.
 */
void event_queue_clean(EventQueue *queue) {
    EventNode *current = queue->head;
    while (current != NULL){
        EventNode *next = current->next;
        free(current);
        current = next;
    }
    sem_destroy(&queue->mutex);
    queue->head = NULL;
}

/**
 * Pushes an `Event` onto the `EventQueue`.
 *
 * Adds the event to the queue in a thread-safe manner, maintaining priority order (highest first).
 *
 * @param[in,out] queue  Pointer to the `EventQueue`.
 * @param[in]     event  Pointer to the `Event` to push onto the queue.
 */
void event_queue_push(EventQueue *queue, const Event *event) {
    EventNode *current = queue->head;
    EventNode *new_node = (EventNode *)malloc(sizeof(EventNode));
    new_node->event = *event;
    EventNode *prev_node = NULL;
    new_node->next = NULL;
    
    while (current != NULL){
        // add if new node is higher priority
        if (new_node->event.priority > current->event.priority){
            if (prev_node == NULL){
                queue->head = new_node;
                new_node->next = current;
            }
            else if (current == NULL){
                new_node->next = prev_node;
                
            }
            else{
                new_node->next = current;
                prev_node->next = new_node;
            }
            return;
        }
        prev_node = current;
        current = current->next;
    }
    // adding if new node is lower or equal priority
    if (prev_node == NULL){
        queue->head = new_node;
    }
    else if (current == NULL){
        prev_node->next = new_node;
        new_node->next = NULL;
    }
    else{
        current->next = new_node;
        prev_node->next = current;
    }    
}


/**
 * Pops an `Event` from the `EventQueue`.
 *
 * Removes the highest priority event from the queue in a thread safe manner and returns it.
 *
 * @param[in,out] queue  Pointer to the `EventQueue`.
 * @param[out]    event  Pointer to the `Event` to fill with the popped data.
 * @return 1 if an event was successfully popped, 0 if the queue was empty.
 */
int event_queue_pop(EventQueue *queue, Event* event) {
    if (queue == NULL || queue->head == NULL){
        return 0;
    }
    EventNode *current = queue->head;
    *event = current->event;
    queue->head = current->next;
    free(current);
    return 1;
}

