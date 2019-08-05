// �N���X�� : �t�B���^�ҏW
#include "ieqfiltereditor.h"

#include <math.h>
#include <qpixmap.h>
#include <qwidget.h>
#include <qpainter.h>
#include <stdexcept>
#include <new>

#ifdef DEBUG
#include <iostream>
#endif

using namespace ieq;
using namespace boost;
using namespace std;

FilterEditor::FilterEditor(
	int width, int height, QWidget* parent, const char* name
)
	: QWidget(parent, name, WNoAutoErase), logicalFilter(0)
{
	resize(width, height);

	coloring = true;
	rangeUpdated = false;
	varied = false;

	startFrequency = 0.0;
	endFrequency = 0.0;
	startAngle = 0.0;
	endAngle = 180.0;

	physicalFilter = new QImage();
	rangeMap = new QPixmap();
	initialize(); // ������
	setMouseTracking(true);
}

FilterEditor::~FilterEditor()
{
	// �������
	delete logicalFilter;
	delete physicalFilter;
	delete rangeMap;
}

// ������()
void FilterEditor::initialize()
{
	saveCount = 0;
	filePath = QString(); // �k���������ݒ�

	if (logicalFilter != 0) {
		delete logicalFilter;
	}
	logicalFilter = new Filter();
	resetBuffer(); // �o�b�t�@�Đݒ�

	repaint(true, true); // �ĕ`��
}

// �t�B���^�Ǎ�()
void FilterEditor::loadFilter(const String& path)
{
	try {
		logicalFilter->load(path); // �Ǎ�
	} catch (...) {
		throw;
	}

	resetBuffer(); // �o�b�t�@�Đݒ�

	repaint(true, true); // �ĕ`��
	saveCount = 0;
	filePath = path;
}

// �t�B���^�ۑ�()
void FilterEditor::saveFilter(const String& path)
{
	try {
		logicalFilter->save(path); // �ۑ�
	} catch (...) {
		throw;
	}

	saveCount = 0;
	filePath = path;
}

// �o�b�t�@�Đݒ�()
void FilterEditor::resetBuffer()
{
	int width = logicalFilter->getXLength();
	int height = logicalFilter->getYLength();

	if ( // ������Ԃ��T�C�Y���ς�����ꍇ
		rangeMap->isNull() ||
		rangeMap->width() != width ||
		rangeMap->height() != height
	) {
		bool successful;
		if (physicalFilter->isNull()) { // ������Ԃ̏ꍇ
			successful = physicalFilter->create(width, height, 8);
		} else { // �T�C�Y���ς�����ꍇ
			QImage* newPhysicalFilter = new QImage();
			successful = newPhysicalFilter->create(width, height, 8);
			if (successful) {
				delete physicalFilter;
				physicalFilter = newPhysicalFilter;
			} else {
				delete newPhysicalFilter;
			}
		}
		if (successful) {
			setPhysicalFilterColorTable(); // �J���[�e�[�u���ݒ�
		} else { // �������m�ێ��s��
			throw bad_alloc();
		}
		rangeMap->resize(width, height);
	}
}

// �F�t���ݒ�()
void FilterEditor::setColoring(bool flg)
{
	coloring = flg;
	repaint(false, false);
}

// ���g���ѐݒ�()
void FilterEditor::setFrequencyRange(double start, double end)
{
	// �����`�F�b�N
	if (start < 0.0) {
		start = 0.0;
	} 
	if (start > 0.708) {
		start = 0.708;
	}
	if (end < 0.0) {
		end = 0.0;
	}
	if (end > 0.708) {
		end = 0.708;
	}
	if (start > end) {
		throw invalid_argument("�J�n���g�����I�����g���𒴂��Ă��܂�");
	}

	startFrequency = start;
	endFrequency = end;
	repaint(true, false);
}

