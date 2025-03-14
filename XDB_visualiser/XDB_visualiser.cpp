
#include "framework.h"
#include "XDB_visualiser.h"
#include "XDB.cpp"
#include "SelectElementList.cpp"
#include "bitmap.cpp"
#include "Errors.cpp"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <strsafe.h>
#include <shellapi.h>
#include <wingdi.h>

#pragma comment (lib, "Msimg32")
#define KEY_CHECK_STATE (1<<15)
char* returnType(char c ,UINT size) {
    char *i=(char*)malloc(25);
    if (i == 0) {
        return 0;
    }
    switch (c) {
    case 'i':
        return (char*)"Int 32";
        break;
    case 'u':
        return (char*)"Uint 32";
        break;
    case 'f':
        return (char*)"Float 64";
        break;
    case 't':
        return (char*)"Text";
        break;
    case 'v':
        sprintf(i, "Varchar [%u]", size);
        return i;
        break;

    }


}
int mouse_input=0;


void render_help_window(HDC DC, bool* open, HDC DC_help) {
    BitBlt(DC, 0, 0, 512, 512, 0, 0, 0, BLACKNESS);
    BitBlt(DC, 0, 0, 53, 53, DC_help, 0, 0, SRCCOPY);
}


void draw_text_box(HDC DC,DataBase* db, int db_selected, UINT* relation_selected,int y,int x,int start_y,RECT* rect) {
    UINT* uint_ptr;
    int* int_ptr;
    WCHAR db_print[400];
    ustr* text_ptr;
    double* f64_ptr;
    size_t slen;
    switch (db[db_selected].r[relation_selected[db_selected]].attribute_data_type[x][0]) {
    case 'u':
        uint_ptr = (UINT*)db[db_selected].r[relation_selected[db_selected]].attribute_data[x];
        uint_ptr += y - 1 + start_y;
        slen = swprintf(db_print, L"%u", *uint_ptr);
        DrawText(DC, db_print, slen , rect, 0);
        break;
    case 'i':
        int_ptr = (int*)db[db_selected].r[relation_selected[db_selected]].attribute_data[x];
        int_ptr += y - 1 + start_y;
        slen = swprintf(db_print, L"%i", *int_ptr);
        DrawText(DC, db_print, slen,rect, 0);
        break;
    case 'f':
        f64_ptr = (double*)db[db_selected].r[relation_selected[db_selected]].attribute_data[x];
        f64_ptr +=y - 1 + start_y;
        slen = swprintf(db_print, L"%lF", *f64_ptr);
        DrawText(DC, db_print, slen , rect, 0);
        break;
    case 't':
        text_ptr = (ustr*)db[db_selected].r[relation_selected[db_selected]].attribute_data[x];
        text_ptr += y - 1 + start_y;
        slen = swprintf(db_print, L"%*hs", text_ptr->size > 175 ? 175 : text_ptr->size, text_ptr->data);
        DrawText(DC, db_print, slen,rect, 0);
        break;
    }
}

int scroll = 0;

#define MAX_LOADSTRING 100

// Zmienne globalne:
HINSTANCE hInst;                                // bieżące wystąpienie
WCHAR szTitle[MAX_LOADSTRING];                  // Tekst paska tytułu
WCHAR szWindowClass[MAX_LOADSTRING];            // nazwa klasy okna głównego

