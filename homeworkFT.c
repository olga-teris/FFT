#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>


typedef double complex cplx;
double PI;
int numThreads;
int n;
double *input;
cplx *output;

void* dft(void *args) {

	int thread_id = *(int*)args;

	int start = thread_id * ceil((double)n / numThreads);
	int end = fmin((thread_id + 1) * ceil((double)n / numThreads), n);

	for (int k = start; k < end; k++) {
		cplx sum = 0.0;
		for (int t = 0; t < n; t++) {  
			double angle = 2 * PI * t * k / n;
			sum += input[t] * cexp(-angle * I);
		}	
		output[k] = sum;
	}
	return NULL;
}

int main(int argc, char * argv[])
{
	PI = atan2(1, 1) * 4;

	FILE *fp, *f;
	numThreads = atoi(argv[3]);
	pthread_t tid[numThreads];
	int thread_id[numThreads];
	int i;

	fp = fopen(argv[1], "r");
	fscanf(fp, "%d", &n);
	input = (double*)calloc (n, sizeof(double));
	output = (cplx*)calloc (n, sizeof(cplx));
	for(int i = 0; i < n; i++){
		fscanf(fp, "%lf", &input[i]);
	}
	fclose(fp);

	for(i = 0;i < numThreads; i++)
		thread_id[i] = i;

	for(i = 0; i < numThreads; i++) {
		pthread_create(&(tid[i]), NULL, dft, &(thread_id[i]));
	}

	for(i = 0; i < numThreads; i++) {
		pthread_join(tid[i], NULL);
	}

	f = fopen(argv[2], "w");
	fprintf(f, "%d\n", n);
	for(int i = 0; i < n; i ++){
		fprintf(f, "%lf %lf\n", creal(output[i]), cimag(output[i]));
	}
	fclose(f);
 
	return 0;
}
