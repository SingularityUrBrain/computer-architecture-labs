#include <stdio.h>
#include <conio.h>
#include <xmmintrin.h>

float* solve_eq_sse(float a, float b, float c);


int main()
{
	bool supSSE = true;
	_asm {
		pushad
		mov EAX, 1
		cpuid
		test EDX, 0x2000000
		jnz found
		mov supSSE, 0
		found:
		popad
	};
	if (supSSE)
	{
		float a, b, c;

		printf("a*x^2 + b*x + c = 0\na = ");
		scanf_s("%f", &a);
		printf("b = ");
		scanf_s("%f", &b);
		printf("c = ");
		scanf_s("%f", &c);

		float *sols = solve_eq_sse(a, b, c);

		if (sols == NULL)
			printf("\nD < 0\n");
		else if (sols[0] != sols[1])
		{
			printf("\nx1 = %f", sols[0]);
			printf("\nx2 = %f\n", sols[1]);
		}
		else if (sols[0] == sols[1])
		{
			printf("\nx = %f\n", sols[0]);
		}
		_aligned_free(sols);
	}
	else
		printf("SSE isn't supported(");
	_getch();
	return 0;
}

float* solve_eq_sse(float a, float b, float c)
{

	__m128 v1 = _mm_set_ps(b, a, a, a);
	__m128 v2 = _mm_set_ps(b, c, 2, -2);

	v1 = _mm_mul_ps(v1, v2);
	v2 = _mm_set_ps(1, -4, 1, 1);
	v1 = _mm_mul_ps(v1, v2);

	float* data = (float*)_aligned_malloc(4 * sizeof(float), 16);

	_mm_storer_ps(data, v1);

	v1 = _mm_set_ss(data[0]);
	v2 = _mm_set_ss(data[1]);
	v1 = _mm_add_ss(v1, v2);

	float D;
	_mm_store_ss(&D, v1);
	if (D < 0)
		return NULL;

	v1 = _mm_sqrt_ss(v1);
	_mm_store_ss(&D, v1);

	v1 = _mm_set_ps(b, D, D, 0);
	v2 = _mm_set_ps(data[3], data[3], data[2], 1);

	v1 = _mm_div_ps(v1, v2);

	_mm_storer_ps(data, v1);

	v1 = _mm_set_ps(data[0], data[0], 0, 0);
	v2 = _mm_set_ps(data[1], data[2], 0, 0);

	v1 = _mm_add_ps(v1, v2);

	_mm_storer_ps(data, v1);
	return data;
}