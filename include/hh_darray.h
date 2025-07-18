//-----------------------------------------------------------------------------
// Anoter dynamic array library for general purpose use
// Use "#define HH_DARRAY_IMPLEMENTATION" ones in your .c file to
// implement the functions of the library
//-----------------------------------------------------------------------------
// Author		: github.com/SMDHuman
// Last Update	: 18.07.2025
//-----------------------------------------------------------------------------
#ifndef HH_DARRAY_INIT_SIZE
#define HH_DARRAY_INIT_SIZE 16
#endif

#ifndef HH_DARRAY_H
#define HH_DARRAY_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

//-----------------------------------------------------------------------------
typedef struct hh_darray_t{
	size_t size; // How much of it available in bytes
	size_t fill; // How much of it used in bytes
	size_t word; // Word size in bytes
	struct hh_darray_t *next; // Next darray for expention
	void *data; // Data buffer location 	
}hh_darray_t;

// Initialize the array
void hh_darray_init(hh_darray_t* array, size_t word); 
// Deinitialize the array
void hh_darray_deinit(hh_darray_t* array); 
// Add the item to end of the array
void hh_darray_append(hh_darray_t* array, void* item); 
// Pop the item from end of the array
void hh_darray_popend(hh_darray_t* array, void* item); 
// Get the item with index form the array
void hh_darray_get(hh_darray_t* array, size_t index, void* item); 
// Set item to the array on given index
void hh_darray_set(hh_darray_t* array, size_t index, void* item); 
// Inset the item into the index by expanding the array
void hh_darray_push(hh_darray_t* array, size_t index, void* item); 
// Set the item with index from the array and remove it by shrink the array
void hh_darray_pop(hh_darray_t* array, size_t index, void* item); 
// Get how much of it is filled in bytes
size_t hh_darray_get_fill(hh_darray_t* array); 
// Get total space allocated for it in bytes
size_t hh_darray_get_size(hh_darray_t* array); 
// Get how much of it is filled in item count
size_t hh_darray_get_item_fill(hh_darray_t* array);
// Check if an item inside the array
size_t hh_darray_is_inside(hh_darray_t* array, void* item);

//-----------------------------------------------------------------------------
// hh_darray function implementations
#ifdef HH_DARRAY_IMPLEMENTATION

void hh_darray_init(hh_darray_t* array, size_t word){
	array->size = word*HH_DARRAY_INIT_SIZE;
	array->fill = 0;
	array->word = word;
	array->next = 0;
	array->data = malloc(array->size);
}
//-----------------------------------------------------------------------------
void hh_darray_deinit(hh_darray_t* array){
	if(array->next){
		hh_darray_deinit(array->next);
		free(array->next);
	}
	free(array->data);
	memset(array, 0, sizeof(hh_darray_t));
}
//-----------------------------------------------------------------------------
void hh_darray_append(hh_darray_t* array, void* item){
	if(array->fill + array->word > array->size){
		if(!array->next){
			array->next = malloc(sizeof(hh_darray_t));
			array->next->size = (array->size << 1) - (array->size >> 1);
			array->next->fill = 0;
			array->next->word = array->word;
			array->next->next = 0;
			array->next->data = malloc(array->next->size);
		}
		hh_darray_append(array->next, item);
	}else{
		if(item) memcpy(array->data+array->fill, item, array->word);
		else memset(array->data+array->fill, 0, array->word);
		array->fill += array->word;
	}
}
//-----------------------------------------------------------------------------
void hh_darray_popend(hh_darray_t* array, void* item){
	if(array->next){
		hh_darray_popend(array->next, item);
		if(array->next->fill == 0){
			hh_darray_deinit(array->next);
			free(array->next);
			array->next = 0;
		}
	}else{
		array->fill -= array->word;
		if(item) memcpy(item, array->data+array->fill, array->word);
		memset(array->data+array->fill, 0, array->word);
	}
}
//-----------------------------------------------------------------------------
void hh_darray_get(hh_darray_t* array, size_t index, void* item){
	if(index >= array->size / array->word){
		if(array->next){
			index -= array->fill / array->word;
			hh_darray_get(array->next, index, item);	
		}
	}else if(index < array->fill / array->word){
		memcpy(item, array->data+(index*array->word), array->word);
	}
}
//-----------------------------------------------------------------------------
void hh_darray_set(hh_darray_t* array, size_t index, void* item){
	if(index >= array->size / array->word){
		if(array->next){
			index -= array->fill / array->word;
			hh_darray_set(array->next, index, item);	
		}
	}else if(index < array->fill / array->word){
		if(item) memcpy(array->data+(index*array->word), item, array->word);
		else memset(array->data+(index*array->word), 0, array->word);
	}
}
//-----------------------------------------------------------------------------
void hh_darray_push(hh_darray_t* array, size_t index, void* item){
	hh_darray_append(array, 0);
	void *buffer = malloc(array->word);
	for(size_t i = hh_darray_get_item_fill(array)-1; i > index ; i--){
		hh_darray_get(array, i-1, buffer);
		hh_darray_set(array, i, buffer);
	}
	hh_darray_set(array, index, item);	
}
//-----------------------------------------------------------------------------
void hh_darray_pop(hh_darray_t* array, size_t index, void* item){
	if(item) hh_darray_get(array, index, item);
	void *buffer = malloc(array->word);
	for(size_t i = index; i < hh_darray_get_item_fill(array)-1; i++){
		hh_darray_get(array, i+1, buffer);
		hh_darray_set(array, i, buffer);
	}
	hh_darray_popend(array, 0);
}
//-----------------------------------------------------------------------------
size_t hh_darray_get_fill(hh_darray_t* array){
	if(array->next){
		return(hh_darray_get_fill(array->next) + array->fill);
	}else{
		return(array->fill);
	}
}
//-----------------------------------------------------------------------------
size_t hh_darray_get_size(hh_darray_t* array){
	if(array->next){
		return(hh_darray_get_size(array->next) + array->size);
	}else{
		return(array->size);
	}
}
//-----------------------------------------------------------------------------
size_t hh_darray_get_item_fill(hh_darray_t* array){
	return(hh_darray_get_fill(array) / array->word);
}

//-----------------------------------------------------------------------------
size_t hh_darray_is_inside(hh_darray_t* array, void* item){
	void *array_item = malloc(array->word); 
	for(size_t i = 0; i < hh_darray_get_item_fill(array); i++){
		hh_darray_get(array, i, array_item);
		if(memcmp(array_item, item, array->word) == 0) return i;
	}
	return -1;
}

#endif
#endif
