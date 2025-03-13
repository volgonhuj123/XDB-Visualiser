#define WIN32_LEAN_AND_MEAN            
#include <windows.h>
#define BUTTON_OFF
#define BUTTON_HOOVER
#define BUTTON_ON

struct button{
	HDC hdc[3];
	
	HDC operator [](size_t i) {
		return hdc[i];
	}

};