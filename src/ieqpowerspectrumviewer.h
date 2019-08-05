// �錾���e : �p���[�X�y�N�g���\���N���X

#ifndef IEQ_POWER_SPECTRUM_VIEWER
#define IEQ_POWER_SPECTRUM_VIEWER

#ifndef IEQ
#include "ieq.h"
#endif

#ifndef IEQ_SPECTRUM_VIEWER
#include "ieqspectrumviewer.h"
#endif

class ieq::PowerSpectrumViewer : public ieq::SpectrumViewer {
public:
	PowerSpectrumViewer(
		int xSize, int ySize,
		QWidget* parent = 0, const char* name = 0, WFlags flag = WNoAutoErase
	);
	~PowerSpectrumViewer();
protected:
	virtual int toLuminosity(const Complex& value); // �P�x�Z�o
	virtual ieq::String toMessage(                  // ���b�Z�[�W�ϊ�
		int xPos, int yPos, const Complex& value
	);
};

#endif
