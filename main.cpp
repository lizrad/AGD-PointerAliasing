#include <stdio.h>


/*
* Question 1:
* How does aliasing behave when references are used? Which role can or does
* the restrict keyword play when references are used?
*/

/*
* Question 2:
* How could code be changed to allow more optimization possibilities for the
* compiler without the usage of the restrict keyword?
*/

/*
* Question 3:
* How does aliasing behave in the context of classes and member variables:
*	- Can member variables alias? If so, which variables are affected and can it be prevented somehow?
*	- Can member functions alias? Can restrict be used there?
*/

struct FVec3
{
	float x;
	float y;
	float z;
};

void Cross(FVec3* A, FVec3* B, FVec3* Result)
{
	Result->x = A->y * B->z + A->z * B->y;
	Result->y = A->z * B->x + A->x * B->z;
	Result->z = A->x * B->y + A->y * B->x;
}

int main()
{
	return 0;
}

