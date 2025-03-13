#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#define UINT unsigned int
#define UINT64 unsigned long long
#include <float.h>
#pragma warning(disable : 4996)
#define byte unsigned char
#define PTRSIZE sizeof(void*)
#define SINT sizeof(int)
#define SRELATION sizeof(Relation)
#define SDATABASE sizeof(DataBase)
#define SSIZE sizeof(size_t)
#define SWCHAR sizeof(wchar_t)
struct Relation{
	char* name = 0; UINT name_size = 0;
	UINT attribute_count=0;
	char** attribute_name = 0; UINT* attribute_name_size = 0;
	byte** attribute_data_type = 0;
	UINT data_count=0;
	void** attribute_data=0;
};


int dump(void* data, size_t size) {
	FILE* out = fopen("DUMP", "wb");
	if (!out) { return 1; }
	fwrite(data, size, 1, out);
	fclose(out);
	return 0;
}

int dumpf(void* data, size_t size,const char* filename) {
	FILE* out = fopen(filename, "wb");
	if (!out) { return 1; }
	fwrite(data, size, 1, out);
	fclose(out);
	return 0;
}



struct DataBase {
	Relation *r=0;
	UINT count = 0;
};
struct ustr {
	UINT size = 0;
	char* data = 0;
};
#define SUSTR sizeof(ustr)

int DB_add_relation(DataBase* db, Relation* r) {
	const UINT new_count = db->count + 1;
	Relation* temp = (Relation*)malloc(new_count * SRELATION);
	if (temp == 0) {
		return 1;
	}
	memcpy(temp, db->r, SRELATION * db->count);
	free(db->r);
	memcpy(&temp[db->count], r, SRELATION);
	db->r = temp;
	db->count = new_count;
	return 0;
}


byte* retreive_type(const char* type...) {
	byte* ret;
	va_list arg_ptr;
	va_start(arg_ptr, type);
	unsigned int size = 0;
	if (type[0] == 'v') {
		if (strcmp(type, "varchar") == 0) {
			ret = (byte*)malloc(5);
			if (ret == 0) {
				return 0;
			}
			size = va_arg(arg_ptr, unsigned int);
			memcpy(ret + 1, &size, 4);
			ret[0] = 'v';
			return ret;
		}
	}
	if (strcmp(type, "uint") == 0) {
		ret = (byte*)malloc(1);
		if (ret == 0) {
			return 0;
		}
		ret[0] = 'u';
		return ret;
	}
	if (strcmp(type, "int") == 0) {
		ret = (byte*)malloc(1);
		if (ret == 0) {
			return 0;
		}
		ret[0] = 'i';
		return ret;
	}
	if (strcmp(type, "float") == 0) {
		ret = (byte*)malloc(1);
		if (ret == 0) {
			return 0;
		}
		ret[0] = 'f';
		return ret;
	}
	if (strcmp(type, "text") == 0) {
		ret = (byte*)malloc(1);
		if (ret == 0) {
			return 0;
		}
		ret[0] = 't';
		return ret;
	}
	return (byte*)1;
}


