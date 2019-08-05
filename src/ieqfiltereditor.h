// 宣言内容 : フィルタ編集クラス

#ifndef IEQ_FILTER_EDITOR
#define IEQ_FILTER_EDITOR

#ifndef IEQ
#include "ieq.h"
#endif

#ifndef IEQ_FILTER
#include "ieqfilter.h"
#endif

#include <qimage.h>
#include <qpixmap.h>
#include <qwidget.h>

class ieq::FilterEditor : public QWidget {
	Q_OBJECT
public:
	FilterEditor(
		int width, int height, QWidget* parent = 0, const char* name = 0
	);
	~FilterEditor();
	void loadFilter(const ieq::String& filePath);     // フィルタ読込
	void saveFilter(const ieq::String& filePath);     // フィルタ保存
	void setColoring(bool coloring);                  // 色付け設定
	void setFrequencyRange(double start, double end); // 周波数帯設定
	void setAngleRange(double start, double end);     // 方向帯設定
	void vary(double variance);                       // 増減実行
	void initialize();                                // 初期化
	void repaint(bool rangeUpdated, bool varied);     // 再描画
	double getMeanOfRange() const;                    // 編集領域平均係数参照
	const ieq::Filter& getFilter() const;             // フィルタ参照
	const ieq::String& getFilePath() const;           // ファイルパス参照
	int getSaveCount() const;                         // 保存カウント参照
	virtual void setTitle(const QString& title);

signals:
	void mouseMoved(int xPos, int yPos);    // マウス移動
	void mousePressed(int xPos, int yPos);  // マウスクリック
	void mouseExited();                     // マウスアウト

protected:
	void paintEvent(QPaintEvent* event);      // 描画
	void mouseMoveEvent(QMouseEvent* event);  // マウス移動イベントハンドラ
	void mousePressEvent(QMouseEvent* event); // マウスクリックイベントハンドラ
	void leaveEvent(QEvent* event);           // マウスアウトイベントハンドラ

private:
	ieq::String filePath;         // 編集中フィルタのファイルパス
	int saveCount;                // 保存カウンタ(何回前に保存したかを表す数)
	bool coloring;                // 編集領域色付けフラグ(真:色付け, 偽:色なし)
	ieq::Filter* logicalFilter;   // 論理フィルタ
	double startFrequency;        // 編集周波数帯開始周波数
	double endFrequency;          // 編集周波数帯終了周波数
	double startAngle;            // 編集方向帯開始角(start declination)
	double endAngle;              // 編集方向帯終了角
	bool rangeUpdated;            // 編集領域更新フラグ
	bool varied;                  // 増減フラグ
	QImage* physicalFilter;       // フィルタイメージ
	QPixmap* rangeMap;            // 編集領域描画バッファ

	void updatePhysicalFilter();              // フィルタイメージ更新
	void drawRangeBorder(QPainter& painter);  // 編集領域枠線描画
	void updateRangeMap();                    // 編集領域描画バッファ更新
	void resetBuffer();                       // バッファ再設定
	void setPhysicalFilterColorTable();       // フィルタカラーテーブル設定
};

#endif
