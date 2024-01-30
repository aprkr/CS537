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
  struct line * nextLine;
  struct line * prevLine;
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
	singleChar *head = NULL;
	singleChar *tail = NULL;
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
	char* wordCpy = (char*) malloc(sizeof(char) * 100);
	strcpy(wordCpy, word);
	ptr->contents = wordCpy;
	ptr->frequency = 1;
	ptr->orderAppeared = pos;
	ptr->nextWord = NULL;
	ptr->prevWord = NULL;
	// if linked list is empty insert at the start of the list
	if((*head) == NULL){
		(*head) = ptr;
		(*tail) = (*head);
		(*len)++;
		return;
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

void readWord(FILE* fp, int trackLongest){
	// grab all of the words out of the file and put it in the structs
	rewind(fp);
	int wordCount = 0, uniqueWord = 0;
	char word[100];

	WORD *head = NULL;
	WORD *tail = NULL;
	while(fscanf(fp, "%50s", word) != EOF){
		wordCount++;
		int match = 0;
		WORD *curr = head;
		while(curr != NULL){
			if(strcmp(curr->contents, word) == 0){
				curr->frequency++;
				match = 1;
				break;
			}
			curr = curr->nextWord;
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
		printf("Word: %s, Freq: %i, Initial Position: %i\n", curr->contents, curr->frequency, (curr->orderAppeared - 1));
		curr = curr -> nextWord;
	}
}

void addLine(char* line, LINE **head, LINE **tail, int* len, int pos, int size){
	LINE* ptr = (LINE*) malloc(sizeof(LINE));
	// initialize the new char node in the linked list
	char* lineCpy = (char*) malloc(sizeof(char) * size);
	strncpy(lineCpy, line, strlen(line)-1);
	ptr->contents = lineCpy;
	ptr->frequency = 1;
	ptr->orderAppeared = pos;
	ptr->nextLine = NULL;
	ptr->prevLine = NULL;
	// if linked list is empty insert at the start of the list
	if((*head) == NULL){
		(*head) = ptr;
		(*tail) = (*head);
		(*len)++;
		return;
	}
	else{
		LINE *curr = (*head);
		while(strcmp((curr)->contents, line) < 0){
			// insert at the end of the list
			if(curr->nextLine == NULL){
				curr->nextLine = ptr;
				ptr->prevLine = curr;
				(*tail) = ptr;
				(*len)++;
				return;
			}
			curr = curr->nextLine;
		}
		// insert at some position in the middle of the linked list
		if(curr->prevLine == NULL){
			curr->prevLine = ptr;
			ptr->nextLine = curr;
			(*head) = ptr;
			(*len)++;
		}
		else{
			ptr->nextLine = curr;
			ptr->prevLine = curr->prevLine;
			curr->prevLine->nextLine = ptr;
			curr->prevLine = ptr;
			(*len)++;
		}
	}
}

void readLine(FILE* fp, int filesize, int trackLongest){
	rewind(fp);
	int lineCount = 0, uniqueLine = 0;
	char line[filesize];
	LINE *head = NULL;
	LINE *tail = NULL;
	while(fgets(line, filesize, fp) != NULL){
		if(strcmp(line,"\n") == 0){
			continue;
		}
		lineCount++;
		int match = 0;
		LINE *curr = head;
		while(curr != NULL){
			if(strcmp(curr->contents, line) == 0){
				curr->frequency++;
				match = 1;
				break;
			}
			curr = curr -> nextLine;
		}
		if(match){
			continue;
		}
		else{
			addLine(line, &head, &tail, &uniqueLine, lineCount, filesize);
		}
	}

	if(feof(fp)){
		printf("Total Number of Lines: %i\nTotal Unique Lines: %i\n\n", lineCount, uniqueLine);
		
		LINE *curr = head;
		while(curr != NULL){
			printf("Line: %s, Freq: %i, Initial Position: %i\n", curr->contents, curr->frequency, (curr->orderAppeared - 1));
			curr = curr -> nextLine;
		}

	}
}


void readBatch(FILE *fp){
	fseek(fp, 0, SEEK_SET);
}

int main(int argc, char* argv[]){
	// check to make sure minimum arguments have been passed in
    if((argc < 3) | (argc > 9)){
    	printf("USAGE:\n\t./MADCounter -f <input file> -o <output file> -c -w -l -Lw -Ll\n\t\tOR\n\t./MADCounter -B <batch file>\n");
		return(1);
    }
    // an int to keep track of which flags were used
	int f_flag=0, B_flag=0, o_flag=0, Lw_flag=0, Ll_flag=0;
	int o_pos=0, f_pos=0, B_pos; 
    for(int i = 0; i < argc; i++){
    	if(strchr(argv[i],'-') > 0x0){
			if(strcmp(argv[i], "-f") == 0){
				f_flag = 1;
				f_pos=i;
			}
			else if(strcmp(argv[i], "-B") == 0){
				B_flag = 1;
				B_pos = i;
			}
			else if(strcmp(argv[i], "-o") == 0){
				o_flag = 1;
				o_pos = i;
			}
			else if(strcmp(argv[i], "-Lw") == 0){
				Lw_flag = 1;
			}
			else if(strcmp(argv[i], "-Ll") == 0){
				Ll_flag = 1;
			}
			else if((strcmp(argv[i], "-c") == 0) || (strcmp(argv[i], "-w") == 0) || (strcmp(argv[i], "-l") == 0)){
				continue;
			}
			else{
				printf("ERROR: Invalid Flag Types\n");
				return(1);
			}
		}
	}
	FILE *fp;
	if(B_flag == 1){
		fp = fopen(argv[B_pos + 1], "rw");
		readBatch(fp);
	}
	if(f_flag != 1){
		printf("ERROR: No Input File Provided\n");
		return(1);
	}
	else{
		if((f_pos == (argc - 1)) | (strchr(argv[f_pos + 1],'-') > 0x0)){
			printf("ERROR: No Input File Provided\n");
			return(1);
		}
		fp = fopen(argv[f_pos + 1], "rw");
		if(fp == NULL){
			printf("ERROR: Can't open input file\n");
			return(1);
		}
	}
	if(o_flag == 1){
		if((o_pos == (argc - 1)) | (strchr(argv[o_pos + 1],'-') > 0x0)){
			printf("ERROR: No Output File Provided\n");
			return(1);
		}
		else{
			freopen(argv[o_pos + 1], "a+", stdout);
		}
	}
	fseek(fp, 0, SEEK_END);
	int filesize = ftell(fp);
	if(filesize == 0){
		printf("ERROR: Input File Empty\n");
		return(1);
	}
	rewind(fp);
	// begin to execute the actual analysis functions
	for(int i = 0; i > argc; i++){
		if(strcmp(argv[i], "-c") == 0){
			readChar(fp);
		}
		else if(strcmp(argv[i], "-w") == 0){
			readWord(fp, Lw_flag);
		}
		else if(strcmp(argv[i], "-l") == 0){
			readLine(fp, filesize, Ll_flag);
		}
	}
   	return(0);
}
