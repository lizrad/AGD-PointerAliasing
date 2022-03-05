#include <stdio.h>

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


/*
* Question 1:
* How does aliasing behave when references are used? Which role can or does
* the restrict keyword play when references are used?
*/

void Cross(FVec3& A, FVec3& B, FVec3& Result)
{
	Result.x = A.y * B.z + A.z * B.y;
	Result.y = A.z * B.x + A.x * B.z;
	Result.z = A.x * B.y + A.y * B.x;
}

// It behaves the same as there is no guarantee that the references are not referencing the same variables.
// As seen in line 4 and 5 and line 16 and 17 below, 'y' gets accessed again due to a possible value change of y as there is no guarantee that it stayed the same.

/*
1	Cross(FVec3&, FVec3&, FVec3&) :
2		movss   xmm3, DWORD PTR[rsi + 8]
3		movss   xmm1, DWORD PTR[rdi + 8]
4		movss   xmm0, DWORD PTR[rdi + 4]
5		movss   xmm2, DWORD PTR[rsi + 4]
6		mulss   xmm0, xmm3
7		mulss   xmm2, xmm1
8		addss   xmm0, xmm2
9		movss   DWORD PTR[rdx], xmm0
10		movss   xmm2, DWORD PTR[rsi]
11		movss   xmm0, DWORD PTR[rdi]
12		mulss   xmm1, xmm2
13		mulss   xmm3, xmm0
14		addss   xmm1, xmm3
15		movss   DWORD PTR[rdx + 4], xmm1
16		mulss   xmm0, DWORD PTR[rsi + 4]
17		mulss   xmm2, DWORD PTR[rdi + 4]
18		addss   xmm0, xmm2
19		movss   DWORD PTR[rdx + 8], xmm0
20		ret
*/


void CrossRestricted(FVec3& __restrict A, FVec3& __restrict B, FVec3& __restrict Result)
{
	Result.x = A.y * B.z + A.z * B.y;
	Result.y = A.z * B.x + A.x * B.z;
	Result.z = A.x * B.y + A.y * B.x;
}

// As you can see below, each component is only loaded once.

/*
1	CrossRestricted(FVec3&, FVec3&, FVec3&):
2		movss   xmm2, DWORD PTR [rdi+4]
3		movss   xmm1, DWORD PTR [rdi+8]
4		movss   xmm3, DWORD PTR [rsi+8]
5		movss   xmm0, DWORD PTR [rsi+4]
6		movaps  xmm4, xmm2
7		movaps  xmm5, xmm1
8		mulss   xmm5, xmm0
9		mulss   xmm4, xmm3
10		addss   xmm4, xmm5
11		movss   xmm5, DWORD PTR [rdi]
12		mulss   xmm3, xmm5
13		movss   DWORD PTR [rdx], xmm4
14		movss   xmm4, DWORD PTR [rsi]
15		mulss   xmm0, xmm5
16		mulss   xmm1, xmm4
17		mulss   xmm2, xmm4
18		addss   xmm1, xmm3
19		addss   xmm0, xmm2
20		movss   DWORD PTR [rdx+4], xmm1
21		movss   DWORD PTR [rdx+8], xmm0
22		ret
*/

// Therefore, references behave like pointers in the context of aliasing.


/*
* Question 2:
* How could code be changed to allow more optimization possibilities for the
* compiler without the usage of the restrict keyword?
*/

// TODO: Should fix it? Possibly slower due to local variables?
void CrossNonRestricted(FVec3& A, FVec3& B, FVec3& Result)
{
	FVec3 a = A;
	FVec3 b = B;
	Result.x = a.y * b.z + a.z * b.y;
	Result.y = a.z * b.x + a.x * b.z;
	Result.z = a.x * b.y + a.y * b.x;
}

// TODO: Difference to above? Can the compiler optimize above or this better?
void CrossNonRestricted(FVec3 A, FVec3 B, FVec3& Result)
{
	Result.x = A.y * B.z + A.z * B.y;
	Result.y = A.z * B.x + A.x * B.z;
	Result.z = A.x * B.y + A.y * B.x;
}

/*
* Question 3:
* How does aliasing behave in the context of classes and member variables:
*	- Can member variables alias? If so, which variables are affected and can it be prevented somehow?
*	- Can member functions alias? Can restrict be used there?
*/


int main()
{
	FVec3 a = { 1.0f, 2.0f, 3.0f };
	FVec3 b = { 3.0f, 3.0f, 3.0f };
	FVec3 c;
	CrossRestricted(a, b, c);
	printf("%f", c.x);
	return 0;
}