int Relation_build(Relation* r, const char* name, UINT attribute_count...) {
	va_list argptr;
	va_start(argptr, attribute_count);
	memset(r, 0, SRELATION);

	const char* temp_attribute_name = 0;
	unsigned int varchar_size=0;
	r->name_size =(UINT) strlen(name);
	r->name = (char*)malloc(r->name_size);
	if (r->name == 0) {
		return 1;
	}
	memcpy(r->name, name, r->name_size);

	r->attribute_count = attribute_count;


	
	r->attribute_name = (char**)malloc(PTRSIZE * attribute_count);
	if (r->attribute_name == 0) {
		return 1;
	}
	
	r->attribute_name_size = (UINT*)malloc(SINT * attribute_count);
	if (r->attribute_name_size == 0) {
		return 1;
	}
	
	r->attribute_data_type = (byte**)malloc(PTRSIZE*attribute_count);
	if (r->attribute_data_type == 0) {
		return 1;
	}
	r->attribute_data = (void**)malloc(PTRSIZE * attribute_count);
	if (r->attribute_data == 0) {
		return 1;
	}
	for (size_t i = 0; i < attribute_count; i++) {
		temp_attribute_name = va_arg(argptr,const char*);
		r->attribute_name_size[i] =(UINT) strlen(temp_attribute_name);
		r->attribute_name[i] =(char*) malloc(r->attribute_name_size[i]);
		if (r->attribute_name[i] == 0) {return 2;}
		memcpy(r->attribute_name[i], temp_attribute_name, r->attribute_name_size[i]);
		//Treat the type
		temp_attribute_name = va_arg(argptr, const char*);
		if (temp_attribute_name[0] == 'v') {
			varchar_size= va_arg(argptr, unsigned int);
		}
		r->attribute_data_type[i] = retreive_type(temp_attribute_name, varchar_size);
		switch ((UINT)r->attribute_data_type[i]) {
		case 0:
			return 1;
			break;
		case 1:
			return 2;
			break;
		}
	}
	return 0;
}

void Relation_free(Relation* r) {
	free(r->name);
	r->name_size = 0;
	for (size_t i = 0; i < r->attribute_count; i++) {
		free(r->attribute_name[i]);
		free(r->attribute_data_type[i]);
		free(r->attribute_data[i]);
	}
	r->data_count = 0;
	r->attribute_count = 0;
	free(r->attribute_data);
	free(r->attribute_name);
	free(r->attribute_name_size);
	free(r->attribute_data_type);
}

void DB_free(DataBase* db) {
	for (size_t i =0; i < db->count; i++) {
		Relation_free(&db->r[i]);
	}
	free(db->r);
	db->count = 0;
}

int Relation_insert(Relation* r ,va_list* arg_ptr) {
	//va_start(*arg_ptr, arg_ptr);
	const UINT new_count = r->data_count + 1;

	char* copy_text = 0;
	const char * text = 0;
	UINT text_size = 0;
	double* f64_attribute_data=0;
	int *i32_attribute_data = 0;
	UINT* u32_attribute_data;
	ustr* ustr_attribute_data;
	UINT max_length = 0;
	
	for (size_t i = 0; i < r->attribute_count; i++) {
		switch (r->attribute_data_type[i][0]) {
		case 'u':
			u32_attribute_data = (UINT*)malloc(4 * new_count);
			if (u32_attribute_data == 0) {
				return 1;
			}
			memcpy(u32_attribute_data,r->attribute_data[i] , 4 *r->data_count);
			u32_attribute_data[r->data_count]= va_arg(*arg_ptr,UINT );
			free(r->attribute_data[i]);
			r->attribute_data[i] = u32_attribute_data;
			break;
		case 'i':
			i32_attribute_data = (int*)malloc(4 * new_count);
			if (i32_attribute_data == 0) {
				return 1;
			}
			memcpy(i32_attribute_data, r->attribute_data[i], 4 * r->data_count);
			i32_attribute_data[r->data_count] = va_arg(*arg_ptr, int);
			free(r->attribute_data[i]);
			r->attribute_data[i] = i32_attribute_data;
			break;
		case 'f':
			f64_attribute_data = (double*)malloc(8 * new_count);
			if (f64_attribute_data == 0) {
				return 1;
			}
			memcpy(f64_attribute_data, r->attribute_data[i], 4 * r->data_count);
			f64_attribute_data[r->data_count] = va_arg(*arg_ptr, UINT);
			free(r->attribute_data[i]);
			r->attribute_data[i] = f64_attribute_data;
			break;
		case 't':
			ustr_attribute_data = (ustr*)malloc(SUSTR*new_count);
			if (ustr_attribute_data == 0) {
				return 1;
			}
			memcpy(ustr_attribute_data, r->attribute_data[i], SUSTR * r->data_count);
			free(r->attribute_data[i]);
			text = va_arg(*arg_ptr, const char*);
			ustr_attribute_data[r->data_count].size = strlen(text);
			ustr_attribute_data[r->data_count].data =(char*) malloc(ustr_attribute_data[r->data_count].size);
			if (ustr_attribute_data[r->data_count].data == 0) {
				return 1;
			}
			memcpy(ustr_attribute_data[r->data_count].data, text, ustr_attribute_data[r->data_count].size);
			r->attribute_data[i] = ustr_attribute_data;
			break;
		case 'v':
			memcpy(&max_length, &r->attribute_data_type[i][1], 4);
			ustr_attribute_data = (ustr*)malloc(SUSTR * new_count);
			if (ustr_attribute_data == 0) {
				return 1;
			}
			memcpy(ustr_attribute_data, r->attribute_data[i], SUSTR * r->data_count);
			free(r->attribute_data[i]);
			text = va_arg(*arg_ptr, const char*);
			ustr_attribute_data[r->data_count].size = strlen(text);
			ustr_attribute_data[r->data_count].size = ustr_attribute_data[r->data_count].size > max_length ? max_length : ustr_attribute_data[r->data_count].size > max_length;
			ustr_attribute_data[r->data_count].data = (char*)malloc(ustr_attribute_data[r->data_count].size);
			if (ustr_attribute_data[r->data_count].data == 0) {
				return 1;
			}
			memcpy(ustr_attribute_data[r->data_count].data, text, ustr_attribute_data[r->data_count].size);
			r->attribute_data[i] = ustr_attribute_data;
			break;
		}

	}

	r->data_count = new_count;
	return 0;
}




