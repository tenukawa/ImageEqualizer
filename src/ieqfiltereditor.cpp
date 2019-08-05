// クラス名 : フィルタ編集
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
	initialize(); // 初期化
	setMouseTracking(true);
}

FilterEditor::~FilterEditor()
{
	// 解放処理
	delete logicalFilter;
	delete physicalFilter;
	delete rangeMap;
}

// 初期化()
void FilterEditor::initialize()
{
	saveCount = 0;
	filePath = QString(); // ヌル文字列を設定

	if (logicalFilter != 0) {
		delete logicalFilter;
	}
	logicalFilter = new Filter();
	resetBuffer(); // バッファ再設定

	repaint(true, true); // 再描画
}

// フィルタ読込()
void FilterEditor::loadFilter(const String& path)
{
	try {
		logicalFilter->load(path); // 読込
	} catch (...) {
		throw;
	}

	resetBuffer(); // バッファ再設定

	repaint(true, true); // 再描画
	saveCount = 0;
	filePath = path;
}

// フィルタ保存()
void FilterEditor::saveFilter(const String& path)
{
	try {
		logicalFilter->save(path); // 保存
	} catch (...) {
		throw;
	}

	saveCount = 0;
	filePath = path;
}

// バッファ再設定()
void FilterEditor::resetBuffer()
{
	int width = logicalFilter->getXLength();
	int height = logicalFilter->getYLength();

	if ( // 初期状態かサイズが変わった場合
		rangeMap->isNull() ||
		rangeMap->width() != width ||
		rangeMap->height() != height
	) {
		bool successful;
		if (physicalFilter->isNull()) { // 初期状態の場合
			successful = physicalFilter->create(width, height, 8);
		} else { // サイズが変わった場合
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
			setPhysicalFilterColorTable(); // カラーテーブル設定
		} else { // メモリ確保失敗時
			throw bad_alloc();
		}
		rangeMap->resize(width, height);
	}
}

// 色付け設定()
void FilterEditor::setColoring(bool flg)
{
	coloring = flg;
	repaint(false, false);
}

// 周波数帯設定()
void FilterEditor::setFrequencyRange(double start, double end)
{
	// 引数チェック
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
		throw invalid_argument("開始周波数が終了周波数を超えています");
	}

	startFrequency = start;
	endFrequency = end;
	repaint(true, false);
}

// 方向帯設定()
void FilterEditor::setAngleRange(double start, double end)
{
	// 引数チェック
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
		throw invalid_argument("開始角が終了角を超えています");
	}

	startAngle = start;
	endAngle = end;
	repaint(true, false);
}

// 係数増減実行()
void FilterEditor::vary(double variance)
{
	int i, j;

	// 2乗計算(領域判定を緩めるため整数化)
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
		// x座標が正の領域に関するループ
		for (j = 0; j < halfXLen; j++) {
			double xx = (double)(j * j) / (xLen * xLen);
			int rr = (int)((xx + yy) * 1000000);
			if (rr < srsr || rr > erer) {
				continue;
			}
			if (i == 0 && j == 0) { // 座標が(0, 0)の場合
				Complex cof = logicalFilter->getCoefficient(0, 0);
				cof += variance;
				if (cof.real() < 0.0) { // ゼロを下回る場合ゼロにする
					cof = Complex(0.0, cof.imag());
				}
				logicalFilter->setCoefficient(0, 0, cof);
				continue;
			}
			// 偏角計算
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
			// 対称位置の係数設定
			int oppositeX = (xLen - j) % xLen;
			int oppositeY = (yLen - i) % yLen;
			logicalFilter->setCoefficient(
			 	// xLen - j, yLen - i, conj(cof) // バグあり
			 	oppositeX, oppositeY, conj(cof)
			);
			if (i == 0) {
				arcZeroUpdated = true;
			}
		}
		// x座標が負の領域に関するループ
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
			// 対称位置の係数設定
			int oppositeX = xLen - j;
			int oppositeY = (yLen - i) % yLen;
			logicalFilter->setCoefficient(
			 	// xLen - j, yLen - i, conj(cof) // バグあり
			 	oppositeX, oppositeY, conj(cof)
			);
		}
	}

	saveCount += 1;
	repaint(false, true);
}

