
#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define SIZE 600000

int mysqrt(float* floater, double * const * doubler);
int main(int argc, char** argv) {
    char* file_name = argv[1];
    FILE* handle = fopen(file_name, "r");

    char input[SIZE] = {0};
    char signature[100][100] = {0};

    size_t end = fread(input, 1, SIZE, handle);

    char storage[SIZE] = {0};
    stb_lexer lexer;
    stb_c_lexer_init(&lexer, input, &input[end], storage, SIZE);

    int i = 0;
    int have_open_paren = false;
    int have_close_paren = false;
    while(stb_c_lexer_get_token(&lexer))
    {

        switch (lexer.token)
        {
            case CLEX_id:
                strlcpy(signature[i++], lexer.string, 100);
                break;
            case ')':
                signature[i++][0] = lexer.token;
                have_close_paren = true;
                break;
            case '(':
                signature[i++][0] = lexer.token;
                have_open_paren = true;
                break;
            case '*':
            case ',':
                signature[i++][0] = lexer.token;
                break;
            default:
               if (have_open_paren && have_close_paren)
               {
                   for (int j = 0; j < i; ++j)
                   {
                       printf("%s ", signature[j]);
                   }
                   printf("\n");
               }
               for (int j = 0; j < i; ++j)
               {
                   have_open_paren = false;
                   have_close_paren = false;
                   memset(signature[j], 0, 100);
               }
               i = 0;
        }
    }


    return 0;
}
