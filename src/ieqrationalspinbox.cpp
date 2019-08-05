// �N���X�� : �����X�s���{�b�N�X

#include "ieqrationalspinbox.h"

#include <qspinbox.h>
#include <qwidget.h>
#include <qvalidator.h>
#include <qlineedit.h>
#include <math.h>
#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#endif

ieq::RationalSpinBox::RationalSpinBox(
	unsigned precision, int downPlaces,
	int minValue, int maxValue, int value, int step,
	QWidget* parent, const char* name
) : QSpinBox(minValue, maxValue, step, parent, name) 
{
	QRegExp rationalExp("-?[0-9]+\\.?[0-9]*"); // �������K�\��
	setValidator(new QRegExpValidator(rationalExp, this)); // ���̓`�F�b�J�ݒ�
	this->precision = precision;
	this->downPlaces = downPlaces;
	editor()->setAlignment(AlignRight); // �\�����E�񂹂ɐݒ�
	setValue(value);
}

ieq::RationalSpinBox::~RationalSpinBox()
{
	// Qt�ɂ��I�u�W�F�N�g�c���[�̎������
}

// int�l���\���e�L�X�g�ϊ�()
QString ieq::RationalSpinBox::mapValueToText(int value)
{
	int seisu;    // ������
	int shousu;   // ������
	QString text;

	if (downPlaces >= 0) {
		int divisor = (int)pow(10.0, (double)downPlaces);
		seisu = abs(value) / divisor;
		shousu = abs(value) % divisor;
		if (shousu != 0) {
			if (value >= 0) {
				text.sprintf("%d.%0*d", seisu, downPlaces, shousu);
			} else {
				text.sprintf("-%d.%0*d", seisu, downPlaces, shousu);
			}
		} else { // �������݂̂̏ꍇ
			if (value >= 0) {
				text.sprintf("%d", seisu);
			} else {
				text.sprintf("-%d", seisu);
			}
		}
	} else {
		seisu = value * (int)pow(10.0, (double)-downPlaces);
		if (value >= 0) {
			text.sprintf("%d", seisu);
		} else {
			text.sprintf("-%d", seisu);
		}
	}

	return text;
}

// ���̓e�L�X�g��int�l�ϊ�()
int ieq::RationalSpinBox::mapTextToValue(bool* ok)
{
	double wk;
	int value = minValue();
	if (sscanf(text().latin1(), "%lf", &wk) == 1) {
		// ���͒l�𐮐������A�L���łȂ�����؂�̂Ă�
		wk *= pow(10.0, (double)downPlaces);
		value = (int)wk % (int)pow(10.0, (double)precision);
		*ok = true;
	} else { // ���͂����������_���`���łȂ��ꍇ
		*ok = false;
	}

	return value;
}



#ifdef UNIT_TEST
#include <qapplication.h>
// �P�̃e�X�g�h���C�o
int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	// ieq::RationalSpinBox box(3, 3, 0, 500, 0, 1);
	ieq::RationalSpinBox box(3, 2, -100, 100, -100, 1);
	box.setMinimumSize(50, 30);
	box.setMaximumSize(50, 30);

	app.setMainWidget(&box);
	box.show();

	return app.exec();
}
#endif
