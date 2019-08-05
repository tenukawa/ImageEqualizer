// ���W���[���� : ���U�I�t�[���G�ϊ�

#include "ieqdft.h"

#include <new>
#include <stdlib.h>
#include <math.h>

// 2�������U�I���t�[���G�ϊ�()
void ieq::forwardDft2d(fftw_complex* buffer, int yLen, int xLen)
{
	// �����`�F�b�N
	if (yLen < 1 || xLen < 1) {
		return;
	}

	// ��Ɨp�o�b�t�@�m��
	fftw_complex* workBuffer = (fftw_complex*)malloc(
		sizeof(fftw_complex) * xLen * yLen
	);
	if (workBuffer == 0) {
		throw std::bad_alloc();
	}

	// �s�񕪉�@�ɂ��2����DFT
	try {
		ieq::loopForwardDft1d(workBuffer, buffer, xLen, yLen);
		ieq::loopForwardDft1d(buffer, workBuffer, yLen, xLen);
	} catch (...) {
		free(workBuffer);
		throw;
	}

	free(workBuffer);
}

// 2�������U�I�t�t�[���G�ϊ�()
void ieq::inverseDft2d(fftw_complex* buffer, int yLen, int xLen)
{
	// �����`�F�b�N
	if (yLen < 1 || xLen < 1) {
		return;
	}

	// ��Ɨp�o�b�t�@�m��
	fftw_complex* workBuffer = (fftw_complex*)malloc(
		sizeof(fftw_complex) * xLen * yLen
	);
	if (workBuffer == 0) {
		throw std::bad_alloc();
	}

	// �s�񕪉�@�ɂ��2����DFT
	try {
		ieq::loopInverseDft1d(workBuffer, buffer, xLen, yLen);
		ieq::loopInverseDft1d(buffer, workBuffer, yLen, xLen);
	} catch (...) {
		free(workBuffer);
		throw;
	}

	free(workBuffer);
}

// 1�������U�I���t�[���G�ϊ����[�v()
// outBuffer�́A�cloopNum�s�E��len���2�����z��B
// inBuffer�́A�clen�s�E��loopNum���2�����z��B
// inBuffer�̊e������ꂼ�ꏇ�t�[���G�ϊ����āA
// �ϊ����ʂ����ꂼ��outBuffer�̊e�s�Ɋi�[����B
void ieq::loopForwardDft1d(
	fftw_complex* outBuffer,      // �o�͔z��
	const fftw_complex* inBuffer, // ���͔z��
	int loopNum,                  // ���[�v��
	int len                       // ���͔z��1�񕪂̗v�f��
)
{
	int i, j, k;

	// �����`�F�b�N
	if (loopNum < 1 || len < 1) {
		return;
	}

	// ��]���q�v�Z
	fftw_complex* twiddleFactor = (fftw_complex*)malloc( // �������m��
		sizeof(fftw_complex) * len
	);
	if (twiddleFactor == 0) {
		throw std::bad_alloc();
	}
	double pi = 4 * atan(1.0); // �~�����擾
	for (i = 0; i < len; i++) {
		twiddleFactor[i].re = cos(2 * i * pi / len);
		twiddleFactor[i].im = -sin(2 * i * pi / len);
	}

	// 1����DFT��loopNum��J��Ԃ�
	int outPos = 0; // �o�͔z��̓]�L�ʒu
	for (i = 0; i < loopNum; i++) {
		// ���͔z���i��ڂ�DFT���ďo�͔z���i�s�ڂɊi�[����
		for (j = 0; j < len; j++) {
			fftw_complex sum;
			sum.re = inBuffer[i].re;
			sum.im = inBuffer[i].im;
			int inPos = i;     // ���͔z��̎Q�ƈʒu
			int remainder = 0; // ��]���q�̃C���f�b�N�X
			for (k = 1; k < len; k++) {
				inPos += loopNum;
				remainder = (remainder + j) % len;
				sum.re += // �������Z
					inBuffer[inPos].re * twiddleFactor[remainder].re -
					inBuffer[inPos].im * twiddleFactor[remainder].im;
				sum.im += // �������Z
					inBuffer[inPos].re * twiddleFactor[remainder].im +
					inBuffer[inPos].im * twiddleFactor[remainder].re;
			}
			outBuffer[outPos].re = sum.re;
			outBuffer[outPos].im = sum.im;
			outPos++;
		}
	}

	free(twiddleFactor); // ���������
}

// 1�������U�t�t�[���G�ϊ����[�v()
// outBuffer�́A�cloopNum�s�E��len���2�����z��B
// inBuffer�́A�clen�s�E��loopNum���2�����z��B
// inBuffer�̊e������ꂼ��t�t�[���G�ϊ����āA
// �ϊ����ʂ����ꂼ��outBuffer�̊e�s�Ɋi�[����B
void ieq::loopInverseDft1d(
	fftw_complex* outBuffer,      // �o�͔z��
	const fftw_complex* inBuffer, // ���͔z��
	int loopNum,                  // ���[�v��
	int len                       // ���͔z��1�񕪂̗v�f��
)
{
	int i, j, k;

	// �����`�F�b�N
	if (loopNum < 1 || len < 1) {
		return;
	}

	// ��]���q�v�Z
	fftw_complex* twiddleFactor = (fftw_complex*)malloc( // �������m��
		sizeof(fftw_complex) * len
	);
	if (twiddleFactor == 0) {
		throw std::bad_alloc();
	}
	double pi = 4 * atan(1.0); // �~�����擾
	for (i = 0; i < len; i++) {
		twiddleFactor[i].re = cos(2 * i * pi / len);
		twiddleFactor[i].im = sin(2 * i * pi / len);
	}

	// 1����DFT��loopNum��J��Ԃ�
	int outPos = 0; // �o�͔z��̓]�L�ʒu
	for (i = 0; i < loopNum; i++) {
		// ���͔z���i��ڂ�DFT���ďo�͔z���i�s�ڂɊi�[����
		for (j = 0; j < len; j++) {
			fftw_complex sum;
			sum.re = inBuffer[i].re;
			sum.im = inBuffer[i].im;
			int inPos = i;     // ���͔z��̎Q�ƈʒu
			int remainder = 0; // ��]���q�̃C���f�b�N�X
			for (k = 1; k < len; k++) {
				inPos += loopNum;
				remainder = (remainder + j) % len;
				sum.re += // �������Z
					inBuffer[inPos].re * twiddleFactor[remainder].re -
					inBuffer[inPos].im * twiddleFactor[remainder].im;
				sum.im += // �������Z
					inBuffer[inPos].re * twiddleFactor[remainder].im +
					inBuffer[inPos].im * twiddleFactor[remainder].re;
			}
			outBuffer[outPos].re = sum.re;
			outBuffer[outPos].im = sum.im;
			outPos++;
		}
	}

	free(twiddleFactor); // ���������
}