int DB_savefile(DataBase* db,const char* filename) {
	FILE* output = fopen(filename, "wb");
	UINT txt_size = 0,varchar_max_txt_size=0;
	if (output == 0) {
		return 1;
	}
	fwrite("XDB", 3, 1, output);
	fwrite("\0", 1, 1, output);
	fwrite(&db->count, 4, 1, output);
	char** cast_attribute_data = 0;
	ustr* ustr_arr;
	for(UINT i =0;i<db->count;i++){
		fwrite(&db->r[i].name_size, 4, 1, output);
		fwrite(db->r[i].name, db->r[i].name_size, 1, output);
		fwrite(&db->r[i].attribute_count, 4, 1, output);
		fwrite(&db->r[i].data_count, 4, 1, output);
		fwrite(db->r[i].attribute_name_size, db->r[i].attribute_count*4, 1, output);
		for (UINT j = 0; j < db->r[i].attribute_count; j++) {
			fwrite(db->r[i].attribute_name[j], db->r[i].attribute_name_size[j], 1, output);
		}
		for (UINT j = 0; j < db->r[i].attribute_count; j++) {
			switch (db->r[i].attribute_data_type[j][0]) {
			case 'u':
				fwrite("u",1 , 1, output);
				break;
			case 'i':
				fwrite("i", 1, 1, output);
				break;
			case 'f':
				fwrite("f", 1, 1, output);
				break;
			case 't':
				fwrite("t", 1, 1, output);
				break;
			case 'v':
				fwrite("v", 1, 1, output);
				fwrite(&db->r[i].attribute_data_type[j][1], 4, 1, output);
				break;
			}
		}
		for (UINT j = 0; j < db->r[i].attribute_count; j++) {
			switch (db->r[i].attribute_data_type[j][0]) {
			case 'u':
				fwrite(db->r[i].attribute_data[j], 4 * db->r[i].data_count, 1, output);
				break;
			case 'i':
				fwrite(db->r[i].attribute_data[j], 4*db->r[i].data_count, 1, output);
				break;
			case 'f':
				fwrite(db->r[i].attribute_data[j], 8 * db->r[i].data_count, 1, output);
				break;
			case 't':
				ustr_arr =(ustr*) db->r[i].attribute_data[j];
				for (UINT k = 0; k < db->r[i].data_count; k++) {
					fwrite(&ustr_arr[k].size, 4, 1, output);
					fwrite(ustr_arr[k].data, ustr_arr[k].size, 1, output);
				}
				break;
			case 'v':
				ustr_arr = (ustr*)db->r[i].attribute_data[j];
				for (UINT k = 0; k < db->r[i].data_count; k++) {
					fwrite(&ustr_arr[k].size, 4, 1, output);
					fwrite(ustr_arr[k].data, ustr_arr[k].size, 1, output);
				}
				break;
			}
		}
		
		
		
		
	}



	fclose(output);
	return 0;
}
int DB_relation_insert(DataBase* db, const char* relation_name...) {
	va_list arg_ptr;
	va_start(arg_ptr, relation_name);
	UINT relation_name_size =(UINT) strlen(relation_name);
	UINT index = 0;
	for (UINT i = 0; i < db->count; i++) {
		if (relation_name_size == db->r[i].name_size) {
			if (memcmp(db->r[i].name, relation_name, relation_name_size) == 0) {
				return Relation_insert(&db->r[i], &arg_ptr);
			}
		}
	}
	return 5;
}
int DB_relation_insert_n(DataBase* db, const char* relation_name,UINT relation_name_size...) {
	va_list arg_ptr;
	va_start(arg_ptr, relation_name);
	UINT index = 0;
	for (UINT i = 0; i < db->count; i++) {
		if (relation_name_size == db->r[i].name_size) {
			if (memcmp(db->r[i].name, relation_name, relation_name_size) == 0) {
				index = i;
				i = db->count;
			}
		}
	}
	return Relation_insert(&db->r[index], &arg_ptr);
}



