#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "p3200077-p3200032-res.h"

void *func(pthread_args *pargs)
{
	pthread_mutex_lock(&mutextel);
	clock_gettime(CLOCK_REALTIME, &start2);
	clock_gettime(CLOCK_REALTIME, &start1);
	
	char * Nzone = pargs->zone;
	int Nseats = pargs->numberoftickets;
	int Customerid = pargs->Customerid;
	
	while (availableTel == 0)
	{
		printf("Δέν υπάρχει διαθέσιμος τηλεφωνητής...Παρακαλώ περιμένετε\n");
		pthread_cond_wait(&condtel, &mutextel);
	}
	clock_gettime(CLOCK_REALTIME, &end1);
	clock_gettime(CLOCK_REALTIME, &end2);
	availableTel--;
	pthread_mutex_unlock(&mutextel);
	
	fulltime = fulltime + end2.tv_sec - start2.tv_sec + end2.tv_nsec/BILLION - start2.tv_nsec/BILLION;
	midtime = midtime + end1.tv_sec - start1.tv_sec + end1.tv_nsec/BILLION - start1.tv_nsec/BILLION;
	
	printf("Νέος πελάτης , θέσεις για  %d\n",Nseats);
	
	int CustSeats[Nseats];
	int i =0;
	int j = 0;
	int counter = 0;
	int counter2 = 0;
	int end=0;
	
	if(strcmp(Nzone,"A") == 0)
	{
		end = NzoneA;
	}
	else
	{
		end = NzoneA + NzoneB;
		i = 10;
	}
	
	//tel
	pthread_mutex_lock(&mutexseats);
	clock_gettime(CLOCK_REALTIME, &start2);
	sleep(rand_r(pargs->seed) % tseathigh) + tseatlow;
	
	while(i < end && counter != Nseats)
	{
		j = 0;
		while(counter < Nseats && j <10)
		{
			if(pargs->Seats[i][j].taken == 0)
				counter++;
			else
				counter = 0;
			j++;
		}
		if (counter == Nseats)
		{
			j--;
			for(int l = abs(j-Nseats + 1) ; l < j + 1; l++)
			{
				CustSeats[counter2] = pargs->Seats[i][l].seat_id;
				pargs->Seats[i][l].taken = 1;
				pargs->Seats[i][l].customerid = Customerid;
				counter2++;
			}
			printf("Βρέθηκαν κατάλληλες θέσεις\n");
			break;
		}
		counter = 0;
		i++;
	}
	clock_gettime(CLOCK_REALTIME, &end2);
	fulltime = fulltime + end2.tv_sec - start2.tv_sec + end2.tv_nsec/BILLION - start2.tv_nsec/BILLION;
	pthread_mutex_unlock(&mutexseats);
	pthread_mutex_lock(&mutextel);
	availableTel++;
	pthread_mutex_unlock(&mutextel);

	//cash
	if(counter != Nseats)
	{
		printf("Η κράτηση απέτυχε γιατί δεν υπάρχουν κατάλληλες θέσεις.\n");
		noSeats++;
	}
	else
	{
		pthread_mutex_lock(&mutexcash);
		clock_gettime(CLOCK_REALTIME, &start2);
		clock_gettime(CLOCK_REALTIME, &start1);
		while (availableCash == 0)
		{
			printf("Δέν υπάρχει διαθέσιμος ταμίας...Παρακαλώ περιμένετε\n");
			pthread_cond_wait(&condtel, &mutexcash);
		}
		clock_gettime(CLOCK_REALTIME, &end1);
		midtime = midtime + end1.tv_sec - start1.tv_sec + end1.tv_nsec/BILLION - start1.tv_nsec/BILLION;
		availableCash --;
		pthread_mutex_unlock(&mutexcash);	
			
		printf("Πραγματοποιείται πληρωμή...\n");
	
		sleep((rand_r(pargs->seed) % tcashhigh) + tcashlow); // time for the cashier to make the payment
		
		int success = (rand_r(pargs->seed) % 10);
		
		if(success < Pcardsuccess ) //configure the payment
		{
			int payment;
			paymentsuccess++;
			printf("Η κράτηση ολοκληρώθηκε επιτυχώς. Οι θέσεις σας είναι στη ζώνη %s ",Nzone);
			int line;
			if((i+1)%10 == 0)
				line = 10;
			else
				line = (i+1)%10;
			printf(",σειρά %d",line);
			printf(",αριθμός ");
			
			if(strcmp(Nzone,"A") == 0)
				payment = Nseats * CzoneA;
			else
				payment = Nseats * CzoneB;
			profit += payment;
			
			for(counter2 = 0; counter2 < Nseats ; counter2++)
				if(CustSeats[counter2] % 10 == 0)
					printf("%d ",10);
				else
					printf("%d ",CustSeats[counter2] % 10);
					
			printf("και το κόστος της παραγγελίας είναι %d",payment);
			printf("\n");
		}
		else //card payment failed -> unbind the seats
		{	
			pthread_mutex_lock(&mutexseats);
			printf("Η κράτηση απέτυχε γιατί η συναλλαγή με την πιστωτική κάρτα δεν έγινε αποδεκτή.\n");
			noCard++;
			int rem;
			for(int i = 0; i < Nseats; i++)
			{
				if(CustSeats[i] % 10 == 0)
					rem = 9;
				else
					rem = 0;
				div_t line = div(CustSeats[i] , 10);
				pargs->Seats[line.quot][line.rem - 1].taken = 0;
				pargs->Seats[line.quot][line.rem - 1 + rem].customerid = 0;
			}
			pthread_mutex_unlock(&mutexseats);
		}
	}
	clock_gettime(CLOCK_REALTIME, &end2);
	fulltime = fulltime + end2.tv_sec - start2.tv_sec + end2.tv_nsec/BILLION - start2.tv_nsec/BILLION;
	
	availableCash++;
	
	pthread_mutex_unlock(&mutexcash);
	pthread_cond_signal(&condtel);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{

	if(argc != 3) //check if arguments are correct
	{
		printf("Λάθος ορίσματα\n");
	}
	else{
		
		char *a = argv[1];
		int Ncust = atoi(a);
		
		unsigned int seed;
		seed = atoi(argv[2]);
		
		pthread_mutex_init(&mutextel, NULL);
		pthread_mutex_init(&mutexseats, NULL);
		pthread_mutex_init(&mutexcash, NULL);
		pthread_cond_init(&condtel, NULL);
		
		//create 2d array for seats
		seat Seats[30][10];
		
		//create the arguments for the threads
		pthread_args *pargs;
		pargs = (pthread_args *) malloc(sizeof(pthread_args));
		
		//create the table for the seats
		for (int i=0; i<NzoneA + NzoneB; i++)
		{
			for(int j = 0; j<Nseat; j++)
			{
				Seats[i][j].seat_id = i*10 + j + 1 ;
				pargs->Seats[i][j].seat_id = Seats[i][j].seat_id;
				pargs->Seats[i][j].taken = 0;
				pargs->Seats[i][j].customerid = 0;
				if(i < 10)
					pargs->Seats[i][j].zone = "A";
				else
					pargs->Seats[i][j].zone = "B";
			}
		}
		
		//create a thread for each costumer 
		pthread_t t_id[Ncust];
		
		for(int i = 0 ; i < Ncust; i++)
		{
			pargs->Customerid = i + 1;
			pargs->numberoftickets = ((rand_r(&seed) % 5) + 1);
	
			if((rand_r(&seed) % 10) <= PzoneA)
				pargs->zone = "A";
			else
				pargs->zone= "Β";
				
			pargs->seed = &seed;
			
			pthread_create(&t_id[i] , NULL , (void*)func , (void *)pargs);
			sleep((rand_r(&seed) % treshigh) + treslow);
			
		}
		for(int i = 0 ; i < Ncust; i++)
		{
			pthread_join(t_id[i] , NULL);
		}
		
		//output
		printf("\nΣυνολικά έσοδα απο τις πωλήσεις =%d\n",profit);
		
		midtime = midtime / Ncust;
		printf("Μέσος χρόνος αναμονής πελατών =%lf s\n",midtime);
		
		fulltime = fulltime / Ncust;
		printf("Μέσος χρόνος εξυπηρέτησης πελατών =%lf s\n",fulltime);
		
		printf("\nΠοσοστό συναλλαγών που ολοκληρώθηκαν επιτυχώς =%.1f",paymentsuccess * 100 / Ncust);
		printf("\nΠοσοστό συναλλαγών που δεν ολοκληρώθηκαν λόγω έλλειψης κατάλληλων θέσεων =%.1f",noSeats * 100 / Ncust);
		printf("\nΠοσοστό συναλλαγών που δεν ολοκληρώθηκαν λόγω μη αποδεκτής συναλλαγής =%.1f\n",noCard * 100 / Ncust);
		
		printf("      Ζώνη  Α\n");
		for (int i=0; i<NzoneA + NzoneB; i++)
		{
			for(int j = 0; j<Nseat; j++)
			{
				if(pargs->Seats[i][j].taken == 1)
					printf("- ");
				else if(Seats[i][j].seat_id % 10 != 0)
					printf("%d ",Seats[i][j].seat_id % 10);
				else
					printf("%d ",10);
			}
			printf("\n");
			if(i == 9)
				printf("      Ζώνη  B\n");
		}
		//
		pthread_mutex_destroy(&mutextel);
		pthread_mutex_destroy(&mutexseats);
		pthread_mutex_destroy(&mutexcash);
		pthread_cond_destroy(&condtel);

	}	
	exit(0);
}
