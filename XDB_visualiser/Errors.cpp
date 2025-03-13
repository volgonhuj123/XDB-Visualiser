#include <windows.h>

int MemoryAllocError(HWND* h){

	return MessageBox(*h, L"Memory allocation error", L"ERROR", MB_ICONSTOP);
}