// �����ѐݒ�()
void FilterEditor::setAngleRange(double start, double end)
{
	// �����`�F�b�N
	if (start < 0.0) {
		start = 0.0;
	}
	if (start > 180.0) {
		start = 180.0;
	}
	if (end < 0.0) {
		end = 0.0;
	}
	if (end > 180.0) {
		end = 180.0;
	}
	if (start > end) {
		throw invalid_argument("�J�n�p���I���p�𒴂��Ă��܂�");
	}

	startAngle = start;
	endAngle = end;
	repaint(true, false);
}

// �W���������s()
void FilterEditor::vary(double variance)
{
	int i, j;

	// 2��v�Z(�̈攻����ɂ߂邽�ߐ�����)
	int srsr = (int)((startFrequency * startFrequency) * 1000000);
	int erer = (int)((endFrequency * endFrequency) * 1000000);
	int strd = (int)((startAngle * ieq::PI / 180.0) * 10000);
	int enrd = (int)((endAngle * ieq::PI / 180.0) * 10000);

	int yLen = logicalFilter->getYLength();
	int xLen = logicalFilter->getXLength();

	bool arcZeroUpdated = false;

	int halfYLen = (int)ceil(yLen / 2.0); // (yLen - 1) / 2 + 1
	int halfXLen = (int)ceil(xLen / 2.0);
	for (i = 0; i < halfYLen; i++) {
		double yy = (double)(i * i) / (yLen * yLen);
		// x���W�����̗̈�Ɋւ��郋�[�v
		for (j = 0; j < halfXLen; j++) {
			double xx = (double)(j * j) / (xLen * xLen);
			int rr = (int)((xx + yy) * 1000000);
			if (rr < srsr || rr > erer) {
				continue;
			}
			if (i == 0 && j == 0) { // ���W��(0, 0)�̏ꍇ
				Complex cof = logicalFilter->getCoefficient(0, 0);
				cof += variance;
				if (cof.real() < 0.0) { // �[���������ꍇ�[���ɂ���
					cof = Complex(0.0, cof.imag());
				}
				logicalFilter->setCoefficient(0, 0, cof);
				continue;
			}
			// �Ίp�v�Z
			int argument = (int)(
				atan2((double)i / yLen, (double)j / xLen) * 10000
			);
			if (argument < strd || argument > enrd) {
				continue;
			}
			Complex cof = logicalFilter->getCoefficient(j, i);
			cof += variance;
			if (cof.real() < 0.0) {
				cof = Complex(0.0, cof.imag());
			}
			logicalFilter->setCoefficient(j, i, cof);
			// �Ώ̈ʒu�̌W���ݒ�
			int oppositeX = (xLen - j) % xLen;
			int oppositeY = (yLen - i) % yLen;
			logicalFilter->setCoefficient(
			 	// xLen - j, yLen - i, conj(cof) // �o�O����
			 	oppositeX, oppositeY, conj(cof)
			);
			if (i == 0) {
				arcZeroUpdated = true;
			}
		}
		// x���W�����̗̈�Ɋւ��郋�[�v
		for (j = xLen / 2 + 1; j < xLen; j++) {
			int x = j - xLen;
			double xx = (double)(x * x) / (xLen * xLen);
			int rr = (int)((xx + yy) * 1000000);
			if (rr < srsr || rr > erer) {
				continue;
			}
			int argument = (int)(
				atan2((double)i / yLen, (double)x / xLen) * 10000
			);
			if (argument < strd || argument > enrd) {
				continue;
			}
			if (i == 0 && arcZeroUpdated) {
				continue;
			}
			Complex cof = logicalFilter->getCoefficient(j, i);
			cof += variance;
			if (cof.real() < 0.0) {
				cof = Complex(0.0, cof.imag());
			}
			logicalFilter->setCoefficient(j, i, cof);
			// �Ώ̈ʒu�̌W���ݒ�
			int oppositeX = xLen - j;
			int oppositeY = (yLen - i) % yLen;
			logicalFilter->setCoefficient(
			 	// xLen - j, yLen - i, conj(cof) // �o�O����
			 	oppositeX, oppositeY, conj(cof)
			);
		}
	}

	saveCount += 1;
	repaint(false, true);
}

