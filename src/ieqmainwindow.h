// 宣言内容 : メインウィンドウクラス

#ifndef IEQ_MAIN_WINDOW
#define IEQ_MAIN_WINDOW

#ifndef IEQ
#include "ieq.h"
#endif

#ifndef IEQ_IMAGE_EDITOR
#include "ieqimageeditor.h"
#endif

#ifndef IEQ_FILTER_EDIT_FORM
#include "ieqfiltereditform.h"
#endif

#ifndef IEQ_POWER_SPECTRUM_VIEWER
#include "ieqpowerspectrumviewer.h"
#endif

// 二次開発用
// #ifndef IEQ_FILTER_ENTRY
// #include "ieqfilterentry.h"
// #endif
//
// #ifndef IEQ_FILTER_MANAGER
// #include "ieqfiltermanager.h"
// #endif

#include <qaction.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qscrollview.h>
#include <qsplitter.h>
#include <qprogressbar.h>
#include <qdialog.h>
#include <qmainwindow.h>

class ieq::MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(
		QWidget* parent = 0, const char* name = 0, WFlags flag = WType_TopLevel
	);
	~MainWindow();
	virtual void setTitle(const QString& title);

protected:
	void closeEvent(QCloseEvent* event);       // ウィンドウクローズ処理

private slots: // イベントハンドラ
	void openActivated();         // 「開く」アクション実行
	void closeActivated();        // 「閉じる」アクション実行
	void saveActivated();         // 「上書き保存」アクション実行
	void saveAsActivated();       // 「名前を付けて保存」アクション実行
	void undoActivated();         // 「元に戻す」アクション実行
	void showToolBarToggled(bool isOn); // 「ツールバー」表示アクショントグル
	void showStatusBarToggled(    // 「ステータスバー」表示アクショントグル
		bool isOn
	);
	void showSideBarToggled(bool isOn); // 「サイドバー」表示アクショントグル
	void showPowerSpectrumToggled( // 「パワースペクトル」表示アクショントグル
		bool isOn
	);
	void editFilterToggled(bool isOn); // 「フィルタ編集」アクショントグル
	void manageFilterActivated();      // 「フィルタ管理」アクション実行
	void aboutActivated();             // 「バージョン情報」アクション実行
	void editorMouseMoved(int xPos, int yPos); // 画像編集領域マウス移動
	void editorMouseExited();                  // 画像編集領域マウスアウト
	void filterApplyButtonClicked();           // フィルタ適用ボタンクリック
	void filterRegisterButtonClicked();        // フィルタ登録ボタンクリック
	void filterFormVisibilityChanged(    // フィルタフォーム可視性変更
		bool isVisible
	);
	void powerSpectrumVisibilityChanged( // パワースペクトル可視性変更
		bool isVisible
	);
	void powerSpectrumMouseMoved(        // パワースペクトルマウス移動
		int xPos, int yPos, const ieq::String& message
	);
	void powerSpectrumMouseExited();     // パワースペクトルマウスアウト

// 	二次開発用
// 	void hideSideBarButtonClicked();
// 	void editFillterButtonClicked();
// 	void manageFillterButtonClicked();
// 	void filterEntryListBoxOnItem(QListBoxItem* item);
// 	void filterEntryListBoxClicked(QListBoxItem* item);
// 	void filterSelectionActivated(int index);

private:
	void loadSettings();         // 設定読込
	void saveSettings();         // 設定保存
	void createActions();        // アクション生成
	void createMenuBar();        // メニューバー生成
	void createToolBar();        // ツールバー生成
	void createCentralWidget();  // 中央領域生成
	void createStatusBar();      // ステータスバー生成
	void updatePowerSpectrum();  // パワースペクトル更新
	bool loadImage();            // 画像を開く
	bool saveImage();            // 画像を上書き保存
	bool saveAsImage();          // 画像を名前を付けて保存
	bool checkSaving();          // 画像保存確認

	ieq::String entryFilePath;      // フィルタ登録ファイルのパス
	ieq::String lastImageDir;       // 最後に参照した画像のディレクトリパス
	ieq::String lastFilterDir;      // 最後に参照したフィルタのディレクトリパス
	ieq::String supportingImageTypes;  // サポートする画像のファイルタイプ
	ieq::String suppottingFilterTypes; // サポートするフィルタのファイルタイプ
	int undoingLimit;               // アンドゥ制限回数
	int windowLeft;                 // ウィンドウ左端のx座標
	int windowTop;                  // ウィンドウ上端のy座標
	int windowWidth;                // ウィンドウの幅
	int windowHeight;               // ウィンドウの高さ

	QPopupMenu* fileMenu;    // 「ファイル」メニュー
	QPopupMenu* editMenu;    // 「編集」メニュー
	QPopupMenu* viewMenu;    // 「表示」メニュー
	QPopupMenu* filterMenu;  // 「フィルタ」メニュー
	QPopupMenu* helpMenu;    // 「ヘルプ」メニュー

	QToolBar* toolBar;       // ツールバー

	QAction* openAction;                // 「開く」アクション
	QAction* closeAction;               // 「閉じる」アクション
	QAction* saveAction;                // 「上書き保存」アクション
	QAction* saveAsAction;              // 「名前を付けて保存」アクション
	QAction* quitAction;                // 「終了」アクション
	QAction* undoAction;                // 「元に戻す」アクション
	QAction* showToolBarAction;         // 「ツールバー」表示アクション
	QAction* showStatusBarAction;       // 「ステータスバー」表示アクション
	QAction* showSideBarAction;         // 「サイドバー」表示アクション
	QAction* showPowerSpectrumAction;   // 「パワースペクトル」表示アクション
	QAction* editFilterAction;          // 「フィルタ編集」アクション
	QAction* manageFilterAction;        // 「フィルタ管理」アクション
	QAction* aboutAction;               // 「バージョン情報」アクション
	QAction* aboutQtAction;             // 「Qtについて」アクション

	QScrollView* imageScrollViewer;          // 画像表示スクロール領域
	ieq::ImageEditor* editor;                // 画像編集領域
	ieq::FilterEditForm* filterForm;         // フィルタフォーム
	ieq::PowerSpectrumViewer* powerSpectrum; // パワースペクトル

// 	二次開発用
// 	QSplitter* centralSplitter;
// 	QVBox* sideBar;
// 	QPushButton* hideSideBarButton;
// 	QPushButton* editFillterButton;
// 	QPushButton* manageFillterButton;
// 	QListBox* filterEntryListBox;
// 	ieq::FilterManager* manager;
// 	const ieq::FilterEntryList* entryList;
// 	ieq::IntVector filterSelectionIds;

	QLabel* statusLabel;       // ステータスバーのラベル
	QProgressBar* progressBar; // プログレスバー
	QLabel* resolutionLabel;   // 解像度(画素数)表示ラベル
};

#endif
