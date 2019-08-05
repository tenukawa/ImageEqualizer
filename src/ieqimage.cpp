// �N���X�� : �摜

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
	// ��̓v��������
	ftForwardPlan = fftw2d_create_plan(
	 	spd.size(), spd[0].size(), FFTW_FORWARD, FFTW_ESTIMATE | FFTW_IN_PLACE
	);
	ftInversePlan = fftw2d_create_plan(
		spd.size(), spd[0].size(), FFTW_BACKWARD, FFTW_ESTIMATE | FFTW_IN_PLACE
	);

	// ��ԗ̈�ݒ�
	setSpaceDomain(spd);
}

Image::Image(const Image& sourceImage)
	: spaceDomain(extents[sourceImage.getYLength()][sourceImage.getXLength()]),
	frequencyDomain(extents[sourceImage.getYLength()][sourceImage.getXLength()])
{
	const Matrix* pspd = &sourceImage.getSpaceDomain();
	const Matrix* pfrd = &sourceImage.getFrequencyDomain();

	// ��̓v��������
	ftForwardPlan = fftw2d_create_plan(
		pspd->size(), (*pspd)[0].size(),
		FFTW_FORWARD, FFTW_ESTIMATE | FFTW_IN_PLACE
	);
	ftInversePlan = fftw2d_create_plan(
		pfrd->size(), (*pfrd)[0].size(),
		FFTW_BACKWARD, FFTW_ESTIMATE | FFTW_IN_PLACE
	);

	// ��ԗ̈�Ǝ��g���̈�̃R�s�[
	spaceDomain = *pspd;
	frequencyDomain = *pfrd;
}

Image::~Image()
{
	// ��̓v�����j��
	fftwnd_destroy_plan(ftForwardPlan);
	fftwnd_destroy_plan(ftInversePlan);
}

// ��ԗ̈�ݒ�()
void Image::setSpaceDomain(const Matrix& spd)
{
	int i, j;

	int yLen = spd.size();
	int xLen = spd[0].size();

	// ��͗p�o�b�t�@�������m��
	fftw_complex* ftBuf = (fftw_complex*)fftw_malloc(
		sizeof(fftw_complex) * yLen * xLen
	);
	if (ftBuf == NULL) { // �m�ێ��s��
		throw bad_alloc();
	}

	// �o�b�t�@�Ɉ����s����R�s�[
	for (i = 0; i < yLen; i++) {
		for (j = 0; j < xLen; j++) {
			ftBuf[i * xLen + j].re = spd[i][j].real();
			ftBuf[i * xLen + j].im = spd[i][j].imag();
		}
	}

	// ��̓v�����ύX�E�s��T�C�Y�ύX
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

	// ���t�[���G�ϊ� (�G���[�����Ȃ�)
#ifdef USE_IEQ_DFT
	forwardDft2d(ftBuf, yLen, xLen);
#else
	fftwnd_one(ftForwardPlan, ftBuf, NULL);
#endif

	// ��ԗ̈�Ɉ����s���ݒ�
	spaceDomain = spd;

	// ���g���̈�ɕϊ����ʂ��R�s�[
	for (i = 0; i < yLen; i++) {
		for (j = 0; j < xLen; j++) {
			frequencyDomain[i][j] = Complex(
				ftBuf[i * xLen + j].re, ftBuf[i * xLen + j].im
			);
		}
	}

	// ��͗p�o�b�t�@���������
	fftw_free(ftBuf);
}

// ���g���̈�ݒ�()
void Image::setFrequencyDomain(const Matrix& frd)
{
	int i, j;

	int yLen = frd.size();
	int xLen = frd[0].size();

	// ��͗p�o�b�t�@�������m��
	fftw_complex* ftBuf = (fftw_complex*)fftw_malloc(
		sizeof(fftw_complex) * yLen * xLen
	);
	if (ftBuf == NULL) { // �m�ێ��s��
		throw bad_alloc();
	}

	// �o�b�t�@�Ɉ����s����R�s�[
	for (i = 0; i < yLen; i++) {
		for (j = 0; j < xLen; j++) {
			ftBuf[i * xLen + j].re = frd[i][j].real();
			ftBuf[i * xLen + j].im = frd[i][j].imag();
		}
	}

	// ��̓v�����ύX�E�s��T�C�Y�ύX
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

	// �t�t�[���G�ϊ� (�G���[�����Ȃ�)
#ifdef USE_IEQ_DFT
	inverseDft2d(ftBuf, yLen, xLen);
#else
	fftwnd_one(ftInversePlan, ftBuf, NULL);
#endif

	// ���g���̈�Ɉ����s���ݒ�
	frequencyDomain = frd;

	// ��ԗ̈�ɕϊ����ʂ��R�s�[
	for (i = 0; i < yLen; i++) {
		for (j = 0; j < xLen; j++) {
			spaceDomain[i][j] = Complex(
				ftBuf[i * xLen + j].re / (xLen * yLen),
				ftBuf[i * xLen + j].im / (xLen * yLen)
			);
		}
	}

	// ��͗p�o�b�t�@���������
	fftw_free(ftBuf);
}

// ��ԗ̈�Q��()
const Matrix& Image::getSpaceDomain() const
{
	return spaceDomain;
}

// ��ԗ̈�Q��(����: x���W, y���W)
const Complex& Image::getSpaceDomain(int x, int y) const // �s�v?
{
	if (x < 0 || x >= spaceDomain[0].size()) {
		throw out_of_range("�̈�͈͊O���Q�Ƃ��܂���");
	}

	if (y < 0 || y >= spaceDomain.size()) {
		throw out_of_range("�̈�͈͊O���Q�Ƃ��܂���");
	}

	return spaceDomain[y][x];
}

// ���g���̈�Q��()
const Matrix& Image::getFrequencyDomain() const
{
	return frequencyDomain;
}

// x�����T�C�Y�Q��()
int Image::getXLength() const
{
	return spaceDomain[0].size();
}

// y�����T�C�Y�Q��()
int Image::getYLength() const
{
	return spaceDomain.size();
}



// �P�̃e�X�g�h���C�o
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
	cout << (double)(endTime - startTime) / CLOCKS_PER_SEC << " �b�o��" << endl;

	return 0;
}

#endif
