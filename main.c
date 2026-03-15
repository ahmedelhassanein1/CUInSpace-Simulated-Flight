#include "defs.h"

void load_data(Manager *manager);

int main(void) {
    Manager manager;
    manager_init(&manager);
    load_data(&manager);
    
    pthread_t Manager_thread;
    // printf("Manager starting...\n");
    pthread_create(&Manager_thread,NULL,manager_thread,(void *)&manager);
    // printf("Systems starting ...\n");
    pthread_t System_threads[manager.system_array.size];

    for (int i = 0; i<manager.system_array.size; i++){
        pthread_create(&System_threads[i],NULL,system_thread,(void *)manager.system_array.systems[i]);
    }
    // printf("Systems joining... \n");
    for (int i = 0; i<manager.system_array.size; i++){
        pthread_join(System_threads[i],NULL);
    }
    // printf("Manager joining... \n");
    pthread_join(Manager_thread,NULL);

    // NOTE: The code to handle the manager run and the systems
    //       will be moved to threading functions.
    // while (manager.simulation_running) {
    //     manager_run(&manager);
    //     for (int i = 0; i < manager.system_array.size; i++) {
    //         system_run(manager.system_array.systems[i]);
    //     }
        
    // }

    // Find the distance resource to print out how far we went
    for (int i = 0; i < manager.resources.size; i++) {
        if (strcmp(manager.resources.resources[i]->name, "Distance") == 0) {
            printf("=> Total Distance Travelled: %d furlongs.\n", manager.resources.resources[i]->amount);
        }
    }

    manager_clean(&manager);
    return 0;
}

void load_data(Manager *manager) {
    // Create resources
    Resource *fuel, *oxygen, *energy, *distance;
    resource_create(&fuel, "Fuel", 1000, 1000);
    resource_create(&oxygen, "Oxygen", 20, 50);
    resource_create(&energy, "Energy", 30, 50);
    resource_create(&distance, "Distance", 0, 1000);

    storage_add(&manager->resources, fuel);
    storage_add(&manager->resources, oxygen);
    storage_add(&manager->resources, energy);
    storage_add(&manager->resources, distance);

    // Create systems
    System *propulsion_system, *life_support_system, *crew_capsule_system, *generator_system;
    Recipe propulsion_recipe, life_support_recipe, crew_capsule_recipe, generator_recipe;

    // Propulsion: consumes fuel, produces distance
    recipe_init(&propulsion_recipe, fuel, distance, 5, 25, 500);
    system_create(&propulsion_system, "Propulsion", propulsion_recipe, &manager->event_queue);

    // Life Support: consumes energy, produces oxygen
    recipe_init(&life_support_recipe, energy, oxygen, 10, 5, 100);
    system_create(&life_support_system, "Life Support", life_support_recipe, &manager->event_queue);

    // Crew Capsule: consumes oxygen, produces nothing
    recipe_init(&crew_capsule_recipe, oxygen, NULL, 5, 0, 200);
    system_create(&crew_capsule_system, "Crew", crew_capsule_recipe, &manager->event_queue);

    // Generator: consumes fuel, produces energy
    recipe_init(&generator_recipe, fuel, energy, 10, 9, 200);
    system_create(&generator_system, "Generator", generator_recipe, &manager->event_queue);

    system_array_add(&manager->system_array, propulsion_system);
    system_array_add(&manager->system_array, life_support_system);
    system_array_add(&manager->system_array, crew_capsule_system);
    system_array_add(&manager->system_array, generator_system);
}