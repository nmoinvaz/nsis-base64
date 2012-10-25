/**
 * Base64 plugin v.001
 *
 *   Angelo Dureghello, Trieste, Italy 09-01-2007
 */
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define FORCE_SWITCH  "/FORCE"
#define NOSAFE_SWITCH "/NOSAFE"
#define MAX_STRLEN       4098

/* NSIS stack structure */
typedef struct _stack_t {
    struct  _stack_t *next;
    char        text[MAX_STRLEN];
} stack_t;

stack_t         **g_stacktop;
char            *g_variables;
unsigned int    g_stringsize;
HINSTANCE       g_hInstance;

#define EXDLL_INIT()                \
{                                   \
    g_stacktop      = stacktop;     \
    g_variables     = variables;    \
    g_stringsize    = string_size;  \
}

static char *encoding_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char *decoding_table = NULL;
static size_t mod_table[] = {0, 2, 1};


void build_decoding_table() {
    int i;
    decoding_table = (char *)malloc(256);
    for (i = 0; i < 0x40; i++)
        decoding_table[encoding_table[i]] = i;
}


void base64_cleanup() {
    free(decoding_table);
}

char *base64_encode(const char *data, size_t input_length, size_t *output_length)
{
    char *encoded_data = NULL;
    size_t i = 0;
    size_t j = 0;

    *output_length = (size_t) (4.0 * ceil((double) input_length / 3.0));

    encoded_data = (char *)malloc(*output_length);
    if (encoded_data == NULL) 
        return NULL;

    while (i < input_length) {

        unsigned int octet_a = i < input_length ? data[i++] : 0;
        unsigned int octet_b = i < input_length ? data[i++] : 0;
        unsigned int octet_c = i < input_length ? data[i++] : 0;

        unsigned int triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    return encoded_data;
}

char *base64_decode(const char *data, size_t input_length, size_t *output_length)
{
    char *decoded_data = NULL;
    size_t i = 0;
    size_t j = 0;

    if (decoding_table == NULL)
        build_decoding_table();
    if (input_length % 4 != 0) 
        return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    decoded_data = (char *)malloc(*output_length);
    if (decoded_data == NULL) 
        return NULL;

    while (i < input_length) {

        unsigned int sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        unsigned int sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        unsigned int sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        unsigned int sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        
        unsigned int triple = (sextet_a << 3 * 6)
            + (sextet_b << 2 * 6)
            + (sextet_c << 1 * 6)
            + (sextet_d << 0 * 6);
        
        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }
    
    return decoded_data;
}

//Function: Removes the element from the top of the NSIS stack and puts it in the buffer
int popstring(char *str)
{
    stack_t *th;
    
    if (!g_stacktop || !*g_stacktop) return 1;
    
    th=(*g_stacktop);
    lstrcpy(str,th->text);
    *g_stacktop = th->next;
    GlobalFree((HGLOBAL)th);
    
    return 0;
}

//Function: Adds an element to the top of the NSIS stack
void pushstring(const char *str)
{
    stack_t *th;
    
    if (!g_stacktop) return;
    
    th=(stack_t*)GlobalAlloc(GPTR, sizeof(stack_t)+g_stringsize);
    lstrcpyn(th->text,str,g_stringsize);
    th->next=*g_stacktop;
    
    *g_stacktop=th;
}

void __declspec(dllexport) Encode(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
    char string_to_encode [MAX_STRLEN] ={0};
    int string_to_encode_len = 0;
    char *string_encoded = NULL;
    int string_encoded_len = 0;

    EXDLL_INIT();
    {
        popstring(string_to_encode);
        string_to_encode_len = strlen(string_to_encode);
        string_encoded = base64_encode(string_to_encode, string_to_encode_len, &string_encoded_len);
        pushstring(string_encoded);
        free(string_encoded);
    }
}

void __declspec(dllexport) Decode(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
    char string_to_decode [MAX_STRLEN] ={0};
    int string_to_decode_len= 0;
    char *string_decoded = NULL;
    int string_decoded_len = 0;

    EXDLL_INIT();
    {
        popstring(string_to_decode);
        string_to_decode_len = strlen(string_to_decode);
        build_decoding_table();
        string_decoded = base64_decode(string_to_decode, string_to_decode_len, &string_decoded_len);
        base64_cleanup();
        pushstring(string_decoded);
        free(string_decoded);
    }
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    g_hInstance=hInst;
    return TRUE;
}
