// �N���X�� : �摜�ҏW

#include "ieqimageeditor.h"

#include <qimage.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qwidget.h>
#include <qfile.h>
#include <new>
#include <math.h>
// #include <assert.h>

#if defined(DEBUG) || defined(UNIT_TEST)
#include <qmainwindow.h>
#include <qscrollview.h>
#include <iostream>
#endif

using namespace ieq;
using namespace boost;
using namespace std;

ImageEditor::ImageEditor(
	int limit, QWidget* parent, const char* name, WFlags flag
)
	:QWidget(parent, name, flag), logicalImage(0)
{
	physicalImage = new QImage();

	if (limit < 0) {
		undoingLimit = 8;
	} else {
		undoingLimit = limit;
	}
	saveCount = 0;
	undoCountDirection = 1;
	setMouseTracking(true);
}

ImageEditor::~ImageEditor()
{
	// �������
	if (logicalImage != 0) {
		delete logicalImage;
	}
	delete physicalImage;
	initializeImageStack(); // �ҏW�X�^�b�N������
}

// �摜�Ǎ�()
void ImageEditor::loadImage(const String& path)
{
	int i, j;

	// �����摜�Ǎ�
	if (!QFile::exists(path)) { // �t�@�C�������݂��Ȃ��ꍇ
		throw NoSuchFileException();
	}
	bool successful = physicalImage->load(path);
	if (!successful) { // �Ǎ����s��
		throw InputException();
	}

	// �_���摜�\�z
	int rows = physicalImage->height();
	int cols = physicalImage->width();
	bool isMonochrome = (physicalImage->depth() <= 8);
	Matrix mat(extents[rows][cols]);
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			int level; // �P�x
			if (isMonochrome) {
				level = physicalImage->pixelIndex(j, i);
			} else {
				QRgb color = physicalImage->pixel(j, i); // ��f�l�̎擾
				level = qGray(color); // ���m�N����
				// level = 0.299 * r + 0.587 * g + 0.114 * b // netpbm����
			}
			mat[i][j] = Complex((double)level, 0.0);
		}
	}
	if (logicalImage != 0) { // ������ԂłȂ��ꍇ
		delete logicalImage;
	}
	logicalImage = new Image(mat);

	// �ҏW�X�^�b�N�̏�����
	initializeImageStack();

	// �����o�̍Đݒ�
	filePath = path;
	saveCount = 0;
	undoCountDirection = 1;

	// �\���̈�̍X�V
	updateViewer();
}

// �摜�ۑ�()
void ImageEditor::saveImage(const String& path, const char* format)
{
	// �����摜�ۑ�
	if (!QFileInfo(path).dir(true).exists()) { // �f�B���N�g�������݂��Ȃ��ꍇ
		throw NoSuchDirectoryException();
	}
	bool successful = physicalImage->save(path, format);
	if (!successful) { // �ۑ����s��
		throw OutputException();
	}

	// �����o�̍Đݒ�
	filePath = path;
	saveCount = 0;
	undoCountDirection = 1;
}

