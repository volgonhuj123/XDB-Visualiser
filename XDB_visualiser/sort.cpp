
#define byte unsigned char
#include "framework.h"
#include <stdlib.h>
#define PTRSIZE sizeof(void*)

struct SortedValueDict{
	int h, w;
	int key_type;
	void** data;

};



int sortby(SortedValueDict*svd,void ** attribute_data, byte** attribute_data_type,UINT attribute_choosed_x,RECT* r_highlighted){
	int w = r_highlighted->right - r_highlighted->left,
		h = r_highlighted->bottom - r_highlighted->top;
	if (w == 1) { return 1; }
	void** data = (void**)malloc(PTRSIZE * (w - 1));
	char*temp_ptr;
	if (!data) { return 1; }
	int key_attribute_size, value_attribute_size, p
		, key_type;
	char* attr_data;

	switch (attribute_data_type[r_highlighted->left +attribute_choosed_x][0]) {
	case 'u':
		key_attribute_size = sizeof(UINT);
		key_type = 1;
		break;
	case 'i':
		key_attribute_size = sizeof(int);
		key_type = 2;
		break;
	case 'f':
		key_attribute_size = 8;
		key_type = 3;
		break;
	default:
		free(data);
		return 1;
	}

	for (int i = 1; i < w; i++) {
		//(attribute_choosed_x+i)%(w)
		p = r_highlighted->left + ((attribute_choosed_x + i) % w);
		switch (attribute_data_type[p][0]) {
		case 'u':
			value_attribute_size = sizeof(UINT);
			break;
		case 'i':
			value_attribute_size = sizeof(int);
			break;
		case 'f':
			value_attribute_size = 8;
			break;
		default:
			free(data);
			return 1;
		}
		data[i]=(void*)malloc((key_attribute_size+ value_attribute_size)*h);
		if (!data[i]) {
			exit(-1);
		}
		for (int j = 0; j < h; j++) {
			temp_ptr = ((char*)data[i]) + (key_attribute_size + value_attribute_size) * j;
			attr_data = (char*)attribute_data[r_highlighted->left+ attribute_choosed_x];
			memcpy(temp_ptr, &attr_data[j], key_attribute_size);
			attr_data = (char*)attribute_data[p];
			memcpy(temp_ptr+ key_attribute_size, &attr_data[j], value_attribute_size);
		}
	}
	svd->data = data;
	svd->h = h;
	svd->key_type = key_type;
	svd->w = w;
	return 0;

}
