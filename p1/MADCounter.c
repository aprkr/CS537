#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
		// otherwise we will need to insert at the front of the linked list
		else{
			ptr->nextChar = curr;
			ptr->prevChar = curr->prevChar;
			curr->prevChar->nextChar = ptr;
			curr->prevChar = ptr;
			(*len)++;
		}
	}
}

void readChar(FILE *fp, FILE* output, int mult){
	// first must get the amount of chars in the file in order to create the array
	rewind(fp);
	int c, charCount = 0, uniqueChar = 0;
	singleChar *head = NULL;
	singleChar *tail = NULL;
    // loop through all the chars in the file
	while((c = fgetc(fp)) != EOF){
		charCount++;
		// match tracks if this char alredy exsists in the linked list
		int match = 0;
		// start at the head and loop through each of them
		singleChar *ptr = head;
		while(ptr != NULL){
			// if it already exsits increment the frequency member
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
		// if the char does not exsist add it to the linked list
		else{
			addChar((char) c, &head, &tail, &uniqueChar, charCount);
		}
	}
	// if a flag has already been specified add a newline
	if(mult){
		fprintf(output, "\n");
	}
	// print out some stats
	fprintf(output, "Total Number of Chars = %i\nTotal Unique Chars = %i\n\n", charCount, uniqueChar);

	// after we create the linked list print out the contents
	singleChar *curr = head;
	while(curr != NULL){
		fprintf(output, "Ascii Value: %d, Char: %c, Count: %i, Initial Position: %i\n", curr->contents, curr->contents, curr->frequency, (curr->pos) - 1);
		curr = curr -> nextChar;
	}
	// free all the chars in the linked list
	singleChar *freed = head -> nextChar;
	while(freed != NULL){
		free(freed -> prevChar);
		freed = freed -> nextChar;
	}
	free(freed);
}

void addWord(char* word, WORD **head, WORD **tail, int* len, int pos){
	WORD* ptr = (WORD*) malloc(sizeof(WORD));
	// initialize the new char node in the linked list
	char* wordCpy = (char*) malloc(sizeof(char) * 100);
	strcpy(wordCpy, word);
	ptr->contents = wordCpy;
	ptr->frequency = 1;
	ptr->numChars = strlen(word);
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
		// otherwise we will insert it at the head
		else{
			ptr->nextWord = curr;
			ptr->prevWord = curr->prevWord;
			curr->prevWord->nextWord = ptr;
			curr->prevWord = ptr;
			(*len)++;
		}
	}
}

void readLongestWord(FILE* fp, FILE* output, int mult){
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
	WORD* longest[uniqueWord]; 
	int amt = 0;
	WORD* largest = head;
	WORD* curr = head -> nextWord;
	while(curr != NULL){
		if(curr->numChars > largest->numChars){
			amt = 0;
			*largest = *curr;
		}
		else if(curr->numChars == largest->numChars){
			longest[amt] = curr;
			amt++;
		}
		curr = curr -> nextWord;
	}
	if(mult){
		fprintf(output, "\n");
	}
	fprintf(output, "Longest Word is %i characters long:\n", largest -> numChars);
	fprintf(output, "\t%s\n", largest -> contents);
	if(amt > 0){
		for(int i = 0; i < amt; i++){
			fprintf(output, "\t%s\n", longest[i]->contents);
		}
	}
	// free all the chars in the linked list
	WORD *freed = head -> nextWord;
	while(freed != NULL){
		free(freed -> prevWord);
		freed = freed -> nextWord;
	}
	free(freed);
}

void readWord(FILE* fp, FILE* output, int mult){
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
	if(mult){
		fprintf(output, "\n");
	}
	fprintf(output, "Total Number of Words: %i\nTotal Unique Words: %i\n\n", wordCount, uniqueWord);
	
	WORD *curr = head;
	while(curr != NULL){
		fprintf(output, "Word: %s, Freq: %i, Initial Position: %i\n", curr->contents, curr->frequency, (curr->orderAppeared - 1));
		curr = curr -> nextWord;
	}
	// free all the chars in the linked list
	WORD *freed = head -> nextWord;
	while(freed != NULL){
		free(freed -> prevWord);
		freed = freed -> nextWord;
	}
	free(freed);
}