// �t�B���^�K�p()
void ImageEditor::applyFilter(const Filter& filter)
{
	int i, j;

	if (logicalImage == 0) {
		return;
	}

	Image* prevImage = new Image(*logicalImage); // �X�V�O�摜�̃R�s�[

	try {
		Matrix mat(logicalImage->getFrequencyDomain()); // �v�Z�p�o�b�t�@
		int imageXLen = mat[0].size();
		int imageYLen = mat.size();
		int filterXLen = filter.getXLength();
		int filterYLen = filter.getYLength();

		// �L�k�W��(�t�B���^���摜�Ɠ����T�C�Y�ɂ��邽�߂̌W��)�Z�o
		double xStretch, yStretch;
		if (imageXLen > 1) {
			xStretch = (double)(filterXLen - 1) / (imageXLen - 1);
		} else {
			xStretch = 0.0;
		}
		if (imageYLen > 1) {
			yStretch = (double)(filterYLen - 1) / (imageYLen - 1);
		} else {
			yStretch = 0.0;
		}

		// �t�B���^�����O�v�Z
		int halfImageYLen = (imageYLen - 1) / 2 + 1;
		int halfImageXLen = (imageXLen - 1) / 2 + 1;
		int halfFilterYLen = (filterYLen - 1) / 2 + 1;
		int halfFilterXLen = (filterXLen - 1) / 2 + 1;
		int startImageNegativeXPos = imageXLen / 2 + 1;
		int startImageNegativeYPos = imageYLen / 2 + 1;
		int startFilterNegativeXPos = filterXLen / 2 + 1;
		int startFilterNegativeYPos = filterYLen / 2 + 1;
#ifdef USE_LINEAR_PHASE
		// �����ʑ��������l������ꍇ
		// �Q�x���Z�o
		double pi = 4 * atan(1.0); // �~�����擾
		double groupDelayX = pi * (1.0 - 1.0 / imageXLen);
		double groupDelayY = pi * (1.0 - 1.0 / imageYLen);
		double bias = 0.0;
		// double bias = pi / 2.0;
#endif
		// y���������[���̎��g�������ɂ���
#ifdef USE_LINEAR_PHASE
		double amplitude = filter.getCoefficient(0, 0).real(); // �U��
		double phase = bias;                                   // �ʑ�
		double re = amplitude * cos(phase);                    // �W������
		double im = amplitude * sin(phase);                    // �W������
		mat[0][0] *= Complex(re, im); // ��������
#else
		mat[0][0] *= filter.getCoefficient(0, 0); // ��������
#endif
		for (i = 1; i < halfImageXLen; i++) {     // �𗬐���
			int filterXPos = (int)(i * xStretch + 0.5);
			if (filterXPos >= halfFilterXLen) {
				filterXPos = halfFilterXLen - 1;
			}
#ifdef USE_LINEAR_PHASE
			double amplitude = filter.getCoefficient( // �U��
				filterXPos, 0
			).real();
			double phase = -i * groupDelayX + bias;   // �ʑ�
			double re = amplitude * cos(phase);       // �W������
			double im = amplitude * sin(phase);       // �W������
			mat[0][i] *= Complex(re, im);
#else
			mat[0][i] *= filter.getCoefficient(filterXPos, 0);
#endif
			// �Ώ̈ʒu�̎��g�������ݒ�
			int oppositeX = imageXLen - i;
			mat[0][oppositeX] = conj(mat[0][i]);
		}

		// y��������������ѕ��̎��g�������ɂ���
		for (i = 1; i < halfImageYLen; i++) {
			int filterYPos = (int)(i * yStretch + 0.5);
			if (filterYPos >= halfFilterYLen) {
				filterYPos = halfFilterYLen - 1;
			}
			// x���������[������ѐ��̎��g�������ɂ���
			for (j = 0; j < halfImageXLen; j++) {
				int filterXPos = (int)(j * xStretch + 0.5);
				if (filterXPos >= halfFilterXLen) {
					filterXPos = halfFilterXLen - 1;
				}
#ifdef USE_LINEAR_PHASE
				double amplitude = filter.getCoefficient( // �U��
					filterXPos, filterYPos
				).real();
				double phase = -(                         // �ʑ�
					j * groupDelayX + i * groupDelayY
				) + bias;
				double re = amplitude * cos(phase);       // �W������
				double im = amplitude * sin(phase);       // �W������
				mat[i][j] *= Complex(re, im);
#else
				mat[i][j] *= filter.getCoefficient(filterXPos, filterYPos);
#endif
				// �Ώ̈ʒu�̎��g�������ݒ�
				int oppositeX = (imageXLen - j) % imageXLen;
				int oppositeY = imageYLen - i;
				mat[oppositeY][oppositeX] = conj(mat[i][j]);
			}
			// x�����������̎��g�������ɂ���
			for (j = startImageNegativeXPos; j < imageXLen; j++) {
				int filterXPos = (int)(j * xStretch + 0.5);
				if (filterXPos < startFilterNegativeXPos) {
					filterXPos = startFilterNegativeXPos;
				}
#ifdef USE_LINEAR_PHASE
				double amplitude = filter.getCoefficient( // �U��
					filterXPos, filterYPos
				).real();
				double phase = -(                         // �ʑ�
					(j - imageXLen) * groupDelayX + i * groupDelayY
				) + bias;
				double re = amplitude * cos(phase);       // �W������
				double im = amplitude * sin(phase);       // �W������
				mat[i][j] *= Complex(re, im);
#else
				mat[i][j] *= filter.getCoefficient(filterXPos, filterYPos);
#endif
				// �Ώ̈ʒu�̎��g�������ݒ�
				int oppositeX = imageXLen - j;
				int oppositeY = imageYLen - i;
				mat[oppositeY][oppositeX] = conj(mat[i][j]);
			}
		}
//
//		// �ȈՔ� (�P��������Ƃ��̂悤�ɂȂ�)
//		for (i = 1; i < halfImageYLen; i++) {
//			for (j = 0; j < imageXLen; j++) {
//				int filterXPos = (int)(j * xStretch + 0.5);
//				int filterYPos = (int)(i * yStretch + 0.5);
//				mat[i][j] *= filter.getCoefficient(filterXPos, filterYPos);
//				int oppositeX = (imageXLen - j) % imageXLen;
//				int oppositeY = imageYLen - i;
//				mat[oppositeY][oppositeX] = conj(mat[i][j]);
//			}
//		}
//
		// �_���摜�̍X�V
		logicalImage->setFrequencyDomain(mat);

		// �����摜�ƕ\���̈�̍X�V
		updatePhysicalImage();
		updateViewer();

	} catch (...) { // ��O���͉������Ȃ��������Ƃɂ��čăX���[
		if (logicalImage != 0) {
			delete logicalImage;
		}
		logicalImage = prevImage;
		throw;
	}

	// �ҏW�X�^�b�N�X�V
	if (undoingLimit > 0) {
		if (imageStack.size() >= undoingLimit) {
			Image* img = imageStack.back();
			if (img != 0) {
				delete img;
			}
			imageStack.pop_back();
		}
		imageStack.push_front(prevImage);
	} else {
		delete prevImage;
	}

	saveCount += 1;
}

