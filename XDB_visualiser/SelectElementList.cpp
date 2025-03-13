#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define UINT unsigned int

#define ES_Array_GROWTH_SIZE 4
#define ESSIZE sizeof(ElementSelected)
struct ElementSelected {
	UINT x=0, y=0,w=0,h=0; 
};



struct ES_Array {
	ElementSelected* arr=0;
	size_t size = 0,capacity= 0;
};

int ESA_add_Elem(ES_Array* arr, UINT x, UINT y, UINT w, UINT h) {
	ElementSelected* temp = 0;
	if (arr->size == arr->capacity) {
		arr->capacity += ES_Array_GROWTH_SIZE;
		temp = (ElementSelected*)malloc(arr->capacity*ESSIZE);
		if (!temp) { return 1; }
		memcpy(temp, arr->arr, arr->size * ESSIZE);
		free(arr->arr);
		arr->arr = temp;
	}
	arr->arr[arr->size].x=x ; arr->arr[arr->size].y = y;
	arr->arr[arr->size].w = w; arr->arr[arr->size].h = h;
	arr->size++; return 0;
}

void ESA_free(ES_Array* arr) {
	free(arr->arr);
	memset(arr, 0, ESSIZE);
}


void ESA_set(ES_Array* arr, UINT x, UINT y, UINT w, UINT h) {
	
	
}

int ESA_insert_single(ES_Array* arr, UINT x, UINT y) {
	for (size_t i = 0; i < arr->size; i++) {
		if ((x >= arr->arr[i].x) && (x <= (arr->arr[i].x + arr->arr[i].w))&& (y >= arr->arr[i].y) && (y <= (arr->arr[i].y + arr->arr[i].w))) {
			return 1;
		}
		if ((x >= arr->arr[i].x) && (x <= (arr->arr[i].x + arr->arr[i].w))&&(x >= arr->arr[i].x) && (arr->arr[i].y+1 == y)) {
			arr->arr[i].h++;
			return 0;
		}
		if ((x >= arr->arr[i].x) && (x <= (arr->arr[i].x + arr->arr[i].w)) && (x >= arr->arr[i].x) && (arr->arr[i].y + 1 == y)) {
			arr->arr[i].h++;
			return 0;
		}
		if ((y >= arr->arr[i].y) && (x <= (arr->arr[i].y + arr->arr[i].h)) && 1 ) {
			arr->arr[i].h++;
			return 0;
		}
	}

	return 0;
}
