// 宣言内容 : 画像編集クラス

#ifndef IEQ_IMAGE_EDITOR
#define IEQ_IMAGE_EDITOR

#ifndef IEQ_IMAGE
#include "ieqimage.h"
#endif

#ifndef IEQ_FILTER
#include "ieqfilter.h"
#endif

#ifndef IEQ_EXCEPTION
#include "ieqexception.h"
#endif

#ifndef IEQ
#include "ieq.h"
#endif

#include <qimage.h>
#include <qwidget.h>

class ieq::ImageEditor : public QWidget {
	Q_OBJECT
public:
	ImageEditor(
		int limit = 0, QWidget* parent = 0, const char* name = 0,
		WFlags flag = WStaticContents | WNoAutoErase
	);
	~ImageEditor();
	void loadImage(const ieq::String& filePath);   // 画像読込
	void saveImage(const ieq::String& filePath, const char* format); // 画像保存
	void applyFilter(const ieq::Filter& filter);   // フィルタ適用
	void undo();                                   // 元に戻す
	int getLuminosity(int x, int y) const;         // 輝度参照
	int getXLength() const;                        // x方向画素数参照
	int getYLength() const;                        // y方向画素数参照
	const ieq::Matrix& getFrequencyDomain() const; // 周波数領域参照
	const ieq::String& getFilePath() const;        // ファイルパス参照
	int getSaveCount() const;                      // 保存カウント参照
	bool isUndoingEnabled() const;                 // アンドゥ実行可能性判定
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
	ieq::String filePath;           // 編集中画像のファイルパス
	int saveCount;                  // 保存カウンタ(何回前に保存したかを表す数)
	int undoCountDirection;         // アンドゥ時に保存カウンタから引かれる数
	QImage* physicalImage;          // 物理画像
	ieq::Image* logicalImage;       // 論理画像
	ieq::ImageList imageStack;      // 画像編集スタック(論理画像のコンテナ)
	int undoingLimit;               // アンドゥ制限回数

	Matrix dummyMatrix;             // ダミー行列

	void updatePhysicalImage();     // 物理画像更新
	void updateViewer();            // 表示領域更新
	void initializeImageStack();    // 編集スタック初期化
};

#endif