// �ҏW�̈敽�όW���Q��()
double FilterEditor::getMeanOfRange() const
{
	int i, j;

	// 2��v�Z(�̈攻����ɂ߂邽�ߐ�����)
	int srsr = (int)((startFrequency * startFrequency) * 1000000);
	int erer = (int)((endFrequency * endFrequency) * 1000000);
	int strd = (int)((startAngle * ieq::PI / 180.0) * 10000);
	int enrd = (int)((endAngle * ieq::PI / 180.0) * 10000);

	int yLen = logicalFilter->getYLength();
	int xLen = logicalFilter->getXLength();

	double sum = 0.0; // �Q�ƃf�[�^���v
	int cnt = 0;      // �Q�ƃf�[�^��

	int halfYLen = (int)ceil(yLen / 2.0); 
	int halfXLen = (int)ceil(xLen / 2.0);
	for (i = 0; i < halfYLen; i++) {
		double yy = (double)(i * i) / (yLen * yLen);
		// x���W�����̗̈�Ɋւ��郋�[�v
		for (j = 0; j < halfXLen; j++) {
			double xx = (double)(j * j) / (xLen * xLen);
			int rr = (int)((xx + yy) * 1000000);
			if (rr < srsr || rr > erer) {
				continue;
			}
			if (i == 0 && j == 0) { // ���W��(0, 0)�̏ꍇ
				Complex cof = logicalFilter->getCoefficient(0, 0);
				sum += cof.real();
				cnt++;
				continue;
			}
			// �Ίp�v�Z
			int argument = (int)(
				atan2((double)i / yLen, (double)j / xLen) * 10000
			);
			if (argument < strd || argument > enrd) {
				continue;
			}
			Complex cof = logicalFilter->getCoefficient(j, i);
			sum += cof.real();
			cnt++;
		}
		// x���W�����̗̈�Ɋւ��郋�[�v
		for (j = xLen / 2 + 1; j < xLen; j++) {
			int x = j - xLen;
			double xx = (double)(x * x) / (xLen * xLen);
			int rr = (int)((xx + yy) * 1000000);
			if (rr < srsr || rr > erer) {
				continue;
			}
			int argument = (int)(
				atan2((double)i / yLen, (double)x / xLen) * 10000
			);
			if (argument < strd || argument > enrd) {
				continue;
			}
			Complex cof = logicalFilter->getCoefficient(j, i);
			sum += cof.real();
			cnt++;
		}
	}

	// ���ϒl�v�Z
	double avr = (cnt > 0) ? sum / cnt : 0.0;

	return avr;
}

// �t�B���^�Q��()
const Filter& FilterEditor::getFilter() const
{
	return *logicalFilter;
}

// �t�@�C���p�X�Q��()
const String& FilterEditor::getFilePath() const
{
	return filePath;
}

// �ۑ��J�E���g�Q��()
int FilterEditor::getSaveCount() const
{
	return saveCount;
}

// �ĕ`��(����: �ҏW�̈�ύX�t���O, �W�������t���O)
void FilterEditor::repaint(bool rflg, bool vflg)
{
	rangeUpdated = rflg;
	varied = vflg;
	QWidget::repaint();
}

// �`��()
void FilterEditor::paintEvent(QPaintEvent* event)
{
	if (varied) { // �W��������
		updatePhysicalFilter(); // �t�B���^�C���[�W�X�V
	}

	if (rangeUpdated) { // �ҏW�̈�ύX��
		updateRangeMap(); // �ҏW�̈�`��}�b�v�X�V
	}

	QPainter painter(this);
	int width = logicalFilter->getXLength();
	int height = logicalFilter->getYLength();
	painter.setWindow(0, 0, width, height); // �`����W���ʐݒ�

	// �t�B���^�`��
	painter.drawImage(0, 0, *physicalFilter);

	// �ҏW�̈�`��
	if (coloring) { // �F�t���I����
		drawRangeBorder(painter);    // �ҏW�̈�g���`��
		painter.setRasterOp(AndROP); // AND���Z�ɂ��F�̏d�ˍ�����ݒ�
		painter.drawPixmap(0, 0, *rangeMap); // �}�b�v�`��
	}

	// �X�V��ԃ��Z�b�g
	rangeUpdated = false;
	varied = false;
}

