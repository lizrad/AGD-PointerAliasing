#include <stdio.h>

#if __cplusplus
#if defined(__GNUC__)
#define XRESTRICT __restrict__
#elif defined(_MSC_VER) && MSC_VER >= 1400
#define XRESTRICT __restrict
#else
#define XRESTRICT
#endif
#else
#define XRESTRICT restrict
#endif

/*
 * General note: all assembly code was generated with godbolt using GCC 11.2 and clang 13.0.1 with O1 optimizations.
 */

struct FVec3
{
	float x;
	float y;
	float z;
};

void Cross(FVec3 *A, FVec3 *B, FVec3 *Result)
{
	Result->x = A->y * B->z + A->z * B->y;
	Result->y = A->z * B->x + A->x * B->z;
	Result->z = A->x * B->y + A->y * B->x;
}

/*
 * QUESTION 1:
 * How does aliasing behave when references are used? Which role can or does
 * the restrict keyword play when references are used?
 */

void Cross(FVec3 &A, FVec3 &B, FVec3 &Result)
{
	Result.x = A.y * B.z + A.z * B.y;
	Result.y = A.z * B.x + A.x * B.z;
	Result.z = A.x * B.y + A.y * B.x;
}

// It behaves the same as there is no guarantee that the references are not referencing the same variables.
// As seen in line 4 and 5 and line 16 and 17 below, 'y' gets accessed again due to a possible value change of y as there is no guarantee that it stayed the same.
// (Output was produced using GCC but clang showed same multiple loads)
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

void CrossRestricted(FVec3 &XRESTRICT A, FVec3 &XRESTRICT B, FVec3 &XRESTRICT Result)
{
	Result.x = A.y * B.z + A.z * B.y;
	Result.y = A.z * B.x + A.x * B.z;
	Result.z = A.x * B.y + A.y * B.x;
}

// As you can see below, each component is only loaded once.
// (Output was produced using GCC but clang showed the same improvement)
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

// Therefore, references behave exactly like pointers in the context of aliasing.

/*
 * QUESTION 2:
 * How could code be changed to allow more optimization possibilities for the
 * compiler without the usage of the restrict keyword?
 */

// VERSION 1: Using local variables to fix input references.
void CrossNonRestricted_V1(FVec3 &A, FVec3 &B, FVec3 &Result)
{
	FVec3 a = A;
	FVec3 b = B;
	Result.x = a.y * b.z + a.z * b.y;
	Result.y = a.z * b.x + a.x * b.z;
	Result.z = a.x * b.y + a.y * b.x;
}
// This version produces very similar code to the restricted one, it just moves all loads to the top (which might be a bit unoptimized)
// and still avoids multiple loads.
// (Output was produced using GCC but clang showed basically the same result)
/*
CrossNonRestricted_V1(FVec3&, FVec3&, FVec3&):
		movss   xmm0, DWORD PTR [rdi]
		movss   xmm2, DWORD PTR [rdi+4]
		movss   xmm1, DWORD PTR [rdi+8]
		movss   xmm5, DWORD PTR [rsi]
		movss   xmm6, DWORD PTR [rsi+4]
		movss   xmm3, DWORD PTR [rsi+8]
		movaps  xmm4, xmm2
		mulss   xmm4, xmm3
		movaps  xmm7, xmm1
		mulss   xmm7, xmm6
		addss   xmm4, xmm7
		movss   DWORD PTR [rdx], xmm4
		mulss   xmm1, xmm5
		mulss   xmm3, xmm0
		addss   xmm1, xmm3
		movss   DWORD PTR [rdx+4], xmm1
		mulss   xmm0, xmm6
		mulss   xmm2, xmm5
		addss   xmm0, xmm2
		movss   DWORD PTR [rdx+8], xmm0
		ret
*/