// ���ɖ߂�()
void ImageEditor::undo()
{
	if (imageStack.empty()) { // �X�^�b�N����̏ꍇ
		return;
	}

	// �X�^�b�N�|�b�v
	Image* img = imageStack.front();
	if (img == 0) {
		return;
	}
	Image* oldImage = logicalImage;
	logicalImage = img;

	// �����摜�ƕ\���̈�̍X�V
	try {
		updatePhysicalImage();
	} catch (bad_alloc) {
		logicalImage = oldImage;
		throw;
	}
	updateViewer();

	// �I�u�W�F�N�g�̉��
	if (oldImage != 0) {
		delete oldImage;
	}
	imageStack.pop_front();

	// �ҏW��ԍX�V
	if (saveCount == 0) {
		undoCountDirection = -1;
	}
	saveCount -= undoCountDirection;
}

// �P�x�Q��()
int ImageEditor::getLuminosity(int x, int y) const
{
	if (physicalImage->isNull()) {
		return -1;
	}

	if (x < 0 || y < 0) {
		return -1;
	}

	if (x >= physicalImage->width() || y >= physicalImage->height()) {
		return -1;
	}

	int luminosity;
	if (physicalImage->depth() <= 8) { // ���m�N���̏ꍇ
		luminosity = physicalImage->pixelIndex(x, y);
	} else { // �J���[�̏ꍇ
		luminosity = qGray(physicalImage->pixel(x, y));
	}

	return luminosity;
}

// x��������f���Q��()
int ImageEditor::getXLength() const
{
	if (logicalImage == 0) {
		return 0;
	}
	return logicalImage->getXLength();
}

// y��������f���Q��()
int ImageEditor::getYLength() const
{
	if (logicalImage == 0) {
		return 0;
	}
	return logicalImage->getYLength();
}

// ���g���̈�Q��()
const Matrix& ImageEditor::getFrequencyDomain() const
{
	if (logicalImage == 0) {
		return dummyMatrix;
	}
	return logicalImage->getFrequencyDomain();
}

// �t�@�C���p�X�Q��()
const String& ImageEditor::getFilePath() const
{
	return filePath;
}

// �ۑ��J�E���^�Q��()
int ImageEditor::getSaveCount() const
{
	return saveCount;
}

// �A���h�D���s�\������()
bool ImageEditor::isUndoingEnabled() const
{
	return !imageStack.empty();
}

// �`��()
void ImageEditor::paintEvent(QPaintEvent* event)
{
	if (physicalImage->isNull()) { // �摜�����[�h����Ă��Ȃ��ꍇ
		erase(); // �̈����
		return;
	}

	int width = physicalImage->width();
	int height = physicalImage->height();

	QPainter painter(this);
	if (this->width() != width || this->height() != height) {
		painter.setWindow(0, 0, width, height); // �`����W���ʐݒ�
	}
	painter.drawImage(0, 0, *physicalImage);
}

// �ҏW�X�^�b�N������()
void ImageEditor::initializeImageStack()
{
	while (!imageStack.empty()) { // �X�^�b�N����ɂȂ�܂�
		Image* img = imageStack.front(); // �X�^�b�N�v�f�̎擾
		if (img != 0) {
			delete img; // �I�u�W�F�N�g�̉��
		}
		imageStack.pop_front(); // �X�^�b�N�v�f�̍폜
	}
}

