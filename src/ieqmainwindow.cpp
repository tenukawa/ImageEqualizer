// クラス名 : メインウィンドウ

#include "ieqmainwindow.h"
#include "ieqexception.h"

#include <qaction.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qvbox.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qscrollview.h>
#include <qsplitter.h>
#include <qprogressbar.h>
#include <qmessagebox.h>
#include <qmainwindow.h>
#include <qapplication.h>
#include <new>

using namespace ieq;
using namespace boost;
using namespace std;

MainWindow::MainWindow(QWidget* parent, const char* name, WFlags flag)
	: QMainWindow(parent, name, flag),
	editor(0), powerSpectrum(0), filterForm(0)
{
	loadSettings();         // 設定読込
	createActions();        // アクション生成
	createMenuBar();        // メニューバー生成
	createToolBar();        // ツールバー生成
	createCentralWidget();  // 中央領域生成
	createStatusBar();      // ステータスバー生成
	setTitle(tr("Image Equalizer"));
	setGeometry(windowLeft, windowTop, windowWidth, windowHeight);
}

MainWindow::~MainWindow()
{
	// QtによるGUI部品オブジェクトの自動解放
}

// ウィンドウクローズ処理()
void MainWindow::closeEvent(QCloseEvent* event)
{
	// フィルタ保存確認
	if (filterForm != 0 && !filterForm->checkSaving()) {
		event->ignore();
		return;
	}

	// 画像保存確認
	if (!checkSaving()) {
		event->ignore();
		return;
	}

	saveSettings();  // 設定保存
	event->accept();
}

// 「開く」アクション実行()
void MainWindow::openActivated()
{
	if (!checkSaving()) {
		return;
	}

	loadImage(); // 画像の読込
}

// 「閉じる」アクション実行()
void MainWindow::closeActivated()
{
	if (editor == 0) {
		return;
	}
	if (!checkSaving()) {
		return;
	}

	delete editor; // 画像編集領域を解放してスクロール領域から除去
	editor = 0;

	closeAction->setEnabled(false);
	saveAction->setEnabled(false);
	saveAsAction->setEnabled(false);
	showPowerSpectrumAction->setEnabled(false);

	resolutionLabel->clear(); // 画素数表示をクリア
	setTitle(tr("Image Equalizer"));
	updatePowerSpectrum(); // パワースペクトル更新
}

// 「上書き保存」アクション実行()
void MainWindow::saveActivated()
{
	saveImage();
}

// 「名前を付けて保存」アクション実行()
void MainWindow::saveAsActivated()
{
	saveAsImage();
}

// 「元に戻す」アクション実行()
void MainWindow::undoActivated()
{
	if (editor == 0 || !editor->isUndoingEnabled()) {
		return;
	}

	editor->undo();

	if (!editor->isUndoingEnabled()) { // これ以上前の状態に戻せなくなった時
		undoAction->setEnabled(false);
	}

	updatePowerSpectrum();
}

// 「ツールバー」表示アクショントグル()
void MainWindow::showToolBarToggled(bool isOn)
{
	// 表示・非表示切替
	if (isOn && toolBar->isHidden()) {
		toolBar->show();
	} else if (!isOn && toolBar->isVisible()) {
		toolBar->hide();
	}
}

// 「ステータスバー」表示アクショントグル()
void MainWindow::showStatusBarToggled(bool isOn)
{
	// 表示・非表示切替
	if (isOn && statusBar()->isHidden()) {
		statusBar()->show();
	} else if (!isOn && statusBar()->isVisible()) {
		statusBar()->hide();
	}
}

// 「サイドバー」表示アクショントグル()
// 二次開発用
void MainWindow::showSideBarToggled(bool isOn)
{
}

// 「パワースペクトル」表示アクショントグル()
void MainWindow::showPowerSpectrumToggled(bool isOn)
{
	updatePowerSpectrum();
}