void addLine(char* line, LINE **head, LINE **tail, int* len, int pos, int size){
	LINE* ptr = (LINE*) malloc(sizeof(LINE));
	// initialize the new char node in the linked list
	char* lineCpy = (char*) malloc(sizeof(char) * size);
	strcpy(lineCpy, line);
	ptr->contents = lineCpy;
	ptr->frequency = 1;
	ptr->numChars = strlen(line);
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

void readLongestLine(FILE* fp, FILE* output, int filesize, int mult){
	rewind(fp);
	int lineCount = 0, uniqueLine = 0;
	char line[filesize];
	LINE *head = NULL;
	LINE *tail = NULL;
	while(fgets(line, filesize, fp) != NULL){
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
		LINE* longest[uniqueLine];
		int amt = 0;
		LINE* largest = head;
		LINE* curr = head -> nextLine;
		while(curr != NULL){
			if(curr->numChars > largest->numChars){
				amt = 0;
				*largest = *curr;
			}
			else if(curr->numChars == largest->numChars){
				longest[amt] = curr;
				amt++;
			}
			curr = curr -> nextLine;
		}
		if(mult){
			fprintf(output, "\n");
		}
		fprintf(output, "Longest Line is %i characters long:\n", (largest -> numChars) - 1);
		char *cutString = malloc(sizeof(char) * largest->numChars);
		strncpy(cutString, largest->contents, (largest->numChars - 1));
		fprintf(output, "\t%s\n", cutString);
		if(amt > 0){
			for(int i = 0; i < amt; i++){
				char *cutString = malloc(sizeof(char) * longest[i]->numChars);
				strncpy(cutString, longest[i]->contents, (longest[i]->numChars - 1));
				fprintf(output, "\t%s\n", cutString);
			}
		}
	}
	// free all the chars in the linked list
	LINE *freed = head -> nextLine;
	while(freed != NULL){
		free(freed -> prevLine);
		freed = freed -> nextLine;
	}
	free(freed);
}

void readLine(FILE* fp, FILE* output, int filesize, int mult){
	rewind(fp);
	int lineCount = 0, uniqueLine = 0;
	char line[filesize];
	LINE *head = NULL;
	LINE *tail = NULL;
	while(fgets(line, filesize, fp) != NULL){
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
		if(mult){
			fprintf(output,"\n");
		}
		fprintf(output, "Total Number of Lines: %i\nTotal Unique Lines: %i\n\n", lineCount, uniqueLine);
		
		LINE *curr = head;
		while(curr != NULL){
			char *cutString = malloc(sizeof(char) * curr->numChars);
			strncpy(cutString, curr->contents, (curr->numChars - 1));
			fprintf(output, "Line: %s, Freq: %i, Initial Position: %i\n", cutString, curr->frequency, (curr->orderAppeared - 1));
			curr = curr -> nextLine;
		}
	}
	// free all the chars in the linked list
	LINE *freed = head -> nextLine;
	while(freed != NULL){
		free(freed -> prevLine);
		freed = freed -> nextLine;
	}
	free(freed);
}

int main(int argc, char* argv[]){
	// set default print location to stdout
	FILE* output = stdout;
	// check to make sure minimum arguments have been passed in
    if((argc < 3) | (argc > 10)){
    	fprintf(output, "USAGE:\n\t./MADCounter -f <input file> -o <output file> -c -w -l -Lw -Ll\n\t\tOR\n\t./MADCounter -B <batch file>\n");
		return(1);
    }
    // an int to keep track of which flags were used
	int f_flag=0, B_flag=0, o_flag=0;
	// positions to track the following flags
	int o_pos=0, f_pos=0, B_pos=0; 
    for(int i = 0; i < argc; i++){
    	if(argv[i][0] == '-'){
			// sets all the flags as well as the positions of the respective flag
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
			else if((strcmp(argv[i], "-c") == 0) || (strcmp(argv[i], "-w") == 0) || (strcmp(argv[i], "-l") == 0) || (strcmp(argv[i], "-Lw") == 0) || strcmp(argv[i], "-Ll") == 0){
				continue;
			}
			// if none of the flags are given then throw an error
			else{
				fprintf(output, "ERROR: Invalid Flag Types\n");
				return(1);
			}
		}
	}
	FILE *fp;
	// create a recursive way to read the batch file
	if(B_flag == 1){
		// check to make sure that the batch file can be opened
		fp = fopen(argv[B_pos + 1], "rw");
		if(fp == NULL){
			fprintf(output, "ERROR: Can't open batch file\n");
			return(1);
		}
		// track the filesize, if empty throw an error
		fseek(fp, 0, SEEK_END);
		int filesize = ftell(fp);
		rewind(fp);
		if(filesize == 0){
			fprintf(output, "ERROR: Batch File Empty\n");
			return(1);
		}
		// create the variables that will hold argv and argc 
		char line[filesize];
		char* word = (char*) malloc(sizeof(char) * 100);
		int lines = 0;
		// tracks how many lines are in the batch file
		while(fgets(line, filesize, fp) != NULL){
			lines++;
		}
		// once we are at the end of the file we can calculate argc and argv
		if(feof(fp)){
			rewind(fp);
			for(int i = 0; i < lines; i++){
				int argcount = 0;
				int c = 0;
				// need to save the position so when we can count the args and restore it for the actual arg strings
				// this has to be offset by one due to the newline
				if(i > 0){
					fseek(fp, 1, SEEK_CUR);
				}
				// otherwise we dont need to consider the newline so just save the fp
				long offset = ftell(fp);
				// count the spaces and add one to get the argc
				while(c != '\n' && c != EOF){
					c = fgetc(fp);
					if(c == ' '){
						argcount++;
					}
				}
				if(argcount == 0){
					continue;
				}
				argcount++;
				// reset the file pointer so we can get the args
				fseek(fp, offset, SEEK_SET);
				char* argvars[argcount];
				for(int j = 0; j < argcount; j++){
					char* wrdCpy = (char*) malloc(sizeof(char) * 100);
					fscanf(fp, "%100s", word);
					strcpy(wrdCpy, word);
					argvars[j] = wrdCpy;
				}
				// run it with the new arguments
				main(argcount, argvars);
			}
		}
		// free the malloc'd word
		free(word);
		return(0);
	}
	// if the -f was not specified throw and error
	if(f_flag != 1){
		fprintf(output, "ERROR: No Input File Provided\n");
		return(1);
	}
	else{
		// check to make sure that the first char of the filename is not '-'
		if((f_pos == (argc - 1)) || argv[f_pos + 1][0] == '-'){
			fprintf(output, "ERROR: No Input File Provided\n");
			return(1);
		}
		// try to open the file and if it failed throw and error
		fp = fopen(argv[f_pos + 1], "rw");
		if(fp == NULL){
			fprintf(output, "ERROR: Can't open input file\n");
			return(1);
		}
	}
	// if -o is spefied we must switch where fprintf() is printed to
	if(o_flag == 1){
		if((o_pos == (argc - 1)) | (strchr(argv[o_pos + 1],'-') > 0x0)){
			fprintf(output, "ERROR: No Output File Provided\n");
			return(1);
		}
		else{
			output = fopen(argv[o_pos + 1], "w+");
		}
	}
	// make sure that the file we are reading is not empty
	fseek(fp, 0, SEEK_END);
	int filesize = ftell(fp);
	if(filesize == 0){
		fprintf(output, "ERROR: Input File Empty\n");
		return(1);
	}
	rewind(fp);
	// begin to execute the actual analysis functions
	int mult = 0;
	for(int i = 0; i < argc; i++){
		if(strcmp(argv[i], "-c") == 0){
			readChar(fp, output, mult);
			mult = 1;
		}
		else if(strcmp(argv[i], "-w") == 0){
			readWord(fp, output, mult);
			mult = 1;
		}
		else if(strcmp(argv[i], "-l") == 0){
			readLine(fp, output, filesize, mult);
			mult = 1;
		}
		else if(strcmp(argv[i], "-Lw") == 0){
			readLongestWord(fp, output, mult);
			mult = 1;
		}
		else if(strcmp(argv[i], "-Ll") == 0){
			readLongestLine(fp, output, filesize, mult);
			mult = 1;
		}
	}
	// make sure to close the already opened files
	fclose(output);
	fclose(fp);
   	return(0);
}
