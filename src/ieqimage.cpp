// クラス名 : 画像

#include "ieqimage.h"

#ifdef USE_IEQ_DFT
#include "ieqdft.h"
#endif

#include <new>
#include <stdexcept>

#if defined(UNIT_TEST) || defined(DEBUG)
#include <iostream>
#endif

using namespace std;
using namespace boost;
using namespace ieq;

Image::Image(const Matrix& spd)
	: spaceDomain(extents[spd.size()][spd[0].size()]),
	frequencyDomain(extents[spd.size()][spd[0].size()])
{
	// 解析プラン生成
	ftForwardPlan = fftw2d_create_plan(
	 	spd.size(), spd[0].size(), FFTW_FORWARD, FFTW_ESTIMATE | FFTW_IN_PLACE
	);
	ftInversePlan = fftw2d_create_plan(
		spd.size(), spd[0].size(), FFTW_BACKWARD, FFTW_ESTIMATE | FFTW_IN_PLACE
	);

	// 空間領域設定
	setSpaceDomain(spd);
}

Image::Image(const Image& sourceImage)
	: spaceDomain(extents[sourceImage.getYLength()][sourceImage.getXLength()]),
	frequencyDomain(extents[sourceImage.getYLength()][sourceImage.getXLength()])
{
	const Matrix* pspd = &sourceImage.getSpaceDomain();
	const Matrix* pfrd = &sourceImage.getFrequencyDomain();

	// 解析プラン生成
	ftForwardPlan = fftw2d_create_plan(
		pspd->size(), (*pspd)[0].size(),
		FFTW_FORWARD, FFTW_ESTIMATE | FFTW_IN_PLACE
	);
	ftInversePlan = fftw2d_create_plan(
		pfrd->size(), (*pfrd)[0].size(),
		FFTW_BACKWARD, FFTW_ESTIMATE | FFTW_IN_PLACE
	);

	// 空間領域と周波数領域のコピー
	spaceDomain = *pspd;
	frequencyDomain = *pfrd;
}

Image::~Image()
{
	// 解析プラン破棄
	fftwnd_destroy_plan(ftForwardPlan);
	fftwnd_destroy_plan(ftInversePlan);
}

// 空間領域設定()
void Image::setSpaceDomain(const Matrix& spd)
{
	int i, j;

	int yLen = spd.size();
	int xLen = spd[0].size();

	// 解析用バッファメモリ確保
	fftw_complex* ftBuf = (fftw_complex*)fftw_malloc(
		sizeof(fftw_complex) * yLen * xLen
	);
	if (ftBuf == NULL) { // 確保失敗時
		throw bad_alloc();
	}

	// バッファに引数行列をコピー
	for (i = 0; i < yLen; i++) {
		for (j = 0; j < xLen; j++) {
			ftBuf[i * xLen + j].re = spd[i][j].real();
			ftBuf[i * xLen + j].im = spd[i][j].imag();
		}
	}

	// 解析プラン変更・行列サイズ変更
	if (spaceDomain.size() != yLen || spaceDomain[0].size() != xLen) {
		fftwnd_plan oldFPlan = ftForwardPlan;
		fftwnd_plan oldIPlan = ftInversePlan;
		ftForwardPlan = fftw2d_create_plan(
			yLen, xLen, FFTW_FORWARD, FFTW_ESTIMATE | FFTW_IN_PLACE
		);
		ftInversePlan = fftw2d_create_plan(
			yLen, xLen, FFTW_BACKWARD, FFTW_ESTIMATE | FFTW_IN_PLACE
		);
		fftwnd_destroy_plan(oldFPlan);
		fftwnd_destroy_plan(oldIPlan);
		spaceDomain.resize(extents[yLen][xLen]);
		frequencyDomain.resize(extents[yLen][xLen]);
	}

	// 順フーリエ変換 (エラー発生なし)
#ifdef USE_IEQ_DFT
	forwardDft2d(ftBuf, yLen, xLen);
#else
	fftwnd_one(ftForwardPlan, ftBuf, NULL);
#endif

	// 空間領域に引数行列を設定
	spaceDomain = spd;

	// 周波数領域に変換結果をコピー
	for (i = 0; i < yLen; i++) {
		for (j = 0; j < xLen; j++) {
			frequencyDomain[i][j] = Complex(
				ftBuf[i * xLen + j].re, ftBuf[i * xLen + j].im
			);
		}
	}

	// 解析用バッファメモリ解放
	fftw_free(ftBuf);
}