// 「フィルタ編集」アクショントグル()
void MainWindow::editFilterToggled(bool isOn)
{
	if (isOn && filterForm == 0) { // 初回処理
		filterForm = new FilterEditForm(this);
		connect(
			filterForm, SIGNAL(applyButtonClicked()),
			this, SLOT(filterApplyButtonClicked())
		);
		connect(
			filterForm, SIGNAL(registerButtonClicked()),
			this, SLOT(filterRegisterButtonClicked())
		);
		connect(
			filterForm, SIGNAL(visibilityChanged(bool)),
			this, SLOT(filterFormVisibilityChanged(bool))
		);
	}

	if (isOn && filterForm->isHidden()) {
		filterForm->show();
	} else if (isOn && filterForm->isVisible()) {
		filterForm->raise();
		filterForm->setActiveWindow();
	} else if (!isOn && filterForm->isVisible()) {
		filterForm->hide();
	}
}

// 「フィルタ管理」アクション実行()
// 二次開発用
void MainWindow::manageFilterActivated()
{
}

// 「バージョン情報」アクション実行()
void MainWindow::aboutActivated()
{
	// QMessageBox::about();
	QMessageBox box(this);

	box.setCaption(tr("Image Equalizer のバージョン情報"));
	box.setTextFormat(RichText);
	box.setText(
		tr(
			"<h2>Image Equalizer v0.10<br>"
			"<small> - 画像の周波数フィルタリングソフト</small></h2>"

			"<p>Copyright &copy; 2005 Niekawa &lt;schole@nifty.com&gt;</p>"

			"<p>本プログラムはフリー・ソフトウェアです。"
			"あなたは、Free Software Foundationが公表した"
			"GNU 一般公有使用許諾の「バージョン2」或いは"
			"それ以降の各バージョンの中からいずれかを選択し、"
			"そのバージョンが定める条項に従って本プログラムを"
			"再頒布または変更することができます。</p>"

			"<p>本プログラムは有用とは思いますが、"
			"頒布にあたっては、市場性及び特定目的適合性についての"
			"暗黙の保証を含めて、いかなる保証も行ないません。"
			"詳細についてはGNU 一般公有使用許諾書をお読みください。</p>"

			"<p>あなたは、本プログラムと一緒にGNU 一般公有使用許諾の"
			"写しを受け取っているはずです。そうでない場合は、"
			"Free Software Foundation, Inc., 675 Mass Ave, Cambridge, "
			"MA 02139, USA へ手紙を書いてください。</p>"
		)
	);
	box.setIconPixmap(QPixmap::fromMimeSource("logo.xpm"));
	box.exec();
}

// 二次開発用
// void MainWindow::filterEntryListBoxOnItem(QListBoxItem* item)
// {
// }

// 二次開発用
// void MainWindow::filterEntryListBoxClicked(QListBoxItem* item)
// {
// }

// 画像編集領域マウス移動()
void MainWindow::editorMouseMoved(int xPos, int yPos)
{
	if (editor == 0) {
		return;
	}

	QString posInfo("");
	QString formatter;
	posInfo += tr("X: ");
	posInfo += formatter.sprintf("%d  ", xPos);
	posInfo += tr("Y: ");
	posInfo += formatter.sprintf("%d   ", yPos);
	posInfo += tr("輝度: ");
	posInfo += formatter.sprintf("%d", editor->getLuminosity(xPos, yPos));
	statusLabel->setText(posInfo);
}

// 画像編集領域マウスアウト()
void MainWindow::editorMouseExited()
{
	statusLabel->clear(); // ステータスラベルをクリア
}

// フィルタ適用ボタンクリック()
void MainWindow::filterApplyButtonClicked()
{
	if (filterForm == 0) {
		return;
	}

	// 画像が読み込まれていない場合は読込
	if (editor == 0) {
		if (!loadImage()) {
			return;
		}
	}

	// フィルタ適用
	try {
		editor->applyFilter(filterForm->getFilter());
	} catch (bad_alloc) {
		QMessageBox::critical( // エラーメッセージを出す
			this, tr("エラー"),
			tr("メモリが不足しているため、処理できません。"),
			QMessageBox::Ok | QMessageBox::Default, 0, 0
		);
		return;
	}

	if (editor->isUndoingEnabled()) { // 元に戻せる場合
		undoAction->setEnabled(true);
	}

	updatePowerSpectrum(); // パワースペクトル更新
}