// 編集領域平均係数参照()
double FilterEditor::getMeanOfRange() const
{
	int i, j;

	// 2乗計算(領域判定を緩めるため整数化)
	int srsr = (int)((startFrequency * startFrequency) * 1000000);
	int erer = (int)((endFrequency * endFrequency) * 1000000);
	int strd = (int)((startAngle * ieq::PI / 180.0) * 10000);
	int enrd = (int)((endAngle * ieq::PI / 180.0) * 10000);

	int yLen = logicalFilter->getYLength();
	int xLen = logicalFilter->getXLength();

	double sum = 0.0; // 参照データ合計
	int cnt = 0;      // 参照データ数

	int halfYLen = (int)ceil(yLen / 2.0); 
	int halfXLen = (int)ceil(xLen / 2.0);
	for (i = 0; i < halfYLen; i++) {
		double yy = (double)(i * i) / (yLen * yLen);
		// x座標が正の領域に関するループ
		for (j = 0; j < halfXLen; j++) {
			double xx = (double)(j * j) / (xLen * xLen);
			int rr = (int)((xx + yy) * 1000000);
			if (rr < srsr || rr > erer) {
				continue;
			}
			if (i == 0 && j == 0) { // 座標が(0, 0)の場合
				Complex cof = logicalFilter->getCoefficient(0, 0);
				sum += cof.real();
				cnt++;
				continue;
			}
			// 偏角計算
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
		// x座標が負の領域に関するループ
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

	// 平均値計算
	double avr = (cnt > 0) ? sum / cnt : 0.0;

	return avr;
}

// フィルタ参照()
const Filter& FilterEditor::getFilter() const
{
	return *logicalFilter;
}

// ファイルパス参照()
const String& FilterEditor::getFilePath() const
{
	return filePath;
}

// 保存カウント参照()
int FilterEditor::getSaveCount() const
{
	return saveCount;
}

// 再描画(引数: 編集領域変更フラグ, 係数増減フラグ)
void FilterEditor::repaint(bool rflg, bool vflg)
{
	rangeUpdated = rflg;
	varied = vflg;
	QWidget::repaint();
}

// 描画()
void FilterEditor::paintEvent(QPaintEvent* event)
{
	if (varied) { // 係数増減時
		updatePhysicalFilter(); // フィルタイメージ更新
	}

	if (rangeUpdated) { // 編集領域変更時
		updateRangeMap(); // 編集領域描画マップ更新
	}

	QPainter painter(this);
	int width = logicalFilter->getXLength();
	int height = logicalFilter->getYLength();
	painter.setWindow(0, 0, width, height); // 描画座標平面設定

	// フィルタ描画
	painter.drawImage(0, 0, *physicalFilter);

	// 編集領域描画
	if (coloring) { // 色付け選択時
		drawRangeBorder(painter);    // 編集領域枠線描画
		painter.setRasterOp(AndROP); // AND演算による色の重ね合せを設定
		painter.drawPixmap(0, 0, *rangeMap); // マップ描画
	}

	// 更新状態リセット
	rangeUpdated = false;
	varied = false;
}

// フィルタイメージ更新()
void FilterEditor::updatePhysicalFilter()
{
	int i, j;

	// フィルタイメージ設定
	int yLen = logicalFilter->getYLength();
	int xLen = logicalFilter->getXLength();
	for (i = 0; i < yLen; i++) {
		// 表示領域の対応するy座標を計算
		// int y = (yLen - 1) - (i + (yLen - 1) / 2) % yLen;
		int y = (i + (yLen - 1) / 2) % yLen;
		for (j = 0; j < xLen; j++) {
			Complex cof = logicalFilter->getCoefficient(j, i);
			// 輝度決定
			int level = ieq::toLuminosity(cof.real(), 128, 255);
			// 表示領域の対応するx座標を計算
			int x = (j + (xLen - 1) / 2) % xLen;
			physicalFilter->setPixel(x, y, level);
		}
	}
}

// 編集領域枠線描画()
void FilterEditor::drawRangeBorder(QPainter& painter)
{
	int xLen = logicalFilter->getXLength();
	int yLen = logicalFilter->getYLength();
	int centerX = (xLen - 1) / 2;                 // 中心のx座標
	int centerY = (yLen - 1) - (yLen - 1) / 2;    // 中心のy座標
	int efRadiusX = (int)(endFrequency * xLen);   // 終了周波数のx軸方向の半径
	int efRadiusY = (int)(endFrequency * yLen);   // 終了周波数のy軸方向の半径
	int sfRadiusX = (int)(startFrequency * xLen); // 開始周波数のx軸方向の半径
	int sfRadiusY = (int)(startFrequency * yLen); // 開始周波数のy軸方向の半径
	int baseAngle = -(int)(startAngle * 16);           // 方向帯描画基底角
	int drawAngle = -(int)(endAngle * 16) - baseAngle; // 方向帯描画角
	int agRadiusX = (int)(xLen * 0.8);            // 方向帯描画のx軸方向の半径
	int agRadiusY = (int)(yLen * 0.8);            // 方向帯描画のy軸方向の半径

	// 角度補整
	if (drawAngle >= -2) { // 描画角が小さすぎると何も描画されないので補整が必要
		baseAngle += 1;
		drawAngle -= 2;
		if (drawAngle >= -2) {
			baseAngle += 1;
			drawAngle -= 2;
		}
	}

	painter.setBrush(NoBrush); // 塗りつぶしの無効化

	// 周波数帯描画
	painter.setPen(QPen(QColor(128, 192, 192), 2)); // ペン幅2では太すぎる?
	painter.drawEllipse( // 終了周波数円描画
		centerX - efRadiusX, centerY - efRadiusY, efRadiusX * 2, efRadiusY * 2
	);
	painter.drawEllipse( // 開始周波数円描画
		centerX - sfRadiusX, centerY - sfRadiusY, sfRadiusX * 2, sfRadiusY * 2
	);

	// 方向帯描画
	painter.setPen(QPen(QColor(192, 192, 128), 2));
	painter.drawPie( // 扇形描画
		centerX - agRadiusX, centerY - agRadiusY, agRadiusX * 2, agRadiusY * 2,
		baseAngle, drawAngle
	);
	painter.drawPie( // 対角の扇形描画
		centerX - agRadiusX, centerY - agRadiusY, agRadiusX * 2, agRadiusY * 2,
		baseAngle + 180 * 16, drawAngle
	);
}

// 編集領域描画バッファ更新()
void FilterEditor::updateRangeMap()
{
	int xLen = logicalFilter->getXLength();
	int yLen = logicalFilter->getYLength();
	int centerX = (xLen - 1) / 2;                 // 中心のx座標
	int centerY = (yLen - 1) - (yLen - 1) / 2;    // 中心のy座標
	int efRadiusX = (int)(endFrequency * xLen);   // 終了周波数のx軸方向の半径
	int efRadiusY = (int)(endFrequency * yLen);   // 終了周波数のy軸方向の半径
	int sfRadiusX = (int)(startFrequency * xLen); // 開始周波数のx軸方向の半径
	int sfRadiusY = (int)(startFrequency * yLen); // 開始周波数のy軸方向の半径
	int baseAngle = -(int)(startAngle * 16);           // 方向帯描画基底角
	int drawAngle = -(int)(endAngle * 16) - baseAngle; // 方向帯描画角
	int agRadiusX = (int)(xLen * 0.8);            // 方向帯描画のx軸方向の半径
	int agRadiusY = (int)(yLen * 0.8);            // 方向帯描画のy軸方向の半径

	// 前準備
	rangeMap->fill(QColor(255, 255, 255)); // ホワイトクリア
	QPainter painter(rangeMap);
	painter.setBackgroundColor(QColor(255, 255, 255));
	painter.setPen(NoPen); // ペンを無効化

	// 周波数帯描画
	painter.setBrush(QBrush(QColor(128, 255, 255), SolidPattern)); // 青ブラシ
	painter.drawEllipse( // 終了周波数の青い円を描画
		centerX - efRadiusX, centerY - efRadiusY, efRadiusX * 2, efRadiusY * 2
	);
	painter.setBrush(QBrush(QColor(255, 255, 255), SolidPattern)); // 白ブラシ
	painter.drawEllipse( // 中央から開始周波数までを白い円でくり抜き
		centerX - sfRadiusX, centerY - sfRadiusY, sfRadiusX * 2, sfRadiusY * 2
	);

	// 方向帯描画
	painter.setRasterOp(AndROP); // AND演算による色の重ね合わせを設定
	painter.setBrush(QBrush(QColor(255, 255, 128), SolidPattern)); // 黄ブラシ
	painter.drawPie( // 扇形描画
		centerX - agRadiusX, centerY - agRadiusY, agRadiusX * 2, agRadiusY * 2,
		baseAngle, drawAngle
	);
	painter.drawPie( // 対角の扇形描画
		centerX - agRadiusX, centerY - agRadiusY, agRadiusX * 2, agRadiusY * 2,
		baseAngle + 180 * 16, drawAngle
	);
}

// マウス移動イベントハンドラ()
void FilterEditor::mouseMoveEvent(QMouseEvent* event)
{
	emit mouseMoved(event->x(), event->y());
}

// マウスクリックイベントハンドラ()
void FilterEditor::mousePressEvent(QMouseEvent* event)
{
	emit mousePressed(event->x(), event->y());
}

// マウスアウトイベントハンドラ()
void FilterEditor::leaveEvent(QEvent* event)
{
	emit mouseExited();
}

// フィルタカラーテーブル設定()
void FilterEditor::setPhysicalFilterColorTable()
{
	int i;

	if (physicalFilter->isNull()) {
		return;
	}

	static const int numColors = 256;
	physicalFilter->setNumColors(numColors);

	for (i = 0; i < numColors; i++) { // 256諧調グレースケールテーブルを設定
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



// テストドライバ
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