// �t�B���^�C���[�W�X�V()
void FilterEditor::updatePhysicalFilter()
{
	int i, j;

	// �t�B���^�C���[�W�ݒ�
	int yLen = logicalFilter->getYLength();
	int xLen = logicalFilter->getXLength();
	for (i = 0; i < yLen; i++) {
		// �\���̈�̑Ή�����y���W���v�Z
		// int y = (yLen - 1) - (i + (yLen - 1) / 2) % yLen;
		int y = (i + (yLen - 1) / 2) % yLen;
		for (j = 0; j < xLen; j++) {
			Complex cof = logicalFilter->getCoefficient(j, i);
			// �P�x����
			int level = ieq::toLuminosity(cof.real(), 128, 255);
			// �\���̈�̑Ή�����x���W���v�Z
			int x = (j + (xLen - 1) / 2) % xLen;
			physicalFilter->setPixel(x, y, level);
		}
	}
}

// �ҏW�̈�g���`��()
void FilterEditor::drawRangeBorder(QPainter& painter)
{
	int xLen = logicalFilter->getXLength();
	int yLen = logicalFilter->getYLength();
	int centerX = (xLen - 1) / 2;                 // ���S��x���W
	int centerY = (yLen - 1) - (yLen - 1) / 2;    // ���S��y���W
	int efRadiusX = (int)(endFrequency * xLen);   // �I�����g����x�������̔��a
	int efRadiusY = (int)(endFrequency * yLen);   // �I�����g����y�������̔��a
	int sfRadiusX = (int)(startFrequency * xLen); // �J�n���g����x�������̔��a
	int sfRadiusY = (int)(startFrequency * yLen); // �J�n���g����y�������̔��a
	int baseAngle = -(int)(startAngle * 16);           // �����ѕ`����p
	int drawAngle = -(int)(endAngle * 16) - baseAngle; // �����ѕ`��p
	int agRadiusX = (int)(xLen * 0.8);            // �����ѕ`���x�������̔��a
	int agRadiusY = (int)(yLen * 0.8);            // �����ѕ`���y�������̔��a

	// �p�x�␮
	if (drawAngle >= -2) { // �`��p������������Ɖ����`�悳��Ȃ��̂ŕ␮���K�v
		baseAngle += 1;
		drawAngle -= 2;
		if (drawAngle >= -2) {
			baseAngle += 1;
			drawAngle -= 2;
		}
	}

	painter.setBrush(NoBrush); // �h��Ԃ��̖�����

	// ���g���ѕ`��
	painter.setPen(QPen(QColor(128, 192, 192), 2)); // �y����2�ł͑�������?
	painter.drawEllipse( // �I�����g���~�`��
		centerX - efRadiusX, centerY - efRadiusY, efRadiusX * 2, efRadiusY * 2
	);
	painter.drawEllipse( // �J�n���g���~�`��
		centerX - sfRadiusX, centerY - sfRadiusY, sfRadiusX * 2, sfRadiusY * 2
	);

	// �����ѕ`��
	painter.setPen(QPen(QColor(192, 192, 128), 2));
	painter.drawPie( // ��`�`��
		centerX - agRadiusX, centerY - agRadiusY, agRadiusX * 2, agRadiusY * 2,
		baseAngle, drawAngle
	);
	painter.drawPie( // �Ίp�̐�`�`��
		centerX - agRadiusX, centerY - agRadiusY, agRadiusX * 2, agRadiusY * 2,
		baseAngle + 180 * 16, drawAngle
	);
}

