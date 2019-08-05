// クラス名 : 画像編集

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
	// 解放処理
	if (logicalImage != 0) {
		delete logicalImage;
	}
	delete physicalImage;
	initializeImageStack(); // 編集スタック初期化
}

// 画像読込()
void ImageEditor::loadImage(const String& path)
{
	int i, j;

	// 物理画像読込
	if (!QFile::exists(path)) { // ファイルが存在しない場合
		throw NoSuchFileException();
	}
	bool successful = physicalImage->load(path);
	if (!successful) { // 読込失敗時
		throw InputException();
	}

	// 論理画像構築
	int rows = physicalImage->height();
	int cols = physicalImage->width();
	bool isMonochrome = (physicalImage->depth() <= 8);
	Matrix mat(extents[rows][cols]);
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			int level; // 輝度
			if (isMonochrome) {
				level = physicalImage->pixelIndex(j, i);
			} else {
				QRgb color = physicalImage->pixel(j, i); // 画素値の取得
				level = qGray(color); // モノクロ化
				// level = 0.299 * r + 0.587 * g + 0.114 * b // netpbm方式
			}
			mat[i][j] = Complex((double)level, 0.0);
		}
	}
	if (logicalImage != 0) { // 初期状態でない場合
		delete logicalImage;
	}
	logicalImage = new Image(mat);

	// 編集スタックの初期化
	initializeImageStack();

	// メンバの再設定
	filePath = path;
	saveCount = 0;
	undoCountDirection = 1;

	// 表示領域の更新
	updateViewer();
}

// 画像保存()
void ImageEditor::saveImage(const String& path, const char* format)
{
	// 物理画像保存
	if (!QFileInfo(path).dir(true).exists()) { // ディレクトリが存在しない場合
		throw NoSuchDirectoryException();
	}
	bool successful = physicalImage->save(path, format);
	if (!successful) { // 保存失敗時
		throw OutputException();
	}

	// メンバの再設定
	filePath = path;
	saveCount = 0;
	undoCountDirection = 1;
}

