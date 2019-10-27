#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>

//#define DEBUG


double frand(double fmin, double fmax)
{
	double f = (double)rand() / RAND_MAX;
	return fmin + f * (fmax - fmin);
}

void print2d(const std::vector<std::vector<double>>& v)
{
	for (int i = 0; i < v.size(); i++) {
		for (int j = 0; j < v[0].size(); j++)
			std::cout << v[i][j] << ' ';
		std::cout << '\n';
	}
}

void print1d(const std::vector<double>& v)
{
	for (int j = 0; j < v.size(); j++)
		std::cout << v[j] << ' ';
	std::cout << '\n';
}

void mvx_omp(const std::vector<std::vector<double>>& a, const std::vector<double>& b, std::vector<double>& c)
{
	size_t m = a.size(), n = a[0].size();
	double s = 0.;
	int i, j;
#pragma omp parallel shared(a, b, c, m, n) private(i, j) reduction(+:s)
	{
#pragma omp for schedule(static)
		for (i = 0; i < m; i++) {
			s = 0.;
			for (j = 0; j < n; j++) {
				s += a[i][j] * b[j];
			}
			c[i] = s;
		}
	}
}

void mvx(const std::vector<std::vector<double>>& a, const std::vector<double>& b, std::vector<double>& c)
{
	size_t m = a.size(), n = a[0].size();
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			c[i] += a[i][j] * b[j];
}

int main(int argc, char** argv)
{
	int m, n;
	std::cin >> m >> n;

	std::vector<std::vector<double>> a(m, std::vector<double>(n));
	std::vector<double> b(n), c(m);

	srand(time(0));
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			a[i][j] = frand(1, 100);
	for (int j = 0; j < n; j++)
		b[j] = frand(1, 100);

	
	auto beg1 = std::chrono::steady_clock::now();
	mvx(a, b, c);
	auto end1 = std::chrono::steady_clock::now();
	auto elapsed_ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - beg1);
	std::cout << "\nThe time: " << elapsed_ms1.count() << " ms\n";

	omp_set_num_threads(8);
	auto beg2 = std::chrono::steady_clock::now();
	mvx_omp(a, b, c);
	auto end2 = std::chrono::steady_clock::now();
	auto elapsed_ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - beg2);
	std::cout << "\nThe time(parallel): " << elapsed_ms2.count() << " ms\n";
#ifdef DEBUG
	std::cout << "A:\n";
	print2d(a);
	std::cout << "B:\n";
	print1d(b);
	std::cout << "C:\n";
	print1d(c);
#endif
	return 0;
}