#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#pragma warning(disable : 4996)


#define UINT unsigned int
#define INT64 long long
#define UINT64 unsigned long long
#define SBITMAP sizeof(bitmap)
#define byte unsigned char





#pragma comment (lib, "Msimg32")

#pragma once
struct bitmap24 {
	char* buffer = 0;
	UINT width = 0, height = 0;
};
struct bitmap {
	UINT* buffer = 0;		    //Pointeur vers un buffer de pixels
	UINT width = 0, height = 0; //Taille
};

#define SBITMAP sizeof(bitmap)


int bitmap_build(bitmap*b,UINT width,UINT height){
	b->width = width;
	b->height = height;
	b->buffer = (UINT*)malloc(4 * width * height);
	return b->buffer == 0 ? 1 : 0;
}

UINT* rainbow_create_basic(int red_start, int green_start, int blue_start, int level, int speed, void (*generator_function)(int*, int*, int*, int, int)) {
	UINT* rainbow = (UINT*)malloc(1024);
	if (rainbow == 0) {
		return 0;
	}
	for (size_t i = 0; i < 256; i++) {
		rainbow[i] = 255 << 24 | red_start << 16 | green_start << 8 | blue_start;
		generator_function(&red_start, &green_start, &blue_start, level, speed);
	}
	return rainbow;
}


UINT bmp24_read(bitmap24* bmp, const char* filename) {
	FILE* file = fopen(filename,"rb");
	if (file == 0) {
		return 1;
	}
	UINT offset = 0;
	fseek(file,10,SEEK_SET);
	fread(&offset, 4, 1, file);
	fseek(file, 0x12, SEEK_SET);
	fread(&bmp->width, 4, 1, file);
	fread(&bmp->height, 4, 1, file);
	size_t buffer_size = 3 * bmp->width * bmp->height;
	fseek(file, offset, SEEK_SET);
	bmp->buffer = (char*)malloc(buffer_size);
	if (bmp->buffer == 0) {
		return 2;
	}
	fread(bmp->buffer, buffer_size, 1, file);
	fclose(file);

}
void bmp24_free(bitmap24* bmp) {
	free(bmp->buffer);
	bmp->width = 0;
	bmp->height = 0;
}
UINT bmp24_savefile(bitmap24* bmp, const char* filename) {
	const UINT file_size = 3 * bmp->width * bmp->height + 54;
	const UINT constant = 0x00180001;
	const UINT64 constants64[] = { 0,0x00000EC400000EC4,0x0000002800000036 };
	FILE* file = fopen(filename, "wb");
	if (file == 0) {
		return 1;
	}
	char* file_buffer = (char*)malloc(file_size);
	if (file_buffer == 0) {
		return 2;
	}
	memcpy(file_buffer, "BM", 2);
	memcpy(file_buffer + 2, &file_size, 4);
	memset(file_buffer + 6, 0, 4);
	memcpy(file_buffer + 10, &constants64[2], 8);
	memcpy(file_buffer + 18, &bmp->width, 4);
	memcpy(file_buffer + 22, &bmp->height, 4);
	memcpy(file_buffer + 26, &constant, 4);
	memcpy(file_buffer + 30, constants64, 8);
	memcpy(file_buffer + 38, &constants64[1], 8);
	memcpy(file_buffer + 46, constants64, 8);
	memcpy(file_buffer + 54, bmp->buffer, file_size-54);
	fwrite(file_buffer, file_size, 1, file);
	free(file_buffer);
	fclose(file);
}

UINT bmp_nBits(const char* filename) {
	FILE* input = fopen(filename, "rb");
	if (input == 0) { return 0; }
	UINT ret = 0;
	fseek(input, 28, SEEK_SET);
	fread(&ret, 1, 1, input);
	fclose(input);
	return ret;
}



UINT bmp_read(bitmap* bmp, const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (file == 0) {
		return 1;
	}
	size_t offset = 0;
	fseek(file, 10, SEEK_SET);
	fread(&offset, 4, 1, file);
	fseek(file, 0x12, SEEK_SET);
	fread(&bmp->width, 4, 1, file);
	fread(&bmp->height, 4, 1, file);
	size_t buffer_size = 4* bmp->width * bmp->height;
	fseek(file, offset, SEEK_SET);
	bmp->buffer = (UINT*)malloc(buffer_size);
	if (bmp->buffer == 0) {
		return 2;
	}
	fread(bmp->buffer, buffer_size, 1, file);
	fclose(file);
	return 0;
}
void bmp_free(bitmap* bmp) {
	free(bmp->buffer);
	bmp->width = 0;
	bmp->height = 0; 
}
UINT bmpv5_savefile(bitmap* bmp, const char* filename) {
	const UINT file_size = 4 * bmp->width * bmp->height + 150;
	const UINT constants[] = { 0x00200001,0x00FF0000};
	const UINT64 constants64[] = { 0,0x00000EC400000EC4,0x0000007C00000096,3,
	0x0000FF0000FF0000,0xFF000000000000FF,0x0000FF00000000FF};
	FILE* file = fopen(filename, "wb");
	if (file == 0) {
		return 1;
	}
	char* file_buffer = (char*)malloc(file_size);

	if (file_buffer == 0) {
		return 2;
	}
	memcpy(file_buffer, "BM", 2);

	;
	memcpy(file_buffer + 2, &file_size, 4);
	memset(file_buffer + 6, 0, 4);
	memcpy(file_buffer + 10, &constants64[2], 8);
	memcpy(file_buffer + 18, &bmp->width, 4);
	memcpy(file_buffer + 22, &bmp->height, 4);
	memcpy(file_buffer + 26, constants, 4);
	memcpy(file_buffer + 30, &constants64[3], 8);
	memcpy(file_buffer + 38, &constants64[1], 8);
	memcpy(file_buffer + 46, constants64, 8);
	memcpy(file_buffer + 54, &constants64[4], 8);
	memcpy(file_buffer + 62, &constants64[5], 8);
	memcpy(file_buffer + 70," niW", 4);
	memset(file_buffer + 74,0, 64);
	memcpy(file_buffer + 138, &constants64[6], 8);
	memcpy(file_buffer + 146, &constants[1], 4);

	memcpy(file_buffer + 150, bmp->buffer, file_size-150);

	fwrite(file_buffer, file_size, 1, file);
	fclose(file);
	free(file_buffer);
}


UINT bmp32_to_bmp24(bitmap24* dst, bitmap* src) {
	memcpy(dst, src, SBITMAP);
	char* cast=(char*)src->buffer;
	dst->buffer = (char*)malloc(3 * src->width * src->height);
	if (dst->buffer == 0) {
		return 1;
	}
	for (size_t y = 0; y < src->height; y++) {
		for (size_t x = 0; x < src->width; x++) {
			memcpy(&dst->buffer[(y * src->width + x) * 3],
				&cast[(y * src->width + x) * 4], 3);
		}
	}
	return 0;
}



