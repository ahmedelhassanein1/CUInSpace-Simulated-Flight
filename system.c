/***************************************************************
 * system.c
 * Contains functionality for running a single system.
 * TODO: Add a `system_thread()` function which runs the system_run() function
 *       in a loop.
 ***************************************************************/

#include "defs.h"

void *system_thread(void* arg){ 
    System *sys = (System *)arg;
    while (system_get_mode(sys) != MODE_TERMINATE){
        system_run(sys);
    }
    pthread_exit(NULL);
}

// Helper functions just used by this C file to clean up our code
// Using static means they can't get linked into other files
static void system_simulate_process_time(System *);
static void report_recipe_thresholds(System *system);

/**
 * Creates and initializes a `System` structure.
 *
 * Dynamically allocates memory for a system and sets its name, recipe, mode, and global event queue.
 * NOTE: Global queue is not actually a data segment global queue, it's just shared between systems.
 *
 * @param[out] system      Pointer to the `System` to initialize.
 * @param[in]  name        Name of the system, copied into a dynamically allocate field.
 * @param[in]  recipe      Recipe containing input/output resources and processing time.
 * @param[in]  event_queue Pointer to the shared event queue for the system, the global_queue field of the system.
 */
void system_create(System **system, const char *name, Recipe recipe, EventQueue *event_queue) {
    *system = (System*)malloc(sizeof(System));
    (*system)->name = (char *)calloc(strlen(name)+1,sizeof(char));
    strcpy((*system)->name,name);
    (*system)->recipe = recipe;
    (*system)->global_queue = event_queue;
    (*system)->mode = MODE_STANDARD;
}

/**
 * Destroys a `System` structure.
 *
 * Frees all dynamic memory for a system.
 *
 * @param[in] system Pointer to the `System` to destroy.
 */
void system_destroy(System *system) {
    free(system->name);
    free(system);
}
/**
 * Gets the current mode of the system.
 *
 * @param[in] system Pointer to the `System` to get the mode from.
 * @return The current mode of the system.
 */
int system_get_mode(const System *system) {
    return system->mode;
}

/**
 * Sets the mode of the system.
 *
 * @param[in,out] system Pointer to the `System` to set the mode for.
 * @param[in]     mode   The new mode to set for the system.
 */
void system_set_mode(System *system, int mode) {
    system->mode = mode;
}

/**
 * Main execution function for a system.
 *
 * Attempts to run the system's recipe, pulling input resources, processing them, and pushing output resources.
 * If SINGLE_THREAD_MODE is defined as a non-zero value, it will not wait for resources to accumulate before continuing.
 *
 * @param[in,out] system Pointer to the `System` to run.
 */
void system_run(System *system) {
    Event event;
    int local_output_amount = 0;

    
    // Pull input resources until we have enough to convert
    int amount_to_pull = system->recipe.input_amount;
    while (amount_to_pull > 0 && system_get_mode(system) != MODE_TERMINATE) {
        sem_wait(&system->recipe.input->mutex);
        resource_transfer_from(system->recipe.input, &amount_to_pull);
        sem_post(&system->recipe.input->mutex);

        if (amount_to_pull > 0) {
            // If we don't have enough input resources, report the low status
            event_init(&event, system, system->recipe.input, EVENT_INSUFFICIENT);
            sem_wait(&system->global_queue->mutex);
            event_queue_push(system->global_queue, &event);
            sem_post(&system->global_queue->mutex);
            usleep(PARAM_SYSTEM_WAIT * 1000 / PARAM_SPEED_MODIFIER);

            if (SINGLE_THREAD_MODE) return;
        }
    }
    
    // If we have enough input resources, process them
    if (amount_to_pull == 0) {
        system_simulate_process_time(system);
        local_output_amount = system->recipe.output_amount;
        event_init(&event, system, system->recipe.input, EVENT_PRODUCED);
        sem_wait(&system->global_queue->mutex);
        event_queue_push(system->global_queue, &event);
        sem_post(&system->global_queue->mutex);
    }
    // Push the resource to the centralized storage, IF there is even an output in the recipe
    while (system->recipe.output && local_output_amount > 0 && system_get_mode(system) != MODE_TERMINATE) {
        sem_wait(&system->recipe.output->mutex);
        resource_transfer_into(system->recipe.output, &local_output_amount);
        sem_post(&system->recipe.output->mutex);

        if (local_output_amount > 0) {
            // If we didn't load everything in, report that we're still at capacity
            event_init(&event, system, system->recipe.output, EVENT_CAPACITY);
            sem_wait(&system->global_queue->mutex);
            event_queue_push(system->global_queue, &event);
            sem_post(&system->global_queue->mutex);
            usleep(PARAM_SYSTEM_WAIT * 1000 / PARAM_SPEED_MODIFIER);

            if (SINGLE_THREAD_MODE) {
               return;
            }
        }
    }
    report_recipe_thresholds(system);
}

