/***********************************************************
  > File Name: work_pthread.c
  > Author: rose
  > Mail: -----------------
  > Created Time: 2023年01月13日 星期五 09时47分30秒
    > Modified Time:2023年01月14日 星期六 12时08分52秒
 *******************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <pthread.h>
#include <signal.h>

pthread_mutex_t lock_A,lock_B;
pthread_cond_t cd_A,cd_B,cd_C;
char buffer_A[1024];
char buffer_B[1024];

void* thread_A(void* arg)
{
	FILE* fd = fopen("ERROR.log","r");
	while(!feof(fd))
	{
		pthread_mutex_lock(&lock_A);
		while (strlen(buffer_A) > 0)
		{
			pthread_cond_wait(&cd_A,&lock_A);
		}
		fgets(buffer_A,1024,fd);
		//printf("%s\n",buffer_A);
		pthread_mutex_unlock(&lock_A);
		pthread_cond_signal(&cd_B);
		usleep(50000);
	}
	pthread_mutex_lock(&lock_A);
	strcpy(buffer_A,"Exit");
	pthread_mutex_unlock(&lock_A);
	fclose(fd);
	exit(0);
}

void* thread_B(void* arg)
{
	while(1)
	{
		pthread_mutex_lock(&lock_A);
		while (strlen(buffer_A) == 0)
		{
			pthread_cond_wait(&cd_B,&lock_A);
		}
		if (buffer_A == "Exit")
		{
			pthread_mutex_unlock(&lock_A);
			exit(0);
		}
		if (strstr(buffer_A,"E CamX") != NULL || 
		strstr(buffer_A,"E CHIUSECASE") != NULL)
		{
			//printf("%s\n",buffer_A);
			pthread_mutex_lock(&lock_B);
			strcpy(buffer_B,buffer_A);
			pthread_mutex_unlock(&lock_B);
			pthread_cond_signal(&cd_C);
		}
		strcpy(buffer_A,"");
		pthread_mutex_unlock(&lock_A);
		usleep(50000);
	}
	pthread_mutex_lock(&lock_B);
	strcpy(buffer_B,"Exit");
	pthread_mutex_unlock(&lock_B);
}

void* thread_C(void* arg)
{
	int fd = open("result.log",O_RDWR|O_CREAT);
	while(1)
	{

		pthread_mutex_lock(&lock_B);
		while(strlen(buffer_B) == 0)
		{
			pthread_cond_wait(&cd_C,&lock_B);
		}
		if (buffer_B == "Exit")
		{
			pthread_mutex_unlock(&lock_B);
			close(fd);
			exit(0);
		}
		printf("%s\n",buffer_B);
		write(fd,buffer_B,strlen(buffer_B));
		strcpy(buffer_B,"");
		pthread_mutex_unlock(&lock_B);
		pthread_cond_signal(&cd_A);
		usleep(50000);
	}
}

int main()
{
	pthread_t tid_A,tid_B,tid_C;

	pthread_mutex_init(&lock_A,NULL);
	pthread_mutex_init(&lock_B,NULL);

	pthread_cond_init(&cd_A,NULL);
	pthread_cond_init(&cd_B,NULL);

	pthread_create(&tid_A,NULL,thread_A,NULL);
	pthread_create(&tid_B,NULL,thread_B,NULL);
	pthread_create(&tid_C,NULL,thread_C,NULL);

	pthread_join(tid_A,NULL);
	pthread_join(tid_B,NULL);
	pthread_join(tid_C,NULL);

	pthread_mutex_destroy(&lock_A);
	pthread_mutex_destroy(&lock_B);

	pthread_cond_destroy(&cd_A);
	pthread_cond_destroy(&cd_B);

	return 0;
}