// フィルタ登録ボタンクリック()
// 二次開発用
void MainWindow::filterRegisterButtonClicked()
{
}

// フィルタフォーム可視性変更()
void MainWindow::filterFormVisibilityChanged(bool isVisible)
{
	// 「フィルタ編集」アクションのオン・オフ切替
	if (isVisible && !editFilterAction->isOn()) {
		editFilterAction->setOn(true);
	} else if (!isVisible && editFilterAction->isOn()) {
		editFilterAction->setOn(false);
	}
}

// パワースペクトル可視性変更()
void MainWindow::powerSpectrumVisibilityChanged(bool isVisible)
{
	if (showPowerSpectrumAction->isEnabled()) {
		showPowerSpectrumAction->setOn(isVisible);
	}
}

// パワースペクトルマウス移動()
void MainWindow::powerSpectrumMouseMoved(
	int xPos, int yPos, const ieq::String& message
)
{
	if (statusBar()->isVisible()) {
		statusLabel->setText(message);
	}
}

// パワースペクトルマウスアウト()
void MainWindow::powerSpectrumMouseExited()
{
	statusLabel->clear();
}

// 二次開発用
// void MainWindow::filterSelectionActivated(int index)
// {
// }

// 設定読込()
void MainWindow::loadSettings()
{
	// デフォルト設定
	entryFilePath = QString::null;
	lastImageDir = QString::null;
	lastFilterDir = QString::null;
	supportingImageTypes = tr(
		"ポータブル グレーマップ (*.pgm);;Windows ビットマップ (*.bmp)"
	);
	suppottingFilterTypes = tr("空間周波数領域フィルタ (*.sff)");
	undoingLimit = 8;
	windowWidth = 640;
	windowHeight = 480;

    QWidget* desktop = QApplication::desktop(); // デスクトップ情報の取得
    windowLeft = (desktop->width() - windowWidth) / 2;
    windowTop = (desktop->height() - windowHeight) / 2 + 30;
}

// 設定保存()
void MainWindow::saveSettings()
{
}