/**
 * Thread safe local helper function that reports the current thresholds for a system's recipe.
 *
 * @param[in] system Pointer to the `System` to report thresholds for.
 */
static void report_recipe_thresholds(System *system) {
    int low_threshold = system->recipe.input_amount * PARAM_RESOURCE_LOW;
    int high_threshold = system->recipe.input_amount * PARAM_RESOURCE_HIGH;
    Event event;
    sem_wait(&system->recipe.input->mutex);
    if (system->recipe.input->amount <= low_threshold) {
        event_init(&event, system, system->recipe.input, EVENT_LOW);
        sem_wait(&system->global_queue->mutex);
        event_queue_push(system->global_queue, &event);
        sem_post(&system->global_queue->mutex);
    } else if (system->recipe.input->amount > high_threshold) {
        event_init(&event, system, system->recipe.input, EVENT_HIGH);
        sem_wait(&system->global_queue->mutex);
        event_queue_push(system->global_queue, &event);
        sem_post(&system->global_queue->mutex);

    }
    sem_post(&system->recipe.input->mutex);
}

/**
 * Local helper function that simulates the processing time of a system.
 * 
 * @param[in] system Pointer to the `System` to simulate processing time for.
 */
static void system_simulate_process_time(System *system) {
    int adjusted_processing_time;
    switch (system->mode) {
        case MODE_SLOW:
            adjusted_processing_time = system->recipe.processing_time * 4;
            break;
        case MODE_FAST:
            adjusted_processing_time = system->recipe.processing_time / 4;
            break;
        default:
            adjusted_processing_time = system->recipe.processing_time;
    }
    usleep(adjusted_processing_time * 1000 / PARAM_SPEED_MODIFIER);
}

/**
 * Initializes a `SystemArray` structure.
 *
 * Allocates memory for the system array and initializes its size and capacity.
 *
 * @param[out] array Pointer to the `SystemArray` to initialize.
 */
void system_array_init(SystemArray *array) {
    array->size = 0;
    array->capacity = 1;
    array->systems = (System **)calloc(array->capacity,sizeof(System *));
}

/**
 * Cleans up a `SystemArray` structure.
 *
 * Frees all memory associated with the array and its systems.
 *
 * @param[in,out] array Pointer to the `SystemArray` to clean.
 */
void system_array_clean(SystemArray *array) {
    for (int i = 0; i<array->size;i++){
        system_destroy(array->systems[i]);
    }
    free(array->systems);
    array->systems = NULL;
}
    
/**
 * Adds a `System` to a `SystemArray`.
 *
 * Resizes the array if necessary and adds the system to the end of the array.
 *
 * @param[in,out] array Pointer to the `SystemArray` to add the system to.
 * @param[in]     system Pointer to the `System` to add.
 */
void system_array_add(SystemArray *array, System *system) {
    if (array->size >= array->capacity){
        //create a new array with double capacity
        System **new_systems = (System **)calloc(array->capacity*2,sizeof(System *));
        //copy all elements in new array
        for(int i = 0; i<array->size; i++){
            new_systems[i] = array->systems[i];
        }
        free(array->systems);
        array->systems = new_systems;
        array->capacity *= 2;
    }
    array->systems[array->size] = system;
    array->size++;
}

