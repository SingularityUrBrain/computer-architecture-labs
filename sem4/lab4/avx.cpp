#include "pch.h"
#include <iostream>
#include <ctime>
#include <immintrin.h>

#define CONSTANT 018000000h

int main() {
	bool supAVX = true;
	_asm {
		pushad
		pushf
		;processor check
		mov eax, 1
		cpuid
		and ecx, CONSTANT
		cmp ecx, CONSTANT
		jne not_supported
		;os check
		mov ecx, 0
		XGETBV
		and eax, 06h
		cmp eax, 06h
		jne not_supported
		jmp done
		not_supported:
		mov supAVX, 0
		done:
		popf
		popad
	};
	if (supAVX) 
	{
		float a[8], b[8], c[8], l = 0.f, h = 20.f;
		double d[8];
		srand(static_cast <unsigned> (time(0)));
		std::cout << "a: ";
		for (int i = 0; i < 8; i++) {
			a[i] = l + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (h - l)));
			b[i] = l + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (h - l)));
			c[i] = l + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (h - l)));
			d[i] = l + static_cast <double> (rand()) / (static_cast <double> (RAND_MAX / (h - l)));

			std::cout << a[i] << ' ';
		}
		std::cout << "\nb: ";
		for (int i = 0; i < 8; i++) std::cout << b[i] << ' ';
		std::cout << "\nc: ";
		for (int i = 0; i < 8; i++) std::cout << c[i] << ' ';
		std::cout << "\nd: ";
		for (int i = 0; i < 8; i++) std::cout << d[i] << ' ';

		__m256 a_vec = _mm256_loadu_ps(a);
		__m256 b_vec = _mm256_loadu_ps(b);
		__m256 c_vec = _mm256_loadu_ps(c);
		__m256d d_vecl = _mm256_loadu_pd(d);
		__m256d d_vecr = _mm256_loadu_pd(&d[4]);

		a_vec = _mm256_sub_ps(a_vec, b_vec);
		_mm256_storeu_ps(a, a_vec);
		double ab_sub_double[8], c_double[8];
		for (int i = 0; i < 8; i++) {
			ab_sub_double[i] = static_cast <double> (a[i]);
			c_double[i] = static_cast <double> (c[i]);
		}
		__m256d ab_sub_vecl = _mm256_loadu_pd(ab_sub_double);
		__m256d ab_sub_vecr = _mm256_loadu_pd(&ab_sub_double[4]);
		__m256d c_vecl = _mm256_loadu_pd(c_double);
		__m256d c_vecr = _mm256_loadu_pd(&c_double[4]);

		__m256d cd_sub_vecl = _mm256_sub_pd(c_vecl, d_vecl);
		__m256d cd_sub_vecr = _mm256_sub_pd(c_vecr, d_vecr);
		__m256d ab_cd_mul_vecl = _mm256_mul_pd(ab_sub_vecl, cd_sub_vecl);
		__m256d ab_cd_mul_vecr = _mm256_mul_pd(ab_sub_vecr, cd_sub_vecr);

		_mm256_store_pd(d, ab_cd_mul_vecl);
		_mm256_store_pd(&d[4], ab_cd_mul_vecr);

		std::cout << "\nf: ";
		for (int i = 0; i < 8; i++) std::cout << d[i] << ' ';
	}
	else
		std::cout << "processor or OS doesn't support AVX\n";
	system("pause");
}