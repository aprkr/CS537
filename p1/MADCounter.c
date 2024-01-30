#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct word{
  char * contents;
  int numChars;
  int frequency;
  int orderAppeared;
  struct word * nextWord;
  struct word * prevWord;
} WORD;

typedef struct line{
  char * contents;
  int numChars;
  int frequency;
  int orderAppeared;
  struct word * nextLine;
  struct word * prevLine;
} LINE;

typedef struct singleChar{
	char contents;
	int frequency;
	int pos;
	struct singleChar * nextChar;
	struct singleChar * prevChar;
} singleChar;

void addChar(char c, singleChar **head, singleChar **tail, int* len, int pos){
	
	singleChar* ptr = (singleChar*) malloc(sizeof(singleChar));
	// initialize the new char node in the linked list
	ptr->contents = c;
	ptr->frequency = 1;
	ptr->pos = pos;
	ptr->nextChar = NULL;
	ptr->prevChar = NULL;
	// if linked list is empty insert at the start of the list
	if((*head) == NULL){
		(*head) = ptr;
		(*tail) = (*head);
		(*len)++;
	}
	else{
		singleChar *curr = (*head);
		while((curr)->contents < c){
			// insert at the end of the list
			if(curr->nextChar == NULL){
				curr->nextChar = ptr;
				ptr->prevChar = curr;
				(*tail) = ptr;
				(*len)++;
				return;
			}
			curr = curr->nextChar;
		}
		// insert at some position in the middle of the linked list
		if(curr->prevChar == NULL){
			curr->prevChar = ptr;
			ptr->nextChar = curr;
			(*head) = ptr;
			(*len)++;
		}
		else{
			ptr->nextChar = curr;
			ptr->prevChar = curr->prevChar;
			curr->prevChar->nextChar = ptr;
			curr->prevChar = ptr;
			(*len)++;
		}
	}
}

void readChar(FILE *fp){
	// first must get the amount of chars in the file in order to create the array
	rewind(fp);
	int c, charCount, uniqueChar = 0;
	singleChar *head, *tail = NULL;
    // loop through all the chars in the file
	while((c = fgetc(fp)) != EOF){
		charCount++;
		int match = 0;
		singleChar *ptr = head;
		while(ptr != NULL){
			if(ptr->contents == (char) c){
				ptr->frequency++;
				match = 1;
				break;
			}
			ptr = ptr->nextChar;
		}
		if(match){
			continue;
		}
		else{
			addChar((char) c, &head, &tail, &uniqueChar, charCount);
		}
	}
	//
	printf("Total Number of Chars: %i\nTotal Unique Chars: %i\n\n", charCount, uniqueChar);
	
	// after we create the linked list print out the contents
	singleChar *curr = head;
	while(curr != NULL){
		printf("Ascii Value: %d, Char: %c, Count: %i, Inital Position: %i\n", curr->contents, curr->contents, curr->frequency, (curr->pos) - 1);
		curr = curr -> nextChar;
	}
}
	
void addWord(char* word, WORD **head, WORD **tail, int* len, int pos){
	
	WORD* ptr = (WORD*) malloc(sizeof(WORD));
	// initialize the new char node in the linked list
	ptr->contents = word;
	ptr->frequency = 1;
	ptr->orderAppeared = pos;
	ptr->nextWord = NULL;
	ptr->prevWord = NULL;
	// if linked list is empty insert at the start of the list
	if((*head) == NULL){
		(*head) = ptr;
		(*tail) = (*head);
		(*len)++;
	}
	else{
		WORD *curr = (*head);
		while(strcmp((curr)->contents, word) < 0){
			// insert at the end of the list
			if(curr->nextWord == NULL){
				curr->nextWord = ptr;
				ptr->prevWord = curr;
				(*tail) = ptr;
				(*len)++;
				return;
			}
			curr = curr->nextWord;
		}
		// insert at some position in the middle of the linked list
		if(curr->prevWord == NULL){
			curr->prevWord = ptr;
			ptr->nextWord = curr;
			(*head) = ptr;
			(*len)++;
		}
		else{
			ptr->nextWord = curr;
			ptr->prevWord = curr->prevWord;
			curr->prevWord->nextWord = ptr;
			curr->prevWord = ptr;
			(*len)++;
		}
	}
}