// アクション生成()
void MainWindow::createActions()
{
	// 「開く」アクション
	openAction = new QAction(tr("開く(&O)..."), tr("Ctrl+O"), this);
	openAction->setIconSet(QPixmap::fromMimeSource("open.xpm"));
	openAction->setToolTip(tr("開く (Ctrl+O)"));
	openAction->setStatusTip(tr("画像ファイルを開きます。"));
	connect(openAction, SIGNAL(activated()), this, SLOT(openActivated()));

	// 「閉じる」アクション
	closeAction = new QAction(tr("閉じる(&C)"), 0, this);
	closeAction->setToolTip(tr("閉じる"));
	closeAction->setStatusTip(tr("編集中の画像を閉じます。"));
	closeAction->setEnabled(false);
	connect(closeAction, SIGNAL(activated()), this, SLOT(closeActivated()));

	// 「上書き保存」アクション
	saveAction = new QAction(tr("上書き保存(&S)"), tr("Ctrl+S"), this);
	saveAction->setIconSet(QPixmap::fromMimeSource("save.xpm"));
	saveAction->setToolTip(tr("上書き保存 (Ctrl+S)"));
	saveAction->setStatusTip(tr("編集中の画像を保存します。"));
	saveAction->setEnabled(false);
	connect(saveAction, SIGNAL(activated()), this, SLOT(saveActivated()));

	// 「名前を付けて保存」アクション
	saveAsAction = new QAction(tr("名前を付けて保存(&A)..."), 0, this);
	saveAsAction->setToolTip(tr("名前を付けて保存"));
	saveAsAction->setStatusTip(tr("編集中の画像を別のファイルへ保存します。"));
	saveAsAction->setEnabled(false);
	connect(saveAsAction, SIGNAL(activated()), this, SLOT(saveAsActivated()));

	// 「終了」アクション
	quitAction = new QAction(tr("終了(&X)"), 0, this);
	quitAction->setToolTip(tr("終了"));
	quitAction->setStatusTip(tr("Image Equalizer を終了します。"));
	connect(quitAction, SIGNAL(activated()), this, SLOT(close()));

	// 「元に戻す」アクション
	undoAction = new QAction(tr("元に戻す(&U)"), tr("Ctrl+Z"), this);
	undoAction->setIconSet(QPixmap::fromMimeSource("undo.xpm"));
	undoAction->setToolTip(tr("元に戻す (Ctrl+Z)"));
	undoAction->setStatusTip(tr("直前の操作を元に戻します。"));
	undoAction->setEnabled(false);
	connect(undoAction, SIGNAL(activated()), this, SLOT(undoActivated()));

	// 「ツールバー」表示アクション
	showToolBarAction = new QAction(tr("ツールバー(&T)"), 0, this);
	showToolBarAction->setToggleAction(true);
	showToolBarAction->setToolTip(tr("ツールバーの表示/非表示"));
	showToolBarAction->setStatusTip(
		tr("ツールバーの表示/非表示を切り替えます。")
	);
	showToolBarAction->setOn(true);
	connect(
		showToolBarAction, SIGNAL(toggled(bool)),
		this, SLOT(showToolBarToggled(bool))
	);

	// 「サイドバー」表示アクション
	showSideBarAction = new QAction(tr("サイドバー(&F)"), 0, this);
	showSideBarAction->setToggleAction(true);
	showSideBarAction->setToolTip(tr("サイドバーの表示/非表示"));
	showSideBarAction->setStatusTip(
		tr("サイドバーの表示/非表示を切り替えます。")
	);
	showSideBarAction->setOn(false);
	showSideBarAction->setEnabled(false);
	connect(
		showSideBarAction, SIGNAL(toggled(bool)),
		this, SLOT(showSideBarToggled(bool))
	);

	// 「ステータス バー」表示アクション
	showStatusBarAction = new QAction(tr("ステータス バー(&I)"), 0, this);
	showStatusBarAction->setToggleAction(true);
	showStatusBarAction->setToolTip(tr("ステータス バーの表示/非表示"));
	showStatusBarAction->setStatusTip(
		tr("ステータス バーの表示/非表示を切り替えます。")
	);
	showStatusBarAction->setOn(true);
	connect(
		showStatusBarAction, SIGNAL(toggled(bool)),
		this, SLOT(showStatusBarToggled(bool))
	);

	// 「パワースペクトル」表示アクション
	showPowerSpectrumAction = new QAction(tr("パワースペクトル(&P)"), 0, this);
	showPowerSpectrumAction->setIconSet(QPixmap::fromMimeSource("power.xpm"));
	showPowerSpectrumAction->setToggleAction(true);
	showPowerSpectrumAction->setOn(false);
	showPowerSpectrumAction->setToolTip(tr("パワースペクトルの表示/非表示"));
	showPowerSpectrumAction->setStatusTip(
		tr("パワースペクトルの表示/非表示を切り替えます。")
	);
	showPowerSpectrumAction->setEnabled(false);
	connect(
		showPowerSpectrumAction, SIGNAL(toggled(bool)),
		this, SLOT(showPowerSpectrumToggled(bool))
	);

	// 「フィルタ編集」アクション
	editFilterAction = new QAction(tr("フィルタ編集(&E)"), 0, this);
	editFilterAction->setIconSet(QPixmap::fromMimeSource("filter.xpm"));
	editFilterAction->setToggleAction(true);
	editFilterAction->setToolTip(tr("フィルタ編集フォームの表示/非表示"));
	editFilterAction->setStatusTip(
		tr("フィルタ編集フォームの表示/非表示を切り替えます。")
	);
	editFilterAction->setOn(false);
	connect(
		editFilterAction, SIGNAL(toggled(bool)),
		this, SLOT(editFilterToggled(bool))
	);

	// 「フィルタ管理」アクション
	manageFilterAction = new QAction(tr("フィルタ管理(&M)..."), 0, this);
	manageFilterAction->setToolTip(tr("フィルタ管理"));
	manageFilterAction->setStatusTip(tr("フィルタの登録情報を管理します。"));
	manageFilterAction->setEnabled(false);
	connect(
		manageFilterAction, SIGNAL(activated()),
		this, SLOT(manageFilterActivated())
	);

	// 「バージョン情報」アクション
	aboutAction = new QAction(tr("バージョン情報(&A)..."), 0, this);
	aboutAction->setToolTip(tr("バージョン情報"));
	aboutAction->setStatusTip(
		tr("著作権およびバージョンに関する情報を表示します。")
	);
	connect(aboutAction, SIGNAL(activated()), this, SLOT(aboutActivated()));

	// 「Qt について」アクション
	aboutQtAction = new QAction(tr("&Qt について..."), 0, this);
	aboutQtAction->setToolTip(tr("Qt について"));
	aboutQtAction->setStatusTip(tr("Qt ライブラリに関する情報を表示します。"));
	connect(aboutQtAction, SIGNAL(activated()), qApp, SLOT(aboutQt()));
}

