#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define INPUT_MAX_SIZE 600000
#define SIG_COMPONENTS 100
#define SIG_COMP_SIZE 100

void processFile(char* file_path)
{
    FILE* handle = fopen(file_path, "r");

    char input[INPUT_MAX_SIZE] = {0};
    char signature[SIG_COMPONENTS][SIG_COMP_SIZE] = {0};

    size_t end = fread(input, 1, INPUT_MAX_SIZE, handle);

    char storage[INPUT_MAX_SIZE] = {0};
    stb_lexer lexer;
    stb_c_lexer_init(&lexer, input, &input[end], storage, INPUT_MAX_SIZE);

    int i = 0;
    int have_open_paren = 0;
    int have_close_paren = 0;
    int paren_depth = 0;
    int min_paren_depth = 0;
    char* signature_start;

    while(stb_c_lexer_get_token(&lexer))
    {

        if (i == 0)
        {
            signature_start = lexer.where_firstchar;
        }

        switch (lexer.token)
        {
            // TODO: to get this to handle C++ we need to be able to handle <, > and :: for template matching
            case CLEX_id:
                strlcpy(signature[i++], lexer.string, SIG_COMP_SIZE);
                break;
            case ')':
                signature[i++][0] = lexer.token;
                paren_depth--;
                if (min_paren_depth < paren_depth) min_paren_depth = paren_depth;
                have_close_paren++;
                break;
            case '(':
                signature[i++][0] = lexer.token;
                paren_depth++;
                have_open_paren++;
                break;
            case '*':
            case ',':
                signature[i++][0] = lexer.token;
                break;
            default:
               /// emit signature if valid
               if (
                       have_open_paren /* at least one parenthesis */ &&
                       have_open_paren == have_close_paren && min_paren_depth == 0 /* parenthesis properly nested */ &&
                       signature[0][0] != '(' && /* not a c-style cast */
                       strcmp(signature[0], "typedef") /* not a typedef*/ )
               {
                   stb_lex_location loc = {0};
                   stb_c_lexer_get_location(&lexer, signature_start, &loc);
                   printf("%s|%d| ", file_path, loc.line_number);
                   for (int j = 0; j < i; ++j)
                   {
                       // ignore extern keyword
                       if (j == 0 && strcmp(signature[j], "extern") == 0) continue;

                       // // ignore names of variables
                       // if (j + 1 < i && signature[j + 1][0] == ',') continue;

                       printf("%s", signature[j]);

                       if (signature[j][0] == '(') continue;

                       /// if ((j + 2 < i) && (signature[j + 2][0] == ',')) continue;

                       if ((j + 1 < i) && (signature[j + 1][0] == '(' || signature[j + 1][0] == ')' || signature[j + 1][0] == '*')) continue;

                       printf(" ");
                   }
                   printf("\n");
               }
               /// reset data for next candidate signature
               min_paren_depth = 0;
               have_open_paren = 0;
               have_close_paren = 0;
               for (int j = 0; j < i; ++j)
               {
                   memset(signature[j], 0, SIG_COMP_SIZE);
               }
               i = 0;
        }
    }
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) processFile(argv[i]);
    return 0;
}