int DB_readfile(DataBase*db,const char* filename){

	ustr* ustr_attribute_data=0;

	FILE* input = fopen(filename,"rb");
	if (input == 0) {
		return 2;
	}
	_fseeki64(input,0,SEEK_END);
	size_t filesize = _ftelli64(input);
	if (filesize < 4) {
		fclose(input);
		return 1;
	}
	
	char* ptr_cast,magic_key[4];
	UINT text_size = 0;
	byte type=0;
	_fseeki64(input, 0,SEEK_SET );
	//Compare
	fread(magic_key, 3, 1, input);

	
	
	 if (memcmp(magic_key, "XDB",4)==0){
		fread(&db->count, 4, 1, input);
		db->r = (Relation*)malloc(SRELATION* db->count);
		if (db->r == 0) {return 3;}
		
		for(UINT re=0;re<db->count;re++){
			fread(&db->r[re].name_size,4,1,input);
			db->r[re].name = (char*)malloc(db->r[re].name_size);
			if (db->r[re].name == 0) { return 3; }
			fread(db->r[re].name, db->r[re].name_size, 1, input);
			fread(&db->r[re].attribute_count,4, 1, input);
			fread(&db->r[re].data_count, 4, 1, input);
			db->r[re].attribute_name_size = (UINT*)malloc(4* db->r[re].attribute_count);
			if (db->r[re].attribute_name_size == 0) {return 3;}
			db->r[re].attribute_name = (char**)malloc(PTRSIZE * db->r[re].attribute_count);
			if (db->r[re].attribute_name == 0) { return 3; }
			fread(db->r[re].attribute_name_size, 4* db->r[re].attribute_count, 1, input);
			for (UINT attribute_name_index = 0; attribute_name_index < db->r[re].attribute_count; attribute_name_index++) {
				db->r[re].attribute_name[attribute_name_index] = (char*)malloc(db->r[re].attribute_name_size[attribute_name_index]);
				if (db->r[re].attribute_name[attribute_name_index] == 0) {return 3;}
				fread(db->r[re].attribute_name[attribute_name_index], db->r[re].attribute_name_size[attribute_name_index], 1, input);
			}
			db->r[re].attribute_data_type = (byte**)malloc(PTRSIZE * db->r[re].attribute_count);
			if (db->r[re].attribute_data_type == 0) { return 3; }
			for(UINT att=0;att<db->r[re].attribute_count;att++){
				fread(&type, 1, 1, input);
				switch (type) {
				case 'i':
					db->r[re].attribute_data_type[att] = (byte*)malloc(1);
					if (db->r[re].attribute_data_type[att] == 0) {return 3;}
					db->r[re].attribute_data_type[att][0] = 'i';
					break;
				case 'u':
					db->r[re].attribute_data_type[att] = (byte*)malloc(1);
					if (db->r[re].attribute_data_type[att] == 0) { return 3; }
					db->r[re].attribute_data_type[att][0] = 'u';
					break;
				case 'f':
					db->r[re].attribute_data_type[att] = (byte*)malloc(1);
					if (db->r[re].attribute_data_type[att] == 0) { return 3; }
					db->r[re].attribute_data_type[att][0] = 'f';
					break;
				case 't':
					db->r[re].attribute_data_type[att] = (byte*)malloc(1);
					if (db->r[re].attribute_data_type[att] == 0) { return 3; }
					db->r[re].attribute_data_type[att][0] = 't';
					break;
				case 'v':
					db->r[re].attribute_data_type[att] = (byte*)malloc(5);
					if (db->r[re].attribute_data_type[att] == 0) { return 3; }
					db->r[re].attribute_data_type[att][0] = 'v';
					fread(&db->r[re].attribute_data_type[att][1], 4, 1, input);
					break;
				}

			}

			db->r[re].attribute_data = (void**)malloc(PTRSIZE * db->r[re].attribute_count);
			if (db->r[re].attribute_data == 0) { return 3; }
			for (UINT att = 0; att < db->r[re].attribute_count; att++) {
				db->r[re].attribute_data[att] =(void**) malloc(PTRSIZE * db->r[re].data_count);
				if (db->r[re].attribute_data[att] == 0) { return 3; }
				switch (db->r[re].attribute_data_type[att][0]) {
				case 'i':
						db->r[re].attribute_data[att] = (void*)malloc(4*db->r[re].data_count);
						if (db->r[re].attribute_data[att] == 0) { return 3; }
						fread(db->r[re].attribute_data[att], 4 * db->r[re].data_count, 1, input);
					break;
				case 'u':
					db->r[re].attribute_data[att] = (void*)malloc(4 * db->r[re].data_count);
					if (db->r[re].attribute_data[att] == 0) { return 3; }
					fread(db->r[re].attribute_data[att], 4 * db->r[re].data_count, 1, input);
					break;
				case 'f':
					db->r[re].attribute_data[att] = (void*)malloc(8 * db->r[re].data_count);
					if (db->r[re].attribute_data[att] == 0) { return 3; }
					fread(db->r[re].attribute_data[att], 8 * db->r[re].data_count, 1, input);
					break;
				case 't':
					ustr_attribute_data = (ustr*)malloc(SUSTR * db->r[re].data_count);
					if (ustr_attribute_data == 0) { return 3; }
					for (UINT d = 0; d < db->r[re].data_count; d++) {
						fread(&ustr_attribute_data[d].size, 4, 1, input);
						ustr_attribute_data[d].data = (char*)malloc(ustr_attribute_data[d].size);
						if (ustr_attribute_data[d].data == 0) {
							return 3;
						}
						fread(ustr_attribute_data[d].data, ustr_attribute_data[d].size, 1, input);
					}
					db->r[re].attribute_data[att] = ustr_attribute_data;
					break;
				case 'v':
					ustr_attribute_data = (ustr*)malloc(SUSTR * db->r[re].data_count);
					if (ustr_attribute_data == 0) { return 3; }
					for (UINT d = 0; d < db->r[re].data_count; d++) {
						fread(&ustr_attribute_data[d].size, 4, 1, input);
						ustr_attribute_data[d].data = (char*)malloc(ustr_attribute_data[d].size);
						if (ustr_attribute_data[d].data == 0) {
							return 3;
						}
						fread(ustr_attribute_data[d].data, ustr_attribute_data[d].size, 1, input);
					}
					db->r[re].attribute_data[att] = ustr_attribute_data;
					break;
				}
			}
		}
	}
	else {
	fclose(input);
		return 1;
	}
	fclose(input);
	return 0;
}





