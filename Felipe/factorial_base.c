#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<time.h>

long long int factorial (int);

int main (int argc, char *argv[]) {
  srand(time(0)); 
  if (argc== 2) {
    printf ("El factorial de %s es %lld\n", argv[1], factorial (atoi (argv[1])));
  }
  return 0;
}

long long int factorial (int n) {
  long long int resultado= 1;
  int num;
  for (num= 2; num<= n; num++) {
    resultado= resultado*num;
    printf ("Factorial de %d, resultado parcial %lld\n", n, resultado);
    sleep (random()%3);
  }
  return resultado;
}