// フィルタ適用()
void ImageEditor::applyFilter(const Filter& filter)
{
	int i, j;

	if (logicalImage == 0) {
		return;
	}

	Image* prevImage = new Image(*logicalImage); // 更新前画像のコピー

	try {
		Matrix mat(logicalImage->getFrequencyDomain()); // 計算用バッファ
		int imageXLen = mat[0].size();
		int imageYLen = mat.size();
		int filterXLen = filter.getXLength();
		int filterYLen = filter.getYLength();

		// 伸縮係数(フィルタを画像と同じサイズにするための係数)算出
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

		// フィルタリング計算
		int halfImageYLen = (imageYLen - 1) / 2 + 1;
		int halfImageXLen = (imageXLen - 1) / 2 + 1;
		int halfFilterYLen = (filterYLen - 1) / 2 + 1;
		int halfFilterXLen = (filterXLen - 1) / 2 + 1;
		int startImageNegativeXPos = imageXLen / 2 + 1;
		int startImageNegativeYPos = imageYLen / 2 + 1;
		int startFilterNegativeXPos = filterXLen / 2 + 1;
		int startFilterNegativeYPos = filterYLen / 2 + 1;
#ifdef USE_LINEAR_PHASE
		// 直線位相特性を考慮する場合
		// 群遅延算出
		double pi = 4 * atan(1.0); // 円周率取得
		double groupDelayX = pi * (1.0 - 1.0 / imageXLen);
		double groupDelayY = pi * (1.0 - 1.0 / imageYLen);
		double bias = 0.0;
		// double bias = pi / 2.0;
#endif
		// y軸方向がゼロの周波数成分について
#ifdef USE_LINEAR_PHASE
		double amplitude = filter.getCoefficient(0, 0).real(); // 振幅
		double phase = bias;                                   // 位相
		double re = amplitude * cos(phase);                    // 係数実部
		double im = amplitude * sin(phase);                    // 係数虚部
		mat[0][0] *= Complex(re, im); // 直流成分
#else
		mat[0][0] *= filter.getCoefficient(0, 0); // 直流成分
#endif
		for (i = 1; i < halfImageXLen; i++) {     // 交流成分
			int filterXPos = (int)(i * xStretch + 0.5);
			if (filterXPos >= halfFilterXLen) {
				filterXPos = halfFilterXLen - 1;
			}
#ifdef USE_LINEAR_PHASE
			double amplitude = filter.getCoefficient( // 振幅
				filterXPos, 0
			).real();
			double phase = -i * groupDelayX + bias;   // 位相
			double re = amplitude * cos(phase);       // 係数実部
			double im = amplitude * sin(phase);       // 係数虚部
			mat[0][i] *= Complex(re, im);
#else
			mat[0][i] *= filter.getCoefficient(filterXPos, 0);
#endif
			// 対称位置の周波数成分設定
			int oppositeX = imageXLen - i;
			mat[0][oppositeX] = conj(mat[0][i]);
		}

		// y軸方向が正および負の周波数成分について
		for (i = 1; i < halfImageYLen; i++) {
			int filterYPos = (int)(i * yStretch + 0.5);
			if (filterYPos >= halfFilterYLen) {
				filterYPos = halfFilterYLen - 1;
			}
			// x軸方向がゼロおよび正の周波数成分について
			for (j = 0; j < halfImageXLen; j++) {
				int filterXPos = (int)(j * xStretch + 0.5);
				if (filterXPos >= halfFilterXLen) {
					filterXPos = halfFilterXLen - 1;
				}
#ifdef USE_LINEAR_PHASE
				double amplitude = filter.getCoefficient( // 振幅
					filterXPos, filterYPos
				).real();
				double phase = -(                         // 位相
					j * groupDelayX + i * groupDelayY
				) + bias;
				double re = amplitude * cos(phase);       // 係数実部
				double im = amplitude * sin(phase);       // 係数虚部
				mat[i][j] *= Complex(re, im);
#else
				mat[i][j] *= filter.getCoefficient(filterXPos, filterYPos);
#endif
				// 対称位置の周波数成分設定
				int oppositeX = (imageXLen - j) % imageXLen;
				int oppositeY = imageYLen - i;
				mat[oppositeY][oppositeX] = conj(mat[i][j]);
			}
			// x軸方向が負の周波数成分について
			for (j = startImageNegativeXPos; j < imageXLen; j++) {
				int filterXPos = (int)(j * xStretch + 0.5);
				if (filterXPos < startFilterNegativeXPos) {
					filterXPos = startFilterNegativeXPos;
				}
#ifdef USE_LINEAR_PHASE
				double amplitude = filter.getCoefficient( // 振幅
					filterXPos, filterYPos
				).real();
				double phase = -(                         // 位相
					(j - imageXLen) * groupDelayX + i * groupDelayY
				) + bias;
				double re = amplitude * cos(phase);       // 係数実部
				double im = amplitude * sin(phase);       // 係数虚部
				mat[i][j] *= Complex(re, im);
#else
				mat[i][j] *= filter.getCoefficient(filterXPos, filterYPos);
#endif
				// 対称位置の周波数成分設定
				int oppositeX = imageXLen - j;
				int oppositeY = imageYLen - i;
				mat[oppositeY][oppositeX] = conj(mat[i][j]);
			}
		}
//
//		// 簡易版 (単純化するとこのようになる)
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
		// 論理画像の更新
		logicalImage->setFrequencyDomain(mat);

		// 物理画像と表示領域の更新
		updatePhysicalImage();
		updateViewer();

	} catch (...) { // 例外時は何もしなかったことにして再スロー
		if (logicalImage != 0) {
			delete logicalImage;
		}
		logicalImage = prevImage;
		throw;
	}

	// 編集スタック更新
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

// 元に戻す()
void ImageEditor::undo()
{
	if (imageStack.empty()) { // スタックが空の場合
		return;
	}

	// スタックポップ
	Image* img = imageStack.front();
	if (img == 0) {
		return;
	}
	Image* oldImage = logicalImage;
	logicalImage = img;

	// 物理画像と表示領域の更新
	try {
		updatePhysicalImage();
	} catch (bad_alloc) {
		logicalImage = oldImage;
		throw;
	}
	updateViewer();

	// オブジェクトの解放
	if (oldImage != 0) {
		delete oldImage;
	}
	imageStack.pop_front();

	// 編集状態更新
	if (saveCount == 0) {
		undoCountDirection = -1;
	}
	saveCount -= undoCountDirection;
}

// 輝度参照()
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
	if (physicalImage->depth() <= 8) { // モノクロの場合
		luminosity = physicalImage->pixelIndex(x, y);
	} else { // カラーの場合
		luminosity = qGray(physicalImage->pixel(x, y));
	}

	return luminosity;
}