// メニューバー生成()
void MainWindow::createMenuBar()
{
	// 「ファイル」メニュー
	fileMenu = new QPopupMenu(this);
	openAction->addTo(fileMenu);
	closeAction->addTo(fileMenu);
	fileMenu->insertSeparator();
	saveAction->addTo(fileMenu);
	saveAsAction->addTo(fileMenu);
	fileMenu->insertSeparator();
	quitAction->addTo(fileMenu);

	// 「編集」メニュー
	editMenu = new QPopupMenu(this);
	undoAction->addTo(editMenu);

	// 「表示」メニュー
	viewMenu = new QPopupMenu(this);
	showPowerSpectrumAction->addTo(viewMenu);
	viewMenu->insertSeparator();
	showToolBarAction->addTo(viewMenu);
	showSideBarAction->addTo(viewMenu);
	showStatusBarAction->addTo(viewMenu);

	// 「フィルタ」メニュー
	filterMenu = new QPopupMenu(this);
	editFilterAction->addTo(filterMenu);
	manageFilterAction->addTo(filterMenu);

	// 「ヘルプ」メニュー
	helpMenu = new QPopupMenu(this);
	aboutAction->addTo(helpMenu);
	aboutQtAction->addTo(helpMenu);

	menuBar()->insertItem(tr("ファイル(&F)"), fileMenu);
	menuBar()->insertItem(tr("編集(&E)"), editMenu);
	menuBar()->insertItem(tr("表示(&V)"), viewMenu);
	menuBar()->insertItem(tr("フィルタ(&I)"), filterMenu);
	menuBar()->insertItem(tr("ヘルプ(&H)"), helpMenu);
}

// ツールバー生成()
void MainWindow::createToolBar()
{
	toolBar = new QToolBar(tr("ツールバー"), this);
	openAction->addTo(toolBar);
	saveAction->addTo(toolBar);
	toolBar->addSeparator();
	undoAction->addTo(toolBar);
	toolBar->addSeparator();
	showPowerSpectrumAction->addTo(toolBar);
	editFilterAction->addTo(toolBar);
	toolBar->setMovingEnabled(false);
}

// 中央領域生成()
void MainWindow::createCentralWidget()
{
	imageScrollViewer = new QScrollView(this);
	setCentralWidget(imageScrollViewer);
}