UINT bmp24_to_bmp32(bitmap* dst, bitmap24* src) {
	memcpy(dst, src, SBITMAP);
	dst->buffer = (UINT*)malloc(4 * src->width * src->height);
	if (dst->buffer == 0) {
		return 1;
	}
	char* cast = (char*)dst->buffer;
	for (size_t y = 0; y < src->height; y++) {
		for (size_t x = 0; x < src->width; x++) {

			memcpy(&cast[(y * src->width + x) * 4],
				&src->buffer[(y * src->width + x) * 3], 3);

		}
	}
	return 0;
}


UINT bmp32_alpha_to_bmp24(bitmap24* dst, bitmap* src) {
	memcpy(dst, src, SBITMAP);
	char* cast = (char*)src->buffer;
	dst->buffer = (char*)malloc(3 * src->width * src->height);
	if (dst->buffer == 0) {
		return 1;
	}

	for (size_t y = 0; y < src->height; y++) {
		for (size_t x = 0; x < src->width; x++) {
			memset(&dst->buffer[(y * src->width + x) * 3], cast[(y * src->width + x) * 4+3],3) ;
		}
	}
	return 0;
}

UINT bmp24_plus_alpha_to_bmp24(bitmap* dst, bitmap24* src,bitmap24*alpha) {
	memcpy(dst, src, SBITMAP);
	dst->buffer = (UINT*)malloc(4 * src->width * src->height);
	if (dst->buffer == 0) {
		return 1;
	}
	char* cast = (char*)dst->buffer;
	for (size_t y = 0; y < src->height; y++) {
		for (size_t x = 0; x < src->width; x++) {
			memcpy(&cast[(y * src->width + x) * 4], &src->buffer[(y * src->width + x) * 3], 3);
			cast[(y * src->width + x) * 4 + 3] = src->buffer[(y * src->width + x) * 3];
		}
	}
	return 0;
}


void bmp_draw(bitmap* bmp) {
	HDC     hScreen = GetDC(NULL);
	HDC     hDC = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateBitmap(bmp->width, bmp->height, 1, 32, bmp->buffer);
	HGDIOBJ old_obj = SelectObject(hDC, hBitmap);

	//BITMAPINFO bmi;

	//bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	//bmi.bmiHeader.biWidth = bmp->width;
	//bmi.bmiHeader.biHeight = bmp->height; // top-down
	//bmi.bmiHeader.biPlanes = 1;
	//bmi.bmiHeader.biBitCount = 32;
	//bmi.bmiHeader.biCompression = BI_RGB;


	BitBlt(hScreen, 0, 0, bmp->width, bmp->height, hDC, 0, 0, SRCCOPY);

	DeleteObject(hBitmap);
	DeleteDC(hDC);

}


UINT bmp_vertical_invert(bitmap* b) {
	int* line = (int*)malloc(b->width * 4);
	if (!line) { return 1; }
	for (size_t y = 0; y < b->height/2; y++) {
		memcpy(line,
			&b->buffer[(b->height - y - 1) * b->width],
			b->width * 4
		);
		memcpy(&b->buffer[(b->height - y - 1) * b->width],
			&b->buffer[y * b->width],
			b->width * 4
		);
		memcpy(&b->buffer[y * b->width],
			line,
			b->width * 4
		);
	}
	free(line);
	return 0;
}
UINT bmp24_vertical_invert(bitmap24* b) {

	char* temp_buffer = (char*)malloc(b->width * b->height * 3);
	if (temp_buffer == 0) {
		return 1;
	}
	for (size_t y = 0; y < b->height; y++) {
		memcpy(&temp_buffer[(b->height - 1 - y) * b->width*3],
			&b->buffer[y * b->width*3],
			b->width * 3
		);
	}
	free(b->buffer);
	b->buffer = temp_buffer;
	return 0;
}


wchar_t* ___bmp_convertCharArrayToLPCWSTR(const char* charArray) {
	wchar_t* wString = (wchar_t*)malloc(sizeof(wchar_t)*1024);
	memset(wString, 0, 1024 * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 1024);
	return wString;
}


void bmp_draw_linetext(char* cstr){
	HDC hScreen;
	RECT r;
	r.left = 0;
	r.top = 0;
	r.bottom = 512;
	r.right = 512;
	//SetTextColor(pDIS->hDC, RGB(200, 10, 60));
	//SetBkMode(pDIS->hDC, TRANSPARENT);
	//DrawText(hScreen, convertCharArrayToLPCWSTR("Huj"), -1, &r, DT_SINGLELINE | DT_NOCLIP);

}

void bmp_xchng(bitmap* a, bitmap* b) {
	bitmap x;
	memcpy(&x, a, SBITMAP);
	memcpy(a, b, SBITMAP);
	memcpy(b, &x, SBITMAP);
}

UINT bmp_copy(bitmap* dest, bitmap* src) {
	const size_t img_size = src->width * 4 * src->height;
	memcpy(dest, src, sizeof(bitmap));
	dest->buffer = (UINT*)malloc(img_size);
	if (dest->buffer == 0) {
		return 1;
	}
	memcpy(dest->buffer,src->buffer , img_size);
	return 0;
}


//EFFECTS:




UINT bmp_glitch(bitmap* bmp,INT64 rx,INT64 ry,
						    INT64 gx,INT64 gy,
						    INT64 bx,INT64 by){

	char* temp = (char*)malloc(4 * bmp->width * bmp->height),*cast=(char*)bmp->buffer;
	if (temp == 0) {
		return 1;
	}

	for (UINT y = 0; y < bmp->height; y++) {
		for (UINT x = 0; x < bmp->width; x++) {

			if((INT(x)+rx >=0)&&(INT(x)+rx< bmp->width)&& (INT(y) + ry >= 0) && (INT(y) + ry < bmp->height)){
				temp[((y + ry) * bmp->width + x + rx) * 4+2] = cast[(y * bmp->width + x ) * 4+2];
			}
			


			if ((INT(x) + gx >= 0) && (INT(x) + gx < bmp->width)&& (INT(y) + gy >= 0) && (INT(y) + gy < bmp->height)) {
				temp[((y+gy) * bmp->width + x + gx) * 4+1] = cast[(y * bmp->width + x) * 4+1];
			}
			

			if ((INT(x) + bx >= 0) && (INT(x) + bx < bmp->width)&& (INT(y) + by >= 0) && (INT(y) + by < bmp->height)) {
				temp[((y + by) * bmp->width + x + bx) * 4 ] = cast[(y * bmp->width + x) * 4 ];
			}
			


		}
	}


	free(bmp->buffer);

	bmp->buffer = (UINT*)temp;


}


