/**
* MIT License
*
* Copyright (c) 2015 - 2021
* <https://github.com/bbqz007, http://www.cnblogs.com/bbqzsl>
*/
#include <vector>
#include <sstream>
#include <algorithm>
#include <functional>
#include <iomanip>
#include "AlgoK.h"

//ffafafa

#ifndef _MSC_VER

template<typename R, typename... A>
R rettype(R(*)(A...))
{
	return R();
}

template<typename C, typename R, typename... A>
R rettype(R(C::*)(A...))
{
	return R();
}

template<typename C, typename... A>
int rettype(void(C::*)(A...))
{
	return 0;
}

template<typename R, typename... A>
R rettype(const std::function<R(A...)>&)
{
	return R();
}

template<typename F, typename R, typename ... Args>
auto rettype (const F& f, R(F::*)(Args...))
{
	return R();
}

template<typename F>
auto rettype(const F& f)
{
	return rettype(&F::operator());
}

//typedef typename std::result_of<decltype(lambda)()>::type return_type;

template<typename T>
struct lambda_traits_helper;  // undefined

// specialize for pointer to member function
template<typename T, typename Result, typename... Args>
struct lambda_traits_helper<Result(T::*)(Args...) const>
{
    using result_type = Result;
};

template<typename Func>
struct lambda_traits 
    : lambda_traits_helper<decltype(&Func::operator())>
    // instantiate helper trait with pointer to operator()
{};

#define rtype(lambda) lambda_traits<decltype(lambda)>::result_type

