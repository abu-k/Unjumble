/*
 *                          unjumble.c
 *                           CSSE2310
 *                        Assignment One
 *                         Youcef Mesbah
 *                 youcef.mesbah@uqconnect.edu.au
 *                           42349343
 *
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Default dictionary location
#define DEFAULT_DICTIONARY "/usr/share/dict/words"

// Longest word size including the newline and null terminator
#define MAX_WORD_SIZE 52

/**
 * Enum to show which display method user chose
 */
typedef enum {
    ALPHA, LEN, LONGEST, DEFAULT
} ResultType;

/**
 * Struct to hold the count of each letter from a word
 */
typedef struct {
    int v[26];
} Lettered;

/**
 * Struct to hold important details of the program
 */
typedef struct {
    int matchTypeFlag;
    ResultType resultType;
    int lettersFlag;
    int includeFlag;
    int dictFlag;
    int aMatchFlag;
    char includeLetter;
    char* letters;
    char* dict;
    FILE* dictHandle;
    char** matches;
    int numMatches;
} UnjumbleArgs;

/* Function Prototypes */
UnjumbleArgs check_args(int argc, char** argv);
void invalid_args_error(void);
void file_error(char*);
void check_letters_valid(UnjumbleArgs);
bool check_word_validity(char*);
Lettered word_to_array(char*);
UnjumbleArgs insert_word(char*, UnjumbleArgs);
void print_all_words(UnjumbleArgs);
UnjumbleArgs check_matches(UnjumbleArgs);
int stringcompare(const void*, const void*);

int main(int argc, char** argv){
    if (argc < 2 || argc > 6) {
        invalid_args_error();
    }
    UnjumbleArgs commandArgs;
    commandArgs = check_args(argc, argv);
    //If no letters given
    if (!commandArgs.lettersFlag) {
        invalid_args_error();
    }
    if (!commandArgs.dictFlag) {
        commandArgs.dict = DEFAULT_DICTIONARY;
    }
    check_letters_valid(commandArgs);
    commandArgs.dictHandle = fopen(commandArgs.dict, "r");
    if (!commandArgs.dictHandle) {
        file_error(commandArgs.dict);
    }
    
    commandArgs.matches = NULL;
    commandArgs.numMatches = 0;
    commandArgs.aMatchFlag = 0;
    commandArgs = check_matches(commandArgs);

    print_all_words(commandArgs);
    return 0;
}

/**
 * check_args() 
 * checks what type of result options the user chose and throws errors when 
 * input doesn't conform
 */
UnjumbleArgs check_args(int argc, char** argv){
    UnjumbleArgs commandArgs;
    commandArgs.resultType = DEFAULT;
    commandArgs.matchTypeFlag = 0;
    commandArgs.lettersFlag = 0;
    commandArgs.includeFlag = 0;
    commandArgs.dictFlag = 0;
    for (int i = 1; i < argc; i++) {
        if (!commandArgs.matchTypeFlag && !commandArgs.lettersFlag 
                && strcmp(argv[i], "-alpha") == 0) {
            commandArgs.matchTypeFlag = 1;
            commandArgs.resultType = ALPHA;
        } else if (!commandArgs.matchTypeFlag && !commandArgs.lettersFlag
                && strcmp(argv[i], "-len") == 0) {
            commandArgs.matchTypeFlag = 1;
            commandArgs.resultType = LEN;
        } else if (!commandArgs.matchTypeFlag && !commandArgs.lettersFlag
                && strcmp(argv[i], "-longest") == 0) {
            commandArgs.matchTypeFlag = 1;
            commandArgs.resultType = LONGEST;
        } else if (!commandArgs.includeFlag && !commandArgs.lettersFlag
                && strcmp(argv[i], "-include") == 0) {
            commandArgs.includeFlag = 1;
            if ((i + 1 >= argc) || (strlen(argv[i + 1]) > 1)) {
                invalid_args_error();
            }
            commandArgs.includeLetter = argv[i + 1][0];
            if (!((('a' <= *argv[i + 1] && *argv[i + 1] <= 'z')) 
                    || (('A' <= *argv[i + 1] && *argv[i + 1] <= 'Z')))) {
                invalid_args_error();
            }
            i++;
        } else if (!commandArgs.lettersFlag && argv[i][0] != '-') {
            commandArgs.lettersFlag = 1;
            commandArgs.letters = malloc(sizeof(char) * strlen(argv[i]));
            strcpy(commandArgs.letters, argv[i]);
        } else if (commandArgs.lettersFlag && argv[i][0] != '-') {
            commandArgs.dictFlag = 1;
            commandArgs.dict = malloc(sizeof(char) * strlen(argv[i]));
            strcpy(commandArgs.dict, argv[i]);
            if ((i + 1) < argc) {
                invalid_args_error();
            }
        } else {
            invalid_args_error();
        }
    }
    return commandArgs;
}