void readWord(FILE* fp){
	// grab all of the words out of the file and put it in the structs
	rewind(fp);
	char word[50];
	int wordCount, uniqueWord;
	WORD *head, *tail = NULL;
	while(fscanf(fp, "%50s", word) != EOF){
		wordCount++;
		int match = 0;
		WORD *ptr = head;
		while(ptr != NULL){
			if(strcmp(ptr->contents, word) == 0){
				ptr->frequency++;
				match = 1;
				break;
			}
			ptr = ptr->nextWord;
		}
		if(match){
			continue;
		}
		else{
			addWord(word, &head, &tail, &uniqueWord, wordCount);
		}
	}
	printf("Total Number of Words: %i\nTotal Unique Words:%i\n\n", wordCount, uniqueWord);

	WORD *curr = head;
	while(curr != NULL){
		printf("Word: %s, Freq: %i, Initial Position: %i", curr->contents, curr->frequency, curr->orderAppeared);
	}
}

	
	// // grab all of the individual lines
	// fseek(fp, 0, SEEK_SET);
	// char line[filesize];
	// int lineCount;
	// while(fgets(line, filesize, fp) != NULL){
	// 	if(feof(fp)){
	// 		printf("%i\n", lineCount);
	// 		break;
	// 	}
	// 	printf("%s\n", line);
	// 	lineCount++;
	// }

void readBatch(FILE *fp, long filesize){
	fseek(fp, 0, SEEK_SET);
	printf("%i", (int) filesize);
}

int main(int argc, char* argv[]){
	// check to make sure minimum arguments have been passed in
    if((argc < 3) | (argc > 9)){
    	printf("USAGE:\n\t./MADCounter -f <input file> -o <output file> -c -w -l -Lw -Ll\n\t\tOR\n\t./MADCounter -B <batch file>\n");
		return(1);
    }
    // ensure that all flags start with are correct 
    char *flags[7] = {"-f", "-o", "-c", "-w", "-l", "-Lw", "-Ll"};
    // an int to keep track of which flags were used
	int o_flag, c_flag, w_flag, o_pos = 0;
    for(int i = 0; i < argc; i++){
    	if(strchr(argv[i],'-') > 0x0){
    		for(int j = 0; j < 7; j++){
				if(strcmp(argv[i],flags[j]) == 0){
					if(strcmp(argv[i], "-o") == 0){
						o_flag = 1;
						o_pos = i;
					}
					else if(strcmp(argv[i], "-c") == 0){
						c_flag = 1;
					}
					else if(strcmp(argv[i], "-w") == 0){
						w_flag = 1;
					}
					break;
				}
				// if an incorrect flag has been used errror out
				else if(i == 6){
					printf("ERROR: Invalid Flag Types\n");
					return(1);
				}
			}
   		}	
	}
    // if -B is specified make sure we can open the batch file and it is not empty
	// if -f is specified make sure we can open it
	// if neither were specified error out
	FILE *fp = fopen(argv[2], "rw");
	// two ints to keep track of weither -B or -f was specified
	int B_flag = 0;
	if(strcmp(argv[1], "-B") == 0){
		B_flag = 1;
		// check a sucessful opening
		if(fp == NULL){
			fclose(fp);
			printf("ERROR: Can't open batch file\n");
			return(1);
		}
		// check to make sure the file is not empty
		fseek(fp, 0, SEEK_END);
		long fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if(fileSize == 0){
			fclose(fp);
			printf("ERROR: Batch File Empty\n");
			return(1);	
		}	
	}
	else if(strcmp(argv[1], "-f") == 0){
		// make sure that the single file flag is not followed by another flag
		if(strchr(argv[2], '-') != NULL){
			fclose(fp);
			printf("ERROR: No Input File Provided\n");
			return(1);
		}
		// make sure we sucessfully open the file
		if(fp == NULL){
			fclose(fp);
			printf("ERROR: Can't open input file\n");
			return(1);
		}
	}
	else{
		fclose(fp);
		printf("ERROR: No Input File Provided\n");
		return(1);
	}
	// if -o flag is specified make sure that there is an output file specificed
	if(o_flag == 1){
		if(strchr(argv[o_pos + 1], '-') != NULL){
			fclose(fp);
			printf("ERROR: No Output File Provided\n");
			return(1);
		}
	}
	// check to make sure that the input file is not empty
	fseek(fp, 0, SEEK_END);
   	long fileSize = ftell(fp);
   	fseek(fp, 0, SEEK_SET);
   	if(fileSize == 0){
   		fclose(fp);
		printf("ERROR: Input File Empty\n");
		return(1);
   	}
   	if(B_flag){
   		readBatch(fp, fileSize);
   	}
   	else if(c_flag == 1){
		readChar(fp);
	}
	else if(w_flag == 1){
		readWord(fp);
	}
   	return(0);
}