UINT bmp_glitch_to(bitmap * dest,bitmap* src, INT64 rx, INT64 ry,
	INT64 gx, INT64 gy,
	INT64 bx, INT64 by) {

	memcpy(dest, src, SBITMAP);

	char * temp = (char*)malloc(4 * src->width *src->height), * cast = (char*)src->buffer;
	if (temp == 0) {
		return 1;
	}

	for (UINT y = 0; y < src->height; y++) {
		for (UINT x = 0; x < src->width; x++) {

			if ((INT(x) + rx >= 0) && (INT(x) + rx < src->width) && (INT(y) + ry >= 0) && (INT(y) + ry < src->height)) {
				temp[((y + ry) * src->width + x + rx) * 4 + 2] = cast[(y * src->width + x) * 4 + 2];
			}



			if ((INT(x) + gx >= 0) && (INT(x) + gx < src->width) && (INT(y) + gy >= 0) && (INT(y) + gy < src->height)) {
				temp[((y + gy) * src->width + x + gx) * 4 + 1] = cast[(y * src->width + x) * 4 + 1];
			}


			if ((int(x) + bx >= 0) && (INT(x) + bx < src->width) && (INT(y) + by >= 0) && (INT(y) + by < src->height)) {
				temp[((y + by) * src->width + x + bx) * 4] = cast[(y * src->width + x) * 4];
			}
			temp[(y  * src->width + x )* 4+3] = cast[(y * src->width + x) * 4+3];


		}
	}
	dest->buffer = (UINT*)temp;

	return 0;



}









int bmp_normalmap_to(bitmap* dest, bitmap* src, UINT strength) {
	memcpy(dest, src, SBITMAP);

	float xy[3],xy2[3],divisor=1.0f/255.0f;
	float falpha = 1.0f / 127.0f,falpha_1=1.0f-falpha;

	dest->buffer = (UINT*)malloc(4 * src->width * src->height);
	if (dest->buffer == 0) {
		return 1;
	}
	byte* dst_cast = (byte*)dest->buffer,
		* src_cast = (byte*)src->buffer;
	for (UINT x = 0; x < src->width; x++) {
		dest->buffer[x] =(UINT)  127 << 16 | 127<<8|255;
	}
	for (UINT y = 1; y < src->height; y++) {
	//	memcpy(&dest->buffer[y* src->width], dest->buffer, src->width * 4);
	}
	UINT swidth = src->width - strength,sheight=src->height-strength;
	 
	for (UINT y = 0; y < src->height; y++) {
		for (UINT x = 0; x < src->width; x++) {

			//dst_cast[(y * src->width + x) * 4 + 2] = 127;
			//dst_cast[(y * src->width + x) * 4 + 1] = 127;
			xy[0] = 0;
			
			if (x < swidth) {
				xy2[0] =  ~src_cast[(y * src->width + x+strength) * 4+2];
				xy2[1] = ~src_cast[(y * src->width + x + strength) * 4 + 1];
				xy2[2] = ~src_cast[(y * src->width + x + strength) * 4];

				xy[0] = src_cast[(y * src->width + x) * 4 + 2];
				xy[1] = src_cast[(y * src->width + x) * 4 + 1];
				xy[2] = src_cast[(y * src->width + x) * 4 ];

				xy[0] = xy[0] * falpha + xy2[0] * falpha_1;
				xy[1] = xy[1] * falpha + xy2[1] * falpha_1;
				xy[2] = xy[2] * falpha + xy2[2] * falpha_1;

				xy[0] = xy[0] * 0.299f + 0.587f * xy[1] + 0.114f * xy[2];

				//xy[0] = (xy[0] *falpha + xy2[0] * falpha * (1-falpha));

				//xy[0] *= 255.0f;
				
			}
			
			dst_cast[(y * src->width + x) * 4 +2] = xy[0];
			
			if (y  < sheight) {
				xy2[0] = ~src_cast[((y + strength) * src->width + x ) * 4 + 2];
				xy2[1] = ~src_cast[((y + strength) * src->width + x ) * 4 + 1];
				xy2[2] = ~src_cast[((y + strength) * src->width + x ) * 4];
				
				xy[0] = src_cast[(y * src->width + x) * 4 + 2];
				xy[1] = src_cast[(y * src->width + x) * 4 + 1];
				xy[2] = src_cast[(y * src->width + x) * 4];

				xy[0] = xy[0] * falpha + xy2[0] * falpha_1;
				xy[1] = xy[1] * falpha + xy2[1] * falpha_1;
				xy[2] = xy[2] * falpha + xy2[2] * falpha_1;

				xy[0] = xy[0] * 0.299f + 0.587f * xy[1] + 0.114f * xy[2];
				//xy[1] = (xy[1] * falpha + xy2[1] * falpha * (1 - falpha));
				//xy[1] *= 255.0f;
			}
			
		
			
			dst_cast[(y * src->width + x) * 4+1] = xy[0];
			dst_cast[(y * src->width + x) * 4] = 255;
			dst_cast[(y * src->width + x) * 4+3] = src_cast[(y * src->width + x) * 4 + 3];
		}
	}
	return 0;
}


int bmp_linear_mapping(bitmap* dest,bitmap *src, float* abcd) {

	memcpy(dest, src, SBITMAP);
	dest->buffer = (UINT*)malloc(src->width * src->height * 4);
	if (!dest->buffer) {
		return 1;
	}

	float xy[2] = {0};
	for (UINT y = 0; y < src->height; y++) {
		for (UINT x = 0; x < src->width; x++) {
			xy[0] = x;xy[1] = y;
			xy[0] = xy[0] * abcd[0] + abcd[1]* xy[1];
			xy[1] = xy[0] * abcd[2] + abcd[3] * xy[1] ;
			if ((src->height > xy[1]) && (src->width > xy[0])&&!(xy[0]<0) && !(xy[1] < 0)) {
				dest->buffer[UINT(xy[1]) * src->width + UINT(xy[0])] = src->buffer[y * src->width + x];
			}
			
		}
	}


	return 0;
}


UINT bmp_vertical_invert_to(bitmap *  dst, bitmap* src) {

	memcpy(dst, src, SBITMAP);



	dst->buffer = (UINT*)malloc(src->width * src->height * 4);

	if (dst->buffer == 0) {
		return 1;
	}
	for (size_t y = 0; y < src->height; y++) {
		memcpy(&dst->buffer[(src->height - 1 - y) * src->width],
			&src->buffer[y * src->width],
			src->width * 4
		);
	}
	return 0;
}

