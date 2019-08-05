// �錾���e : �����X�s���{�b�N�X�N���X

#ifndef IEQ_RATIONAL_SPIN_BOX
#define IEQ_RATIONAL_SPIN_BOX

#ifndef IEQ
#include "ieq.h"
#endif

#include <qspinbox.h>

class ieq::RationalSpinBox : public QSpinBox {
public:
	RationalSpinBox(
		unsigned precision, int downPlaces,
		int minValue, int maxValue, int value, int step = 1,
		QWidget* parent = 0, const char* name = 0
	);
	~RationalSpinBox();

protected:
	QString mapValueToText(int value); // int�l���\���e�L�X�g�ϊ�
	int mapTextToValue(bool* ok);      // ���̓e�L�X�g��int�l�ϊ�

private:
	unsigned precision;    // ���x(�L�������̌���)
	int downPlaces;        // �����_�ȉ��̌���
};

#endif