// VERSION 2: Using local variable to fix output reference.
void CrossNonRestricted_V2(FVec3 &A, FVec3 &B, FVec3 &Result)
{
	FVec3 temp_result;
	temp_result.x = A.y * B.z + A.z * B.y;
	temp_result.y = A.z * B.x + A.x * B.z;
	temp_result.z = A.x * B.y + A.y * B.x;
	Result = temp_result;
}
// Thought this might be a bit faster because it only creates one local variable instead of two like in VERSION 1
// but apparantly it produces exactly the same code
// (Output was produced using GCC but clang showed basically the same result)
/*
CrossNonRestricted_V2(FVec3&, FVec3&, FVec3&):
		movss   xmm2, DWORD PTR [rdi+4]
		movss   xmm3, DWORD PTR [rsi+8]
		movss   xmm1, DWORD PTR [rdi+8]
		movss   xmm0, DWORD PTR [rsi+4]
		movss   xmm5, DWORD PTR [rsi]
		movss   xmm6, DWORD PTR [rdi]
		movaps  xmm4, xmm2
		mulss   xmm4, xmm3
		movaps  xmm7, xmm1
		mulss   xmm7, xmm0
		addss   xmm4, xmm7
		movss   DWORD PTR [rdx], xmm4
		mulss   xmm1, xmm5
		mulss   xmm3, xmm6
		addss   xmm1, xmm3
		movss   DWORD PTR [rdx+4], xmm1
		mulss   xmm0, xmm6
		mulss   xmm2, xmm5
		addss   xmm0, xmm2
		movss   DWORD PTR [rdx+8], xmm0
		ret
*/

// VERSION 3: Use non reference parameter
void CrossNonRestricted_V3(FVec3 A, FVec3 B, FVec3 &Result)
{
	Result.x = A.y * B.z + A.z * B.y;
	Result.y = A.z * B.x + A.x * B.z;
	Result.z = A.x * B.y + A.y * B.x;
}
// Does not really make a difference in the output still avoiding the multiple loads, but is clearer for the user,
// so in our opinion this should be prefered over VERSION 1 or VERSION 2
// (Output was produced using GCC but clang showed basically the same result)
/*
CrossNonRestricted_V3(FVec3, FVec3, FVec3&):
		movq    QWORD PTR [rsp-16], xmm0
		movq    QWORD PTR [rsp-32], xmm2
		movss   xmm0, DWORD PTR [rsp-16]
		movss   xmm2, DWORD PTR [rsp-12]
		movss   xmm5, DWORD PTR [rsp-32]
		movss   xmm6, DWORD PTR [rsp-28]
		movaps  xmm4, xmm2
		mulss   xmm4, xmm3
		movaps  xmm7, xmm1
		mulss   xmm7, xmm6
		addss   xmm4, xmm7
		movss   DWORD PTR [rdi], xmm4
		mulss   xmm1, xmm5
		mulss   xmm3, xmm0
		addss   xmm1, xmm3
		movss   DWORD PTR [rdi+4], xmm1
		mulss   xmm0, xmm6
		mulss   xmm2, xmm5
		addss   xmm0, xmm2
		movss   DWORD PTR [rdi+8], xmm0
		ret
*/

// VERSION 4: Using return value for out output, thus only needing reads for potentially aliasing parameters
FVec3 CrossNonRestricted_V4(FVec3 &A, FVec3 &B)
{
	FVec3 Result;
	Result.x = A.y * B.z + A.z * B.y;
	Result.y = A.z * B.x + A.x * B.z;
	Result.z = A.x * B.y + A.y * B.x;
	return Result;
}
// Again avoids multiple loads and is basically equal to VERSION 2 except writing to a different register.
// This is the most clear version for the user though (even better when using const for the parameters) so it should be
// the prefered one.
// (Output was produced using GCC but clang showed basically the same result)
/*
CrossNonRestricted_V4(FVec3&, FVec3&):
		movss   xmm1, DWORD PTR [rdi+4]
		movss   xmm3, DWORD PTR [rsi+8]
		movss   xmm0, DWORD PTR [rdi+8]
		movss   xmm2, DWORD PTR [rsi+4]
		movss   xmm5, DWORD PTR [rsi]
		movss   xmm6, DWORD PTR [rdi]
		movaps  xmm4, xmm1
		mulss   xmm4, xmm3
		movaps  xmm7, xmm0
		mulss   xmm7, xmm2
		addss   xmm4, xmm7
		movss   DWORD PTR [rsp-20], xmm4
		mulss   xmm0, xmm5
		mulss   xmm3, xmm6
		addss   xmm0, xmm3
		movss   DWORD PTR [rsp-16], xmm0
		mulss   xmm2, xmm6
		mulss   xmm1, xmm5
		movq    xmm0, QWORD PTR [rsp-20]
		addss   xmm1, xmm2
		ret
*/