// ステータスバー生成()
void MainWindow::createStatusBar()
{
	statusLabel = new QLabel(tr("X: 32767  Y: 32767"), this);
	QSize labelSize = statusLabel->sizeHint();
	labelSize.setHeight(labelSize.height() + 4);
	statusLabel->setMinimumSize(labelSize);
	statusLabel->clear();
	resolutionLabel = new QLabel(tr(" 32767 x 32767 "), this);
	resolutionLabel->setAlignment(AlignCenter);
	resolutionLabel->setMinimumSize(
		resolutionLabel->sizeHint().width(), labelSize.height()
	);
	resolutionLabel->clear();
	progressBar = new QProgressBar(this);
	progressBar->setCenterIndicator(false);
	progressBar->setFixedSize(120, labelSize.height());
	progressBar->setPercentageVisible(false);

	statusBar()->setSizeGripEnabled(false);
	statusBar()->addWidget(statusLabel, 1);  // 伸縮あり
	statusBar()->addWidget(progressBar);     // 伸縮なし
	statusBar()->addWidget(resolutionLabel); // 伸縮なし
}

// 画像保存確認()
bool MainWindow::checkSaving()
{
	if (editor == 0 || editor->getSaveCount() == 0) {
		return true;
	}

	// メッセージボックス生成
	QString message("");
	QString filePath = editor->getFilePath();
	if (filePath.isEmpty()) {
		message += tr("(新規画像)");
	} else {
		message += QDir::convertSeparators(filePath);
	}
	message += tr(" は変更されています。保存しますか?");
	QMessageBox box(
		tr("Image Equalizer"), message, QMessageBox::Warning,
		QMessageBox::Yes | QMessageBox::Default,
		QMessageBox::No,
		QMessageBox::Cancel | QMessageBox::Escape
	);
	box.setButtonText(QMessageBox::Yes, tr("はい(&Y)"));
	box.setButtonText(QMessageBox::No, tr("いいえ(&N)"));
	box.setButtonText(QMessageBox::Cancel, tr("キャンセル"));

	int ret = box.exec();
	bool saved;
	switch (ret) {
	case QMessageBox::Yes:
		saved = saveImage();
		if (!saved) {
			return false;
		}
		return true;
	case QMessageBox::No:
		return true;
	case QMessageBox::Cancel:
		return false;
	}

	return false;
}

// 画像を開く()
bool MainWindow::loadImage()
{
	QString filePath = QFileDialog::getOpenFileName(
		QString::null, supportingImageTypes,
		0, 0, tr("開く")
	);

	if (filePath.isEmpty()) { // キャンセル選択時
		return false;
	}

	// 読込実行
	QString errorMessage;
	ImageEditor* newEditor = new ImageEditor(undoingLimit, this);
	try {
		newEditor->loadImage(filePath);
	} catch (NoSuchFileException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" が見つかりません。\n"
			"ファイル名を確認してください。"
		);
	} catch (InputException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" の読込に失敗しました。\n"
			"対応できないファイル形式が指定された可能性があります。\n"
			"または、アクセス権がないか、ファイルがロックされています。"
		);
	}

	if (!errorMessage.isNull()) { // エラー発生時
		delete newEditor;
		QMessageBox::warning( // エラーメッセージを出す
			this, tr("画像読込エラー"), errorMessage,
			QMessageBox::Ok | QMessageBox::Default, 0, 0
		);
		return false;
	}

	// 読み込み成功時
	if (editor != 0) {
		delete editor; // 前の画像編集領域を解放してスクロール領域から除去
	}
	editor = newEditor;
	imageScrollViewer->addChild(editor);
	editor->show();
	connect(
		editor, SIGNAL(mouseMoved(int, int)),
		this, SLOT(editorMouseMoved(int, int))
	);
	connect(editor, SIGNAL(mouseExited()), this, SLOT(editorMouseExited()));

	closeAction->setEnabled(true);
	saveAction->setEnabled(true);
	saveAsAction->setEnabled(true);
	showPowerSpectrumAction->setEnabled(true);

	// 解像度表示更新
	QString resolution("");
	QString formatter;
	resolution += formatter.sprintf(" %d", editor->getXLength());
	resolution += tr(" x ");
	resolution += formatter.sprintf("%d ", editor->getYLength());
	resolutionLabel->setText(resolution);

	// タイトルバー文字列更新
	setTitle(
		QDir::convertSeparators(editor->getFilePath()) + tr(
			" - Image Equalizer"
		)
	);

	updatePowerSpectrum(); // パワースペクトル更新

	return true;
}

