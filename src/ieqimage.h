// �錾���e : �摜�N���X

#ifndef IEQ_IMAGE
#define IEQ_IMAGE

#ifndef IEQ
#include "ieq.h"
#endif

#include <fftw.h>

class ieq::Image {
public:
	Image(const ieq::Matrix& spaceDomain);
	Image(const ieq::Image& sourceImage);
	~Image();
	void setSpaceDomain(const ieq::Matrix& spaceDomain); // ��ԗ̈�ݒ�
	void setFrequencyDomain(                             // ���g���̈�ݒ�
		const ieq::Matrix& frequencyDomain
	);
	const ieq::Matrix& getSpaceDomain() const;           // ��ԗ̈�Q��
	const ieq::Complex& getSpaceDomain(int x, int y) const; // �s�v?
	const ieq::Matrix& getFrequencyDomain() const;       // ���g���̈�Q��
	int getXLength() const;                              // x�����T�C�Y�Q��
	int getYLength() const;                              // y�����T�C�Y�Q��

private:
	ieq::Matrix spaceDomain;     // ��ԗ̈�
	ieq::Matrix frequencyDomain; // ���g���̈�
	fftwnd_plan ftForwardPlan;   // ����̓v����
	fftwnd_plan ftInversePlan;   // �t��̓v����
};

#endif