// Therefore to sum up our answer to Question 2:
// Only use references where it makes sense to avoid unnecessary extraneous loads. If references are necessary for some reason, copying to local variables
// helps avoiding multiple loads and it does not really make any difference how exactly this happens as the compiler will produce similar results anyway.

/*
 * Question 3:
 * How does aliasing behave in the context of classes and member variables:
 *	- Can member variables alias? If so, which variables are affected and can it be prevented somehow?
 *	- Can member functions alias? Can restrict be used there?
 */

class NumberStore
{
public:
	NumberStore(int i, int *ip, int &ir, float f, float *fp, float &fr, int *ip2) : i(i), ip(ip), ir(ir), f(f), fp(fp), fr(fr), ip2(ip2) {}

	int BasicPointerAliasTest(int *i);
	int BasicPointerAliasRestrictedTest(int * XRESTRICT i) XRESTRICT;
	int SameObjectAliasTest(NumberStore *n);
	int InternalAliasTest();

	int i;
	int *ip;
	int &ir;
	float f;
	float *fp;
	float &fr;

	int *ip2;
};

int NumberStore::BasicPointerAliasTest(int *i)
{
	*ip = 11;
	*i = 99;
	return *ip;
}
int NumberStore::BasicPointerAliasRestrictedTest(int * XRESTRICT i) XRESTRICT
{
	*ip = 11;
	*i = 99;
	return *ip;
}
// The compiler accesses `ip` again, so it assumes that `i` and `ip` could alias each other.
/*
NumberStore::BasicPointerAliasTest(int*):
        mov     rax, QWORD PTR [rdi+8]
        mov     DWORD PTR [rax], 11
        mov     DWORD PTR [rsi], 99
        mov     rax, QWORD PTR [rdi+8]
        mov     eax, DWORD PTR [rax]
        ret
*/
// With added XRESTRICTs, as expected, this is not the case anymore and 11 is returned right away:
/*
NumberStore::BasicPointerAliasRestrictedTest(int*):
        mov     rax, QWORD PTR [rdi+8]
        mov     DWORD PTR [rax], 11
        mov     DWORD PTR [rsi], 99
        mov     eax, 11
        ret
*/
// Note that the XRESTRICT needs to be added both to the parameter and to the NumberStore object (more accurately: the `this` pointer).
// More detail on this here: https://gcc.gnu.org/onlinedocs/gcc/Restricted-Pointers.html#Restricted-Pointers
// Same behavior in CLANG.


int NumberStore::SameObjectAliasTest(NumberStore *n)
{
	*ip = 11;
	*n->ip = 99;
	return *ip;
}
// The compiler assumes that the `ip` variables from both objects can alias each other.
/*
NumberStore::SameObjectAliasTest(NumberStore*):
        mov     rax, QWORD PTR [rdi+8]
        mov     DWORD PTR [rax], 11
        mov     rax, QWORD PTR [rsi+8]
        mov     DWORD PTR [rax], 99
        mov     rax, QWORD PTR [rdi+8]
        mov     eax, DWORD PTR [rax]
        ret
*/
// We found no way of using XRESTRICT to alter this behavior: it can only apply to the `NumberStore` objects, but not to their member variables.


int NumberStore::InternalAliasTest()
{
	*ip = 11;
	*ip2 = 99;
	return *ip;
}
// The compiler assumes that `ip` and `ip2` can alias each other.
/*
NumberStore::InternalAliasTest():
        mov     rax, QWORD PTR [rdi+8]
        mov     DWORD PTR [rax], 11
        mov     rax, QWORD PTR [rdi+48]
        mov     DWORD PTR [rax], 99
        mov     rax, QWORD PTR [rdi+8]
        mov     eax, DWORD PTR [rax]
        ret
*/
// Again, no way of using XRESTRICT to change this since there's no way of applying it to member variables.




