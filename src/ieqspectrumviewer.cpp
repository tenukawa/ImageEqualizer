// 抽象クラス名 : スペクトル表示

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

// 周波数領域設定()
void SpectrumViewer::setFrequencyDomain(const ieq::Matrix& frd)
{
	int currentYLen = frequencyDomain.size();
	int currentXLen = frequencyDomain[0].size();
	int newYLen = frd.size();
	int newXLen = frd[0].size();
	// サイズが変わった場合
	if (currentYLen != newYLen || currentXLen != newXLen) {
		frequencyDomain.resize(extents[newYLen][newXLen]);
	}

	frequencyDomain = frd; // 行列のコピー
	updateDrawingBuffer();
}

// 周波数領域参照()
const Matrix& SpectrumViewer::getFrequencyDomain() const
{
	return frequencyDomain;
}

// x方向サイズ参照()
int SpectrumViewer::getXLength() const
{
	return frequencyDomain[0].size();
}

// y方向サイズ参照()
int SpectrumViewer::getYLength() const
{
	return frequencyDomain.size();
}

// 描画()
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

// マウス移動イベントハンドラ()
void SpectrumViewer::mouseMoveEvent(QMouseEvent* event)
{
	int xPos = event->x();
	int yPos = event->y();
	int width = this->width();
	int height = this->height();
	int xLen = frequencyDomain[0].size();
	int yLen = frequencyDomain.size();

	// 表示座標を論理座標に変換
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

	// 安全対策
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

	// 4象限座標計算
	int shiftedX = xPos - (xLen / 2 - 1);
	int shiftedY = yPos - (yLen / 2 - 1);

	// 行列格納位置計算
	int xIndex = (shiftedX + xLen) % xLen;
	int yIndex = (shiftedY + yLen) % yLen;

	String message( // メッセージ取得
		this->toMessage(xPos, yPos, frequencyDomain[yIndex][xIndex])
	);
	emit mouseMoved(xPos, yPos, message);
}

// マウスアウトイベントハンドラ()
void SpectrumViewer::leaveEvent(QEvent* event)
{
	emit mouseExited();
}

// 可視化()
void SpectrumViewer::showEvent(QShowEvent* event)
{
	emit visibilityChanged(true);
}

// 不可視化()
void SpectrumViewer::hideEvent(QHideEvent* event)
{
	emit visibilityChanged(false);
}

// 描画バッファ更新()
void SpectrumViewer::updateDrawingBuffer()
{
	int i, j;

	int yLen = frequencyDomain.size();
	int xLen = frequencyDomain[0].size();

	bool isNull = drawingBuffer->isNull();
	bool widthUpdated = (drawingBuffer->width() != xLen);
	bool heightUpdated = (drawingBuffer->height() != yLen);
	if (isNull || widthUpdated || heightUpdated) { // 初期状態またはサイズ変更時
		bool successful;
		if (isNull) { // 初期状態の場合
			successful = drawingBuffer->create(xLen, yLen, 8);
		} else { // サイズ変更時
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
		} else { // バッファ確保失敗時
			throw bad_alloc();
		}
	}

	for (i = 0; i < yLen; i++) {
		// 表示領域の対応するy座標を計算
		int y = (i + (yLen - 1) / 2) % yLen;
		for (j = 0; j < xLen; j++) {
			Complex value = frequencyDomain[i][j];
			// 輝度決定
			int level = this->toLuminosity(value);
			// 表示領域の対応するx座標を計算
			int x = (j + (xLen - 1) / 2) % xLen;
			drawingBuffer->setPixel(x, y, level);
		}
	}

	if (isVisible()) {
		repaint();
	}
}

// 描画バッファカラーテーブル設定()
void SpectrumViewer::setDrawingBufferColorTable()
{
	int i;

	if (drawingBuffer->isNull()) {
		return;
	}

	static const int numColors = 256;
	drawingBuffer->setNumColors(numColors);

	for (i = 0; i < numColors; i++) { // 256諧調グレースケールテーブル設定
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