// 画像を上書き保存()
bool MainWindow::saveImage()
{
	if (editor == 0) {
		return true;
	}

	QString filePath = editor->getFilePath();
	if (filePath.isEmpty()) { // (新規画像) の場合
		return saveAsImage();
	}

	// 保存形式取得
	const char* format; // 保存形式
	QString ext = QFileInfo(filePath).extension(false); // 拡張子取得
	if (!ext.isEmpty() && ext.lower() == "pgm") {        // pgmの場合
		format = "PGM";
	} else if (!ext.isEmpty() && ext.lower() == "bmp") { // bmpの場合
		format = "BMP";
	} else { // 上記以外の拡張子の場合、または拡張子がない場合
		format = QImage::imageFormat(filePath); // 実際の形式を取得
		if (format == 0) { // 形式不明の場合
			format = "PPM";
		}
	}

	// 保存実行
	QString errorMessage;
	try {
		editor->saveImage(filePath, format);
	} catch (NoSuchDirectoryException) {
		return saveAsImage();
	} catch (OutputException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" の保存に失敗しました。\n"
			"対応できないファイル形式が指定された可能性があります。\n"
			"または、アクセス権がないか、ファイルがロックされています。"
		);
	}

	if (!errorMessage.isNull()) { // エラー発生時
		QMessageBox::warning( // エラーメッセージを出す
			this, tr("画像保存エラー"), errorMessage,
			QMessageBox::Ok | QMessageBox::Default, 0, 0
		);
		return false;
	}

	return true;
}

// 画像を名前を付けて保存()
bool MainWindow::saveAsImage()
{
	if (editor == 0) {
		return true;
	}

	QString currentFilePath = editor->getFilePath();
	if (currentFilePath.isEmpty() || !QFile::exists(currentFilePath)) {
		currentFilePath = QString::null;
	}

	QString selectedType; // 選択されたファイルタイプ
	QString filePath = QFileDialog::getSaveFileName( // ダイアログオープン
		currentFilePath, supportingImageTypes,
		0, 0, tr("名前を付けて保存"), &selectedType
	);
	if (filePath.isEmpty()) { // キャンセル選択時
		return false;
	}

	// 選択されたファイルタイプの拡張子取得
	QString selectedTypeExt;
	if (selectedType.isEmpty()) {
		selectedTypeExt = "";
	} else if (selectedType.find(QString("pgm"), 0, false) > 0) { // PGM
		selectedTypeExt = "pgm";
	} else if (selectedType.find(QString("bmp"), 0, false) > 0) { // BMP
		selectedTypeExt = "bmp";
	} else {
		selectedTypeExt = "";
	}
	// 選択されたファイルの拡張子取得
	QString selectedFileExt = QFileInfo(filePath).extension(false);
	// ファイルの拡張子がないかファイルタイプの拡張子と違う場合
	if (selectedFileExt.isEmpty() || selectedFileExt.lower()!=selectedTypeExt) {
		filePath += "." + selectedTypeExt; // 拡張子を付加
	}

	if (QFile::exists(filePath)) { // 既存のファイルを選択した場合
		// 上書きの確認ダイアログを出す
		QString message = QDir::convertSeparators(filePath) + tr(
			" は既に存在します。\n上書きしますか?"
		);
		QMessageBox box(
			tr("名前を付けて保存"), message, QMessageBox::Warning,
			QMessageBox::Yes | QMessageBox::Default,
			QMessageBox::No | QMessageBox::Escape, 0
		);
		box.setButtonText(QMessageBox::Yes, tr("はい(&Y)"));
		box.setButtonText(QMessageBox::No, tr("いいえ(&N)"));
		int ret = box.exec();
		if (ret != QMessageBox::Yes) { // Yes以外を選択した場合
			return false;
		}
	}

	// 保存形式取得
	const char* format; // 保存形式
	QString ext = QFileInfo(filePath).extension(false); // 拡張子取得
	if (!ext.isEmpty() && ext.lower() == "pgm") {        // pgmの場合
		format = "PGM";
	} else if (!ext.isEmpty() && ext.lower() == "bmp") { // bmpの場合
		format = "BMP";
	} else { // 上記以外の拡張子の場合、または拡張子がない場合
		format = QImage::imageFormat(filePath); // 実際の形式を取得
		if (format == 0) { // 形式不明の場合
			format = "PPM";
		}
	}

	// 保存実行
	QString errorMessage;
	try {
		editor->saveImage(filePath, format);
	} catch (NoSuchDirectoryException) {
		errorMessage = QDir::convertSeparators(
			QFileInfo(filePath).dir(true).absPath()
		) + tr(" という名前のディレクトリが見つかりません。");
	} catch (OutputException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" の保存に失敗しました。\n"
			"対応できないファイル形式が指定された可能性があります。\n"
			"または、アクセス権がないか、ファイルがロックされています。"
		);
	}

	if (!errorMessage.isNull()) { // エラー発生時
		QMessageBox::warning( // エラーメッセージを出す
			this, tr("画像保存エラー"), errorMessage,
			QMessageBox::Ok | QMessageBox::Default, 0, 0
		);
		return false;
	}

	setTitle( // タイトルバー文字列変更
		QDir::convertSeparators(editor->getFilePath()) + tr(
			" - Image Equalizer"
		)
	);

	return true;
}