// Member variable tests:
// (Detailed explanations here; see MemberVariableTestCases.png for the results)

// TESTCASE: Using a non pointer member, with a pointer of the same type
int Member_AliasTest_1(NumberStore *n, int *i)
{
	n->i = 11;
	*i = 99;
	return n->i;
}
/* (gcc)
Member_AliasTest_1(NumberStore*, int*):
        mov     DWORD PTR [rdi], 11
        mov     DWORD PTR [rsi], 99
        mov     eax, DWORD PTR [rdi]
        ret
*/
// Aliasing is possible.
// Adding XRESTRICT to the parameter `i` allows the compiler to optimize and return 11 directly:
// mov     eax, 11 (instead of: mov     eax, DWORD PTR [rdi])

// TESTCASE: Using a non pointer member, with a pointer of a non compatible type
int Member_AliasTest_2(NumberStore *n, float *f)
{
	n->i = 11;
	*f = 99;
	return n->i;
}
/* (gcc)
Member_AliasTest_2(NumberStore*, float*):
        mov     DWORD PTR [rdi], 11
        mov     DWORD PTR [rsi], 0x42c60000
        mov     eax, DWORD PTR [rdi]
        ret
*/
// Aliasing is possible, but only in GCC! CLANG returns 11 right away:
/* (clang)
Member_AliasTest_2(NumberStore*, float*): # @Member_AliasTest_2(NumberStore*, float*)
        mov     dword ptr [rdi], 11
        mov     dword ptr [rsi], 1120272384
        mov     eax, 11
        ret
*/
// Adding XRESTRICT causes GCC to produce the same output as CLANG.

// TESTCASE: Using a non pointer member, with a reference of the same type
int Member_AliasTest_3(NumberStore *n, int &i)
{
	n->i = 11;
	i = 99;
	return n->i;
}
/* (gcc)
Member_AliasTest_3(NumberStore*, int&):
        mov     DWORD PTR [rdi], 11
        mov     DWORD PTR [rsi], 99
        mov     eax, DWORD PTR [rdi]
        ret
*/
// Aliasing is possible.
// Adding XRESTRICT causes 11 to be returned directly.

// TESTCASE: Using a non pointer member, with a reference of a non compatible type
int Member_AliasTest_4(NumberStore *n, float &f)
{
	n->i = 11;
	f = 99;
	return n->i;
}
/* (gcc)
Member_AliasTest_4(NumberStore*, float&):
        mov     DWORD PTR [rdi], 11
        mov     DWORD PTR [rsi], 0x42c60000
        mov     eax, DWORD PTR [rdi]
        ret
*/
// Just like in Member_AliasTest_2, GCC allows aliasing whereas CLANG does not:
/* (clang)
Member_AliasTest_4(NumberStore*, float&): # @Member_AliasTest_4(NumberStore*, float&)
        mov     dword ptr [rdi], 11
        mov     dword ptr [rsi], 1120272384
        mov     eax, 11
        ret
*/
// Adding XRESTRICT again causes GCC to produce the same output as CLANG.

// TESTCASE: Using a reference member, with a pointer of the same type
int ReferenceMember_AliasTest_1(NumberStore *n, int *i)
{
	n->ir = 11;
	*i = 99;
	return n->ir;
}
/* (gcc)
ReferenceMember_AliasTest_1(NumberStore*, int*):
        mov     rax, QWORD PTR [rdi+16]
        mov     DWORD PTR [rax], 11
        mov     DWORD PTR [rsi], 99
        mov     rax, QWORD PTR [rdi+16]
        mov     eax, DWORD PTR [rax]
        ret
*/
// Aliasing is possible.
// Adding XRESTRICT causes no change in GCC, but it does cause a direct return of 11 in CLANG:
/* (clang)
ReferenceMember_AliasTest_1(NumberStore*, int*): # @ReferenceMember_AliasTest_1(NumberStore*, int*)
        mov     rax, qword ptr [rdi + 16]
        mov     dword ptr [rax], 11
        mov     dword ptr [rsi], 99
        mov     eax, 11
        ret
*/

