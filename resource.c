/***************************************************************
 * resource.c
 * Contains functionality for managing resources.
 * This includes: Resources, Storage (Resource Array), Recipes
 ***************************************************************/

#include "defs.h"
/**
 * Creates and initializes a `Resource` structure.
 * Dynamically allocates memory for a resource and the resource name field and initializes all of the values.
 * 
 * @param[out] resource     Pointer to the `Resource` to initialize.
 * @param[in]  name         Name of the resource, copied into a dynamically allocated field.
 * @param[in]  amount       Initial amount of the resource avaialble.
 * @param[in]  max_capacity Maximum capacity this resource can hold.
 */
void resource_create(Resource **resource, const char *name, int amount, int max_capacity) {
   *resource = (Resource*)malloc(sizeof(Resource));
    (*resource)->name = (char *)calloc(strlen(name)+1,sizeof(char));
    strcpy((*resource)->name,name);
    (*resource)->amount = amount;
    (*resource)->max_capacity = max_capacity;
    sem_init(&(*resource)->mutex,0,1);
}
/**
 * Destroys a `Resource` structure.
 * Frees all dynamic memory for a resource.
 *
 * @param[in] resource Pointer to the `Resource` to destroy.
 */
void resource_destroy(Resource *resource) {
    free(resource->name);
    sem_destroy(&resource->mutex);
    free(resource);
    
}
/**
 * Thread safe function to add as much resource from the amount as possible and decreases amount by the amount that was added.
 * 
 * @param[in,out] resource Pointer to the `Resource` to add to.
 * @param[in,out] amount   Pointer to the amount of resource to add, will be decreased by the amount that was added.
 */
void resource_transfer_into(Resource *resource, int *amount) {
    int remaining_capacity = resource->max_capacity - resource->amount;
    int amount_to_transfer = (remaining_capacity >= *amount)? *amount : remaining_capacity;
    
    resource->amount += amount_to_transfer;
    *amount -= amount_to_transfer; // Decrease the amount by what was added
}

/**
 * Thread safe function to remove as much of a resource as it can and returns the actual amount that was removed.
 * 
 * @param[in,out] resource Pointer to the `Resource` to remove from.
 * @param[in,out] amount   Pointer to the amount of resource to remove, will be decreased by the amount that was removed.
 */
void resource_transfer_from(Resource *resource, int *amount) {
    int amount_to_transfer = (resource->amount < *amount) ? resource->amount : *amount; // Remove all that's available
    
    resource->amount -= amount_to_transfer;
    *amount -= amount_to_transfer;
}

/**
 * Initializes a `Recipe` structure.
 * Sets the input and output resources, their amounts, and the processing time.
 * 
 * @param[out] recipe          Pointer to the `Recipe` to initialize.
 * @param[in]  input           Pointer to the input `Resource` for the recipe.
 * @param[in]  output          Pointer to the output `Resource` for the recipe.
 * @param[in]  input_amount    Amount of the input resource consumed.
 * @param[in]  output_amount   Amount of the output resource produced.
 * @param[in]  processing_time Processing time in milliseconds.
*/
void recipe_init(Recipe *recipe, Resource *input, Resource *output, int input_amount, int output_amount, int processing_time) {
    recipe->input = input;
    recipe->output = output;
    recipe->input_amount = input_amount;
    recipe->output_amount = output_amount;
    recipe->processing_time = processing_time;
}

/**
 * Initializes the resource array with a given initial capacity.
 * 
 * @param[out] array Pointer to the `SharedResourceArray` to initialize.
 */
void storage_init(SharedResourceArray *array) {
    array->size = 0;
    array->capacity = 1;
    array->resources = (Resource **)calloc(array->capacity,sizeof(Resource*));

}
/**
 * Cleans up the resource array by destroying all resources and freeing memory.
 * 
 * @param[in,out] array Pointer to the `SharedResourceArray` to clean.
 */
void storage_clean(SharedResourceArray *array) {
    for (int i = 0; i<array->size; i++){
        resource_destroy(array->resources[i]);
    }
    free(array->resources);
    array->resources = NULL;
    
}    
    
/**
 * Adds a resource to the resource array, resizing if necessary.
 * 
 * @param[in,out] storage Pointer to the `SharedResourceArray` to add the resource to.
 * @param[in]     resource Pointer to the `Resource` to add.
 */
void storage_add(SharedResourceArray *storage, Resource *resource) {
    if (storage->size >= storage->capacity){
        //create a new array with double capacity
        Resource **new_resources = (Resource **)calloc(storage->capacity*2,sizeof(Resource *));
        //copy all elements in new array
        for(int i = 0; i<storage->size; i++){
            new_resources[i] = storage->resources[i];
        }
        free(storage->resources);
        storage->resources = new_resources;
        storage->capacity *= 2;
    }
    
    storage->resources[storage->size] = resource;
    storage->size++;

}