/**
 * invalid_args_error()
 * prints message to stderr and exits with nonzero code
 */
void invalid_args_error() {
    fprintf(stderr, "Usage: unjumble [-alpha|-len|-longest] [-include letter]"
            " letters [dictionary]\n");
    exit(1);
}

/* 
 * file_error()
 * prints message to stderr and exits with nonzero code
 * */
void file_error(char* filename) {
    fprintf(stderr, "unjumble: file \"%s\" can not be opened\n", filename);
    exit(2);
}

/**
 * check_letters_valid()
 * checks if letters given by user are conforming and if not throws error
 * and exits with non zero code
 */
void check_letters_valid(UnjumbleArgs commandArgs) {
    if (strlen(commandArgs.letters) < 3) {
        fprintf(stderr, "unjumble: must supply at least three letters\n");
        exit(3);
    }
    for (int i = 0; i < strlen(commandArgs.letters); i++) {
        if (!isalpha((int)commandArgs.letters[i])) {
            fprintf(stderr, "unjumble: can only unjumble alphabetic"
                    " characters\n");
            exit(4);
        }
    }
}

/**
 * word_to_array()
 * counts number of letters in each word and put's into a Lettered object
 */
Lettered word_to_array(char* word) {
    Lettered lettered;
    memset(lettered.v, 0, sizeof(int) * 26);
    for (int i = 0; i < strlen(word); i++) {
        lettered.v[toupper(word[i]) - 65]++;
    }
    return lettered;
}

/**
 * check_word_validity()
 * checks if word is valid to be returned
 */
bool check_word_validity(char* word) {
    int maxIndex = 0;
    if (strlen(word) < 4) {
        return false;
    }
    if (word[strlen(word) - 1] == '\n') {
        maxIndex = strlen(word) - 1;
    } else {
        maxIndex = strlen(word);
    }
    // Ensure all characters are letters
    for (int i = 0; i < maxIndex; i++) {
        if (!isalpha((int)word[i]) || word[i] == ' ') {
            return false;
        }
    }
    return true;
}

/**
 * insert_word()
 * inserts word to array
 */
UnjumbleArgs insert_word(char* word, UnjumbleArgs commandArgs) {
    commandArgs.aMatchFlag = 1;
    char* plainWord = malloc(sizeof(char*) * strlen(word));
    // Remove newline
    if (word[strlen(word) - 1] == '\n') {
        strcpy(plainWord, word);
        plainWord[strlen(word) - 1] = '\0';
    } else {
        strcpy(plainWord, word);
    }
    // Resize the array
    commandArgs.matches = realloc(commandArgs.matches, sizeof(char*) 
            * (commandArgs.numMatches + 1));
    commandArgs.matches[commandArgs.numMatches] = plainWord;
    commandArgs.numMatches = commandArgs.numMatches + 1;
    return commandArgs;
}

/**
 * print_all_words()
 * All words found in dictionary, now print them according to result type
 * user chose
 */