UINT bmp_draw_line(bitmap* bmp, int x0, int y0, int x1, int y1, UINT RGBA) {
	
	UINT xchng;
	if (x1 == x0) {
		return 0;
	}


	x0 = x0 <  INT64(bmp->width) ? (x0 < 0 ? 0 : x0): bmp->width - 1 ;
	x1= x1  <  INT64(bmp->width) ? (x1 < 0 ? 0 : x1): bmp->width - 1;
	y0 = y0 < INT64(bmp->height) ? (y0 < 0 ? 0 : y0) :bmp->height - 1;
	y1 = y1 < INT64(bmp->height) ? (y1 < 0 ? 0 : y1): bmp->height - 1;

	if (x0 > x1) {
		xchng = x0;
		x0 = x1;
		x1 = xchng;
		xchng = y0;
		y0 = y1;
		y1 = xchng;
	}
	float m, fx0 = x0, fx1 = x1, fy1 = y1, fy0 = y0, p = 0;
	m = float(y1 - y0) / float(x1 - x0);
	p = y0 - x0 * m;

	while (fx0 < fx1) {

		
		fy0 = fx0 * m+p;

		bmp->buffer[UINT(fy0) * bmp->width +UINT(fx0)] = RGBA;

		fx0+=0.001f;
	}

	return 0;
}

#define SFLOAT sizeof(float)

int bmp_3d_mapping(bitmap* dest, bitmap* src,float *camera ,float* angle,float fov) {

	float c[3],s[3];
	c[0] = cos(angle[0]); c[1] = cos(angle[0]); c[2] = cos(angle[0]);
	s[0] = sin(angle[0]); s[1] = cos(angle[0]); s[2] = sin(angle[0]);


	/*
	float matrix[27];


	matrix[0] = 1; matrix[1] = 0; matrix[2] = 0;
	matrix[3] = 0; matrix[4] = c[0]; matrix[5] = s[0];
	matrix[6] = 0; matrix[7] = -s[0]; matrix[8] = c[0];

	matrix[9] = c[1]; matrix[10] = 0; matrix[11] =-s[1];
	matrix[12] = 0; matrix[13] = 1; matrix[14] = 0;
	matrix[15] = s[1]; matrix[16] = 0; matrix[17] = c[1];

	matrix[18] = c[2]; matrix[19] = s[2]; matrix[20] = 0;
	matrix[21] = -s[2]; matrix[22] = c[2]; matrix[23] = 0;
	matrix[24] = 0; matrix[25] = 0; matrix[26] =1;
	*/




	

	//FILE* out = fopen("out.txt", "wb");
	float xyz[3],dist_xyz[3] ;
	
	

	for (UINT y = 0; y < src->height; y++) {
		for (UINT x = 0; x < src->width; x++) {
			//memcpy(xyz, camera, SFLOAT * 3);

			//dist_xyz[0] = double(x)/double(src->width);
			//dist_xyz[1] = double(y)/double(src->height);

			dist_xyz[0] = x;
			dist_xyz[1] = y;
			dist_xyz[2] = 0;

			dist_xyz[0] -= camera[0];
			dist_xyz[1] -= camera[1];
			dist_xyz[2] -= camera[2];

			memcpy(xyz, dist_xyz, 2 * SFLOAT);


			//dx=cy(sz*Y+cz*X)-sy*Z
			//xyz[0] = c[1] * (s[2] * dist_xyz[1] + c[2] * dist_xyz[0])-s[1]*dist_xyz[2];
			//dy=sx(cy*Z+sy(sz*Y+cz*X))+cx(cz*Y-sz*X)
			xyz[1] = s[0]*(c[1]*dist_xyz[2]+s[1]*(s[2]*dist_xyz[1]+c[2]*dist_xyz[0]))-c[0]*(c[2]*dist_xyz[1]-s[2]*dist_xyz[0]);
			//dz=cx(cy*Z+sy(sz*Y+cz*X))-sx(cz*Y-sz*X)
			//xyz[2] = c[0]*(c[1]*dist_xyz[2]+s[1]*(s[2]*dist_xyz[1]+c[2]*dist_xyz[0]))-s[0]*(c[2]*dist_xyz[1]-s[2]*dist_xyz[0]);

			xyz[0] = xyz[0] * fov / (xyz[2] + fov);
			xyz[1] = xyz[1] * fov / (xyz[2] + fov);
			//fwrite(xyz, 12, 1, out);
			
			if ((src->height > xyz[1]) && (src->width > xyz[0]) && !(xyz[0] < 0) && !(xyz[1] < 0)) {
				dest->buffer[UINT(xyz[1]) * src->width + UINT(xyz[0])] = src->buffer[y * src->width + x];
			}

		}
	}
	//fclose(out);

	return 0;
}


void bmp_bucket_fill(bitmap *bmp,UINT x,UINT y,UINT RGBA){
	x = x < bmp->width ?  x : bmp->width - 1;
	y = y < bmp->height ? y : bmp->height - 1;

	UINT clr_chng = bmp->buffer[y * bmp->width + x];
	bool not_end = 1;
	UINT lx=x,rx=x,uy=y>0?y-1:0,dy=y<bmp->height-1?y+1: bmp->height - 1;
	while ((lx-- > 0) && (bmp->buffer[y * bmp->width + lx] == clr_chng));
	while ((rx++ < bmp->width - 1) && (bmp->buffer[y * bmp->width + rx] == clr_chng));
	for (size_t i = lx; i < rx; i++) {
		bmp->buffer[uy * bmp->width + i] = RGBA;
	}
	
	


}

void bmp_greyscale(bitmap* bmp) {

	UINT r,g,b,Y;

	byte* cast =(byte*) bmp->buffer;
	for (size_t i = 0; i < bmp->height*bmp->width; i++) {
		
			r = *cast++;
			g = *cast++;
			b = *cast;
			cast+=2;

			Y = ((r << 1) + g + (g << 2) + b) >> 3;
			
			bmp->buffer[i] = 255<<24 | Y<<16 | Y<<8 | Y;
			

	}

}


void bmp_greyscale_to_rainbow(bitmap* bmp,UINT* rainbow) {
	byte* cast = (byte*)bmp->buffer;
	for (size_t i = 0; i < bmp->height* bmp->width; i++) {
			bmp->buffer[i] =rainbow[*cast];
			cast += 4;
	}

}



void bmp_rainbow_cycle(bitmap* bmp, int level, int speed, void (*generator_function)(int*, int*, int*, int, int)) {
	float r, g, b,min,max,diff,S,one_diff,r1,g1,b1,H;

	float p6 = 1.0f / 6.0f;

	byte* cast = (byte*)bmp->buffer;
	
	for (size_t i = 0; i < bmp->height* bmp->width; i++) {
		r = *cast++;
		g = *cast++;
		b = *cast;

		r /= 255; g /= 255;b /= 255;



		min = r > g ? (g > b ? b : g) : (r > b ? b : r);
		max = r > g ? (r > b ? r : b) : (g > b ? g : b);  //V

		diff = max - min;							      //S
		one_diff = 1.0f / diff;
		if (diff != 0) {
			S =  diff/max;

			r1 = ((max-r)* p6 +(diff* 0.5f))* one_diff;
			g1 = ((max - g) * p6 + (diff * 0.5f)) * one_diff;
			b1 = ((max - b) * p6 + (diff * 0.5f)) * one_diff;

			if (max == r) { H = b1 - g1; }
			if (max == g) { H =1/3 +r1 - b1; }
			if (max == b) { H = 2 / 3 + g1 - r1; }
		}

		cast += 2;
	}

}