// TESTCASE: Using a reference member, with a pointer of a non compatible type
int ReferenceMember_AliasTest_2(NumberStore *n, float *f)
{
	n->ir = 11;
	*f = 99;
	return n->ir;
}
/* (gcc)
ReferenceMember_AliasTest_2(NumberStore*, float*):
        mov     rax, QWORD PTR [rdi+16]
        mov     DWORD PTR [rax], 11
        mov     DWORD PTR [rsi], 0x42c60000
        mov     rax, QWORD PTR [rdi+16]
        mov     eax, DWORD PTR [rax]
        ret
*/
// Aliasing is possible in GCC; adding XRESTRICT causes no change.
// In CLANG, 11 is returned right away regardless of XRESTRICT or not:
/* (clang)
ReferenceMember_AliasTest_2(NumberStore*, float*): # @ReferenceMember_AliasTest_2(NumberStore*, float*)
        mov     rax, qword ptr [rdi + 16]
        mov     dword ptr [rax], 11
        mov     dword ptr [rsi], 1120272384
        mov     eax, 11
        ret
*/

// TESTCASE: Using a reference member, with a reference of the same type
int ReferenceMember_AliasTest_3(NumberStore *n, int &i)
{
	n->ir = 11;
	i = 99;
	return n->ir;
}
/* (gcc)
ReferenceMember_AliasTest_3(NumberStore*, int&):
        mov     rax, QWORD PTR [rdi+16]
        mov     DWORD PTR [rax], 11
        mov     DWORD PTR [rsi], 99
        mov     rax, QWORD PTR [rdi+16]
        mov     eax, DWORD PTR [rax]
        ret
*/
// Aliasing is possible in GCC; adding XRESTRICT causes no change.
// Aliasing is also possible in CLANG, but adding XRESTRICT does return 11 directly there:
/* (clang)
ReferenceMember_AliasTest_3(NumberStore*, int&): # @ReferenceMember_AliasTest_3(NumberStore*, int&)
        mov     rax, qword ptr [rdi + 16]
        mov     dword ptr [rax], 11
        mov     dword ptr [rsi], 99
        mov     eax, 11
        ret
*/

// TESTCASE: Using a reference member, with a reference of a non compatible type
int ReferenceMember_AliasTest_4(NumberStore *n, float &f)
{
	n->ir = 11;
	f = 99;
	return n->ir;
}
/* (gcc)
ReferenceMember_AliasTest_4(NumberStore*, float&):
        mov     rax, QWORD PTR [rdi+16]
        mov     DWORD PTR [rax], 11
        mov     DWORD PTR [rsi], 0x42c60000
        mov     rax, QWORD PTR [rdi+16]
        mov     eax, DWORD PTR [rax]
        ret
*/
// GCC allows aliasing and does not react to XRESTRICT.
// CLANG never allows aliasing regardless of XRESTRICT:
/* (clang)
ReferenceMember_AliasTest_4(NumberStore*, float&): # @ReferenceMember_AliasTest_4(NumberStore*, float&)
        mov     rax, qword ptr [rdi + 16]
        mov     dword ptr [rax], 11
        mov     dword ptr [rsi], 1120272384
        mov     eax, 11
        ret
*/

// TESTCASE: Using a pointer member, with a pointer of the same type
int PointerMember_AliasTest_1(NumberStore *n, int *i)
{
	*n->ip = 11;
	*i = 99;
	return *n->ip;
}
/* (gcc)
PointerMember_AliasTest_1(NumberStore*, int*): # @PointerMember_AliasTest_1(NumberStore*, int*)
        mov     rax, qword ptr [rdi + 8]
        mov     dword ptr [rax], 11
        mov     dword ptr [rsi], 99
        mov     eax, dword ptr [rax]
        ret
*/
// Aliasing is possible in GCC; adding XRESTRICT causes no change.
// Aliasing is also possible in CLANG, but it does react to XRESTRICT, where it returns 11 directly:
/* (clang)
PointerMember_AliasTest_1(NumberStore*, int*): # @PointerMember_AliasTest_1(NumberStore*, int*)
        mov     rax, qword ptr [rdi + 8]
        mov     dword ptr [rax], 11
        mov     dword ptr [rsi], 99
        mov     eax, 11
        ret
*/

