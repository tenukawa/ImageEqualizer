// �N���X�� : �p���[�X�y�N�g���\��

#include "ieqpowerspectrumviewer.h"

#include <math.h>

using namespace ieq;

PowerSpectrumViewer::PowerSpectrumViewer(
	int xSize, int ySize, QWidget* parent, const char* name, WFlags flag
)
: SpectrumViewer(xSize, ySize, parent, name, flag)
{
}

PowerSpectrumViewer::~PowerSpectrumViewer()
{
}

// �P�x�Z�o()
int PowerSpectrumViewer::toLuminosity(const Complex& value)
{
	static const double mean = 65168795.0;
	static const int maxLumin = 255;
	static const int meanLumin = 128;

	double power = value.real() * value.real() + value.imag() * value.imag();
	double work = meanLumin * power;
	int lumin = (int)(
		(maxLumin * work) / ((maxLumin - meanLumin) * mean + work) + 0.5
	);

	// ���S�΍�
	if (lumin < 0) {
		lumin = 0;
	}
	if (lumin > 255) {
		lumin = 255;
	}

	return lumin;
}

// ���b�Z�[�W�ϊ�()
String PowerSpectrumViewer::toMessage(int xPos, int yPos, const Complex& value)
{
	int xLen = getXLength();
	int yLen = getYLength();

	// 4�ی����W�v�Z
	int shiftedX = xPos - (xLen / 2 - 1);
	int shiftedY = yPos - (yLen / 2 - 1);

	// �_�����W�v�Z
	double logicalX = (double)shiftedX / xLen;
	double logicalY = (double)shiftedY / yLen;

	// ���g���E�Ίp�v�Z
	double frequency = sqrt(logicalX * logicalX + logicalY * logicalY);
	double angle;
	if (logicalX != 0.0 || logicalY != 0.0) {
		angle = atan2(logicalY, logicalX) * 180.0 / ieq::PI;
	} else {
		angle = 0.0;
	}

	// �p���[�Z�o
	double power = value.real() * value.real() + value.imag() * value.imag();

	// ���b�Z�[�W����
	QString message("");
	QString formatter;
	message += tr("���g��: ");
	message += formatter.sprintf("%.3f", frequency);
	message += tr(" r/pix   ");
	message += tr("�Ίp: ");
	message += formatter.sprintf("%.1f", angle);
	message += tr(" �x   ");
	message += tr("�p���[: ");
	message += formatter.sprintf("%.5f", power / 1000000);
	// message += tr(" x 10") + QString(QChar(0x2086));
	// message += tr(" x 10<sup>6</sup>");
	message += tr(" x 10^6");

	return message;
}

