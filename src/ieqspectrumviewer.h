// 宣言内容 : スペクトル表示抽象クラス

#ifndef IEQ_SPECTRUM_VIEWER
#define IEQ_SPECTRUM_VIEWER

#ifndef IEQ
#include "ieq.h"
#endif

#include <qdialog.h>
#include <qstatusbar.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qimage.h>

class ieq::SpectrumViewer : public QWidget {
	Q_OBJECT
public:
	SpectrumViewer(
		int xSize, int ySize,
		QWidget* parent = 0, const char* name = 0, WFlags flag = WNoAutoErase
	);
	~SpectrumViewer();
	void setFrequencyDomain(                       // 周波数領域設定
		const ieq::Matrix& frequencyDomain
	);
	const ieq::Matrix& getFrequencyDomain() const; // 周波数領域参照
	int getXLength() const;                        // x方向サイズ参照
	int getYLength() const;                        // y方向サイズ参照
	virtual void setTitle(const QString& title);

signals:
	void visibilityChanged(bool visibility);  // 可視性変更
	void mouseMoved(                          // マウス移動
		int xPos, int yPos, const ieq::String& message
	);
	void mouseExited();                       // マウスアウト

protected:
	virtual int toLuminosity(      // 輝度算出抽象メソッド
		const Complex& value
	) = 0;
	virtual ieq::String toMessage( // メッセージ変換抽象メソッド
		int xPos, int yPos, const Complex& value
	) = 0;
	void paintEvent(QPaintEvent* event);  // 描画
	void showEvent(QShowEvent* event);    // 可視化
	void hideEvent(QHideEvent* event);    // 不可視化
	void mouseMoveEvent(QMouseEvent* event); // マウス移動イベントハンドラ
	void leaveEvent(QEvent* event);          // マウスアウトイベントハンドラ

private:
	ieq::Matrix frequencyDomain;        // 周波数領域
	QImage* drawingBuffer;              // 描画バッファ

	void updateDrawingBuffer();         // 描画バッファ更新
	void setDrawingBufferColorTable();  // 描画バッファカラーテーブル設定
};

#endif
