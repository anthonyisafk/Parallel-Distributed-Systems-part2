#include <stdio.h>
#include <stdlib.h>
#include <math.h>
 
#define SWAP(x, y) { float temp = x; x = y; y = temp; }

int maxPower(int num, int base, int rep) {
	if (pow(base, rep) > num) {
		return -1;
	} 
	else if (pow(base, rep) == num) {
		return 0;
	} else {
		return 1 + maxPower(num, base, rep+1);
	}
}

 
// Partition using Lomuto partition scheme
int partition(float* a, int left, int right, int pIndex)
{
    // pick `pIndex` as a pivot from the array
    float pivot = a[pIndex];
 
    // Move pivot to end
    SWAP(a[pIndex], a[right]);
 
    // elements less than the pivot will be pushed to the left of `pIndex`;
    // elements more than the pivot will be pushed to the right of `pIndex`;
    // equal elements can go either way
    pIndex = left;
 
    // each time we find an element less than or equal to the pivot, `pIndex`
    // is incremented, and that element would be placed before the pivot.
    for (int i = left; i < right; i++)
    {
        if (a[i] <= pivot)
        {
            SWAP(a[i], a[pIndex]);
            pIndex++;
        }
    }
 
    // move pivot to its final place
    SWAP(a[pIndex], a[right]);
 
    // return `pIndex` (index of the pivot element)
    return pIndex;
}
 
// Returns the k'th smallest element in the list within `left…right`
// (i.e., left <= k <= right). The search space within the array is
// changing for each round – but the list is still the same size.
// Thus, `k` does not need to be updated with each round.
float kthSmallest(float* nums, int left, int right, int k)
{
    // If the array contains only one element, return that element
    if (left == right) {
        return nums[left];
    }
 
    // select `pIndex` between left and right
    int pIndex = left + rand() % (right - left + 1);
 
    pIndex = partition(nums, left, right, pIndex);
 
    // The pivot is in its final sorted position
    if (k == pIndex) {
        return nums[k];
    }
 
    // if `k` is less than the pivot index
    else if (k < pIndex) {
        return kthSmallest(nums, left, pIndex - 1, k);
    }
 
    // if `k` is more than the pivot index
    else {
        return kthSmallest(nums, pIndex + 1, right, k);
    }
}

float quickselect(float *distances, uint end) {
	// The index where the median is supposed to be.
	uint mid_index = (end+1) / 2;

	// The median is calculated depending on whether the population is even or odd.
	if ((end+1) % 2 == 0) {
	
		float mid1 = kthSmallest(distances, 0, end, mid_index - 1);
		float mid2 = kthSmallest(distances, 0, end, mid_index);
		
		printf("mid1 = %f, mid2 = %f\n", mid1, mid2);

	
		return (float) ((mid1 + mid2) / 2);
	} else {
        printf("GAMIESAI");
		return kthSmallest(distances, 0, end, mid_index);
	}
	
}

int main(){
    //float* dist_array = malloc(10*sizeof(float));
    float dist_array[] = {3,1,2,1};

    float error[] = {83.009216, 76.469833, 54.580318, 88.321083, 91.231255, 106.687050, 63.697247, 0.000000, 30.440390, 70.357079, 83.118713,
     59.834812, 111.626709, 111.519493, 61.334122, 93.576027, 99.146637, 68.391045, 79.646072, 60.714073, 124.949226, 111.355766, 63.672546, 19.306379,
     74.907074, 118.068932, 70.789757, 135.932373, 140.168671, 56.048656, 94.848129, 74.569061, 97.825386, 66.083183, 98.031700, 65.496521, 106.087410,
      110.781540, 91.676865, 121.301514, 51.650311 ,70.163841, 68.344475 ,64.258461, 70.098267 ,95.894127, 58.693130, 87.051743 ,84.499969, 90.404243 ,
      73.791367, 158.577545, 129.467697, 47.175442, 116.530136, 86.626175 ,156.653519, 75.782372, 116.912163, 14.415248, 133.861542 ,49.810383, 90.532402,
       137.078018 ,126.533913, 61.896076 ,105.016426, 20.615841, 115.714104, 138.125778,83.009216,76.469833,54.580318 ,88.321083 ,91.231255 ,106.687050,63.697247,
        0.000000 ,30.440390, 70.357079 };

    float error1[] = {83.009216, 76.469833, 54.580318, 88.321083, 91.231255, 106.687050, 63.697247, 0.000000, 30.440390, 70.357079, 83.118713,
     59.834812, 111.626709, 111.519493, 61.334122, 93.576027, 99.146637, 68.391045, 79.646072, 60.714073, 124.949226, 111.355766, 63.672546, 19.306379,
     74.907074, 118.068932, 70.789757, 135.932373, 140.168671, 56.048656, 94.848129, 74.569061, 97.825386, 66.083183, 98.031700, 65.496521, 106.087410,
      110.781540, 91.676865, 121.301514, 51.650311 ,70.163841, 68.344475 ,64.258461, 70.098267 ,95.894127, 58.693130, 87.051743 ,84.499969, 90.404243 ,
      73.791367, 158.577545, 129.467697, 47.175442, 116.530136, 86.626175 ,156.653519, 75.782372, 116.912163, 14.415248, 133.861542 ,49.810383, 90.532402,
       137.078018 ,126.533913, 61.896076 ,105.016426, 20.615841, 115.714104, 138.125778,83.009216,76.469833,54.580318 ,88.321083 ,91.231255 ,106.687050,63.697247,
        0.000000 ,30.440390, 70.357079 };

    
    //quickselect(dist_array,3);
    //quickselect(error, 79);
    int end = 79;
    uint mid_index = (end+1) / 2;
    float mid1 = kthSmallest(error, 0, end, mid_index);
	float mid2 = kthSmallest(error, 0, end, mid_index-1);
    printf("  %f ", mid1);

    printf("  %f \n", mid2);
	printf("%f", quickselect(error, end));

    int test_pow = 69;
    int maxPow = maxPower(test_pow, 2, 0);
    printf("\nMax power of %d is %d\n", test_pow, maxPow);

    
}