void bmp_blend_normal(bitmap* a, bitmap* b,byte alpha) {
	size_t width = a->width > b->width ? b->width : a->width,
		height= a->height > b->height ? b->height : a->height;
	float rgba[4],rgba2[4],alpha_output,reverse_1, alpha_output_1;
	byte* cast_a =(byte*) a->buffer,*cast_b= (byte*)b->buffer;
	float divisor = 1.0f / 255.0f;
	float falpha = float(alpha)/255.0f ;
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			rgba[3] = float(cast_a[(y * a->width + x) * 4 + 3]) * divisor;
			rgba2[3]= float(cast_b[(y * b->width + x) * 4 + 3]) * divisor;
			reverse_1 = 1.0f - rgba2[3];
			alpha_output = rgba2[3] + rgba[3] * reverse_1;
			if (alpha_output != 0) {
				alpha_output_1 = 1.0f / alpha_output;
				rgba[0] = float(cast_a[(y * a->width + x) * 4])* divisor;
				rgba[1] = float(cast_a[(y * a->width + x) * 4+1])* divisor;
				rgba[2] = float(cast_a[(y * a->width + x) * 4 +2])* divisor;

				rgba2[0] = float(cast_b[(y * b->width + x) * 4]) * divisor;
				rgba2[1] = float(cast_b[(y * b->width + x) * 4 + 1]) * divisor;
				rgba2[2] = float(cast_b[(y * b->width + x) * 4 + 2]) * divisor;

				rgba2[0] = (rgba2[0] * rgba2[3] + rgba[0] * rgba[3] * reverse_1)* alpha_output_1;
				rgba2[1] = (rgba2[1] * rgba2[3] + rgba[1] * rgba[3] * reverse_1)* alpha_output_1;
				rgba2[2] = (rgba2[2] * rgba2[3] + rgba[2] * rgba[3] * reverse_1)* alpha_output_1;
				rgba2[3] = alpha_output;

				rgba2[0] = rgba2[0]*falpha +rgba[0] * (1.0f-falpha);
				rgba2[1] = rgba2[1] * falpha + rgba[1] * (1.0f - falpha);
				rgba2[2] = rgba2[2] * falpha + rgba[2] * (1.0f - falpha);
				rgba2[3] = rgba2[3] * falpha + rgba[3] * (1.0f - falpha);

				rgba2[0] *= 255.0f;
				rgba2[1] *= 255.0f;
				rgba2[2] *= 255.0f;
				rgba2[3] *= 255.0f;

				cast_a[(y * a->width + x) * 4] = rgba2[0];
				cast_a[(y * a->width + x) * 4+1] = rgba2[1];
				cast_a[(y * a->width + x) * 4+2] = rgba2[2];
				cast_a[(y * a->width + x) * 4 +3] = rgba2[3];
			}
		}
	}

}




void bmp_blend_add(bitmap* a, bitmap* b, byte alpha) {
	size_t width = a->width > b->width ? b->width : a->width,
		height = a->height > b->height ? b->height : a->height;
	float rgba[4], rgba2[4], alpha_output, reverse_1, alpha_output_1;
	byte* cast_a = (byte*)a->buffer, * cast_b = (byte*)b->buffer;
	float divisor = 1.0f / 255.0f;
	float falpha = float(alpha) / 255.0f;
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			rgba[3] = float(cast_a[(y * a->width + x) * 4 + 3]) * divisor;
			rgba2[3] = float(cast_b[(y * b->width + x) * 4 + 3]) * divisor;
			reverse_1 = 1.0f - rgba[3];
			alpha_output = rgba[3] + rgba2[3] * reverse_1;
			alpha_output_1 = 1.0f / alpha_output;
			if (alpha_output != 0) {
				rgba[0] = float(cast_a[(y * a->width + x) * 4]) * divisor;
				rgba[1] = float(cast_a[(y * a->width + x) * 4 + 1]) * divisor;
				rgba[2] = float(cast_a[(y * a->width + x) * 4 + 2]) * divisor;

				rgba2[0] = float(cast_b[(y * b->width + x) * 4]) * divisor;
				rgba2[1] = float(cast_b[(y * b->width + x) * 4 + 1]) * divisor;
				rgba2[2] = float(cast_b[(y * b->width + x) * 4 + 2]) * divisor;


				//(operand.rgb*operand.a + res.rgb*res.a*(1-res.a))*(res.a+operand.a*(1-res.a))
				rgba2[0] = (rgba2[0] * rgba2[3] + rgba[0] * rgba[3])*alpha_output;
				rgba2[1] = (rgba2[1] * rgba2[3] + rgba[1] * rgba[3]) *alpha_output;
				rgba2[2] = (rgba2[2] * rgba2[3] + rgba[2] * rgba[3]) *alpha_output;
				rgba2[3] = alpha_output;

			

				rgba2[0] = rgba2[0] * falpha + rgba[0]* (1.0f - falpha);
				rgba2[1] = rgba2[1] * falpha + rgba[1] * (1.0f - falpha);
				rgba2[2] = rgba2[2] * falpha + rgba[2] * (1.0f - falpha);
				rgba2[3] = rgba2[3] * falpha + rgba[3] * (1.0f - falpha);

				rgba2[0] = rgba2[0] > 1.0f ? 1.0f : rgba2[0];
				rgba2[1] = rgba2[1] > 1.0f ? 1.0f : rgba2[1];
				rgba2[2] = rgba2[2] > 1.0f ? 1.0f : rgba2[2];
				rgba2[3] = rgba2[3] > 1.0f ? 1.0f : rgba2[3];

				rgba2[0] *= 255.0f;
				rgba2[1] *= 255.0f;
				rgba2[2] *= 255.0f;
				rgba2[3] *= 255.0f;

				cast_a[(y * a->width + x) * 4] = rgba2[0];
				cast_a[(y * a->width + x) * 4 + 1] = rgba2[1];
				cast_a[(y * a->width + x) * 4 + 2] = rgba2[2];
				cast_a[(y * a->width + x) * 4 + 3] = rgba2[3];
			}
		}
	}

}