int DB_readfile_write(DataBase* db, const char* filename,HDC dc,bitmap* buffer) {

    BitBlt(dc, 0, 0, buffer->width, buffer->height - 17,dc, 0, 17, SRCCOPY);
    BitBlt(dc, 0, buffer->height - 17,buffer->width, 17, dc, 0, 0, WHITENESS);
    

    //
    RECT r_txt;
    r_txt.left = 0;
    r_txt.top = buffer->height - 17;
    r_txt.right = buffer->width;
    r_txt.bottom = buffer->height;
    size_t filename_size = strlen(filename),open_filename_size= filename_size+8,
       failed_open_filename_size= open_filename_size+15 ;
    wchar_t* wstr_opening_filename = (wchar_t*)malloc(open_filename_size * SWCHAR),info[256],
        *wstr_failed_open_filename_size=(wchar_t*)malloc(failed_open_filename_size*SWCHAR);
    if ((wstr_opening_filename == 0)|| (wstr_failed_open_filename_size==0)) {
        SetTextColor(dc, RGB(255, 0, 0));
        DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
        BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
        BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
        SetTextColor(dc, RGB(0, 0, 0));
        return 3;
    }
    
    memcpy(wstr_opening_filename, L"Opening ", 8 * SWCHAR);


  



    
    wchar_t* wtemp = 0;
    for (size_t i = 0; i < filename_size; i++) {
        wstr_opening_filename[8 + i] = filename[i];
    }
    SetTextColor(dc, RGB(0, 127, 127));
    DrawText(dc, wstr_opening_filename, open_filename_size, &r_txt, 0);
    BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
    BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
    free(wstr_opening_filename);

    //mbstowcs
    ustr* ustr_attribute_data = 0;
    
    FILE* input = fopen(filename, "rb");
    if (input == 0) {
        SetTextColor(dc, RGB(255, 0, 0));
        memcpy(wstr_failed_open_filename_size,L"Failed to open ",15 * SWCHAR);
        for (size_t i = 0; i < filename_size; i++) {
            wstr_failed_open_filename_size[15 + i] = filename[i];
        }
        
        DrawText(dc, wstr_failed_open_filename_size, failed_open_filename_size, &r_txt, 0);
        BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
        BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
        SetTextColor(dc, RGB(0,0, 0));
        free(wstr_failed_open_filename_size);
        return 2;
    }
    _fseeki64(input, 0, SEEK_END);
    size_t filesize = _ftelli64(input);
    if (filesize < 4) {
        SetTextColor(dc, RGB(255, 0, 0));
        memcpy(wstr_failed_open_filename_size, L"Failed to open ", 15 * SWCHAR);
        for (size_t i = 0; i < filename_size; i++) {
            wstr_failed_open_filename_size[15 + i] = filename[i];
        }
        DrawText(dc, wstr_failed_open_filename_size, failed_open_filename_size, &r_txt, 0);
        BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
        BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
        SetTextColor(dc, RGB(0, 0, 0));
        free(wstr_failed_open_filename_size);
        return 1;
    }
    free(wstr_failed_open_filename_size);
   
    char* type_name = 0;
    char* ptr_cast, magic_key[4];
    UINT text_size = 0;
    byte type = 0;
    _fseeki64(input, 0, SEEK_SET);
    //Compare
    fread(magic_key, 4, 1, input);
    UINT varchar_size = 0;


    if (memcmp(magic_key, "XDB", 4) == 0) {
        SetTextColor(dc, RGB(0, 0,255));
        fread(&db->count, 4, 1, input);
        swprintf(info, L"%u relation%c", db->count, db->count > 1 ? 's':'\0');
        DrawText(dc, info, -1, &r_txt, 0);
        memset(info, 0, 100 * SWCHAR);
        BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
        BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
        db->r = (Relation*)malloc(SRELATION * db->count);
        if (db->r == 0) {
            SetTextColor(dc, RGB(255, 0, 0));
            DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
            BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
            BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
            SetTextColor(dc, RGB(0, 0, 0));
            return 3;
        }

        for (UINT re = 0; re < db->count; re++) {
            swprintf(info, L"Relation count %u/%u", re+1,db->count);
            DrawText(dc, info, -1, &r_txt, 0);
            memset(info, 0, 100 * SWCHAR);
            BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
            BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
            fread(&db->r[re].name_size, 4, 1, input);
            db->r[re].name = (char*)malloc(db->r[re].name_size);
            if (db->r[re].name == 0) {
                SetTextColor(dc, RGB(255, 0, 0));
                DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                SetTextColor(dc, RGB(0, 0, 0));
                return 3;
            }
            fread(db->r[re].name, db->r[re].name_size, 1, input);
            if (db->r[re].name_size < 1007) {
                swprintf(info, L"Relation name : %.*S", db->r[re].name_size, db->r[re].name);
                DrawText(dc, info, -1, &r_txt, 0);
                memset(info, 0, 100 * SWCHAR);
                BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
            }
            if (db->r[re].name_size > 1007) {
                SetTextColor(dc, RGB(0, 192, 0));
                DrawText(dc, L"The relation name will be displayed as truncated", 24, &r_txt, 0);
                BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                SetTextColor(dc, RGB(0,0, 255));
                swprintf(info, L"Relation name : %.*S", db->r[re].name_size, db->r[re].name);
                DrawText(dc, info, -1, &r_txt, 0);
                memset(info, 0, 100 * SWCHAR);
                BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
            }
            memset(info, 0, 100 * SWCHAR);
            fread(&db->r[re].attribute_count, 4, 1, input);
            swprintf(info, L"Attribute count : %u", db->r[re].attribute_count);
            DrawText(dc, info, -1, &r_txt, 0);
            memset(info, 0, 100 * SWCHAR);
            BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
            BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);

            fread(&db->r[re].data_count, 4, 1, input);
            swprintf(info, L"Data count : %u", db->r[re].data_count);
            DrawText(dc, info, -1, &r_txt, 0);
            memset(info, 0, 100 * SWCHAR);
            BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
            BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);

            db->r[re].attribute_name_size = (UINT*)malloc(4 * db->r[re].attribute_count);
            if (db->r[re].attribute_name_size == 0) {
                SetTextColor(dc, RGB(255, 0, 0));
                DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                SetTextColor(dc, RGB(0, 0, 0));
                return 3;
            }
            db->r[re].attribute_name = (char**)malloc(PTRSIZE * db->r[re].attribute_count);
            if (db->r[re].attribute_name == 0) {
                SetTextColor(dc, RGB(255, 0, 0));
                DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                SetTextColor(dc, RGB(0, 0, 0));
                return 3; }
            SetTextColor(dc, RGB(127, 127, 0));
            swprintf(info, L"HOLD CTRL IF YOU WANT TO SEE ATTRIBUTES NAME", db->r[re].data_count);
            DrawText(dc, info, -1, &r_txt, 0);
            memset(info, 0, 100 * SWCHAR);
            BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
            BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
            fread(db->r[re].attribute_name_size, 4 * db->r[re].attribute_count, 1, input);
            for (UINT attribute_name_index = 0; attribute_name_index < db->r[re].attribute_count; attribute_name_index++) {
                db->r[re].attribute_name[attribute_name_index] = (char*)malloc(db->r[re].attribute_name_size[attribute_name_index]);
                if (db->r[re].attribute_name[attribute_name_index] == 0) {
                    SetTextColor(dc, RGB(255, 0, 0));
                    DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                    BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                    BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                    SetTextColor(dc, RGB(0,0, 0));
                    return 3;
                }
                fread(db->r[re].attribute_name[attribute_name_index], db->r[re].attribute_name_size[attribute_name_index], 1, input);
            }
            db->r[re].attribute_data_type = (byte**)malloc(PTRSIZE * db->r[re].attribute_count);
            if (db->r[re].attribute_data_type == 0) {
                SetTextColor(dc, RGB(255, 0, 0));
                DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                SetTextColor(dc, RGB(0, 0, 0));
                return 3;
            }
            for (UINT att = 0; att < db->r[re].attribute_count; att++) {
                fread(&type, 1, 1, input);
                switch (type) {
                case 'i':
                    type_name = returnType('i', 0);
                    db->r[re].attribute_data_type[att] = (byte*)malloc(1);
                    if (db->r[re].attribute_data_type[att] == 0) {
                        SetTextColor(dc, RGB(255, 0, 0));
                        DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                        BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                        BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                        SetTextColor(dc, RGB(0, 0, 0));
                        return 3;
                    }
                    db->r[re].attribute_data_type[att][0] = 'i';
                    break;
                case 'u':
                    type_name = returnType('u', 0);
                    db->r[re].attribute_data_type[att] = (byte*)malloc(1);
                    if (db->r[re].attribute_data_type[att] == 0) {
                        SetTextColor(dc, RGB(255, 0, 0));
                        DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                        BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                        BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                        SetTextColor(dc, RGB(0, 0, 0));
                        return 3;
                    }
                    db->r[re].attribute_data_type[att][0] = 'u';
                    break;
                case 'f':
                    type_name = returnType('f', 0);
                    db->r[re].attribute_data_type[att] = (byte*)malloc(1);
                    if (db->r[re].attribute_data_type[att] == 0) {
                        SetTextColor(dc, RGB(255, 0, 0));
                        DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                        BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                        BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                        SetTextColor(dc, RGB(0, 0, 0));
                        return 3;
                    }
                    db->r[re].attribute_data_type[att][0] = 'f';
                    break;
                case 't':
                    type_name = returnType('t', 0);
                    db->r[re].attribute_data_type[att] = (byte*)malloc(1);
                    if (db->r[re].attribute_data_type[att] == 0) {
                        SetTextColor(dc, RGB(255, 0, 0));
                        DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                        BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                        BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                        SetTextColor(dc, RGB(0, 0, 0));
                        return 3;
                    }
                    db->r[re].attribute_data_type[att][0] = 't';
                    break;
                case 'v':
                    db->r[re].attribute_data_type[att] = (byte*)malloc(5);
                    if (db->r[re].attribute_data_type[att] == 0) {
                        SetTextColor(dc, RGB(255, 0, 0));
                        DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                        BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                        BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                        SetTextColor(dc, RGB(0, 0, 0));
                        return 3;
                    }
                    db->r[re].attribute_data_type[att][0] = 'v';
                    fread(&db->r[re].attribute_data_type[att][1], 4, 1, input);
                    memcpy(&varchar_size, &db->r[re].attribute_data_type[att][1], 4);
                    type_name = returnType('t', varchar_size);
                    break;
                }
                if (GetAsyncKeyState(VK_CONTROL)) {
                    swprintf(info, L"%.*S %S", db->r[re].attribute_name_size[att], db->r[re].attribute_name[att], type_name);
                    DrawText(dc, info, -1, &r_txt, 0);
                    memset(info, 0, 100 * SWCHAR);
                    BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                    BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                }

            }

            db->r[re].attribute_data = (void**)malloc(PTRSIZE * db->r[re].attribute_count);
            if (db->r[re].attribute_data == 0) {
                SetTextColor(dc, RGB(255, 0, 0));
                DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                SetTextColor(dc, RGB(0, 0, 0));
                return 3;
            }
            for (UINT att = 0; att < db->r[re].attribute_count; att++) {
                db->r[re].attribute_data[att] = (void**)malloc(PTRSIZE * db->r[re].data_count);
                if (db->r[re].attribute_data[att] == 0) {
                    SetTextColor(dc, RGB(255, 0, 0));
                    DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                    BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                    BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                    SetTextColor(dc, RGB(0, 0, 0));
                    return 3;
                }
                switch (db->r[re].attribute_data_type[att][0]) {
                case 'i':
                    
                    db->r[re].attribute_data[att] = (void*)malloc(4 * db->r[re].data_count);
                    if (db->r[re].attribute_data[att] == 0) {
                        SetTextColor(dc, RGB(255, 0, 0));
                        DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                        BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                        BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                        SetTextColor(dc, RGB(0, 0, 0));
                        return 3;
                    }
                    fread(db->r[re].attribute_data[att], 4 * db->r[re].data_count, 1, input);
                    break;
                case 'u':
                    
                    db->r[re].attribute_data[att] = (void*)malloc(4 * db->r[re].data_count);
                    if (db->r[re].attribute_data[att] == 0) {
                        SetTextColor(dc, RGB(255, 0, 0));
                        DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                        BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                        BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                        SetTextColor(dc, RGB(0, 0, 0));
                        return 3;
                    }
                    fread(db->r[re].attribute_data[att], 4 * db->r[re].data_count, 1, input);
                    break;
                case 'f':
                    
                    db->r[re].attribute_data[att] = (void*)malloc(8 * db->r[re].data_count);
                    if (db->r[re].attribute_data[att] == 0) {
                        SetTextColor(dc, RGB(255, 0, 0));
                        DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                        BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                        BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                        SetTextColor(dc, RGB(0, 0, 0));
                        return 3;
                    }
                    fread(db->r[re].attribute_data[att], 8 * db->r[re].data_count, 1, input);
                    break;
                case 't':
                    
                    ustr_attribute_data = (ustr*)malloc(SUSTR * db->r[re].data_count);
                    if (ustr_attribute_data == 0) {
                        SetTextColor(dc, RGB(255, 0, 0));
                        DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                        BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                        BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                        SetTextColor(dc, RGB(0, 0, 0));
                        return 3;
                    }
                    for (UINT d = 0; d < db->r[re].data_count; d++) {
                        fread(&ustr_attribute_data[d].size, 4, 1, input);
                        ustr_attribute_data[d].data = (char*)malloc(ustr_attribute_data[d].size);
                        if (ustr_attribute_data[d].data == 0) {
                            SetTextColor(dc, RGB(255, 0, 0));
                            DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                            BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                            BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                            SetTextColor(dc, RGB(0, 0, 0));
                            return 3;
                        }
                        fread(ustr_attribute_data[d].data, ustr_attribute_data[d].size, 1, input);
                    }
                    db->r[re].attribute_data[att] = ustr_attribute_data;
                    break;
                case 'v':
                    free(type_name);
                    ustr_attribute_data = (ustr*)malloc(SUSTR * db->r[re].data_count);
                    if (ustr_attribute_data == 0) {
                        SetTextColor(dc, RGB(255, 0, 0));
                        DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                        BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                        BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                        SetTextColor(dc, RGB(0, 0, 0));
                        return 3;
                    }
                    for (UINT d = 0; d < db->r[re].data_count; d++) {
                        fread(&ustr_attribute_data[d].size, 4, 1, input);
                        ustr_attribute_data[d].data = (char*)malloc(ustr_attribute_data[d].size);
                        if (ustr_attribute_data[d].data == 0) {
                            SetTextColor(dc, RGB(255, 0, 0));
                            DrawText(dc, L"Memory allocation failed", 24, &r_txt, 0);
                            BitBlt(dc, 0, 0, buffer->width, buffer->height - 17, dc, 0, 17, SRCCOPY);
                            BitBlt(dc, 0, buffer->height - 17, buffer->width, 17, dc, 0, 0, WHITENESS);
                            SetTextColor(dc, RGB(0, 0, 0));
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
        return 1;
    }
    fclose(input);
    return 0;
}

int __str_to_wstr(wchar_t * wstr,const char* str){
    size_t len = strlen(str);
    wstr = (wchar_t*)malloc(len * 2+2);
    if (wstr == 0) {
        return 1;
    }
    for (size_t i = 0; i < len; i++) {
        wstr[i] = str[i];
    }
    wstr[len] = 0;
    return 0;

}

wchar_t *__get_only_filename(wchar_t *wstr){
    size_t l= wcslen(wstr);
    if (l == 0) {
        return wstr;
    }
     wchar_t* temp=wstr+l-1;
    while((*temp!=L'/') && (*temp != L'\\')&&(l--)){
        temp--;
    }
    return   ++temp;
}

UINT getReturnButton(POINT*cur_pos,RECT* r,bitmap *buffer,int * num_rect,DataBase* db,RECT *r_wind,size_t db_columns,size_t db_rows){


    float horizontal_multiplier = float(r_wind->right - r_wind->left) / 1600.0f,
        vertical_multiplier= float(r_wind->top - r_wind->bottom) / 900.0f;
   // float y = cur_pos->y / vertical_multiplier,x= cur_pos->x / horizontal_multiplier;
    float y = cur_pos->y , x = cur_pos->x;
    float vertical_pos = 0,horizontal_pos = 0;

    if ((cur_pos->x <0 ) ||( cur_pos->x > r->right- r->left)||(cur_pos->y>r->bottom- r->top)|| (cur_pos->y < 0)) {
        return 0;
    }
    if ((cur_pos->x <203) &&(cur_pos->x > 83) && (cur_pos->y >= 0) && (cur_pos->y <53)) {
        return 1;
    }
    if ((cur_pos->x < 53) && (cur_pos->x >= 0) && (cur_pos->y >= buffer->height-53) && (cur_pos->y < buffer->height)) {
        return 2;
    }
    if ((cur_pos->x < 113) && (cur_pos->x >= 60) && (cur_pos->y >= buffer->height - 53) && (cur_pos->y < buffer->height)) {
        return 3;
    }
    if ((cur_pos->x < 205) && (cur_pos->x >= 0) && (cur_pos->y >52) && (cur_pos->y < 750)) {
        
        return 50;
    }
    if ((cur_pos->x < r_wind->right) && (cur_pos->x >= 204) && (cur_pos->y >= 0) && (cur_pos->y < 867)) {
        //db_attribute_text_box[y * 13 + x].left = 204 + x * 1396 / db_columns;
        //db_attribute_text_box[y * 13 + x].right = db_attribute_text_box[y * 13 + x].left + 1396 / db_columns - 1;
        //db_attribute_text_box[y * 13 + x].top = y * 866 / db_rows;
        //db_attribute_text_box[y * 13 + x].bottom = db_attribute_text_box[y * 13 + x].top + 866 / db_rows - 1;
        
        horizontal_pos = cur_pos->x;
        horizontal_pos = (horizontal_pos -204)/1396*db_columns ;
        //vertical_pos = vertical_pos < 0 ? -vertical_pos : vertical_pos;

        vertical_pos = cur_pos->y;
        vertical_pos = vertical_pos /866*db_rows;

        return 0xff | ((UINT(horizontal_pos)&0xff)<<8) | ((UINT(vertical_pos) & 0xff) << 16);
    }

    return 0;
}

void getReturnAttributeSelected(POINT* cur,UINT* selected) {
    if ((cur->y >= 0) && (cur->y < 20)) {
        *selected = 0;
        if ((cur->y > 203) && (cur->y <254)) {
            selected[1] = 0;
        }
    }
}
HWND h_window;

// Przekaż dalej deklaracje funkcji dołączone w tym module kodu:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int,HWND*);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    //DrawText()
    // TODO: W tym miejscu umieść kod.

    // Inicjuj ciągi globalne
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_XDBVISUALISER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    
    // Wykonaj inicjowanie aplikacji:
    if (!InitInstance(hInstance, nCmdShow, &h_window))
    {
        MessageBox(h_window, L"Window creation error", L"Error", MB_ICONSTOP);
        return FALSE;
    }

    WNDCLASSEXW win_class_help;

    win_class_help.cbSize = sizeof(WNDCLASSEX);

    win_class_help.style = CS_HREDRAW | CS_VREDRAW;
    win_class_help.lpfnWndProc = WndProc;
    win_class_help.cbClsExtra = 0;
    win_class_help.cbWndExtra = 0;
    win_class_help.hInstance = hInstance;
    win_class_help.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_XDBVISUALISER));
    win_class_help.hCursor = LoadCursor(nullptr, IDC_ARROW);
    win_class_help.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    win_class_help.lpszMenuName = MAKEINTRESOURCEW(IDC_XDBVISUALISER);
    win_class_help.lpszClassName = L"Help";
    win_class_help.hIconSm = LoadIcon(win_class_help.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&win_class_help);
    HWND hwnd_help=0; //It wont let me compile
    HDC DC_wind_help ;


    char* temp_filename = 0;
    int num_of_files = 0;
    wchar_t** file_opened_names = CommandLineToArgvW(lpCmdLine, &num_of_files);


    //num_of_files--;
    POINT cursor_pos, old_cursor_pos = { 0 };
    GetCursorPos(&old_cursor_pos);
    old_cursor_pos.x -= 8;
    old_cursor_pos.y -= 52;
    DataBase* TEMP_DB;

    bool unclosed_help = 1;


    wchar_t info_files[128] = { 0 }, info_visible[128] = {0};
    swprintf(info_files, L"%i File%c",num_of_files, num_of_files>1?L's': L'\0');
    time_t last_wait=0;
   
    int wait = 4000;
    wchar_t wstr_wait[32];
    int ctrl_hold = 0;


    HDC DC_window = GetDC(h_window), DC_bg_buffer = CreateCompatibleDC(DC_window),DC_back_buffer= CreateCompatibleDC(DC_window);
    DataBase* db=0;
    size_t min_filename_size = (size_t)0xffffffffffffffff,temp_size_filename_size=0,
        max_filename_size=0 ;
    
    wchar_t* temp_filename_return;
    bitmap buffer, bg, help[4],save[4];

    RECT info_rect, command_rect,r_or_press_shift, command_rect2;
    info_rect.left = 0;
    info_rect.top = buffer.height - 17;
    info_rect.right = buffer.width;
    info_rect.bottom = buffer.height;

    command_rect.left=0;
    command_rect.right = 1600;
    command_rect.top =900 - 17;
    command_rect.bottom = 900 ;

    command_rect2.left = 0;
    command_rect2.right = 203;
    command_rect2.top =17;
    command_rect2.bottom = 51;



    const wchar_t* wstr_MemAllocError=L"Memory allocation error";
   
    

    DrawText(DC_window, L"Opening res/bg.bmp", 18, &command_rect, 0);
    BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
    BitBlt(DC_window, 0, 900 - 17,1600, 17, DC_window, 0, 0, WHITENESS);
    switch (bmp_read(&buffer, "res/bg.bmp")) {
    case 1:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"Error opening res/bg.bmp", 24, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    case 2:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"Background memory allocation error", 34, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    }
    if (bmp_vertical_invert(&buffer)) {
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"Background invert memory allocation error",41, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
    }
    DrawText(DC_window, L"Opening res/close_0.bmp",23, &command_rect, 0);
    BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
    BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);

    DrawText(DC_window, L"Opening res/help_0.bmp", 22, &command_rect, 0);
    BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
    BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
    switch (bmp_read(help, "res/help_0.bmp")) {
    case 1:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"Error opening res/help_0.bmp", 28, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    case 2:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"help_0 memory allocation error", 30, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    }

    DrawText(DC_window, L"Opening res/help_hoover.bmp", 27, &command_rect, 0);
    BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
    BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
    switch (bmp_read(&help[1], "res/help_hoover.bmp")) {
    case 1:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"Error opening res/help_hoover.bmp", 33, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    case 2:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"help_hoover memory allocation error", 35, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    }
    DrawText(DC_window, L"Opening res/help_1.bmp", 22, &command_rect, 0);
    BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
    BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
    switch (bmp_read(&help[2], "res/help_1.bmp")) {
    case 1:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"Error opening res/help_1.bmp", 28, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    case 2:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"help_1 memory allocation error", 30, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    }
    DrawText(DC_window, L"Opening res/help_blocked.bmp", 28, &command_rect, 0);
    BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
    BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
    switch (bmp_read(&help[3], "res/help_blocked.bmp")) {
    case 1:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"Error opening res/help_blocked.bmp", 34, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    case 2:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"help_blocked memory allocation error", 36, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    }
    DrawText(DC_window, L"Opening res/save_0.bmp", 22, &command_rect, 0);
    BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
    BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
    switch (bmp_read(save, "res/save_0.bmp")) {
    case 1:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"Error opening res/save_0.bmp", 28, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    case 2:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"save_0 memory allocation error", 30, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    }
    DrawText(DC_window, L"Opening res/save_1.bmp", 22, &command_rect, 0);
    BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
    BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
    switch (bmp_read(&save[2], "res/save_1.bmp")) {
    case 1:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"Error opening res/save_1.bmp", 28, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    case 2:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"save_1 memory allocation error", 30, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    }
    DrawText(DC_window, L"Opening res/save_hoover.bmp", 27, &command_rect, 0);
    BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
    BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
    switch (bmp_read(&save[1], "res/save_hoover.bmp")) {
    case 1:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"Error opening res/save_hoover.bmp", 33, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    case 2:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"save_hoover memory allocation error", 35, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    }
    DrawText(DC_window, L"Opening res/save_blocked.bmp", 28, &command_rect, 0);
    BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
    BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
    switch (bmp_read(&save[3], "res/save_blocked.bmp")) {
    case 1:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"Error opening res/save_blocked.bmp", 34, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    case 2:
        SetTextColor(DC_window, RGB(255, 0, 0));
        DrawText(DC_window, L"save_blocked memory allocation error", 36, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
        break;
    }

    
    DrawText(DC_window, info_files, -1, &command_rect, 0);
    /*
    if (KEY_CHECK_STATE&GetAsyncKeyState(VK_CONTROL)) {
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        while (wait) {
            if ((clock() - last_wait) > 1000) {
                swprintf(wstr_wait, L"Wait time : %i ms", wait);
                BitBlt(DC_window, 0, buffer.height - 17, buffer.width, 17, DC_window, 0, 0, WHITENESS);
                DrawText(DC_window, wstr_wait, -1, &command_rect, 0);
                wait--;
                if (GetAsyncKeyState(VK_CONTROL)) { wait++; }
            }
        }
    }
    */
    
 
    if(num_of_files>0){
        if (_wcsicmp(file_opened_names[0], L"/c")==0) {
            num_of_files--;
            db = (DataBase*)malloc(SDATABASE * num_of_files);
            if (!db) {
                DrawText(DC_window, wstr_MemAllocError, 23, &command_rect, 0);
                BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
                BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
                Sleep(10000);
                return -1;
            }


        
        }
        else {
            for (int i = 0; i < num_of_files; i++) {
                BitBlt(DC_window, 0, 0, buffer.width, buffer.height - 17, DC_window, 0, 17, SRCCOPY);
                BitBlt(DC_window, 0, buffer.height - 17, buffer.width, 17, DC_window, 0, 0, WHITENESS);
                DrawText(DC_window, file_opened_names[i], -1, &command_rect, 0);

                temp_size_filename_size = wcslen(file_opened_names[i]);
                if (temp_size_filename_size < min_filename_size) {
                    min_filename_size = temp_size_filename_size;
                }
                if (temp_size_filename_size > max_filename_size) {
                    max_filename_size = temp_size_filename_size;
                }
            }

            if (KEY_CHECK_STATE & GetAsyncKeyState(VK_CONTROL)) {
                BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
                BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
                while (wait) {
                    if ((clock() - wait) > 1000) {
                        swprintf(wstr_wait, L"Wait time : %i ms", wait);
                        BitBlt(DC_window, 0, buffer.height - 17, buffer.width, 17, DC_window, 0, 0, WHITENESS);
                        DrawText(DC_window, wstr_wait, -1, &command_rect, 0);
                    }
                    wait--;
                    if (KEY_CHECK_STATE & GetAsyncKeyState(VK_CONTROL)) { wait++; }
                }

            }
            BitBlt(DC_window, 0, 0, buffer.width, buffer.height - 17, DC_window, 0, 17, SRCCOPY);
            BitBlt(DC_window, 0, buffer.height - 17, buffer.width, 17, DC_window, 0, 0, WHITENESS);
            DrawText(DC_window, L"________________________________________________________", 56, &command_rect, 0);
            //SetTextColor(DC_window, RGB(192, 192, 192));
            last_wait = clock();

            SetTextColor(DC_window, RGB(0, 0, 0));
            //For DB_readfile
            temp_filename = (char*)malloc(max_filename_size + 1);
            if (temp_filename == 0) {
                SetTextColor(DC_window, RGB(255, 0, 0));
                DrawText(DC_window, wstr_MemAllocError, 23, &command_rect, 0);
                BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
                BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
                Sleep(10000);
                return -1;
            }
            temp_filename_return = (wchar_t*)malloc(sizeof(wchar_t) * (max_filename_size + 16));
            if (temp_filename_return == 0) {
                DrawText(DC_window, wstr_MemAllocError, 23, &command_rect, 0);
                BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
                BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
                Sleep(10000);
                return -1;
            }
            memcpy(temp_filename_return, L"Failed opening ", sizeof(wchar_t) * 15);
            temp_filename_return[max_filename_size + 15] = 0;
            db = (DataBase*)malloc(SDATABASE * num_of_files);
            if (!db ) {
                DrawText(DC_window, wstr_MemAllocError, 23, &command_rect, 0);
                BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
                BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
                Sleep(10000);
                return -1;
            }
            max_filename_size++;
            for (int i = 0; i < num_of_files; i++) {
                memset(temp_filename, 0, max_filename_size);
                wcstombs(temp_filename, file_opened_names[i], wcslen(file_opened_names[i]));
                switch (DB_readfile_write(&db[i], temp_filename, DC_window, &buffer)) {
                case 1:
                    SetTextColor(DC_window, RGB(255, 0, 0));
                    DrawText(DC_window, L"Error", -1, &command_rect, 0);
                    BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
                    BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
                    SetTextColor(DC_window, RGB(0, 0, 0));
                    wait = 2000;

                    num_of_files--;
                    swprintf(info_files, L"%i File%c", num_of_files, num_of_files > 1 ? L's' : L'\0');
                    TEMP_DB = (DataBase*)malloc(SDATABASE * num_of_files);
                    if (TEMP_DB == 0) {
                        DrawText(DC_window, wstr_MemAllocError, 23, &command_rect, 0);
                        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
                        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
                        Sleep(10000);
                        return -1;
                    }
                    memcpy(TEMP_DB, db, SDATABASE * i);
                    memcpy(TEMP_DB + i, db + i + 1, SDATABASE * (num_of_files - i));
                    free(db);
                    db = TEMP_DB;

                    break;
                case 2:
                    num_of_files--;
                    swprintf(info_files, L"%i File%c", num_of_files, num_of_files > 1 ? L's' : L'\0');
                    memcpy(temp_filename_return + 15, file_opened_names[i], sizeof(wchar_t) * wcslen(file_opened_names[i]));
                    //MessageBox(h_window, temp_filename_return, L"Error", MB_ICONSTOP);
                    TEMP_DB = (DataBase*)malloc(SDATABASE * num_of_files);
                    if (TEMP_DB == 0) {
                        DrawText(DC_window, wstr_MemAllocError, 23, &command_rect, 0);
                        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
                        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
                        Sleep(10000);
                        return -1;
                    }
                    memcpy(TEMP_DB, db, SDATABASE * i);
                    memcpy(TEMP_DB + i, db + i + 1, SDATABASE * (num_of_files - i));
                    free(db);
                    db = TEMP_DB;
                    break;
                case 3:
                    BitBlt(DC_window, 0, 0, buffer.width, buffer.height - 17, DC_window, 0, 17, SRCCOPY);
                    BitBlt(DC_window, 0, buffer.height - 17, buffer.width, 17, DC_window, 0, 0, WHITENESS);
                    DrawText(DC_window, L"Memory allocation error", 23, &command_rect, 0);
                    num_of_files--;
                    swprintf(info_files, L"%i File%c", num_of_files, num_of_files > 1 ? L's' : L'\0');
                    TEMP_DB = (DataBase*)malloc(SDATABASE * num_of_files);
                    if (TEMP_DB == 0) {
                        DrawText(DC_window, wstr_MemAllocError, 23, &command_rect, 0);
                        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
                        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
                        Sleep(10000);
                        return -1;
                    }
                    memcpy(TEMP_DB, db, SDATABASE * i);
                    memcpy(TEMP_DB + i, db + i, SDATABASE * (num_of_files - i));
                    free(db);
                    db = TEMP_DB;
                    break;


                }
                memset(temp_filename, 0, max_filename_size);
            }
        }
    }
    

    
    r_or_press_shift.left = 150;
    r_or_press_shift.right = 1600 - 17;
    r_or_press_shift.top = 900 - 17;
    r_or_press_shift.bottom = 900;
    //system("pause");
    swprintf(wstr_wait, L"Or press shift", wait);
    BitBlt(DC_window, 0, buffer.height - 17, 150, 17, DC_window, 0, 0, WHITENESS);
    DrawText(DC_window, wstr_wait, -1, &r_or_press_shift, 0);


 
    wait = 10000;
    while (wait&&!GetAsyncKeyState(VK_SHIFT)) {
            if (GetAsyncKeyState(VK_CONTROL)) {
                wait++;
            }
            if ((clock() - last_wait) >=1) {
                swprintf(wstr_wait, L"Wait time : %i ms", wait/10);
                BitBlt(DC_window, 0, buffer.height - 17, 150, 17, DC_window, 0, 0, WHITENESS);
                DrawText(DC_window, wstr_wait, -1, &command_rect, 0);
                wait--;
            }
    }

   
    command_rect.left = 0;
    command_rect.right = buffer.width;
    command_rect.top = 0;
    command_rect.bottom = 17;
    

    if (bmp_vertical_invert(&help[3])) {
        DrawText(DC_window, L"close_blocked memory allocation error", 37, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
    }
    if (bmp_vertical_invert(&help[2])) {
        DrawText(DC_window, L"help_1 memory allocation error",30, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
    }
    if (bmp_vertical_invert(&help[1])) {
        DrawText(DC_window, L"help_hoover memory allocation error", 35, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
    }
    if (bmp_vertical_invert(help)) {
        DrawText(DC_window, L"help_0 memory allocation error", 30, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
    }

    if (bmp_vertical_invert(save)) {
        DrawText(DC_window, L"save_0 memory allocation error", 30, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
    }
    if (bmp_vertical_invert(&save[1])) {
        DrawText(DC_window, L"save_hoover memory allocation error", 35, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
    }
    if (bmp_vertical_invert(&save[2])) {
        DrawText(DC_window, L"save_1 memory allocation error", 30, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
    }
    if (bmp_vertical_invert(&save[3])) {
        DrawText(DC_window, L"save_blocked memory allocation error", 36, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
    }

    
    HBITMAP HBMP = CreateBitmap(1600, 900, 1, 32,buffer.buffer);
    SelectObject(DC_bg_buffer, HBMP);
    srand(time(NULL));

    HBITMAP HBMP_close[3],HBMP_help[4],HBMP_save[4];

    HDC DC_close[3],DC_help[4],DC_save[4] ;


    //help
    DC_help[0] = CreateCompatibleDC(DC_window);
    DC_help[1] = CreateCompatibleDC(DC_window);
    DC_help[2] = CreateCompatibleDC(DC_window);
    DC_help[3] = CreateCompatibleDC(DC_window);
    HBMP_help[0]= CreateBitmap(help[0].width, help[0].height, 1, 32, help[0].buffer);
    HBMP_help[1] = CreateBitmap(help[1].width, help[1].height, 1, 32, help[1].buffer);
    HBMP_help[2] = CreateBitmap(help[2].width, help[2].height, 1, 32, help[2].buffer);
    HBMP_help[3] = CreateBitmap(help[3].width, help[3].height, 1, 32, help[3].buffer);
    SelectObject(DC_help[0], HBMP_help[0]);
    SelectObject(DC_help[1], HBMP_help[1]);
    SelectObject(DC_help[2], HBMP_help[2]);
    SelectObject(DC_help[3], HBMP_help[3]);

    //save
    DC_save[0] = CreateCompatibleDC(DC_window);
    DC_save[1] = CreateCompatibleDC(DC_window);
    DC_save[2] = CreateCompatibleDC(DC_window);
    DC_save[3] = CreateCompatibleDC(DC_window);
    HBMP_save[0] = CreateBitmap(save[0].width, save[0].height, 1, 32, save[0].buffer);
    HBMP_save[1] = CreateBitmap(save[1].width, save[1].height, 1, 32, save[1].buffer);
    HBMP_save[2] = CreateBitmap(save[2].width, save[2].height, 1, 32, save[2].buffer);
    HBMP_save[3] = CreateBitmap(save[3].width, save[3].height, 1, 32, save[3].buffer);
    SelectObject(DC_save[0], HBMP_save[0]);
    SelectObject(DC_save[1], HBMP_save[1]);
    SelectObject(DC_save[2], HBMP_save[2]);
    SelectObject(DC_save[3], HBMP_save[3]);

   
    SetTextColor(DC_window, RGB(255, 255, 255));
    SetBkMode(DC_window, TRANSPARENT);



    wchar_t *only_filename = 0;
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_XDBVISUALISER));

    


   
    int last_db_selected = 0;

    
    
    //num_of_files--;
    int num_rect = 750<(num_of_files * 20)?37: num_of_files;
    RECT *draw_rect=(RECT*)malloc(sizeof(RECT)* num_rect);
    if (draw_rect == 0) {
        MessageBox(h_window, L"DRAW_RECT invert memory allocation error", L"ERROR", MB_ICONSTOP);
        return -1;
    }
    for (int i = 0; i < num_rect; i++) {
        draw_rect[i].left = 0;
        draw_rect[i].right = 204;
        draw_rect[i].top = i*20+54;
        draw_rect[i].bottom = i*20+74;
    }
    
    MSG msg;
    BitBlt(DC_window, 0, 0, 1600, 900, DC_bg_buffer, 0, 0, SRCCOPY);
    DrawText(DC_window, info_files, -1, &command_rect, 0);
    // Główna pętla komunikatów:

    int filename_select_x = 0, filename_select_y=0;
    UINT filename_list_change = 0;
    time_t last_left = 0, last_right=0,last_up=0,last_down=0,last_lmb=0;


    bool help_on = 0;

    UINT x_button_redraw = 0;
    RECT W_RECT;
    GetWindowRect(h_window, &W_RECT);
    BitBlt(DC_window, 0, buffer.height - 53, help[0].width, help[0].height, DC_help[0], 0, 0, SRCCOPY);
    BitBlt(DC_window, 60, buffer.height - 53, save[0].width, save[0].height, DC_save[0], 0, 0, SRCCOPY);
   

    const wchar_t* test_txt;
    UINT db_selected = 0, db_columns = 0, db_rows = 0, db_draw = 0,
        db_horizontal_offset = 0, db_vertical_offset = 0;
    UINT* relation_selected = (UINT*)calloc(num_of_files , 4),
        * relation_offsets = (UINT*)calloc(num_of_files, 4);
    if (!relation_selected) {
        DrawText(DC_window, L"Relation_selected memory allocation error", 31, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
    }
    if (!relation_offsets) {
        DrawText(DC_window, L"Relation_selected memory allocation error", 31, &command_rect, 0);
        BitBlt(DC_window, 0, 0, 1600, 900 - 17, DC_window, 0, 17, SRCCOPY);
        BitBlt(DC_window, 0, 900 - 17, 1600, 17, DC_window, 0, 0, WHITENESS);
        Sleep(10000);
        return -1;
    }
    
    /*
    ES_Array **highlighted_elements;
    highlighted_elements = (ES_Array**)malloc(PTRSIZE * num_of_files);
    if (!highlighted_elements) { return 1; }
    for (int i = 0; i < num_of_files; i++) {
        highlighted_elements[i] = (ES_Array*)calloc(db[i].count,ESSIZE );
        if (!highlighted_elements[i]) { return 1; }
    }
    */
    for (int i = 0; i < num_rect; i++) {
        only_filename = __get_only_filename(file_opened_names[i]) ;
        if (i == db_selected) {
            SetTextColor(DC_window, RGB(0, 0, 0));
            SetBkMode(DC_window, OPAQUE);
            DrawText(DC_window, only_filename, -1, &draw_rect[i - filename_select_y], 0);
            SetTextColor(DC_window, RGB(255, 255, 255));
            SetBkMode(DC_window, TRANSPARENT);
        }
        else {
            DrawText(DC_window, only_filename, -1, &draw_rect[i - filename_select_y], 0);
        }
    }
    RECT db_attribute_text_box[13*43];
    RECT relation_text_box[13];
    UINT relation_rows = 0;
    wchar_t db_print[400] = { 0 },gunwo[54];
   

    size_t start_x=0, start_y = 0;

    int *int_ptr=0;
    UINT* uint_ptr=0;
    double *f64_ptr;
   
    UINT xy[2], old_xy[2] = {0,0};
    size_t slen = 0;
    UINT gRB_return = 0;

    if (num_of_files > 0) {
        //43 =(int) 866/20
        //13=(int) 1396/100
        db_rows = db[db_selected].r[relation_selected[db_selected]].data_count + 1 > 43 ? 43 : db[db_selected].r[relation_selected[db_selected]].data_count + 1;
        db_columns = db[db_selected].r[relation_selected[db_selected]].attribute_count > 13 ? 13 : db[db_selected].r[relation_selected[db_selected]].attribute_count;
        //DB_savefile(db->)
        for (int y = 0; y < db_rows; y++) {
            for (int x = 0; x < db_columns; x++) {
                db_attribute_text_box[y * 13 + x].left = 204 + x * 1396 / db_columns;
                db_attribute_text_box[y * 13 + x].right = db_attribute_text_box[y * 13 + x].left + 1396 / db_columns - 1;
                db_attribute_text_box[y * 13 + x].top = y * 866 / db_rows;
                db_attribute_text_box[y * 13 + x].bottom = db_attribute_text_box[y * 13 + x].top + 866 / db_rows - 1;
                if (y != 0) {
                    draw_text_box(DC_window, db, db_selected, relation_selected, y, x, start_y, &db_attribute_text_box[y * 13 + x]);
                }
                else {
                    slen=swprintf(db_print, L"%.*hs", db[db_selected].r[relation_selected[db_selected]].attribute_name_size[x] > 175 ? 175 : db[db_selected].r[relation_selected[db_selected]].attribute_name_size[x]
                        , db[db_selected].r[relation_selected[db_selected]].attribute_name[x]);
                    DrawText(DC_window, db_print, slen, &db_attribute_text_box[y * 13 + x], 0);
                }
                //vertical
                BitBlt(DC_window, 204 + (x + 1) * 1396 / db_columns - 1, 0, 1, 866, DC_window, 0, 0, WHITENESS);
            }
            //horizontal
            BitBlt(DC_window, 204, (y + 1) * 866 / db_rows - 1, 1396, 1, DC_window, 0, 0, WHITENESS);

        }


        if (db[db_selected].count) {
            relation_rows = db[db_selected].count > 13 ? 13 : db[db_selected].count;
            for (UINT x = 0; x < relation_rows; x++) {
                relation_text_box[x].left = 204 + x * 1396 / relation_rows;
                relation_text_box[x].right = relation_text_box[x].left + 1396 / relation_rows - 1;
                relation_text_box[x].left = ((relation_text_box[x].left + relation_text_box[x].right) >> 1)-(db[db_selected].r[x].name_size >> 1) * 15;
                relation_text_box[x].top = 873;
                relation_text_box[x].bottom = 895;
                swprintf(db_print, L"%.*hs", db[db_selected].r[x].name_size > 175 ? 175 : db[db_selected].r[x].name_size
                    , db[db_selected].r[x].name);
                DrawText(DC_window, db_print, -1, &relation_text_box[x], 0);
                BitBlt(DC_window, 204 + (x + 1) * 1396 / relation_rows - 1, 0, 1, 866, DC_window, 0, 0, WHITENESS);
            }

          

        }
    }
    memset(db_print, 0, sizeof(db_print));
    POINT temp_cur_pos;
    ustr* text_ptr=0;
    RECT r_wind;
    last_db_selected = db_selected;
    int last_r_selected;
    int py, px;
    if (relation_selected) {
        last_r_selected = relation_selected[db_selected];
    }
    GetWindowRect(h_window, &r_wind);
    RECT r_highlight,r_last_highlight;
    int refresh_highlight=0,last_highligted=0,highlight_innit=0;
    int temp;
    int file_change = 0;
    int quick_mode = 0;
    memset(&r_highlight, 0, sizeof(RECT));
    memset(&r_last_highlight, 0, sizeof(RECT));
    while (GetMessage(&msg, nullptr, 0, 0)){
       if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)){
           TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
        //GetWindowRect(h_window, &r_wind);
        GetCursorPos(&cursor_pos);
        memcpy(&temp_cur_pos, &cursor_pos, sizeof(POINT));
        cursor_pos.x -=  8 + r_wind.left;
        cursor_pos.y -=  52 + r_wind.top;
        scroll /= -60;
        if (num_of_files > 0) {
            if (scroll < 0) {
                slen = start_y;
                if (-scroll > start_y) {
                    start_y = 0;
                    db_draw = slen != start_y;
                }
                else {
                    start_y += scroll;
                    db_draw = 1;
                }
            }
            if (scroll > 0) {
                if (start_y + db_rows + scroll <= db[db_selected].r->data_count) {
                    slen = start_y;
                    start_y += scroll;
                    db_draw =1;
                }
                else {
                    temp = db[db_selected].r->data_count - db_rows;
                    start_y = temp;
                    db_draw = 1;
                }
                
                
            }
        }
        

        if(KEY_CHECK_STATE & GetAsyncKeyState(VK_LEFT)){
            if (clock() - last_left > 100) {
            filename_select_x = filename_select_x == 0 ? 0 : filename_select_x - 1;
            filename_list_change = 1;
            }
            last_left =clock();
        }
        if (KEY_CHECK_STATE & GetAsyncKeyState(VK_RIGHT)) {
            if (clock() - last_right > 100) {
                filename_select_x= filename_select_x< min_filename_size ? filename_select_x+1: filename_select_x;
                filename_list_change = 1;
            }
            last_right = clock();
        }
        if (KEY_CHECK_STATE & GetAsyncKeyState(VK_UP)) {
            if (clock() - last_up > 100) {
                filename_select_y = num_rect + filename_select_y < num_of_files ? filename_select_y + 1 : filename_select_y;
                filename_list_change = 1;
            }
            last_up = clock();
        }
        if (KEY_CHECK_STATE & GetAsyncKeyState(VK_DOWN)) {
            if (clock() - last_down > 100) {
                filename_select_y = filename_select_y ==0 ? filename_select_y  : filename_select_y - 1;
                filename_list_change = 1;
            }
            last_down = clock();
        }

        if (KEY_CHECK_STATE & GetAsyncKeyState(VK_F2)) {
            quick_mode ^= 1;
        }

        if ((num_of_files > 0) && (db_draw)) {
            //43 =(int) 866/20
            //13=(int) 1396/100
           
            //DC_buffer
            if (!file_change  && quick_mode) {
                SetBkMode(DC_window, OPAQUE);
                SetBkColor(DC_window, RGB(0, 0, 255));
                if (refresh_highlight) {
                    if (r_highlight.left < r_last_highlight.left) {
                        for (int y = r_last_highlight.top; y <= r_last_highlight.bottom; y++) {
                            py = y - start_y + 1;
                            for (int x = r_highlight.left; x <= r_last_highlight.left; x++) {
                                 draw_text_box(DC_window, db, db_selected, relation_selected, py, x, start_y, &db_attribute_text_box[(py) * 13 + x]); 
                            }
                        }
                        r_last_highlight.left = r_highlight.left;
                    }
                    if (r_highlight.right > r_last_highlight.right) {
                        for (int y = r_last_highlight.top; y <= r_last_highlight.bottom; y++) {
                            py = y - start_y + 1;
                            for (int x = r_last_highlight.right; x <= r_highlight.right; x++) {
                               draw_text_box(DC_window, db, db_selected, relation_selected, py, x, start_y, &db_attribute_text_box[(py) * 13 + x]); 
                            }
                        }
                        r_last_highlight.right = r_highlight.right;
                    }
                    if (r_highlight.top < r_last_highlight.top) {
                        for (int y = r_highlight.top; y <= r_last_highlight.top; y++) {
                            py = y - start_y + 1;
                            for (int x = r_last_highlight.left; x <= r_last_highlight.right; x++) {
                                draw_text_box(DC_window, db, db_selected, relation_selected, py, x, start_y, &db_attribute_text_box[(py) * 13 + x]); 
                            }
                        }
                        r_last_highlight.top = r_highlight.top;
                    }
                    if (r_highlight.bottom > r_last_highlight.bottom) {
                        for (int y = r_last_highlight.bottom; y <= r_highlight.bottom; y++) {
                            py = y - start_y + 1;
                            for (int x = r_last_highlight.left; x <= r_last_highlight.right; x++) {
                                 draw_text_box(DC_window, db, db_selected, relation_selected, py, x, start_y, &db_attribute_text_box[(py) * 13 + x]); 
                            }
                        }
                        r_last_highlight.bottom = r_highlight.bottom;
                    }
                }
                last_highligted = 0;
                memcpy(&r_highlight, &r_last_highlight, sizeof(r_highlight));
                SetBkMode(DC_window, TRANSPARENT);
                SetBkColor(DC_window, RGB(255,255,255));
            if(scroll>0){
               //Scroll down
               py = scroll *( 866 / db_rows);
               BitBlt(DC_window, 204,20, 1396, 846-py, DC_window, 204,20+py, SRCCOPY);
               BitBlt(DC_window, 204, 866 - py, 1396,  py, 0, 0,0, BLACKNESS);
              memcpy(&r_last_highlight, &r_highlight, sizeof(r_highlight));
               for (int y = db_rows - scroll; y < db_rows; y++) {
                   for (int x = 0; x < db_columns; x++) {
                       if (r_highlight.top <= y + start_y && r_highlight.bottom >= y + start_y && r_highlight.left <= x && r_highlight.right >= x) {
                           if (!last_highligted) {
                               SetBkMode(DC_window, OPAQUE);
                               SetBkColor(DC_window, RGB(0, 0, 255));
                               last_highligted = 1;
                           }
                       }
                       else {
                           if (last_highligted) {   
                               SetBkColor(DC_window, RGB(255, 255, 255));
                               SetBkMode(DC_window, TRANSPARENT);
                               last_highligted = 0;
                           }
                       }
                       draw_text_box(DC_window, db, db_selected, relation_selected, y, x, start_y, &db_attribute_text_box[y * 13 + x]);
                       BitBlt(DC_window, 204 + (x + 1) * 1396 / db_columns - 1, 0, 1, 866, DC_window, 0, 0, WHITENESS);
                   }
                   BitBlt(DC_window, 204, (y + 1) * 866 / db_rows - 1, 1396, 1, DC_window, 0, 0, WHITENESS);
               }
            }
            if (scroll< 0) {
                //Scroll up
                scroll = -scroll;
                py = scroll * (866 / db_rows);
                BitBlt(DC_window, 204,20 +py, 1396, 846 -py, DC_window, 204, 20, SRCCOPY);
                BitBlt(DC_window, 204, 20, 1396, py, 0, 0, 0, BLACKNESS);
                for (int y = 1 ; y < scroll+1; y++) {
                    for (int x = 0; x < db_columns; x++) {
                        if (r_highlight.top <= y + start_y && r_highlight.bottom >= y + start_y && r_highlight.left <= x && r_highlight.right >= x) {
                            if (!last_highligted) {
                                SetBkMode(DC_window, OPAQUE);
                                SetBkColor(DC_window, RGB(0, 0, 255));
                                last_highligted = 1;
                            }
                        }
                        else {
                            if (last_highligted) {
                                SetBkColor(DC_window, RGB(255, 255, 255));
                                SetBkMode(DC_window, TRANSPARENT);
                                last_highligted = 0;
                            }
                        }
                        draw_text_box(DC_window, db, db_selected, relation_selected, y, x, start_y, &db_attribute_text_box[y * 13 + x]);
                        BitBlt(DC_window, 204 + (x + 1) * 1396 / db_columns - 1, 0, 1, 866, DC_window, 0, 0, WHITENESS);
                    }
                    BitBlt(DC_window, 204, (y + 1) * 866 / db_rows - 1, 1396, 1, DC_window, 0, 0, WHITENESS);
                }
                
            }
            }
            else {
                //memset(&r_highlight, 0, sizeof(RECT));
                //memset(&r_last_highlight, 0, sizeof(RECT));
                file_change = 0;
            BitBlt(DC_window, 204, 0, 1396, 866, 0, 0, 0, BLACKNESS);
            memset(db_print, 0, 400 * SWCHAR);
            memset(db_attribute_text_box, 0, sizeof(RECT)* 13 * 43);
            command_rect2.left = 0;
            command_rect2.right = 200;
            command_rect2.top = 17;
            command_rect2.bottom = 51;
            SetBkMode(DC_window, TRANSPARENT);
            SetBkColor(DC_window, RGB(0, 0, 255));
            last_highligted = 0;
            db_rows = db[db_selected].r[relation_selected[db_selected]].data_count + 1 > 43 ? 43 : db[db_selected].r[relation_selected[db_selected]].data_count + 1;
            db_columns = db[db_selected].r[relation_selected[db_selected]].attribute_count > 13 ? 13 : db[db_selected].r[relation_selected[db_selected]].attribute_count;
            for (int y = 0; y < db_rows; y++) {
                for (int x = 0; x < db_columns; x++) {
                    db_attribute_text_box[y * 13 + x].left = 204 + x * 1396 / db_columns;
                    db_attribute_text_box[y * 13 + x].right = db_attribute_text_box[y * 13 + x].left + 1396 / db_columns - 1;
                    db_attribute_text_box[y * 13 + x].top = y * 866 / db_rows;
                    db_attribute_text_box[y * 13 + x].bottom = db_attribute_text_box[y * 13 + x].top + 866 / db_rows - 1;
                    if (y != 0) {
                        py = y - 1+ start_y;
                        if (r_highlight.top <= py   && r_highlight.bottom >= py && r_highlight.left <= x && r_highlight.right >= x) {
                            if (!last_highligted) {
                                SetBkMode(DC_window, OPAQUE);
                                SetBkColor(DC_window, RGB(0, 0, 255));
                                last_highligted = 1;
                            }
                        }
                        else {
                            if (last_highligted) {
                                SetBkMode(DC_window, TRANSPARENT);
                                last_highligted = 0;
                            }
                        }
                        draw_text_box(DC_window, db, db_selected, relation_selected, y, x, start_y, &db_attribute_text_box[y * 13 + x]);
                    }
                    else {
                        slen=swprintf(db_print, L"%.*hs", db[db_selected].r[relation_selected[db_selected]].attribute_name_size[x] > 175 ? 175 : db[db_selected].r[relation_selected[db_selected]].attribute_name_size[x]
                           , db[db_selected].r[0].attribute_name[x]);
                        DrawText(DC_window, db_print, slen, &db_attribute_text_box[y * 13 + x], 0);
                    }
                    BitBlt(DC_window, 204 + (x + 1) * 1396 / db_columns - 1, 0, 1, 866, DC_window, 0, 0, WHITENESS);
                }
                BitBlt(DC_window, 204, (y + 1) * 866 / db_rows - 1, 1396, 1, DC_window, 0, 0, WHITENESS);
            }
            }
            SetBkMode(DC_window, TRANSPARENT);
            SetBkColor(DC_window, RGB(255, 255, 255));
            last_highligted = 0;
            db_draw = 0;
            BitBlt(DC_window, 0, 17, 200, 34, DC_bg_buffer, 0, 17, SRCCOPY);
            slen = swprintf(info_visible, L"%u->%u/%u      ", start_y, start_y + db_rows-1, db[db_selected].r[relation_selected[db_selected]].data_count-1);
            DrawText(DC_window, info_visible, slen, &command_rect2, 0);
        }
        if(filename_list_change || (db_draw)){
            BitBlt(DC_window, 0, 0, 204, 846, DC_bg_buffer, 0, 0, SRCCOPY);
            DrawText(DC_window, info_files, -1, &command_rect, 0);
            for (int i = filename_select_y; i < num_rect; i++) {
                only_filename = __get_only_filename(file_opened_names[i])+ filename_select_x;
                if (i == db_selected) {
                    SetTextColor(DC_window, RGB(0,0, 0));
                    SetBkMode(DC_window, OPAQUE);
                    SetBkColor(DC_window, RGB(255, 255, 255));
                    DrawText(DC_window, only_filename, -1, &draw_rect[i - filename_select_y], 0);
                    SetTextColor(DC_window, RGB(255, 255, 255));
                    SetBkMode(DC_window, TRANSPARENT);
                }
                else {
                    DrawText(DC_window, only_filename, -1, &draw_rect[i - filename_select_y], 0);
                }
            }
            filename_list_change = 0;
            db_draw = 1;
        }
            gRB_return = getReturnButton(&cursor_pos, &r_wind, &buffer, &num_rect, &db[db_selected], &r_wind, db_columns, db_rows);
            if ((gRB_return & 0xff)==0xff) {
                xy[0] = (gRB_return & 0xff00)>>8;
                xy[1] = (gRB_return & 0xff0000)>>16;
                if (mouse_input==1) {
                    SetBkMode(DC_window, OPAQUE);
                    SetBkColor(DC_window, RGB(0, 0, 255));
                    if(!highlight_innit){
                        r_highlight.left = xy[0];
                        r_highlight.right = xy[0];
                        r_highlight.top = xy[1] - 1 + start_y;
                        r_highlight.bottom = xy[1] - 1 + start_y;
                        memcpy(&r_last_highlight, &r_highlight, sizeof(RECT));
                        highlight_innit = 1;
                    }
                    else{
                    refresh_highlight = 0;
                    if (xy[0] < r_highlight.left) {
                        r_highlight.left = xy[0];
                        refresh_highlight = 1;
                    }
                    if (xy[0] > r_highlight.right) {
                        r_highlight.right = xy[0];
                        refresh_highlight = 1;
                    }
                    if (xy[1] + start_y < r_highlight.top) {
                        r_highlight.top = xy[1]+ start_y;
                        refresh_highlight = 1;
                    }
                    if (xy[1]+ start_y > r_highlight.bottom) {
                        r_highlight.bottom = xy[1]  + start_y;
                        refresh_highlight = 1;
                    }
                    if (refresh_highlight &&xy[0] < db[db_selected].r[relation_selected[db_selected]].attribute_count) {
                        db_draw = 1;
                    }
                    }
                    SetBkMode(DC_window, TRANSPARENT);
                    SetBkColor(DC_window, RGB(255, 255, 255));
                }
            }
            else {
            switch (gRB_return) {
            case 0:
                switch(x_button_redraw){
                case 2:
                    if (!help_on) {
                        BitBlt(DC_window, 0, buffer.height - 53, help[0].width, help[0].height, DC_help[0], 0, 0, SRCCOPY);
                    }
                    break;
                case 3:
                    BitBlt(DC_window, 60, buffer.height - 53, save[0].width, save[0].height, DC_save[0], 0, 0, SRCCOPY);
                    break;
                }
                x_button_redraw = 0;
                break;
            case 1:
                break;
            case 2:
                if (!help_on) {
                    BitBlt(DC_window, 0, buffer.height - 53, help[1].width, help[1].height, DC_help[1], 0, 0, SRCCOPY);
                    if (mouse_input == 1) {
                        BitBlt(DC_window, 0, buffer.height - 53, help[2].width, help[2].height, DC_help[2], 0, 0, SRCCOPY);
                        hwnd_help = CreateWindowW(L"Help", L"Help", WS_OVERLAPPEDWINDOW,
                            544, 194, 512 + 16, 512 + 56, nullptr, nullptr, hInstance, nullptr);
                        if (!hwnd_help){
                            MessageBox(h_window, L"Help Window creation error", L"Error", MB_ICONSTOP);
                            return FALSE;
                        }
                        ShowWindow(hwnd_help, nCmdShow);
                        UpdateWindow(hwnd_help);
                        DC_wind_help = GetDC(hwnd_help);
                        render_help_window(DC_wind_help, &unclosed_help, *DC_help);
                        help_on = 1;
                        BitBlt(DC_window, 0, buffer.height - 53, help[3].width, help[3].height, DC_help[3], 0, 0, SRCCOPY);
                    }
                    x_button_redraw = 2;
                }
                break;
            case 3:
                BitBlt(DC_window, 60, buffer.height - 53, save[1].width, save[1].height, DC_save[1], 0, 0, SRCCOPY);
                if (mouse_input == 1)  {
                    BitBlt(DC_window, 60, buffer.height - 53, save[2].width, save[2].height, DC_save[2], 0, 0, SRCCOPY);
                    if((db!=0)&&(num_of_files>0)){
                    memset(temp_filename, 0, max_filename_size);
                    wcstombs(temp_filename, file_opened_names[0], wcslen(file_opened_names[0]));
                    BitBlt(DC_window, 60, buffer.height - 53, save[3].width, save[3].height, DC_save[3], 0, 0, SRCCOPY);
                    DB_savefile(db, temp_filename);
                    }
                }
                x_button_redraw = 3;
                break;
            case 50:
                switch (x_button_redraw) {
                case 2:
                    if (!help_on) {
                        BitBlt(DC_window, 0, buffer.height - 53, help[0].width, help[0].height, DC_help[0], 0, 0, SRCCOPY);
                    }
                    break;
                case 3:
                    BitBlt(DC_window, 60, buffer.height - 53, save[0].width, save[0].height, DC_save[0], 0, 0, SRCCOPY);
                    break;
                }
                if ((last_lmb>100)&&(mouse_input == 1)) {
                    db_selected = (cursor_pos.y - 53) / 20;
                    db_selected = num_of_files > db_selected ? db_selected : num_of_files - 1;
                    if (last_db_selected != db_selected) {
                        filename_list_change = 1;
                        last_db_selected = db_selected;
                        file_change = 1;
                        start_y = 0;
                    }
                }
                last_lmb = clock();
                break;
            }
            }
            old_cursor_pos = cursor_pos;
        
        if (!unclosed_help) {
            if(hwnd_help){ DestroyWindow(hwnd_help); }
            
            unclosed_help = 1;help_on = 0;
            BitBlt(DC_window, 0, buffer.height - 53, help[0].width, help[0].height, DC_help[0], 0, 0, SRCCOPY);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNKCJA: MyRegisterClass()
//
//  PRZEZNACZENIE: Rejestruje klasę okna.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_XDBVISUALISER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_XDBVISUALISER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNKCJA: InitInstance(HINSTANCE, int)
//
//   PRZEZNACZENIE: Zapisuje dojście wystąpienia i tworzy okno główne
//
//   KOMENTARZE:
//
//        W tej funkcji dojście wystąpienia jest zapisywane w zmiennej globalnej i
//        jest tworzone i wyświetlane okno główne programu.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow,HWND *hWnd)
{
   hInst = hInstance; // Przechowuj dojście wystąpienia w naszej zmiennej globalnej
   RECT wr;

   wr.left = 0;
   wr.top = 0;
   wr.bottom = 900;
   wr.right = 1600;
   AdjustWindowRect(&wr ,WS_OVERLAPPEDWINDOW, FALSE);
   *hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
     0, 0, wr.right-wr.left , wr.bottom-wr.top+20, nullptr, nullptr, hInstance, nullptr);

   if (!*hWnd)
   {
      return FALSE;
   }

   ShowWindow(*hWnd, nCmdShow);
   UpdateWindow(*hWnd);

   return TRUE;
}

//
//  FUNKCJA: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PRZEZNACZENIE: Przetwarza komunikaty dla okna głównego.
//
//  WM_COMMAND  - przetwarzaj menu aplikacji
//  WM_PAINT    - Maluj okno główne
//  WM_DESTROY  - opublikuj komunikat o wyjściu i wróć
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Analizuj zaznaczenia menu:
            switch (wmId)
            {
            case IDM_ABOUT:
                //DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                //BitBlt(GetDC(0),0,0,512,512,0,0,0,WHITENESS);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_LBUTTONDOWN:
        mouse_input = 1;
        break;
    case WM_LBUTTONUP:
        mouse_input = 0;
        break;
    case  WM_MOUSEWHEEL:
        scroll=GET_WHEEL_DELTA_WPARAM(wParam);
        break;
    case WM_DESTROY:
        
        if (hWnd == h_window) {  PostQuitMessage(0); }
        else {
            DestroyWindow(hWnd);
        }

        break;
    default:
       return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Procedura obsługi komunikatów dla okna informacji o programie.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
        break;
   

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}









