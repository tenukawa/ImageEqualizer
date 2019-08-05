// 宣言内容 : 離散的フーリエ変換関数プロトタイプ

// このモジュールが提供するDFT対は正規化されていない。
// m * n の行列 X に対する2次元DFT対が次の関係式を満たすように実装してある。
//     inverseDFT(forwardDFT(X)) == m * n * X
// これは、FFTWの提供するDFT対との置き換えを可能にするためである。
//  → http://www.fftw.org/fftw2_doc/fftw_3.html#SEC23 および
//     http://www.fftw.org/fftw2_doc/fftw_3.html#SEC28 を参照のこと。

#ifndef IEQ_DFT
#define IEQ_DFT

#include <fftw.h>

namespace ieq {
	extern void forwardDft2d( // 2次元離散的順フーリエ変換
		fftw_complex* buffer, int yLen, int xLen
	);
	extern void inverseDft2d( // 2次元離散的逆フーリエ変換
		fftw_complex* buffer, int yLen, int xLen
	);
	extern void loopForwardDft1d( // 1次元離散的順フーリエ変換ループ
		fftw_complex* outBuffer, const fftw_complex* inBuffer,
		int loopNum, int len
	);
	extern void loopInverseDft1d( // 1次元離散逆フーリエ変換ループ
		fftw_complex* outBuffer, const fftw_complex* inBuffer,
		int loopNum, int len
	);
}

#endif
