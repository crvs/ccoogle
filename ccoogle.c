#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define INPUT_MAX_SIZE 600000
#define SIG_COMPONENTS 100
#define SIG_COMP_SIZE 100

bool isTypeQualifier(char* string)
{
    return !(strcmp(string, "const") || strcmp(string, "volatile") || strcmp(string, "extern"));
}


void processFile(char* file_path)
{
    FILE* handle = fopen(file_path, "r");

    /// TODO: dynamically size the input stream (may want to use stat)
    char input[INPUT_MAX_SIZE] = {0};
    /// TODO: dynamically size the signature
    char signature[SIG_COMPONENTS][SIG_COMP_SIZE] = {0};

    size_t end = fread(input, 1, INPUT_MAX_SIZE, handle);
    fclose(handle);

    char storage[INPUT_MAX_SIZE] = {0};
    stb_lexer lexer;
    stb_c_lexer_init(&lexer, input, &input[end], storage, INPUT_MAX_SIZE);

    int i = 0;
    int have_open_paren = 0;
    int have_close_paren = 0;
    int num_ids_before_paren = 0;
    int paren_depth = 0;
    bool nested_parens = true;
    char* signature_start;

    while(stb_c_lexer_get_token(&lexer))
    {
        if (i == 0)
        {
            signature_start = lexer.where_firstchar;
        }

        if (paren_depth < 0) nested_parens = false;
        switch (lexer.token)
        {
            // TODO: to get this to handle C++ we need to be able to handle <, > and :: for template matching
            case CLEX_id:
                strlcpy(signature[i++], lexer.string, SIG_COMP_SIZE);
                if (have_open_paren == 0 && isTypeQualifier(lexer.string) == 0) num_ids_before_paren++;
                break;
            case ')':
                signature[i++][0] = lexer.token;
                paren_depth--;
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
                       have_open_paren == have_close_paren && nested_parens /* parenthesis properly nested */ &&
                       signature[0][0] != '(' /* not a c-style cast */ &&
                       num_ids_before_paren >= 2 /* need type and function name */ &&
                       strcmp(signature[0], "typedef") /* not a typedef*/ )
               {
                   stb_lex_location loc = {0};
                   stb_c_lexer_get_location(&lexer, signature_start, &loc);
                   printf("%s|%d| ", file_path, loc.line_number);
                   for (int j = 0; j < i; ++j)
                   {
                       // ignore extern keyword
                       if ((j == 0) && (strcmp(signature[j], "extern") == 0)) continue;

                       printf("%s", signature[j]);

                       if (signature[j][0] == '(') continue;
                       if ((j + 1 < i) && (signature[j + 1][0] == '(')) continue;
                       if ((j + 1 < i) && (signature[j + 1][0] == ')')) continue;
                       if ((j + 1 < i) && (signature[j + 1][0] == '*')) continue;
                       printf(" ");
                   }
                   printf("\n");
               }
               /// reset data for next candidate signature
               paren_depth = 0;
               nested_parens = true;
               have_open_paren = 0;
               have_close_paren = 0;
               num_ids_before_paren = 0;
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