void bmp_blend_multiply(bitmap* a, bitmap* b, byte alpha) {
	size_t width = a->width > b->width ? b->width : a->width,
		height = a->height > b->height ? b->height : a->height;
	float rgba[4], rgba2[4], alpha_output, reverse_1, alpha_output_1;
	byte* cast_a = (byte*)a->buffer, * cast_b = (byte*)b->buffer;
	float divisor = 1.0f / 255.0f;
	float falpha = float(alpha) / 255.0f;
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			rgba[3] = float(cast_a[(y * a->width + x) * 4 + 3]) * divisor;
			rgba2[3] = float(cast_b[(y * b->width + x) * 4 + 3]) * divisor;
			reverse_1 = 1.0f - rgba[3];
			alpha_output = rgba[3] + rgba2[3] * reverse_1;
			alpha_output_1 = 1.0f / alpha_output;
			if (alpha_output != 0) {
				rgba[0] = float(cast_a[(y * a->width + x) * 4]) * divisor;
				rgba[1] = float(cast_a[(y * a->width + x) * 4 + 1]) * divisor;
				rgba[2] = float(cast_a[(y * a->width + x) * 4 + 2]) * divisor;

				rgba2[0] = float(cast_b[(y * b->width + x) * 4]) * divisor;
				rgba2[1] = float(cast_b[(y * b->width + x) * 4 + 1]) * divisor;
				rgba2[2] = float(cast_b[(y * b->width + x) * 4 + 2]) * divisor;


				//(operand.rgb*operand.a + res.rgb*res.a*(1-res.a))*(res.a+operand.a*(1-res.a))
				rgba2[0] = (rgba2[0] * rgba2[3] * rgba[0] * rgba[3])/ alpha_output_1;
				rgba2[1] = (rgba2[1] * rgba2[3] * rgba[1] * rgba[3]) / alpha_output_1;
				rgba2[2] = (rgba2[2] * rgba2[3] * rgba[2] * rgba[3]) / alpha_output_1;
				rgba2[3] = alpha_output;



				rgba2[0] = rgba2[0] * falpha + rgba[0] * (1.0f - falpha);
				rgba2[1] = rgba2[1] * falpha + rgba[1] * (1.0f - falpha);
				rgba2[2] = rgba2[2] * falpha + rgba[2] * (1.0f - falpha);
				rgba2[3] = rgba2[3] * falpha + rgba[3] * (1.0f - falpha);

				rgba2[0] = rgba2[0] > 1.0f ? 1.0f : rgba2[0];
				rgba2[1] = rgba2[1] > 1.0f ? 1.0f : rgba2[1];
				rgba2[2] = rgba2[2] > 1.0f ? 1.0f : rgba2[2];
				rgba2[3] = rgba2[3] > 1.0f ? 1.0f : rgba2[3];

				rgba2[0] *= 255.0f;
				rgba2[1] *= 255.0f;
				rgba2[2] *= 255.0f;
				rgba2[3] *= 255.0f;

				cast_a[(y * a->width + x) * 4] = rgba2[0];
				cast_a[(y * a->width + x) * 4 + 1] = rgba2[1];
				cast_a[(y * a->width + x) * 4 + 2] = rgba2[2];
				cast_a[(y * a->width + x) * 4 + 3] = rgba2[3];
			}
		}
	}

}


void bmp_blend_and(bitmap* a, bitmap* b, byte alpha) {
	size_t width = a->width > b->width ? b->width : a->width,
		height = a->height > b->height ? b->height : a->height;
	float rgba[4], rgba2[4], alpha_output, reverse_1, alpha_output_1;
	byte* cast_a = (byte*)a->buffer, * cast_b = (byte*)b->buffer;
	float divisor = 1.0f / 255.0f;
	float falpha = float(alpha) / 255.0f;
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			rgba[3] = float(cast_a[(y * a->width + x) * 4 + 3]) * divisor;
			rgba2[3] = float(cast_b[(y * b->width + x) * 4 + 3]) * divisor;
			reverse_1 = 1.0f - rgba[3];
			alpha_output = rgba[3] + rgba2[3] * reverse_1;
			alpha_output_1 = 1.0f / alpha_output;
			if (alpha_output != 0) {
				rgba[0] = float(cast_a[(y * a->width + x) * 4]) * divisor;
				rgba[1] = float(cast_a[(y * a->width + x) * 4 + 1]) * divisor;
				rgba[2] = float(cast_a[(y * a->width + x) * 4 + 2]) * divisor;

				rgba2[0] = float(cast_a[(y * a->width + x) * 4] & cast_b[(y * b->width + x) * 4]) * divisor;
				rgba2[1] = float(cast_a[(y * a->width + x) * 4+1] & cast_b[(y * b->width + x) * 4 + 1]) * divisor;
				rgba2[2] = float(cast_a[(y * a->width + x) * 4+2] & cast_b[(y * b->width + x) * 4 + 2]) * divisor;


				//(operand.rgb*operand.a + res.rgb*res.a*(1-res.a))*(res.a+operand.a*(1-res.a))
				rgba2[0] = (rgba2[0] * rgba2[3] * rgba[0] * rgba[3]) * alpha_output_1;
				rgba2[1] = (rgba2[1] * rgba2[3] * rgba[1] * rgba[3]) *alpha_output_1;
				rgba2[2] = (rgba2[2] * rgba2[3] * rgba[2] * rgba[3]) * alpha_output_1;
				rgba2[3] = alpha_output;



				rgba2[0] = rgba2[0] * falpha + rgba[0] * (1.0f - falpha);
				rgba2[1] = rgba2[1] * falpha + rgba[1] * (1.0f - falpha);
				rgba2[2] = rgba2[2] * falpha + rgba[2] * (1.0f - falpha);
				rgba2[3] = rgba2[3] * falpha + rgba[3] * (1.0f - falpha);

				rgba2[0] = rgba2[0] > 1.0f ? 1.0f : rgba2[0];
				rgba2[1] = rgba2[1] > 1.0f ? 1.0f : rgba2[1];
				rgba2[2] = rgba2[2] > 1.0f ? 1.0f : rgba2[2];
				rgba2[3] = rgba2[3] > 1.0f ? 1.0f : rgba2[3];

				rgba2[0] *= 255.0f;
				rgba2[1] *= 255.0f;
				rgba2[2] *= 255.0f;
				rgba2[3] *= 255.0f;

				cast_a[(y * a->width + x) * 4] = rgba2[0];
				cast_a[(y * a->width + x) * 4 + 1] = rgba2[1];
				cast_a[(y * a->width + x) * 4 + 2] = rgba2[2];
				cast_a[(y * a->width + x) * 4 + 3] = rgba2[3];
			}
		}
	}

}



