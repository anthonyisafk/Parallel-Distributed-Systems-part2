#include <stdio.h> 
#include <stdlib.h>
#include <time.h>

void swap(int i, int j, int* array){
	int temp = 0;
	temp = array[i];
	array[i] = array[j];
	array[j] = temp;
}



int main(int argc, char const *argv[])
{
	int* array;
	int len = 12;
	array = malloc(len*sizeof(int));	
	int i = 0;
	int r = len;
	int cl = 0;
	int cr = -1;
	float median = 2;
	srand((unsigned) time(NULL));
	for(int i = 0 ; i< len ; i++){
		array[i]=rand()%6;
	}

	for(int i = 0 ; i< len ; i++){
		printf("%d ",array[i] );	
	}
	printf("\n");

	while(i!=r){
		if(array[i] < median ){
			swap(i, cl, array);
			cl++;
			cr++;
			i++;
		}
		else if(array[i] > median){
			if(array[r-1]<= median){
				swap(i, r-1, array);
			}
			r--;
		}
		else{
			cr++;
			i++;
		}


		for(int i = 0 ; i< len ; i++){
			printf("%d ",array[i] );	
		}
		printf("\n");

	}
	
	for(int i = 0 ; i< len ; i++){
		printf("%d ",array[i] );	
	}
	printf("\n");

	return 0;
	
}