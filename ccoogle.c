#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"

#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct SB { size_t capacity; size_t size; char* data; } SB;
#define SB_INIT(CAPACITY) {.capacity = CAPACITY, .size = 0, .data = malloc(CAPACITY) }
#define SB_HEAD(sb) &(sb.data[sb.size])
#define SB_CAP(sb) (sb.capacity - sb.size)
#define SB_EXTENDTOFIT(sb, EXTRACAP) do { if (SB_CAP(sb) < EXTRACAP + 1) { sb.capacity = sb.size + EXTRACAP + 1; sb.data = realloc(sb.data, sb.capacity); } } while (0)
#define SB_PRINTF(sb, ...) do { sb.size += snprintf(SB_HEAD(sb), SB_CAP(sb), __VA_ARGS__); } while(0)
#define SB_NPRINTF(sb, N, ...) do { SB_EXTENDTOFIT(sb, N); SB_PRINTF(sb, __VA_ARGS__); } while (0)
#define SB_RESET(sb) do { memset(sb.data, 0, sb.size); sb.size = 0; } while (0)


bool isKeyWord(char* string)
{
    return !(
            strcmp(string, "return") &&
            strcmp(string, "static") &&
            strcmp(string, "const") &&
            strcmp(string, "volatile") &&
            strcmp(string, "extern")
    );
}


void processFile(char* file_path)
{
    FILE* handle = fopen(file_path, "r");

    struct stat file_stat = {0};
    stat(file_path, &file_stat);
    size_t input_size = (file_stat.st_blocks + 10) * 512;
    char* input = malloc(input_size);
    memset(input, 0, input_size);
    SB signature = SB_INIT(50);

    size_t end = fread(input, 1, input_size, handle);
    fclose(handle);

    char storage[1000] = {0};
    stb_lexer lexer;
    stb_c_lexer_init(&lexer, input, &input[end], storage, 1000);

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
                // ignore extern and static keywords when printing
                if (strcmp(lexer.string, "extern") == 0 || strcmp(lexer.string, "static") == 0) continue;
                if (have_open_paren == 0 && isKeyWord(lexer.string) == 0) num_ids_before_paren++;
                if (i == 0 || signature.data[signature.size - 1] == '(')
                    SB_NPRINTF(signature, lexer.string_len + 50, "%s", lexer.string);
                else
                    SB_NPRINTF(signature, lexer.string_len + 50, " %s", lexer.string);
                break;
            case ')':
            case '(':
            case '*':
            case ',':
                have_open_paren += (lexer.token == '(');
                have_close_paren += (lexer.token == ')');
                paren_depth = have_open_paren - have_close_paren;
                SB_NPRINTF(signature, 1, "%c", (char)lexer.token);
                break;
            default:
               /// emit signature if valid
               if (
                       have_open_paren /* at least one parenthesis */ &&
                       have_open_paren == have_close_paren && nested_parens /* parenthesis properly nested */ &&
                       signature.data[0] != '(' /* not a c-style cast */ &&
                       num_ids_before_paren >= 2 /* need type and function name */)
               {
                   stb_lex_location loc = {0};
                   stb_c_lexer_get_location(&lexer, signature_start, &loc);
                   printf("%s|%d| %s\n", file_path, loc.line_number, signature.data);
               }
               /// reset data for next candidate signature
               paren_depth = 0;
               nested_parens = true;
               have_open_paren = 0;
               have_close_paren = 0;
               num_ids_before_paren = 0;
               SB_RESET(signature);
               i = 0;
               continue;
        }
        i++;
    }
    free(input);
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) processFile(argv[i]);
    return 0;
}
