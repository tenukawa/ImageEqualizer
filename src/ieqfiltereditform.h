// 宣言内容 : フィルタ編集フォームクラス

#ifndef IEQ_FILTER_EDIT_FORM
#define IEQ_FILTER_EDIT_FORM

#ifndef IEQ_RATIONAL_SPIN_BOX
#include "ieqrationalspinbox.h"
#endif

#ifndef IEQ_FILTER_EDITOR
#include "ieqfiltereditor.h"
#endif

#ifndef IEQ
#include "ieq.h"
#endif

#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qstatusbar.h>
#include <qlabel.h>
#include <qdialog.h>

class ieq::FilterEditForm : public QDialog {
	Q_OBJECT
public:
	FilterEditForm(
		QWidget* parent = 0, const char* name = 0, bool modal = false,
		WFlags flags =
			WStyle_Customize | WStyle_DialogBorder |
			WStyle_Title | WStyle_SysMenu | WStyle_Dialog
	);
	~FilterEditForm();
	const ieq::Filter& getFilter() const;   // フィルタ参照
	const ieq::String& getFilePath() const; // ファイルパス参照
	bool checkSaving();                     // 保存確認
	virtual void setTitle(const QString& title);

	static ieq::String getFilterFileTypes() // 対応ファイルタイプ参照
	{
		return tr("空間周波数領域フィルタ (*.sff)");
	}

signals:
	void applyButtonClicked();               // フィルタ適用ボタンクリック
	void registerButtonClicked();            // フィルタ登録ボタンクリック
	void visibilityChanged(bool visibility); // 可視性変更

protected:
	void showEvent(QShowEvent* event);       // 可視化
	void closeEvent(QCloseEvent* event);     // ダイアログクローズ

private slots: // イベントハンドラ
	void editorMouseMoved(int xPos, int yPos);   // 編集領域マウス移動
	void editorMousePressed(int xPos, int yPos); // 編集領域マウスクリック
	void editorMouseExited();        // 編集領域マウスアウト
	void createButtonClicked();      // 新規作成ボタンクリック
	void loadButtonClicked();        // 読込ボタンクリック
	void saveButtonClicked();        // 上書き保存ボタンクリック
	void saveAsButtonClicked();      // 新規保存ボタンクリック
	void increaseButtonClicked();    // 増ボタンクリック
	void decreaseButtonClicked();    // 減ボタンクリック
	void varianceBoxChanged();       // 変位ボックス値変更
	void varianceSliderChanged();    // 変位スライダ値変更
	void startFrequencyBoxChanged(); // 開始周波数ボックス値変更
	void endFrequencyBoxChanged();   // 終了周波数ボックス値変更
	void startAngleBoxChanged();     // 開始角ボックス値変更
	void endAngleBoxChanged();       // 終了角ボックス値変更
	void coloringSelectorToggled(bool isOn); // 色付け選択ボックストグル

private:
	ieq::FilterEditor* editor;      // 編集領域
	QGroupBox* frequencyGroup;      // 周波数帯設定グループ
	QGroupBox* angleGroup;          // 方向帯設定グループ
	QGroupBox* varianceGroup;       // 変位設定グループ
	QGroupBox* meanGroup;           // 平均係数グループ
	QPushButton* createButton;      // 新規作成ボタン
	QPushButton* loadButton;        // 読込ボタン
	QPushButton* saveButton;        // 上書き保存ボタン
	QPushButton* saveAsButton;      // 新規保存ボタン
	QPushButton* applyButton;       // フィルタ適用ボタン
	QPushButton* registerButton;    // フィルタ登録ボタン
	QPushButton* increaseButton;    // 増ボタン
	QPushButton* decreaseButton;    // 減ボタン
	QSlider* varianceSlider;        // 増減幅設定スライダ
	QSlider* startFrequencySlider;  // 開始周波数設定スライダ
	QSlider* endFrequencySlider;    // 終了周波数設定スライダ
	QSlider* startAngleSlider;      // 開始角設定スライダ
	QSlider* endAngleSlider;        // 終了角設定スライダ
	ieq::RationalSpinBox* varianceBox;        // 増減幅設定スピンボックス
	ieq::RationalSpinBox* startFrequencyBox;  // 開始周波数設定スピンボックス
	ieq::RationalSpinBox* endFrequencyBox;    // 終了周波数設定スピンボックス
	ieq::RationalSpinBox* startAngleBox;      // 開始角設定スピンボックス
	ieq::RationalSpinBox* endAngleBox;        // 終了角設定スピンボックス
	QCheckBox* coloringSelector;    // 色付け選択チェックボックス
	QLabel* meanOfRangeLabel;       // 編集領域平均係数表示ラベル
	QStatusBar* statusBar;          // ステータスバー
	QLabel* statusLabel;            // ステータスラベル

	void createParts();     // GUI部品生成
	void arrangeParts();    // GUI部品配置
	void setFocusChain();   // フォーカスチェーン設定
	void connectEvent();    // イベント接続
	bool loadFilter();      // フィルタ読込
	bool saveFilter();      // フィルタ上書き保存
	bool saveAsFilter();    // フィルタ新規保存
};
#endif
