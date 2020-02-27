#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
 
double PI;
typedef double complex cplx;
int numThreads;
int n;
double *input_double;
cplx *output;
cplx *input_cplx;
pthread_barrier_t barrier;
 
void _fft(cplx input[], cplx out[], int n, int step)
{
	if (step < n) {
		_fft(out, input, n, step * 2);
		_fft(out + step, input + step, n, step * 2);
 
		for (int i = 0; i < n; i += 2 * step) {
			cplx t = cexp(-I * PI * i / n) * out[i + step];
			input[i / 2]     = out[i] + t;
			input[(i + n)/2] = out[i] - t;
		}
	}
}

void* fft(void *args) {

	int thread_id = *(int*)args;

	if(numThreads == 1){
		_fft(input_cplx, output, n, 1);
	}

	if(numThreads == 2){
		if(thread_id == 0){
			_fft(output, input_cplx, n, 2);
		}
		if(thread_id == 1){
			_fft(output + 1, input_cplx + 1, n, 2);
		}
		if(thread_id == 0){
			for (int i = 0; i < n; i += 2) {
				cplx t = cexp(-I * PI * i / n) * output[i + 1];
				input_cplx[i / 2]     = output[i] + t;
				input_cplx[(i + n)/2] = output[i] - t;
			}
		}
	}

	if(numThreads == 4){
		if(thread_id == 0){
			_fft(input_cplx, output, n, 4);
		}
		if(thread_id == 1){
			_fft(input_cplx + 2, output + 2, n, 4);
		}
		if(thread_id == 2){
			_fft(input_cplx + 1, output + 1, n, 4);
		}
		if(thread_id == 3){
			_fft(input_cplx + 3, output + 3, n, 4);
		}

		pthread_barrier_wait(&barrier);

		if(thread_id == 2){

			for (int i = 0; i < n; i += 4) {
				cplx t = cexp(-I * PI * i / n) * input_cplx[i + 2];
				output[i / 2]     = input_cplx[i] + t;
				output[(i + n)/2] = input_cplx[i] - t;
			}

			output = output + 1;
			input_cplx = input_cplx + 1;

			for (int i = 0; i < n; i += 4) {
				cplx t = cexp(-I * PI * i / n) * input_cplx[i + 2];
				output[i / 2]     = input_cplx[i] + t;
				output[(i + n)/2] = input_cplx[i] - t;
			}
			output = output - 1;
			input_cplx = input_cplx - 1;
		}

		pthread_barrier_wait(&barrier);

		if(thread_id == 0){
			for (int i = 0; i < n; i += 2) {
				cplx t = cexp(-I * PI * i / n) * output[i + 1];
				input_cplx[i / 2]     = output[i] + t;
				input_cplx[(i + n)/2] = output[i] - t;
			}
		}
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
	pthread_barrier_init(&barrier, NULL, numThreads);

	fp = fopen(argv[1], "r");
	fscanf(fp, "%d", &n);
	input_double = (double*)calloc (n, sizeof(double));
	output = (cplx*)calloc (n, sizeof(cplx));
	input_cplx = (cplx*)calloc (n, sizeof(cplx));

	for(int i = 0; i < n; i++){
		fscanf(fp, "%lf", &input_double[i]);
	}
	fclose(fp);

	for(int i = 0; i < n; i++){
		input_cplx[i] = input_double[i];
		output[i] = input_double[i];
	}

	for(i = 0;i < numThreads; i++)
		thread_id[i] = i;

	for(i = 0; i < numThreads; i++) {
		pthread_create(&(tid[i]), NULL, fft, &(thread_id[i]));
	}

	for(i = 0; i < numThreads; i++) {
		pthread_join(tid[i], NULL);
	}

	f = fopen(argv[2], "w");
	fprintf(f, "%d\n", n);
	for(int i = 0; i < n; i ++){
		fprintf(f, "%lf %lf\n", creal(input_cplx[i]), cimag(input_cplx[i]));
	}
	fclose(f);
 
	return 0;
}