void bmp_blend_or(bitmap* a, bitmap* b, byte alpha) {
	size_t width = a->width > b->width ? b->width : a->width,
		height = a->height > b->height ? b->height : a->height;
	float rgba[4], rgba2[4], alpha_output, reverse_1, alpha_output_1;
	byte* cast_a = (byte*)a->buffer, * cast_b = (byte*)b->buffer;
	float divisor = 1.0f / 255.0f;
	float falpha = float(alpha) / 255.0f;
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			rgba[3] = float(cast_a[(y * a->width + x) * 4 + 3]) * divisor;
			rgba2[3] = float(cast_b[(y * b->width + x) * 4 + 3]) * divisor;
			reverse_1 = 1.0f - rgba[3];
			alpha_output = rgba[3] + rgba2[3] * reverse_1;
			alpha_output_1 = 1.0f / alpha_output;
			if (alpha_output != 0) {
				rgba[0] = float(cast_a[(y * a->width + x) * 4]) * divisor;
				rgba[1] = float(cast_a[(y * a->width + x) * 4 + 1]) * divisor;
				rgba[2] = float(cast_a[(y * a->width + x) * 4 + 2]) * divisor;

				rgba2[0] = float(cast_a[(y * a->width + x) * 4] | cast_b[(y * b->width + x) * 4]) * divisor;
				rgba2[1] = float(cast_a[(y * a->width + x) * 4+1] | cast_b[(y * b->width + x) * 4 + 1]) * divisor;
				rgba2[2] = float(cast_a[(y * a->width + x) * 4+2] | cast_b[(y * b->width + x) * 4 + 2]) * divisor;


				//(operand.rgb*operand.a + res.rgb*res.a*(1-res.a))*(res.a+operand.a*(1-res.a))
				rgba2[0] = (rgba2[0] * rgba2[3] * rgba[0] * rgba[3]) * alpha_output_1;
				rgba2[1] = (rgba2[1] * rgba2[3] * rgba[1] * rgba[3]) * alpha_output_1;
				rgba2[2] = (rgba2[2] * rgba2[3] * rgba[2] * rgba[3]) * alpha_output_1;
				rgba2[3] = alpha_output;



				rgba2[0] = rgba2[0] * falpha + rgba[0] * (1.0f - falpha);
				rgba2[1] = rgba2[1] * falpha + rgba[1] * (1.0f - falpha);
				rgba2[2] = rgba2[2] * falpha + rgba[2] * (1.0f - falpha);
				rgba2[3] = rgba2[3] * falpha + rgba[3] * (1.0f - falpha);

				rgba2[0] = rgba2[0] > 1.0f ? 1.0f : rgba2[0];
				rgba2[1] = rgba2[1] > 1.0f ? 1.0f : rgba2[1];
				rgba2[2] = rgba2[2] > 1.0f ? 1.0f : rgba2[2];
				rgba2[3] = rgba2[3] > 1.0f ? 1.0f : rgba2[3];

				rgba2[0] *= 255.0f;
				rgba2[1] *= 255.0f;
				rgba2[2] *= 255.0f;
				rgba2[3] *= 255.0f;

				cast_a[(y * a->width + x) * 4] = rgba2[0];
				cast_a[(y * a->width + x) * 4 + 1] = rgba2[1];
				cast_a[(y * a->width + x) * 4 + 2] = rgba2[2];
				cast_a[(y * a->width + x) * 4 + 3] = rgba2[3];
			}
		}
	}

}

struct BMP_VERTEX3D{
	float xyz[3] = { 0 };
	COLOR16 RGBA[4] = { 0 };
};
struct BMP_VERTEX3DWB {
	float xyz[3] = { 0 };
};
struct BMP_Triangle {
	BMP_VERTEX3DWB points[3];
};
struct BMP_Camera {
	float xyz[3] = { 0 }, angles[3] = {0};
};
struct BMP_Model{
	BMP_Triangle* triangles=0;
	size_t length = 0;
};


int mdl_build(BMP_Model* mdl,size_t length,float **points) {
	mdl->length = length;
	mdl->triangles = (BMP_Triangle*)malloc(sizeof(BMP_Triangle)*length);
	if (!mdl->triangles) { return 1; }
	for (size_t i = 0; i < length; i++) {
		memcpy(mdl->triangles[i].points, points[i], sizeof(float) * 3);
		memcpy(&mdl->triangles[i].points[1], &points[i][3], sizeof(float) * 3);
		memcpy(&mdl->triangles[i].points[2], &points[i][6], sizeof(float) * 3);
	}
	return 0;
}



void BMP_V3D_init(BMP_VERTEX3D* V, float x, float y, float z, UINT rgb) {
	V->xyz[0] = x; V->xyz[1] = y; V->xyz[2] = z;

	V->RGBA[0] = (rgb & 0xff) << 8;
	V->RGBA[1] = (rgb & 0xff00) ;
	V->RGBA[2] = ((rgb>>8) & 0xff00);
	V->RGBA[3] = 255<<8;
}

void ComputeXYZ(float* pos, float* cos_arr, float* sin_arr) {
	float new_pos[3];
	//dx=cy(sz*Y+cz*X)-sy*Z
	*new_pos = pos[1] * sin_arr[0] * sin_arr[1] * cos_arr[2] - pos[2] * cos_arr[0] * sin_arr[1] * cos_arr[2]
		+ pos[1] * cos_arr[0] * sin_arr[2] + pos[2] * sin_arr[0] * sin_arr[2] +
		pos[0] * cos_arr[1] * cos_arr[2];
	//dy=sx(cy*Z+sy(sz*Y+cz*X))+cx(cz*Y-sz*X)
	new_pos[1]= pos[1] * cos_arr[0] * cos_arr[2]  + pos[2] * sin_arr[0] * cos_arr[2] 
		-pos[1]*sin_arr[0]*sin_arr[1]*sin_arr[2]+pos[2]*cos_arr[0]*sin_arr[1]*sin_arr[2]
		-pos[0] * cos_arr[1] *sin_arr[2];
	new_pos[2]=pos[2]*cos_arr[0]* cos_arr[1]- pos[1]*sin_arr[0]*cos_arr[1]+pos[0]*sin_arr[1];
		
		
	//dz=cx(cy*Z+sy(sz*Y+cz*X))-sx(cz*Y-sz*X)
	//new_pos[2]=cos_arr[0] * (cos_arr[1] * pos[2] + sin_arr[1] * (sin_arr[2] * pos[1] + cos_arr[2] * pos[0])) - sin_arr[0] * (cos_arr[2] * pos[1] - sin_arr[2] * pos[0]);
	memcpy(pos, new_pos, SFLOAT * 3);
}