void print_all_words(UnjumbleArgs commandArgs) {
    int longestWord = 0;
    if (!commandArgs.aMatchFlag) {
        exit(10);
    }
    // Find length of longest word
    for (int i = 0; i < commandArgs.numMatches; i++) {
        if (strlen(commandArgs.matches[i]) > longestWord) {
            longestWord = strlen(commandArgs.matches[i]);
        }
    }
    if (!(commandArgs.matchTypeFlag)) {
        for (int i = 0; i < commandArgs.numMatches; i++) {
            printf("%s\n", commandArgs.matches[i]);
        }
    } else {
        // All options to be sorted
        qsort(commandArgs.matches, commandArgs.numMatches,
                sizeof(char* ), stringcompare);
        if (commandArgs.resultType == ALPHA) {
            for (int i = 0; i < commandArgs.numMatches; i++) {
                printf("%s\n", commandArgs.matches[i]);
            }
        } else if (commandArgs.resultType == LEN) {
            for (int i = longestWord; i > 0; i--) {
                for (int j = 0; j < commandArgs.numMatches; j++) {
                    if (strlen(commandArgs.matches[j]) == i) {
                        printf("%s\n", commandArgs.matches[j]);
                    }
                }
            }
        } else if (commandArgs.resultType == LONGEST) {
            for (int i = 0; i < commandArgs.numMatches; i++) {
                if (strlen(commandArgs.matches[i]) == longestWord) {
                    printf("%s\n", commandArgs.matches[i]);
                }   
            }
        }
    }
}

/**
 * string_compare()
 * Idea borrowed from manpage qsort
 * qsort needs a function pointer that returns an int and takes 2 pointers to
 * pointers to chars as per the manpage
 */
int stringcompare(const void* word1, const void* word2) {
    // Convert words back to string
    char* wrd1 = (*(char**) word1);
    char* wrd2 = (*(char**) word2);
    
    // Length of smallest word to iterate over
    int min = strlen(wrd1) <= strlen(wrd2) 
            ? (int)strlen(wrd1) : (int)strlen(wrd2);
    for (int i = 0; i < min; i++) {
        // If letters different, return 1 if the letter of first word greater
        // and -1 if letter of second word greater
        if (toupper(wrd1[i]) > toupper(wrd2[i])) {
            return 1;
        } else if (toupper(wrd1[i]) < toupper(wrd2[i])) {
            return -1;
        } else {
            //Both letters equal, move to next letter
            continue;
        }
    }
    // Words are the same until here. One word must be longer than the other
    // return 1 if word 1 bigger or -1 if second word
    return strlen(wrd1) > strlen(wrd2) ? 1 : -1;
}

/**
 * check_matches()
 * Iterates over dictionary and adds good words to the array
 */
UnjumbleArgs check_matches(UnjumbleArgs commandArgs) {
    Lettered dictLettered;
    Lettered lettersLettered;
    bool goodWord;
    int includeLetterCheck = toupper(commandArgs.includeLetter) - 65;
    lettersLettered = word_to_array(commandArgs.letters);

    char* word = malloc(sizeof(char) * MAX_WORD_SIZE);
    while (fgets(word, MAX_WORD_SIZE + 2, commandArgs.dictHandle) != NULL) {
        goodWord = true;
        if (check_word_validity(word)) {
            dictLettered = word_to_array(word);
            for (int i = 0; i < 26; i++) {
                if (lettersLettered.v[i] >= dictLettered.v[i]) {
                    continue;
                } else {
                    goodWord = false;
                    break;
                }
            }
            if (goodWord) {
                if (commandArgs.includeFlag) {
                    if (dictLettered.v[includeLetterCheck] > 0) {
                        commandArgs = insert_word(word, commandArgs);
                    }
                    if (dictLettered.v[includeLetterCheck] == 0) {
                    }
                } else {
                    commandArgs = insert_word(word, commandArgs);
                }
            }
        }
    }
    return commandArgs;
}
