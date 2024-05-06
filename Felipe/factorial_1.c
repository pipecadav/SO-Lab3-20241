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


int main(int argc, char* argv[]){
  srand(time(0));

  if (argc != 2){
    printf("Usage: %s <number1,number2>\n", argv[0]);
    return 1;
  }

  FactorialMap factorialData[2];
  int nums[2];
  pthread_t threads[2];

  parse_numbers(argv[1], nums);


  for (int i = 0; i < 2; i++){
    factorialData[i].inputNumber = nums[i];
    pthread_create(&threads[i], NULL, factorial, &factorialData[i]);
  }

  for (int i = 0; i < 2; i++){
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
  Parses the input provided that the input is an array
  of numbers separated by commas
*/
void parse_numbers(char* input, int nums[]){
  char* token;
  int count = 0;

  token = strtok(input, ",");

  while (token != NULL && count < 2){
    nums[count++] = atoi(token);
    token = strtok(NULL, ",");
  }

  if (count != 2){
    fprintf(stderr, "Error: Please enter exactly two numbers separated by a comma.\n");
    exit(1);
  }
}
