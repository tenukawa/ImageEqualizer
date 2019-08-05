// �錾���e : ���U�I�t�[���G�ϊ��֐��v���g�^�C�v

// ���̃��W���[�����񋟂���DFT�΂͐��K������Ă��Ȃ��B
// m * n �̍s�� X �ɑ΂���2����DFT�΂����̊֌W���𖞂����悤�Ɏ������Ă���B
//     inverseDFT(forwardDFT(X)) == m * n * X
// ����́AFFTW�̒񋟂���DFT�΂Ƃ̒u���������\�ɂ��邽�߂ł���B
//  �� http://www.fftw.org/fftw2_doc/fftw_3.html#SEC23 �����
//     http://www.fftw.org/fftw2_doc/fftw_3.html#SEC28 ���Q�Ƃ̂��ƁB

#ifndef IEQ_DFT
#define IEQ_DFT

#include <fftw.h>

namespace ieq {
	extern void forwardDft2d( // 2�������U�I���t�[���G�ϊ�
		fftw_complex* buffer, int yLen, int xLen
	);
	extern void inverseDft2d( // 2�������U�I�t�t�[���G�ϊ�
		fftw_complex* buffer, int yLen, int xLen
	);
	extern void loopForwardDft1d( // 1�������U�I���t�[���G�ϊ����[�v
		fftw_complex* outBuffer, const fftw_complex* inBuffer,
		int loopNum, int len
	);
	extern void loopInverseDft1d( // 1�������U�t�t�[���G�ϊ����[�v
		fftw_complex* outBuffer, const fftw_complex* inBuffer,
		int loopNum, int len
	);
}

#endif