// �ҏW�̈�`��o�b�t�@�X�V()
void FilterEditor::updateRangeMap()
{
	int xLen = logicalFilter->getXLength();
	int yLen = logicalFilter->getYLength();
	int centerX = (xLen - 1) / 2;                 // ���S��x���W
	int centerY = (yLen - 1) - (yLen - 1) / 2;    // ���S��y���W
	int efRadiusX = (int)(endFrequency * xLen);   // �I�����g����x�������̔��a
	int efRadiusY = (int)(endFrequency * yLen);   // �I�����g����y�������̔��a
	int sfRadiusX = (int)(startFrequency * xLen); // �J�n���g����x�������̔��a
	int sfRadiusY = (int)(startFrequency * yLen); // �J�n���g����y�������̔��a
	int baseAngle = -(int)(startAngle * 16);           // �����ѕ`����p
	int drawAngle = -(int)(endAngle * 16) - baseAngle; // �����ѕ`��p
	int agRadiusX = (int)(xLen * 0.8);            // �����ѕ`���x�������̔��a
	int agRadiusY = (int)(yLen * 0.8);            // �����ѕ`���y�������̔��a

	// �O����
	rangeMap->fill(QColor(255, 255, 255)); // �z���C�g�N���A
	QPainter painter(rangeMap);
	painter.setBackgroundColor(QColor(255, 255, 255));
	painter.setPen(NoPen); // �y���𖳌���

	// ���g���ѕ`��
	painter.setBrush(QBrush(QColor(128, 255, 255), SolidPattern)); // �u���V
	painter.drawEllipse( // �I�����g���̐��~��`��
		centerX - efRadiusX, centerY - efRadiusY, efRadiusX * 2, efRadiusY * 2
	);
	painter.setBrush(QBrush(QColor(255, 255, 255), SolidPattern)); // ���u���V
	painter.drawEllipse( // ��������J�n���g���܂ł𔒂��~�ł��蔲��
		centerX - sfRadiusX, centerY - sfRadiusY, sfRadiusX * 2, sfRadiusY * 2
	);

	// �����ѕ`��
	painter.setRasterOp(AndROP); // AND���Z�ɂ��F�̏d�ˍ��킹��ݒ�
	painter.setBrush(QBrush(QColor(255, 255, 128), SolidPattern)); // ���u���V
	painter.drawPie( // ��`�`��
		centerX - agRadiusX, centerY - agRadiusY, agRadiusX * 2, agRadiusY * 2,
		baseAngle, drawAngle
	);
	painter.drawPie( // �Ίp�̐�`�`��
		centerX - agRadiusX, centerY - agRadiusY, agRadiusX * 2, agRadiusY * 2,
		baseAngle + 180 * 16, drawAngle
	);
}

// �}�E�X�ړ��C�x���g�n���h��()
void FilterEditor::mouseMoveEvent(QMouseEvent* event)
{
	emit mouseMoved(event->x(), event->y());
}

// �}�E�X�N���b�N�C�x���g�n���h��()
void FilterEditor::mousePressEvent(QMouseEvent* event)
{
	emit mousePressed(event->x(), event->y());
}

// �}�E�X�A�E�g�C�x���g�n���h��()
void FilterEditor::leaveEvent(QEvent* event)
{
	emit mouseExited();
}

// �t�B���^�J���[�e�[�u���ݒ�()
void FilterEditor::setPhysicalFilterColorTable()
{
	int i;

	if (physicalFilter->isNull()) {
		return;
	}

	static const int numColors = 256;
	physicalFilter->setNumColors(numColors);

	for (i = 0; i < numColors; i++) { // 256�~���O���[�X�P�[���e�[�u����ݒ�
		physicalFilter->setColor(i, qRgb(i, i, i));
	}
}

void FilterEditor::setTitle(const QString& title)
{
	ieq::setWindowTitle(this, title);
#if defined(QT_NON_COMMERCIAL) && defined(Q_WS_WIN)
	topData()->caption = title;
#endif
}



// �e�X�g�h���C�o
#ifdef UNIT_TEST
#include <qapplication.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	FilterEditor editor(256, 256);
	app.setMainWidget(&editor);
	editor.show();

	editor.setFrequencyRange(0.1, 0.5);
	editor.setAngleRange(0.0, 120.0);
//	editor.setVariance(0.5);
	(void)getchar();
//	editor.vary();
	editor.vary(0.5);
	(void)getchar();
	editor.setColoring(false);
	(void)getchar();
//	editor.setVariance(-2.0);
	editor.vary(-2.0);

	return app.exec();
}

#endif