// TESTCASE: Using a pointer member, with a pointer of a non compatible type
int PointerMember_AliasTest_2(NumberStore *n, float *f)
{
	*n->ip = 11;
	*f = 99;
	return *n->ip;
}
/* (gcc)
PointerMember_AliasTest_2(NumberStore*, float*):
        mov     rax, QWORD PTR [rdi+8]
        mov     DWORD PTR [rax], 11
        mov     DWORD PTR [rsi], 0x42c60000
        mov     rax, QWORD PTR [rdi+8]
        mov     eax, DWORD PTR [rax]
        ret
*/
// GCC always allows aliasing, CLANG never does, XRESTRICT doesn't make a difference either way:
/* (clang)
PointerMember_AliasTest_2(NumberStore*, float*): # @PointerMember_AliasTest_2(NumberStore*, float*)
        mov     rax, qword ptr [rdi + 8]
        mov     dword ptr [rax], 11
        mov     dword ptr [rsi], 1120272384
        mov     eax, 11
        ret
*/

// TESTCASE: Using a pointer member, with a reference of the same type
int PointerMember_AliasTest_3(NumberStore *n, int &i)
{
	*n->ip = 11;
	i = 99;
	return *n->ip;
}
/* (gcc)
PointerMember_AliasTest_3(NumberStore*, int&):
        mov     rax, QWORD PTR [rdi+8]
        mov     DWORD PTR [rax], 11
        mov     DWORD PTR [rsi], 99
        mov     rax, QWORD PTR [rdi+8]
        mov     eax, DWORD PTR [rax]
        ret
*/
// Again, aliasing is possible in GCC and adding XRESTRICT causes no change.
// This time though, CLANG also allows aliasing, but it does react to XRESTRICT:
/* (clang)
PointerMember_AliasTest_3(NumberStore*, int&): # @PointerMember_AliasTest_3(NumberStore*, int&)
        mov     rax, qword ptr [rdi + 8]
        mov     dword ptr [rax], 11
        mov     dword ptr [rsi], 99
        mov     eax, 11
        ret
*/

// TESTCASE: Using a pointer member, with a reference of a non compatible type
int PointerMember_AliasTest_4(NumberStore *n, float &f)
{
	*n->ip = 11;
	f = 99;
	return *n->ip;
}
/* (gcc)
PointerMember_AliasTest_4(NumberStore*, float&):
        mov     rax, QWORD PTR [rdi+8]
        mov     DWORD PTR [rax], 11
        mov     DWORD PTR [rsi], 0x42c60000
        mov     rax, QWORD PTR [rdi+8]
        mov     eax, DWORD PTR [rax]
        ret
*/
// GCC always allows aliasing, also with XRESTRICT. CLANG Never allows aliasing:
/* (clang)
PointerMember_AliasTest_4(NumberStore*, float&): # @PointerMember_AliasTest_4(NumberStore*, float&)
        mov     rax, qword ptr [rdi + 8]
        mov     dword ptr [rax], 11
        mov     dword ptr [rsi], 1120272384
        mov     eax, 11
        ret
*/

// All of these results were noted in a clearer format in MemberVariableTestCases.png.

// Takeaways from this:
// - Without XRESTRICT
//   - GCC always allows aliasing.
//   - Clang only allows aliasing when it's the same type.
// - With XRESTRICT
//   - GCC doesn't allow aliasing with non-pointer members, but it does with reference and pointer members.
//   - Clang never allows aliasing.

// It seems like CLANG generally optimizes more aggressively, whereas GCC is careful even with XRESTRICT added.

int main()
{
	FVec3 a = {1.0f, 2.0f, 3.0f};
	FVec3 b = {3.0f, 3.0f, 3.0f};
	FVec3 c;
	CrossRestricted(a, b, c);
	printf("%f\n", c.x);
	
    int *ip1 = new int(1);
    int ir = 1;
    float *fp1 = new float(2.0f);
    float fr = 2.0f;
    int *ip2 = new int(2);

    NumberStore ns(1, ip1, ir, 2.0, fp1, fr, ip2);

    int result = ns.BasicPointerAliasTest(ip1);

    printf("%i\n", result);
}