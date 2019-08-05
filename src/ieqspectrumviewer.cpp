// ���ۃN���X�� : �X�y�N�g���\��

#include "ieqspectrumviewer.h"

#include <qpainter.h>
#include <qcolor.h>
#include <qpen.h>
#include <new>
// #include <assert.h>

#if defined(UNIT_TEST) || defined(DEBUG)
#include <iostream>
#endif

using namespace ieq;
using namespace boost;
using namespace std;

//public:
SpectrumViewer::SpectrumViewer(
	int xs, int ys, QWidget* parent, const char* name, WFlags flag
)
	: QWidget(parent, name, flag), frequencyDomain(extents[1][1])
{
	resize(xs, ys);
	setMouseTracking(true);
	drawingBuffer = new QImage();
}

SpectrumViewer::~SpectrumViewer()
{
}

// ���g���̈�ݒ�()
void SpectrumViewer::setFrequencyDomain(const ieq::Matrix& frd)
{
	int currentYLen = frequencyDomain.size();
	int currentXLen = frequencyDomain[0].size();
	int newYLen = frd.size();
	int newXLen = frd[0].size();
	// �T�C�Y���ς�����ꍇ
	if (currentYLen != newYLen || currentXLen != newXLen) {
		frequencyDomain.resize(extents[newYLen][newXLen]);
	}

	frequencyDomain = frd; // �s��̃R�s�[
	updateDrawingBuffer();
}

// ���g���̈�Q��()
const Matrix& SpectrumViewer::getFrequencyDomain() const
{
	return frequencyDomain;
}

// x�����T�C�Y�Q��()
int SpectrumViewer::getXLength() const
{
	return frequencyDomain[0].size();
}

// y�����T�C�Y�Q��()
int SpectrumViewer::getYLength() const
{
	return frequencyDomain.size();
}

// �`��()
void SpectrumViewer::paintEvent(QPaintEvent* event)
{
	if (drawingBuffer->isNull()) {
		erase();
		return;
	}

	int width = drawingBuffer->width();
	int height = drawingBuffer->height();
	QPainter painter(this);
	painter.setWindow(0, 0, width, height);
	painter.drawImage(0, 0, *drawingBuffer);
}

// �}�E�X�ړ��C�x���g�n���h��()
void SpectrumViewer::mouseMoveEvent(QMouseEvent* event)
{
	int xPos = event->x();
	int yPos = event->y();
	int width = this->width();
	int height = this->height();
	int xLen = frequencyDomain[0].size();
	int yLen = frequencyDomain.size();

	// �\�����W��_�����W�ɕϊ�
	if (width < 2) {
		xPos = 0;
	} else {
		xPos = (int)((double)(xPos * (xLen - 1)) / (width - 1) + 0.5);
	}
	if (height < 2) {
		yPos = 0;
	} else {
		yPos = (int)((double)(yPos * (yLen - 1)) / (height - 1) + 0.5);
	}

	// ���S�΍�
	if (xPos < 0) {
		xPos = 0;
	} else if (xPos >= xLen) {
		xPos = xLen - 1;
	}
	if (yPos < 0) {
		yPos = 0;
	} else if (yPos >= yLen) {
		yPos = yLen - 1;
	}

	// 4�ی����W�v�Z
	int shiftedX = xPos - (xLen / 2 - 1);
	int shiftedY = yPos - (yLen / 2 - 1);

	// �s��i�[�ʒu�v�Z
	int xIndex = (shiftedX + xLen) % xLen;
	int yIndex = (shiftedY + yLen) % yLen;

	String message( // ���b�Z�[�W�擾
		this->toMessage(xPos, yPos, frequencyDomain[yIndex][xIndex])
	);
	emit mouseMoved(xPos, yPos, message);
}

// �}�E�X�A�E�g�C�x���g�n���h��()
void SpectrumViewer::leaveEvent(QEvent* event)
{
	emit mouseExited();
}

// ����()
void SpectrumViewer::showEvent(QShowEvent* event)
{
	emit visibilityChanged(true);
}

// �s����()
void SpectrumViewer::hideEvent(QHideEvent* event)
{
	emit visibilityChanged(false);
}

// �`��o�b�t�@�X�V()
void SpectrumViewer::updateDrawingBuffer()
{
	int i, j;

	int yLen = frequencyDomain.size();
	int xLen = frequencyDomain[0].size();

	bool isNull = drawingBuffer->isNull();
	bool widthUpdated = (drawingBuffer->width() != xLen);
	bool heightUpdated = (drawingBuffer->height() != yLen);
	if (isNull || widthUpdated || heightUpdated) { // ������Ԃ܂��̓T�C�Y�ύX��
		bool successful;
		if (isNull) { // ������Ԃ̏ꍇ
			successful = drawingBuffer->create(xLen, yLen, 8);
		} else { // �T�C�Y�ύX��
			QImage* newBuffer = new QImage();
			successful = newBuffer->create(xLen, yLen, 8);
			if (successful) {
				delete drawingBuffer;
				drawingBuffer = newBuffer;
			} else {
				delete newBuffer;
			}
		}
		if (successful) {
			setDrawingBufferColorTable();
		} else { // �o�b�t�@�m�ێ��s��
			throw bad_alloc();
		}
	}

	for (i = 0; i < yLen; i++) {
		// �\���̈�̑Ή�����y���W���v�Z
		int y = (i + (yLen - 1) / 2) % yLen;
		for (j = 0; j < xLen; j++) {
			Complex value = frequencyDomain[i][j];
			// �P�x����
			int level = this->toLuminosity(value);
			// �\���̈�̑Ή�����x���W���v�Z
			int x = (j + (xLen - 1) / 2) % xLen;
			drawingBuffer->setPixel(x, y, level);
		}
	}

	if (isVisible()) {
		repaint();
	}
}

// �`��o�b�t�@�J���[�e�[�u���ݒ�()
void SpectrumViewer::setDrawingBufferColorTable()
{
	int i;

	if (drawingBuffer->isNull()) {
		return;
	}

	static const int numColors = 256;
	drawingBuffer->setNumColors(numColors);

	for (i = 0; i < numColors; i++) { // 256�~���O���[�X�P�[���e�[�u���ݒ�
		drawingBuffer->setColor(i, qRgb(i, i, i));
	}
}

void SpectrumViewer::setTitle(const QString& title)
{
	ieq::setWindowTitle(this, title);
#if defined(QT_NON_COMMERCIAL) && defined(Q_WS_WIN)
	topData()->caption = title;
#endif
}