// 周波数領域設定()
void Image::setFrequencyDomain(const Matrix& frd)
{
	int i, j;

	int yLen = frd.size();
	int xLen = frd[0].size();

	// 解析用バッファメモリ確保
	fftw_complex* ftBuf = (fftw_complex*)fftw_malloc(
		sizeof(fftw_complex) * yLen * xLen
	);
	if (ftBuf == NULL) { // 確保失敗時
		throw bad_alloc();
	}

	// バッファに引数行列をコピー
	for (i = 0; i < yLen; i++) {
		for (j = 0; j < xLen; j++) {
			ftBuf[i * xLen + j].re = frd[i][j].real();
			ftBuf[i * xLen + j].im = frd[i][j].imag();
		}
	}

	// 解析プラン変更・行列サイズ変更
	if (frequencyDomain.size() != yLen || frequencyDomain[0].size() != xLen) {
		fftwnd_plan oldFPlan = ftForwardPlan;
		fftwnd_plan oldIPlan = ftInversePlan;
		ftForwardPlan = fftw2d_create_plan(
			yLen, xLen, FFTW_FORWARD, FFTW_ESTIMATE | FFTW_IN_PLACE
		);
		ftInversePlan = fftw2d_create_plan(
			yLen, xLen, FFTW_BACKWARD, FFTW_ESTIMATE | FFTW_IN_PLACE
		);
		fftwnd_destroy_plan(oldFPlan);
		fftwnd_destroy_plan(oldIPlan);
		spaceDomain.resize(extents[yLen][xLen]);
		frequencyDomain.resize(extents[yLen][xLen]);
	}

	// 逆フーリエ変換 (エラー発生なし)
#ifdef USE_IEQ_DFT
	inverseDft2d(ftBuf, yLen, xLen);
#else
	fftwnd_one(ftInversePlan, ftBuf, NULL);
#endif

	// 周波数領域に引数行列を設定
	frequencyDomain = frd;

	// 空間領域に変換結果をコピー
	for (i = 0; i < yLen; i++) {
		for (j = 0; j < xLen; j++) {
			spaceDomain[i][j] = Complex(
				ftBuf[i * xLen + j].re / (xLen * yLen),
				ftBuf[i * xLen + j].im / (xLen * yLen)
			);
		}
	}

	// 解析用バッファメモリ解放
	fftw_free(ftBuf);
}

// 空間領域参照()
const Matrix& Image::getSpaceDomain() const
{
	return spaceDomain;
}

// 空間領域参照(引数: x座標, y座標)
const Complex& Image::getSpaceDomain(int x, int y) const // 不要?
{
	if (x < 0 || x >= spaceDomain[0].size()) {
		throw out_of_range("領域範囲外を参照しました");
	}

	if (y < 0 || y >= spaceDomain.size()) {
		throw out_of_range("領域範囲外を参照しました");
	}

	return spaceDomain[y][x];
}

// 周波数領域参照()
const Matrix& Image::getFrequencyDomain() const
{
	return frequencyDomain;
}

// x方向サイズ参照()
int Image::getXLength() const
{
	return spaceDomain[0].size();
}

// y方向サイズ参照()
int Image::getYLength() const
{
	return spaceDomain.size();
}



// 単体テストドライバ
#ifdef UNIT_TEST
#include <stdio.h>
#include <time.h>

void printMatrix(const Matrix& mat)
{
	/* int i, j;

	for (i = 0; i < mat.size(); i++) {
		for (j = 0; j < mat[i].size(); j++) {
			cout << mat[i][j].real() <<" + "<< mat[i][j].imag() <<" i"<< endl;
		}
		cout << endl;
	} */
}

int main(void)
{
	clock_t startTime = clock();
	int i, j;
	Matrix mat(extents[768][1024]);
	for (i = 0; i < mat.size(); i++) {
		for (j = 0; j < mat[0].size(); j++) {
			mat[i][j] = Complex(3.0, 0.0);
		}
	}
	/* mat[0][0] = Complex(5.0, 0.0);
	mat[0][1] = Complex(3.0, 0.0);
	mat[0][2] = Complex(10.0, 0.0);
	mat[1][0] = Complex(2.0, 0.0);
	mat[1][1] = Complex(6.0, 0.0);
	mat[1][2] = Complex(7.0, 0.0);
	mat[2][0] = Complex(1.0, 0.0);
	mat[2][1] = Complex(9.0, 0.0);
	mat[2][2] = Complex(5.0, 0.0); */
	/* Matrix mat(extents[1][5]);
	mat[0][0] = Complex(1, 0);
	mat[0][1] = Complex(2, 0);
	mat[0][2] = Complex(3, 0);
	mat[0][3] = Complex(4, 0);
	mat[0][4] = Complex(5, 0); */

	cout << "source matrix" << endl;
	printMatrix(mat);

	cout << "constructor test" << endl;
	Image img(mat);
	cout << "frequency domain" << endl;
	printMatrix(img.getFrequencyDomain());
	cout << "space domain" << endl;
	printMatrix(img.getSpaceDomain());

	cout << endl;
	cout << "setFrequencyDomain() method test" << endl;
	Matrix mat2(img.getFrequencyDomain());
	img.setFrequencyDomain(mat2);
	cout << "frequency domain" << endl;
	printMatrix(img.getFrequencyDomain());
	cout << "space domain" << endl;
	printMatrix(img.getSpaceDomain());

	clock_t endTime = clock();
	cout << endl;
	cout << (double)(endTime - startTime) / CLOCKS_PER_SEC << " 秒経過" << endl;

	return 0;
}

#endif