// パワースペクトル更新()
void MainWindow::updatePowerSpectrum()
{
	bool isEnabled = showPowerSpectrumAction->isEnabled();
	bool isOn = showPowerSpectrumAction->isOn();

	// 初期状態かつ(無効またはOFF)
	if (powerSpectrum == 0 && (!isEnabled || !isOn)) {
		return;
	}

	if (powerSpectrum == 0 && isEnabled && isOn) { // 初期状態かつ有効かつON
		powerSpectrum = new PowerSpectrumViewer(
			256, 256, this, 0, 
			WType_TopLevel | WStyle_Customize | WStyle_DialogBorder |
			WStyle_Title | WStyle_SysMenu | WStyle_Dialog | WNoAutoErase
		);
		powerSpectrum->setTitle(tr("パワースペクトル"));
		connect(
			powerSpectrum, SIGNAL(visibilityChanged(bool)),
			this, SLOT(powerSpectrumVisibilityChanged(bool))
		);
		connect(
			powerSpectrum, SIGNAL(mouseMoved(int, int, const ieq::String&)),
			this, SLOT(powerSpectrumMouseMoved(int, int, const ieq::String&))
		);
		connect(
			powerSpectrum, SIGNAL(mouseExited()),
			this, SLOT(powerSpectrumMouseExited())
		);
	}

	bool isVisible = powerSpectrum->isVisible();

	if (editor != 0 && isEnabled && isOn) { // 画像が存在かつ有効かつONの場合
		if (!isVisible) { // 見えていない場合
			powerSpectrum->show(); // 表示
		}
		// 表示内容更新
		powerSpectrum->setFrequencyDomain(editor->getFrequencyDomain());
	} else { // 画像が存在しないまたは無効またはOFFの場合
		if (isVisible) { // 見えている場合
			powerSpectrum->close();
		}
		// 見えていない場合は何もしない
	}
}

void MainWindow::setTitle(const QString& title)
{
	ieq::setWindowTitle(this, title);
#if defined(QT_NON_COMMERCIAL) && defined(Q_WS_WIN)
	topData()->caption = title;
#endif
}

