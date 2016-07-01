#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

// Estructura de dades auxiliar per treballar amb PThreads
struct Thread_operation_data{
  int vectorSize;
  float *vector;    //Operand 1
  float op2;        //Operand 2
};

//Imprimeix el vector en pantalla
void print_vector(float *vector, int vectorSize){
  printf("[ ");
  for(int i = 0; i < vectorSize; ++i){
    printf("%f ", vector[i]);
  }
  printf("] \n");
}

// Ajunta els PThreads de division. Cap retorna resultat.
void join_division_pthreads(pthread_t threads[], const int nThreads){
  for(int i = 0; i < nThreads; ++i){
    pthread_join(threads[i], NULL);
  }
}

// Divideix cada element del vector pel modul del mateix i ho guarda en el mateix vector ja normalitzat.
void *division(void *arg)
{
  Thread_operation_data data_in = *((Thread_operation_data *) arg);

  for(int i = 0; i < data_in.vectorSize; ++i){
    data_in.vector[i] = data_in.vector[i] / data_in.op2 ;
  }

  pthread_exit(NULL);
}

// Divideix el vector en tantes parts iguals com PThreads.
// A cada PThread li assigna una part del vector per treballar: Aplicar la funcion division amb el operand module com denominador.
// Si no podem dividir el vector en parts iguals, el primer PThread es queda amb la resta de feina.
void create_division_pthreads(float *vector, const int vectorSize, pthread_t threads[], Thread_operation_data tasks[] ,const int nThreads, const float module){
  //printf("Creating division pthreads...\n");

  const int range = (vectorSize / nThreads);
  const int rest = (vectorSize % nThreads);
  for(int i = 0; i < nThreads; ++i){
    tasks[i].vectorSize = i==0 ? range + rest : range;
    tasks[i].op2 = module;
    tasks[i].vector = (i== 0 ? &vector[(tasks[i].vectorSize * i)] : &vector[(tasks[i].vectorSize * i) + rest]);
    pthread_create(&threads[i], NULL, division, &tasks[i]);
  }
}

// Ajuda tots els pthreads.
// Suma els resultats dels diferents PThreads i els retorna.
float join_power_pthreads(pthread_t threads[], const int nThreads, float* sum){
  //printf("Joining power pthreads\n");
  *sum = 0 ;
  for(int i = 0; i < nThreads; ++i){
    float *result = 0;
    pthread_join(threads[i], ((void **) &result));
    *sum += *result;
    free(result);
  }
  return *sum;
}

// Al segment de vector donat, aplica la funcio power amb el operarnd donat.
// Suma totes les potencies i les retorna.
void *power(void *arg)
{
  Thread_operation_data data = *((Thread_operation_data *) arg);
  float sum = 0;
  for(int i = 0; i < data.vectorSize; ++i){
    sum += pow(data.vector[i], data.op2);
  }
  float * result = (float*) malloc(sizeof(float));
  *result = sum;
  return (void *) result;
}

// Divideix el vector en tantes parts iguals com PThreads.
// A cada PThread li assigna una part del vector per treballar: Aplicar la funcion power amb exponent com operand2.
// Si no podem dividir el vector en parts iguals, el primer PThread es queda amb la resta de feina.
void create_power_pthreads(float *vector, const int vectorSize, pthread_t threads[], Thread_operation_data tasks[] ,const int nThreads, const int exponent){
  //printf("Creating power pthreads...\n");

  const int range = (vectorSize / nThreads);
  const int rest = (vectorSize % nThreads);
  for(int i = 0; i < nThreads; ++i){
    tasks[i].vectorSize = (i==0 ? range + rest : range);
    tasks[i].op2 = exponent;
    tasks[i].vector = (i== 0 ? &vector[(tasks[i].vectorSize * i)] : &vector[(tasks[i].vectorSize * i) + rest]);
    pthread_create(&threads[i], NULL, power, &tasks[i]);
  }
}
// Normalitza un vector de mida vectorSize de manera paralela amb nThreads.
void norm_vector_pthreads(float *vector, const int vectorSize, const int nThreads)
{
  pthread_t threads[nThreads];
  Thread_operation_data tasks[nThreads];
  float *sum = (float *) malloc(sizeof(float));

  create_power_pthreads(vector, vectorSize, threads, tasks, nThreads, 2);
  join_power_pthreads(threads, nThreads, sum);
  create_division_pthreads(vector, vectorSize, threads, tasks, nThreads, sqrt(*sum));
  join_division_pthreads(threads, nThreads);

  free(sum);
}

// Normalitza un vector de mida size de manera sequencial.
void norm_vector_seq(float *vector, int size){
  float module = 0;

  for(long i = 0; i < size; ++i){
    module += pow(vector[i],2);
  }

  module = sqrt(module);


  for(long i = 0; i < size; ++i){
    vector[i] = vector[i]/module;
  }
}

// Retorna un nombre en coma flotant que pot estar entre min i max.
float random_float(const int min, const int max){
  float random = min + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(max-min)));
  return random;
}

// Generem un vector aleatori de nombres que van entre 0 i vectorSize.
// Nomes hi ha un nombre entre cada interval de diferencia 1.
// Es a dir, nomes hi ha un nombre este 0 i 1, un nomes entre 1 i 2, etc.
void generate_random_vector(float* vector, const int vectorSize){
  srand(time(NULL));
  for(int i = 0; i < vectorSize; ++i){
    vector[i] = random_float(i, i+1);
  }

  for(int i = vectorSize - 1; i > 0; i--){
    int random = rand()%i;

    float new_value = vector[i];
    vector[i] = vector[random];
    vector[random] = new_value;
  }
}

// Retorna la diferencia de temps en nanosegons entre start i end.
timespec diff(timespec &start, timespec &end) {
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

// Exemple d'execucio amb un vector de mida 10 i dos Pthreads
// ./pthreads.out 10 2
int main(int argc, char **argv)
{
  timespec start, end, elapsed;
  int executions;
  int vectorSize = atoi(argv[1]);
  int nThreads = atoi(argv[2]);

  float* vector = (float *) malloc(vectorSize * sizeof(float));

  printf("Starting...\tSize = %d\tnThreads = %d\n", vectorSize, nThreads);
  generate_random_vector(vector, vectorSize);
  //printf("Random vector: \t"); print_vector(vector, vectorSize);

  clock_gettime(CLOCK_REALTIME, &start);
  norm_vector_pthreads(vector, vectorSize, nThreads);

  //printf("Result vector: \t"); print_vector(vector, vectorSize);

  free(vector);

  clock_gettime(CLOCK_REALTIME, &end);

  elapsed = diff(start, end);
  double time_sec = (double) elapsed.tv_sec;
  double time_nsec = (double) elapsed.tv_nsec;
  double time_msec = (time_sec * 1000) + (time_nsec / 1000000);

  printf("Time: \t %f msecs \n",  time_msec);
  printf("Exit\n");
  exit(EXIT_SUCCESS);
}
