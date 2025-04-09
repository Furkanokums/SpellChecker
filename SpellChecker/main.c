#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MaxSizeOfLex 10000
#define MaxSizeOfWord 50
#define MaxTips 100
#define MaxHashTableSize 10007

// Structure for hash table entry
typedef struct HashEntry {
    char word[MaxSizeOfWord];
    struct HashEntry* next;
} HashEntry;

// Hash table structure
typedef struct HashTable {
    HashEntry* table[MaxHashTableSize];
} HashTable;

// Hash function to map word to a table index
unsigned int hash(const char* word) {
    unsigned int hashValue = 0;
    while (*word) {
        hashValue = (hashValue * 31) + tolower(*word);
        word++;
    }
    return hashValue % MaxHashTableSize;
}

// Function to initialize the hash table
void initHashTable(HashTable* ht) {
    for (int i = 0; i < MaxHashTableSize; i++) {
        ht->table[i] = NULL;
    }
}

// Function to insert a word into the hash table
void insertIntoHashTable(HashTable* ht, const char* word) {
    unsigned int index = hash(word);
    HashEntry* newEntry = (HashEntry*)malloc(sizeof(HashEntry));
    strcpy(newEntry->word, word);
    newEntry->next = ht->table[index];
    ht->table[index] = newEntry;
}

// Function to check if a word exists in the hash table
int isWordInHashTable(HashTable* ht, const char* word) {
    unsigned int index = hash(word);
    HashEntry* entry = ht->table[index];
    while (entry) {
        if (strcasecmp(entry->word, word) == 0) {
            return 1;  // Word found
        }
        entry = entry->next;
    }
    return 0;  // Word not found
}

// Function to load the dictionary into the hash table
void loadDictionary(HashTable* ht, const char* filePath) {
    FILE* file = fopen(filePath, "r");

    char word[MaxSizeOfWord];
    while (fgets(word, sizeof(word), file)) {
        word[strcspn(word, "\n")] = '\0';  // Remove newline
        insertIntoHashTable(ht, word);
    }

    fclose(file);
}

// Function to generate spelling tips based on word similarity
void generateTips(HashTable* hashTable, const char* word, char tips[MaxTips][MaxSizeOfWord], int* tipsCount) {
    int len = strlen(word);

    // Try replacing each character with every letter
    for (int i = 0; i < len; i++) {
        char originalChar = word[i];
        for (char c = 'a'; c <= 'z'; c++) {
            if (c != originalChar) {
                char modifiedWord[MaxSizeOfWord];
                strcpy(modifiedWord, word);
                modifiedWord[i] = c;
                if (isWordInHashTable(hashTable, modifiedWord)) {
                    strcpy(tips[(*tipsCount)++], modifiedWord);
                }
            }
        }
    }

    // Try adding a character at every position
    for (int i = 0; i <= len; i++) {
        for (char c = 'a'; c <= 'z'; c++) {
            char modifiedWord[MaxSizeOfWord];
            strncpy(modifiedWord, word, i);
            modifiedWord[i] = c;
            strcpy(modifiedWord + i + 1, word + i);
            if (isWordInHashTable(hashTable, modifiedWord)) {
                strcpy(tips[(*tipsCount)++], modifiedWord);
            }
        }
    }

    // Try removing a character
    for (int i = 0; i < len; i++) {
        char modifiedWord[MaxSizeOfWord];
        strncpy(modifiedWord, word, i);
        strcpy(modifiedWord + i, word + i + 1);
        if (isWordInHashTable(hashTable, modifiedWord)) {
            strcpy(tips[(*tipsCount)++], modifiedWord);
        }
    }
}

void processWord(HashTable* hashTable, const char* word, FILE* outputFile) {
    // If word is not in the dictionary, generate suggestions
    if (!isWordInHashTable(hashTable, word)) {
        fprintf(outputFile, "Incorrect word: %s\n", word);

        // Generate tips
        char tips[MaxTips][MaxSizeOfWord];
        int tipCount = 0;
        generateTips(hashTable, word, tips, &tipCount);

        // Write suggestions to file
        if (tipCount > 0) {
            fprintf(outputFile, "Did you mean this:\n");
            for (int i = 0; i < tipCount; i++) {
                fprintf(outputFile, "  - %s\n", tips[i]);
            }
        } else {
            fprintf(outputFile, "Cannot find any word.\n");
        }

        fprintf(outputFile, "\n");
    }
}

// Main function to process user input
int main() {
    // Create and initialize the hash table
    HashTable hashTable;
    initHashTable(&hashTable);

    // Load the dictionary into the hash table
    loadDictionary(&hashTable, "lexicon.txt");

    // Open output file
    FILE* outputFile = fopen("output.txt", "w");

    // Use a buffer to store the user input sentence
    char sentence[256];
    printf("Write a sentence: ");
    fgets(sentence, sizeof(sentence), stdin);
    sentence[strcspn(sentence, "\n")] = '\0';  // Remove trailing newline

    // Create a pointer to process each word from the sentence
    char* word = strtok(sentence, " ");

    // Process each word in the sentence
    while (word) {
        processWord(&hashTable, word, outputFile);
        word = strtok(NULL, " ");  // Move to the next word
    }

    fclose(outputFile);
    printf("Spell check complete.\n");

    return 0;
}