// x軸方向画素数参照()
int ImageEditor::getXLength() const
{
	if (logicalImage == 0) {
		return 0;
	}
	return logicalImage->getXLength();
}

// y軸方向画素数参照()
int ImageEditor::getYLength() const
{
	if (logicalImage == 0) {
		return 0;
	}
	return logicalImage->getYLength();
}

// 周波数領域参照()
const Matrix& ImageEditor::getFrequencyDomain() const
{
	if (logicalImage == 0) {
		return dummyMatrix;
	}
	return logicalImage->getFrequencyDomain();
}

// ファイルパス参照()
const String& ImageEditor::getFilePath() const
{
	return filePath;
}

// 保存カウンタ参照()
int ImageEditor::getSaveCount() const
{
	return saveCount;
}

// アンドゥ実行可能性判定()
bool ImageEditor::isUndoingEnabled() const
{
	return !imageStack.empty();
}

// 描画()
void ImageEditor::paintEvent(QPaintEvent* event)
{
	if (physicalImage->isNull()) { // 画像がロードされていない場合
		erase(); // 領域消去
		return;
	}

	int width = physicalImage->width();
	int height = physicalImage->height();

	QPainter painter(this);
	if (this->width() != width || this->height() != height) {
		painter.setWindow(0, 0, width, height); // 描画座標平面設定
	}
	painter.drawImage(0, 0, *physicalImage);
}

// 編集スタック初期化()
void ImageEditor::initializeImageStack()
{
	while (!imageStack.empty()) { // スタックが空になるまで
		Image* img = imageStack.front(); // スタック要素の取得
		if (img != 0) {
			delete img; // オブジェクトの解放
		}
		imageStack.pop_front(); // スタック要素の削除
	}
}

// 物理画像更新()
void ImageEditor::updatePhysicalImage()
{
	int i, j;

	if (logicalImage == 0) {
		return;
	}

	int width = logicalImage->getXLength();
	int height = logicalImage->getYLength();

	bool successful = true;
	if (physicalImage->isNull()) { // 物理画像が初期状態の場合
		successful = physicalImage->create(width, height, 32);
	} else if (
		physicalImage->width() != width || physicalImage->height() != height
	) { // 画像サイズが異なる場合
		// 物理画像を再構築
		QImage* newImage = new QImage();
		successful = newImage->create(width, height, 32);
		if (successful) {
			delete physicalImage;
			physicalImage = newImage;
		} else {
			delete newImage;
		}
	}
	if (!successful) { // 画像のメモリ確保に失敗した場合
		throw bad_alloc();
	}

	bool isMonochrome = (physicalImage->depth() <= 8);
	// 論理画像→物理画像変換
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

// 表示領域更新()
void ImageEditor::updateViewer()
{
	// 画像サイズ取得
	int width = physicalImage->width();
	int height = physicalImage->height();

	// 画像サイズが変わった場合
	if (this->width() != width || this->height() != height) {
		resize(width, height); // サイズ変更 + 再描画
		return;
	}

	repaint();
}

// マウス移動イベントハンドラ()
void ImageEditor::mouseMoveEvent(QMouseEvent* event)
{
	if (physicalImage->isNull()) {
		return;
	}

	int imageWidth = physicalImage->width();
	int imageHeight = physicalImage->height();

	// 伸縮係数算出
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

	// 論理座標計算
	int xPos = (int)(event->x() * xStretch + 0.5);
	int yPos = (int)(event->y() * yStretch + 0.5);

	// 安全対策
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

// マウスクリックイベントハンドラ()
void ImageEditor::mousePressEvent(QMouseEvent* event)
{
	emit mousePressed(event->x(), event->y());
}

// マウスアウトイベントハンドラ()
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



// テストドライバ
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
		cout << "画像読込エラー" << endl;
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
		cout << "フィルタ読込エラー" << endl;
		return 1;
	}

	editor.applyFilter(filter);

	return app.exec();
}

#endif

