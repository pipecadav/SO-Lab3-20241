#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>


/*
Equivalent to a Java class/enum
*/
typedef struct{
  int inputNumber;
  long long result;
} FactorialMap;

void *factorial(void *arg);
void parse_numbers(char* input, int nums[]);
int count_numbers(char* input);


int main(int argc, char* argv[]){
  srand(time(0));

  if (argc != 2){
    printf("Usage: %s <number1,number2>\n", argv[0]);
    return 1;
  }

  int count = count_numbers(argv[1]);

  FactorialMap factorialData[count];
  int nums[count];
  pthread_t threads[count];

  parse_numbers(argv[1], nums);


  for (int i = 0; i < count; i++){
    factorialData[i].inputNumber = nums[i];
    pthread_create(&threads[i], NULL, factorial, &factorialData[i]);
  }

  for (int i = 0; i < count; i++){
    pthread_join(threads[i], NULL);
    printf("Factorial of %d is %lld\n", factorialData[i].inputNumber, factorialData[i].result);
  }

  exit(0);
}

/*
  Modified method to comply with the POSIX thread library
  It also uses a FactorialMapObject type to handle the result
*/
void *factorial(void *arg){
  FactorialMap* map = (FactorialMap*)arg;
  (*map).result = 1;

  for (int num = 2; num <= (*map).inputNumber; num++){
    (*map).result = (*map).result * num;
    printf("Factorial of %d, partial result %lld\n", (*map).inputNumber, (*map).result);
    sleep(random() % 3);
  }
}

/*
  Modified method to comply with the POSIX thread library
  Removed condition that only iterates until 2
*/
void parse_numbers(char* input, int nums[]){
  char* token;
  int index = 0;

  token = strtok(input, ",");
  while (token != NULL){
    nums[index++] = atoi(token);
    token = strtok(NULL, ",");
  }
}

int count_numbers(char* input){
  int count = 1;
  const char* charPointer = input;
  while(*charPointer){
    if(*charPointer == ',') count++;
    charPointer++;
  }

  return count;
}