void bmp_surface_3D(bitmap *dst_preallocated,bitmap* src,float *pos,float *angles,float fov) {

	float cos_arr[3],sin_arr[3],new_pos[3],img_pos[3];
	cos_arr[0] = cos(angles[0]);
	cos_arr[1] = cos(angles[1]); 
	cos_arr[2] = cos(angles[2]);
	sin_arr[0] = sin(angles[0]);
	sin_arr[1] = sin(angles[1]);
	sin_arr[2] = sin(angles[2]);
	float ooz;
	UINT xp, yp;


	for (img_pos[1] = 0; img_pos[1] < src->height; img_pos[1]+=0.1) {
		for (img_pos[0] = 0; img_pos[0] < src->width; img_pos[0] += 0.1) {
			memcpy( new_pos, img_pos, 3 * SFLOAT);
			ComputeXYZ(new_pos, cos_arr,sin_arr);
			ooz = 1 / new_pos[2];
			xp = src->width / 2 + fov * ooz * new_pos[0] * 2;
			yp = src->height / 2 + fov * ooz * new_pos[1];
			xp = xp < src->width ? xp: src->width-1;
			yp = yp < src->height ? yp : src->height - 1;
			dst_preallocated->buffer[yp * dst_preallocated->width + xp] = src->buffer[UINT(pos[1]) * src->width + UINT(pos[0])];
		}
	}
}

int bmp_render_polygon(HDC DC,float* camera, float* angle, float fov, BMP_VERTEX3D *V, long V_len, GRADIENT_TRIANGLE * TR,long TR_len, TRIVERTEX* V2D){
	float xyz[3],rot_xyz[3];

	float c[3], s[3];
	c[0] = cos(angle[0]); c[1] = cos(angle[1]); c[2] = cos(angle[2]);
	s[0] = sin(angle[0]); s[1] = cos(angle[1]); s[2] = sin(angle[2]);
	
	float r = c[0] * c[2],
		o=c[0] * s[2],
		y=s[0]*c[2],
		g=s[2]*s[0];



	//FILE* out = fopen("out.txt", "wb");
	for (long vert_i = 0; vert_i < V_len; vert_i++) {
		memcpy(xyz,V[vert_i].xyz,3*SFLOAT);
		xyz[0] -= camera[0] ;
		xyz[1] -= camera[1];
		xyz[2] -= camera[2];
		//dx=cy(sz*Y+cz*X)-sy*Z
		//rot_xyz[0] = c[1] * (s[2] * xyz[1] + c[2] * xyz[0])-s[1]*xyz[2]
		//dy=sx(cy*Z+sy(sz*Y+cz*X))+cx(cz*Y-sz*X)
		//rot_xyz[1] = s[0] * (c[1] * xyz[2] + s[1] * (s[2] * xyz[1] + c[2] * xyz[0])) + c[0] * (c[2] * xyz[1] - s[2] * xyz[0]);
		//dz=cx(cy*Z+sy(sz*Y+cz*X))-sx(cz*Y-sz*X)
		//rot_xyz[2] = c[0]*(c[1]*xyz[2]+s[1]*(s[2]*xyz[1]+c[2]*xyz[0]))-s[0]*(c[2]*xyz[1]-s[2]*xyz[0]);


		//dx=cos(B)*cos(Y) * x  +  y(yellow*sin(B)-orange) + z(red*sin(B)+green)
		rot_xyz[0] = c[1] * c[2] * xyz[0] + xyz[1] * (y * s[1] - o) + xyz[2] * (r * s[1] + g);
		rot_xyz[1] = c[1] * s[2] * xyz[0] + xyz[1] * (g * s[1] + r) + xyz[2] * (o * s[1]-y);
		rot_xyz[2] = -s[1] * xyz[0] + xyz[1] * s[0] * c[1] + xyz[2] * c[0] * c[1];
		

		//xyz[0] = (camera[2]-0.01f) /rot_xyz[2] * rot_xyz[0]+ (camera[0] - 0.01f);
		//xyz[1] = (camera[2] - 0.01f) /rot_xyz[2] * rot_xyz[1] + (camera[1] - 0.01f);
		rot_xyz[0] =( rot_xyz[0]) * fov / rot_xyz[2]  + camera[0];
		rot_xyz[1]= (rot_xyz[1]) * fov / rot_xyz[2] + camera[1];

		V2D[vert_i].x = rot_xyz[0];
		V2D[vert_i].y = rot_xyz[1];
		memcpy(&V2D[vert_i].Red, V[vert_i].RGBA,4*sizeof(COLOR16));
		
		//fwrite(&V2D[vert_i], sizeof(TRIVERTEX), 1, out);
	}
	//fclose(out);
	GradientFill(DC, V2D, V_len, TR, TR_len, GRADIENT_FILL_TRIANGLE);
	int ret = 0;
	 return ret;

}

int bmp_render_model(HDC DC, bitmap* buffer, float* camera, float* angle, float fov, BMP_VERTEX3D* V, long V_len, GRADIENT_TRIANGLE* TR, long TR_len) {
	float xyz[3], rot_xyz[3];

	float c[3], s[3];
	c[0] = cos(angle[0]); c[1] = cos(angle[0]); c[2] = cos(angle[0]);
	s[0] = sin(angle[0]); s[1] = cos(angle[0]); s[2] = sin(angle[0]);

	TRIVERTEX* V2D = (TRIVERTEX*)malloc(V_len * sizeof(TRIVERTEX));
	if (!V2D) {
		return 1;
	}
	//FILE* out = fopen("out.txt", "wb");
	for (long vert_i = 0; vert_i < V_len; vert_i++) {
		memcpy(xyz, V[vert_i].xyz, 3 * SFLOAT);
		xyz[0] -= camera[0];
		xyz[1] -= camera[1];
		xyz[2] -= camera[2];

		//dx=cy(sz*Y+cz*X)-sy*Z
		rot_xyz[0] = c[1] * (s[2] * xyz[1] + c[2] * xyz[0]) - s[1] * xyz[2];
		//dy=sx(cy*Z+sy(sz*Y+cz*X))+cx(cz*Y-sz*X)
		rot_xyz[1] = s[0] * (c[1] * xyz[2] + s[1] * (s[2] * xyz[1] + c[2] * xyz[0])) - c[0] * (c[2] * xyz[1] - s[2] * xyz[0]);
		//dz=cx(cy*Z+sy(sz*Y+cz*X))-sx(cz*Y-sz*X)
		rot_xyz[2] = c[0] * (c[1] * xyz[2] + s[1] * (s[2] * xyz[1] + c[2] * xyz[0])) - s[0] * (c[2] * xyz[1] - s[2] * xyz[0]);

		xyz[0] = rot_xyz[0] * fov / (rot_xyz[2] + fov);
		xyz[1] = rot_xyz[1] * fov / (rot_xyz[2] + fov);
		V2D[vert_i].x = xyz[0];
		V2D[vert_i].y = xyz[1];
		memcpy(&V2D[vert_i].Red, V[vert_i].RGBA, 4 * sizeof(COLOR16));
		//fwrite(&V2D[vert_i], sizeof(TRIVERTEX), 1, out);
	}
	//fclose(out);

	int ret = GradientFill(DC, V2D, V_len, TR, TR_len, GRADIENT_FILL_TRIANGLE);
	free(V2D);
	return ret;

}


