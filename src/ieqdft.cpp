// モジュール名 : 離散的フーリエ変換

#include "ieqdft.h"

#include <new>
#include <stdlib.h>
#include <math.h>

// 2次元離散的順フーリエ変換()
void ieq::forwardDft2d(fftw_complex* buffer, int yLen, int xLen)
{
	// 引数チェック
	if (yLen < 1 || xLen < 1) {
		return;
	}

	// 作業用バッファ確保
	fftw_complex* workBuffer = (fftw_complex*)malloc(
		sizeof(fftw_complex) * xLen * yLen
	);
	if (workBuffer == 0) {
		throw std::bad_alloc();
	}

	// 行列分解法による2次元DFT
	try {
		ieq::loopForwardDft1d(workBuffer, buffer, xLen, yLen);
		ieq::loopForwardDft1d(buffer, workBuffer, yLen, xLen);
	} catch (...) {
		free(workBuffer);
		throw;
	}

	free(workBuffer);
}

// 2次元離散的逆フーリエ変換()
void ieq::inverseDft2d(fftw_complex* buffer, int yLen, int xLen)
{
	// 引数チェック
	if (yLen < 1 || xLen < 1) {
		return;
	}

	// 作業用バッファ確保
	fftw_complex* workBuffer = (fftw_complex*)malloc(
		sizeof(fftw_complex) * xLen * yLen
	);
	if (workBuffer == 0) {
		throw std::bad_alloc();
	}

	// 行列分解法による2次元DFT
	try {
		ieq::loopInverseDft1d(workBuffer, buffer, xLen, yLen);
		ieq::loopInverseDft1d(buffer, workBuffer, yLen, xLen);
	} catch (...) {
		free(workBuffer);
		throw;
	}

	free(workBuffer);
}

// 1次元離散的順フーリエ変換ループ()
// outBufferは、縦loopNum行・横len列の2次元配列。
// inBufferは、縦len行・横loopNum列の2次元配列。
// inBufferの各列をそれぞれ順フーリエ変換して、
// 変換結果をそれぞれoutBufferの各行に格納する。
void ieq::loopForwardDft1d(
	fftw_complex* outBuffer,      // 出力配列
	const fftw_complex* inBuffer, // 入力配列
	int loopNum,                  // ループ回数
	int len                       // 入力配列1列分の要素数
)
{
	int i, j, k;

	// 引数チェック
	if (loopNum < 1 || len < 1) {
		return;
	}

	// 回転因子計算
	fftw_complex* twiddleFactor = (fftw_complex*)malloc( // メモリ確保
		sizeof(fftw_complex) * len
	);
	if (twiddleFactor == 0) {
		throw std::bad_alloc();
	}
	double pi = 4 * atan(1.0); // 円周率取得
	for (i = 0; i < len; i++) {
		twiddleFactor[i].re = cos(2 * i * pi / len);
		twiddleFactor[i].im = -sin(2 * i * pi / len);
	}

	// 1次元DFTをloopNum回繰り返す
	int outPos = 0; // 出力配列の転記位置
	for (i = 0; i < loopNum; i++) {
		// 入力配列のi列目をDFTして出力配列のi行目に格納する
		for (j = 0; j < len; j++) {
			fftw_complex sum;
			sum.re = inBuffer[i].re;
			sum.im = inBuffer[i].im;
			int inPos = i;     // 入力配列の参照位置
			int remainder = 0; // 回転因子のインデックス
			for (k = 1; k < len; k++) {
				inPos += loopNum;
				remainder = (remainder + j) % len;
				sum.re += // 実部加算
					inBuffer[inPos].re * twiddleFactor[remainder].re -
					inBuffer[inPos].im * twiddleFactor[remainder].im;
				sum.im += // 虚部加算
					inBuffer[inPos].re * twiddleFactor[remainder].im +
					inBuffer[inPos].im * twiddleFactor[remainder].re;
			}
			outBuffer[outPos].re = sum.re;
			outBuffer[outPos].im = sum.im;
			outPos++;
		}
	}

	free(twiddleFactor); // メモリ解放
}

// 1次元離散逆フーリエ変換ループ()
// outBufferは、縦loopNum行・横len列の2次元配列。
// inBufferは、縦len行・横loopNum列の2次元配列。
// inBufferの各列をそれぞれ逆フーリエ変換して、
// 変換結果をそれぞれoutBufferの各行に格納する。
void ieq::loopInverseDft1d(
	fftw_complex* outBuffer,      // 出力配列
	const fftw_complex* inBuffer, // 入力配列
	int loopNum,                  // ループ回数
	int len                       // 入力配列1列分の要素数
)
{
	int i, j, k;

	// 引数チェック
	if (loopNum < 1 || len < 1) {
		return;
	}

	// 回転因子計算
	fftw_complex* twiddleFactor = (fftw_complex*)malloc( // メモリ確保
		sizeof(fftw_complex) * len
	);
	if (twiddleFactor == 0) {
		throw std::bad_alloc();
	}
	double pi = 4 * atan(1.0); // 円周率取得
	for (i = 0; i < len; i++) {
		twiddleFactor[i].re = cos(2 * i * pi / len);
		twiddleFactor[i].im = sin(2 * i * pi / len);
	}

	// 1次元DFTをloopNum回繰り返す
	int outPos = 0; // 出力配列の転記位置
	for (i = 0; i < loopNum; i++) {
		// 入力配列のi列目をDFTして出力配列のi行目に格納する
		for (j = 0; j < len; j++) {
			fftw_complex sum;
			sum.re = inBuffer[i].re;
			sum.im = inBuffer[i].im;
			int inPos = i;     // 入力配列の参照位置
			int remainder = 0; // 回転因子のインデックス
			for (k = 1; k < len; k++) {
				inPos += loopNum;
				remainder = (remainder + j) % len;
				sum.re += // 実部加算
					inBuffer[inPos].re * twiddleFactor[remainder].re -
					inBuffer[inPos].im * twiddleFactor[remainder].im;
				sum.im += // 虚部加算
					inBuffer[inPos].re * twiddleFactor[remainder].im +
					inBuffer[inPos].im * twiddleFactor[remainder].re;
			}
			outBuffer[outPos].re = sum.re;
			outBuffer[outPos].im = sum.im;
			outPos++;
		}
	}

	free(twiddleFactor); // メモリ解放
}

