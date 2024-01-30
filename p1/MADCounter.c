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

typedef struct lines{
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
	printf("Total Number of Chars = %i\nTotal Unique Chars = %i\n\n", charCount, uniqueChar);
	
	// after we create the linked list print out the contents
	singleChar *curr = head;
	while(curr != NULL){
		printf("Ascii Value: %d, Char: %c, Count: %i, Inital Position: %i\n", curr->contents, curr->contents, curr->frequency, (curr->pos) - 1);
		curr = curr -> nextChar;
	}
	

	// // grab all of the words out of the file and put it in the structs
	// fseek(fp, 0, SEEK_SET);
	// char word[50];
	// int wordCount;
	// while(fscanf(fp, "%50s", word) != EOF){
	// 	wordCount++;
	// 	printf("%s\n", word);
	// }
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
	fclose(fp);
}

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
    char flags[7][3] = {"-f", "-o", "-c", "-w", "-l", "-Lw", "-Ll"};
    // an int to keep track of if the -o flag was used
    int o_flag, c_flag, o_pos = 0;
    for(int i = 0; i < argc; i++){
    	if(strchr(argv[i],'-') != NULL){
    		for(int j = 0; j < 7; j++){
				if(strcmp(argv[i],flags[j]) == 0){
					if(strcmp(argv[i], "-o") == 0){
						o_flag = 1;
						o_pos = i;
					}
					else if(strcmp(argv[i], "-c") == 0){
						c_flag = 1;
					}
					break;
				}
				// if an incorrect flag has been used errror out
				else if(i == 7){
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
   	else{
   		if(c_flag == 1){
			readChar(fp);
		}
   	}
   	return(0);
}
