#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

const int Ntel = 3;
const int Ncash = 2;
const int Nseat = 10;
const int NzoneA = 10;
const int NzoneB = 20;
const int PzoneA = 3;
const int CzoneA = 30;
const int CzoneB = 20;
const int Nseatlow = 1;
const int Nseathigh = 5;
const int treslow = 1;
const int treshigh = 5;
const int tseatlow = 5;
const int tseathigh = 13;
const int tcashlow = 4;
const int tcashhigh = 8;
const int Pcardsuccess = 9;

#define BILLION  1000000000L

int availableTel = Ntel;
int availableCash = Ncash;
int profit = 0;
double midtime = 0;
double fulltime = 0;
float noSeats = 0;
float noCard = 0;
float paymentsuccess =0;

struct timespec start2, end2, start1 , end1;

pthread_cond_t condtel = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutextel = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexseats = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	mutexcash = PTHREAD_MUTEX_INITIALIZER;

typedef struct seat
{
	int seat_id;
	int taken;
	int customerid;
	char * zone;
}seat;

typedef struct pthread_args{
	seat Seats[30][10];
	unsigned int numberoftickets;
	char * zone;
	unsigned int * seed;
	int Customerid;
	
} pthread_args;