#define singlepara \
#pargma omp parallel \
{ \
#pragma omp single \
{

int AlgoK::main()
{ 
#pragma omp parallel
#pragma omp single
{
	try {
		throw 1;
	}
	catch(...)
	{
	}
	AlgoK::Bench bench("main");
	auto& C = CLOSE();

	/// native MA function
	auto nMA = [this](auto& seq, const int N){
		AlgoK::Bench bench("nMA" + std::to_string(N));
		return std::move(MA(seq, N));
	};

	/// native SMA function
	auto nSMA = [this](auto& seq, const int N, const int M){
		AlgoK::Bench bench("nSMA" + std::to_string(N) + "_" + std::to_string(M));
		return std::move(SMA(seq, N, M));
	};
	auto S = C;
	decltype(nMA(C, 10)) MA10;
#pragma omp task shared(MA10)
	MA10 = nMA(C, 10);

	std::ostringstream oss;
	oss << std::ios::fixed << std::setprecision(2);
	//std::for_each(C.begin(), C.end(), [&](const float& val) { oss << val << "\n"; });

	/// an sample shows how to use openmp
	auto Mid = C;
	auto& H = HIGH();
	auto& L = LOW();
	int i;
#if 0	/// change 0 to 1, to choose parallel mode
#pragma omp parallel for
#elif 1
#pragma omp taskloop shared(Mid,H,L) private(i) grainsize(2000) nogroup
#else
#pragma omp simd
#endif
	for (i = 0; i < Mid.size(); ++i)
		Mid[i] = (H[i] + L[i]) /2;

	/// samples how to write algorithm
	/// fnma, fnsma, fnema, fndif, fndea, fnmacd
	/// 
	auto fnma = [](decltype(C) C, const int& _N){
		AlgoK::Bench bench("ma" + std::to_string(_N));
		auto S = C;
		const float N = std::max(2, _N);
		for (int i = 0, z = N - 1; i < z; ++i)
		{
			std::transform(C.begin() + i, C.begin() + C.size() - z + i, S.begin() + z, S.begin() + z, [](const float& L, float& R) { return R + L; });
		}
		std::transform(S.begin() + N - 1, S.end(), S.begin() + N - 1, [=](float& L) { return L / N; });

		//std::for_each(S.begin(), S.end(), [&](const float& val) { oss << val << "\n"; });
		return std::move(S);
	};
	/// init Y with X[0], N/M*C[0] + (M-N)/M*C[0] = 1*C[0]
	auto fnsma = [](decltype(C) C, const int& _N, const int& _M){
		AlgoK::Bench bench("sma" + std::to_string(_N) + "_" + std::to_string(_M));
		const float N = _N;
		float M = _M;
		M = std::min(M, N - 1);
		const float kN = M / N;
		const float kM = (N - M) / N;
		auto S = C;
		std::transform(C.begin() + 1, C.end(), S.begin(), S.begin() + 1, [=](const float& L, const float& R) { return kN * L + kM * R; });

		return std::move(S);
	};
	/// 2/N+1 * (sum((N-1/N+1)^k * x@(n-k))
	/// y = 2 / (N + 1) * x + (N - 1) / (N + 1) * y';
	auto fnema = [](decltype(C) C, const int& _N) {
		const float N = _N;
		float kN = 2.f / (N + 1);
		const float kM = (N - 1) / (float)(N + 1);
		auto S = C;
		std::transform(S.begin() + 1, S.end(), S.begin(), S.begin() + 1, [=](const float& x, const float& y) { return kM * y + kN * x; });

		return std::move(S);
	};
	/// ema12 - ema26
	auto fndif = [=](decltype(C) C) {
		AlgoK::Bench bench("fndif");
		auto ema12 = fnema(C, 12);
		auto S = fnema(C, 26);
		std::transform(ema12.begin(), ema12.end(), S.begin(), S.begin(), [=](const float& L, const float& R) { return L - R; });
		return std::move(S);
	};
	auto fndea = [=](decltype(C) C) {
		AlgoK::Bench bench("fndea");
		auto dif = fndif(C);
		auto S = fnema(dif, 9);
		return std::move(S);
	};
	auto fnmacd = [=](decltype(C) C) {
		auto dif = fndif(C);
		auto S = fnema(dif, 9);
		std::transform(dif.begin(), dif.end(), S.begin(), S.begin(), [=](const float& L, const float& R) { return (L - R) * 2; });
		return std::move(S);
	};
	//OutputDebugString(oss.str().c_str());

	decltype(fnsma(C, 3, 1)) sma2;
#pragma omp task shared(sma2)
	sma2 = fnsma(C, 3, 1);

	decltype(sma2) sma;
#pragma omp task shared(sma)
	sma = fnsma(C, 9, 4);
	auto ma5 = fnma(C, 5);
	auto ma10 = fnma(C, 10);
	auto ema12 = fnema(C, 12);
	auto ema26 = fnema(C, 26);
	auto dif = fndif(C);
	auto dea = fndea(C);

	decltype(fnmacd(C)) macd;
#pragma omp task shared(macd)
	macd = fnmacd(C);


	decltype(fnma(C, 120)) ma120;
#pragma omp task shared(ma120) 
	ma120 = fnma(C, 120);

	auto MA120 = nMA(C, 120);
	decltype(nSMA(C, 9, 4)) SMA31;
#pragma omp task shared(SMA31)
	SMA31 = nSMA(C, 9, 4);

#pragma omp taskwait
	/// samples how to output your results onto the KTL view.
	///
	//OUTPUT(Mid);
	
	std::copy(ma10.begin() + 9, ma10.end(), ma10.begin());
	ma10.resize(ma10.size() - 9);
	
	std::copy(ma120.begin() + 119, ma120.end(), ma120.begin());
	ma120.resize(ma120.size() - 119);
	
	OUTPUT(C);
	OUTPUT(ma5);
	OUTPUT(ma10);
	OUTPUT(sma); 
	OUTPUT(sma2);
	OUTPUT(ema12);
	OUTPUT(ema26);
	OUTPUT(dif); OutputYOffset(.2f);
	OUTPUT(dea); OutputYOffset(.2f); 
	OUTPUT(macd); OutputYOffset(.2f); 
	OUTPUT(MA10);
	OUTPUT(ma120);
	OUTPUT(MA120);
	OUTPUT(SMA31);

} // end single parallel
	return 0;
};

#endif