// �����摜�X�V()
void ImageEditor::updatePhysicalImage()
{
	int i, j;

	if (logicalImage == 0) {
		return;
	}

	int width = logicalImage->getXLength();
	int height = logicalImage->getYLength();

	bool successful = true;
	if (physicalImage->isNull()) { // �����摜��������Ԃ̏ꍇ
		successful = physicalImage->create(width, height, 32);
	} else if (
		physicalImage->width() != width || physicalImage->height() != height
	) { // �摜�T�C�Y���قȂ�ꍇ
		// �����摜���č\�z
		QImage* newImage = new QImage();
		successful = newImage->create(width, height, 32);
		if (successful) {
			delete physicalImage;
			physicalImage = newImage;
		} else {
			delete newImage;
		}
	}
	if (!successful) { // �摜�̃������m�ۂɎ��s�����ꍇ
		throw bad_alloc();
	}

	bool isMonochrome = (physicalImage->depth() <= 8);
	// �_���摜�������摜�ϊ�
	const Matrix* spaceDomain = &logicalImage->getSpaceDomain();
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			double logicalLevel = (*spaceDomain)[i][j].real();
			int level;
			if (logicalLevel >= 0.0) {
				level = (int)(logicalLevel + 0.5);
			} else {
				level = (int)(logicalLevel - 0.5);
			}
			if (level < 0) {
				level = 0;
			}
			if (level > 255) {
				level = 255;
			}
			if (isMonochrome) {
				physicalImage->setPixel(j, i, level);
			} else {
				QRgb color = qRgb(level, level, level);
				physicalImage->setPixel(j, i, color);
			}
		}
	}
}

// �\���̈�X�V()
void ImageEditor::updateViewer()
{
	// �摜�T�C�Y�擾
	int width = physicalImage->width();
	int height = physicalImage->height();

	// �摜�T�C�Y���ς�����ꍇ
	if (this->width() != width || this->height() != height) {
		resize(width, height); // �T�C�Y�ύX + �ĕ`��
		return;
	}

	repaint();
}

// �}�E�X�ړ��C�x���g�n���h��()
void ImageEditor::mouseMoveEvent(QMouseEvent* event)
{
	if (physicalImage->isNull()) {
		return;
	}

	int imageWidth = physicalImage->width();
	int imageHeight = physicalImage->height();

	// �L�k�W���Z�o
	double xStretch, yStretch;
	if (width() > 1) {
		xStretch = (double)(imageWidth - 1) / (width() - 1);
	} else {
		xStretch = 0.0;
	}
	if (height() > 1) {
		yStretch = (double)(imageHeight - 1) / (height() - 1);
	} else {
		yStretch = 0.0;
	}

	// �_�����W�v�Z
	int xPos = (int)(event->x() * xStretch + 0.5);
	int yPos = (int)(event->y() * yStretch + 0.5);

	// ���S�΍�
	if (xPos >= imageWidth) {
		xPos = imageWidth - 1;
	}
	if (xPos < 0) {
		xPos = 0;
	}
	if (yPos >= imageHeight) {
		yPos = imageHeight - 1;
	}
	if (yPos < 0) {
		yPos = 0;
	}

	emit mouseMoved(xPos, yPos);
}

// �}�E�X�N���b�N�C�x���g�n���h��()
void ImageEditor::mousePressEvent(QMouseEvent* event)
{
	emit mousePressed(event->x(), event->y());
}

// �}�E�X�A�E�g�C�x���g�n���h��()
void ImageEditor::leaveEvent(QEvent* event)
{
	emit mouseExited();
}

void ImageEditor::setTitle(const QString& title)
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
	QMainWindow window;
	QScrollView viewer(&window);
	window.setCentralWidget(&viewer);

	ImageEditor editor;
	try {
		editor.loadImage("H:\\sotsuken\\images\\image256x256.pgm");
	} catch (...) {
		cout << "�摜�Ǎ��G���[" << endl;
		return 1;
	}

	viewer.addChild(&editor);

	app.setMainWidget(&window);
	window.show();
	(void)getchar();

	Filter filter;
	try {
		filter.load("H:\\sotsuken\\filters\\lowpass2.sff");
	} catch (...) {
		cout << "�t�B���^�Ǎ��G���[" << endl;
		return 1;
	}

	editor.applyFilter(filter);

	return app.exec();
}

#endif

