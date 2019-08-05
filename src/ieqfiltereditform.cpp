// クラス名 : フィルタ編集フォーム

#include "ieqfiltereditform.h"

#include <qpushbutton.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qstatusbar.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qdialog.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <math.h>

#ifdef DEBUG
#include <stdexcept>
#include <iostream>
#endif

using namespace ieq;
using namespace boost;
using namespace std;

FilterEditForm::FilterEditForm(
	QWidget* parent, const char* name, bool isModal, WFlags flags
)
	: QDialog(parent, name, isModal, flags)
{
	createParts();   // GUI部品生成
	arrangeParts();  // GUI部品配置
	setFocusChain(); // フォーカスチェーン設定
	connectEvent();  // イベント接続
	setTitle(tr("(新規フィルタ) - フィルタ編集"));
	QString mean;
	mean.sprintf("=  %.2f", editor->getMeanOfRange());
	meanOfRangeLabel->setText(mean); // 編集領域平均係数表示
}

FilterEditForm::~FilterEditForm()
{
	// QtによるGUI部品オブジェクトの自動解放
}

// フィルタ参照()
const Filter& FilterEditForm::getFilter() const
{
	return editor->getFilter();
}

// ファイルパス参照()
const String& FilterEditForm::getFilePath() const
{
	return editor->getFilePath();
}

// 保存確認()
bool FilterEditForm::checkSaving()
{
	if (editor->getSaveCount() == 0) {
		return true;
	}

	// メッセージボックス生成
	QString message("");
	QString filePath = editor->getFilePath();
	if (filePath.isEmpty()) {
		message += tr("(新規フィルタ)");
	} else {
		message += QDir::convertSeparators(filePath);
	}
	message += tr(" は変更されています。保存しますか?");
	QMessageBox box(
		tr("フィルタ編集"), message, QMessageBox::Warning,
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
		saved = saveFilter();
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

// フィルタ読込()
bool FilterEditForm::loadFilter()
{
	QString filePath = QFileDialog::getOpenFileName(
		QString::null, FilterEditForm::getFilterFileTypes(),
		0, 0, tr("フィルタを開く")
	);

	if (filePath.isEmpty()) { // キャンセル選択時
		return false;
	}

	QString errorMessage;
	try {
		editor->loadFilter(filePath); // フィルタ読込の実行
	} catch (NoSuchFileException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" が見つかりません。\n"
			"ファイル名を確認してください。"
		);
	} catch (FileOpenException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" を開けません。\n"
			"ファイルがロックされているか、アクセス権がありません。"
		);
	} catch (InputException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" の読込に失敗しました。"
		);
	} catch (UnknownFormatException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" を開けません。\n"
			"未対応のファイル形式です。"
		);
	} catch (InvalidFormatException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" を開けません。\n"
			"ファイル形式に誤りがあります。"
		);
	}

	if (!errorMessage.isNull()) { // エラー発生時
		QMessageBox::warning( // エラーメッセージを出す
			this, tr("フィルタ読込エラー"), errorMessage,
			QMessageBox::Ok | QMessageBox::Default, 0, 0
		);
		return false;
	}

	meanOfRangeLabel->setText( // 編集領域平均係数表示
		QString().sprintf("=  %.2f", editor->getMeanOfRange())
	);
	setTitle( // タイトルバー文字列変更
		QDir::convertSeparators(editor->getFilePath()) + tr(
			" - フィルタ編集"
		)
	);

	return true;
}

// フィルタ保存()
bool FilterEditForm::saveFilter()
{
	QString filePath = editor->getFilePath();
	if (filePath.isEmpty()) { // (新規フィルタ) の場合
		return saveAsFilter();
	}

	QString errorMessage;
	try {
		editor->saveFilter(filePath); // フィルタ保存の実行
	} catch (NoSuchDirectoryException) {
		return saveAsFilter();
	} catch (FileOpenException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" を開けません。\n"
			"ファイルがロックされているか、アクセス権がありません。"
		);
	} catch (OutputException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" の保存に失敗しました。"
		);
	}

	if (!errorMessage.isNull()) { // エラー発生時
		QMessageBox::warning( // エラーメッセージを出す
			this, tr("フィルタ保存エラー"), errorMessage,
			QMessageBox::Ok | QMessageBox::Default, 0, 0
		);
		return false;
	}

	return true;
}

// フィルタ新規保存()
bool FilterEditForm::saveAsFilter()
{
	QString currentFilePath = editor->getFilePath();
	if (currentFilePath.isEmpty() || !QFile::exists(currentFilePath)) {
		currentFilePath = QString::null;
	}

	QString filePath = QFileDialog::getSaveFileName( // ダイアログオープン
		currentFilePath, FilterEditForm::getFilterFileTypes(),
		0, 0, tr("フィルタの保存")
	);
	if (filePath.isEmpty()) { // キャンセル選択時
		return false;
	}

	QString ext = QFileInfo(filePath).extension(false); // 拡張子取得
	if (ext.isEmpty() || ext != QString("sff")) { // 拡張子がないか違う場合
		filePath += QString(".sff"); // 拡張子を付加
	}

	if (QFile::exists(filePath)) { // 既存のファイルを選択した場合
		// 上書きの確認ダイアログを出す
		QString message = QDir::convertSeparators(filePath) + tr(
			" は既に存在します。\n上書きしますか?"
		);
		QMessageBox box(
			tr("フィルタの保存"), message, QMessageBox::Warning,
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

	QString errorMessage;
	try {
		editor->saveFilter(filePath); // フィルタ保存の実行
	} catch (NoSuchDirectoryException) {
		errorMessage = QDir::convertSeparators(
			QFileInfo(filePath).dir(true).absPath()
		) + tr(" という名前のディレクトリが見つかりません。");
	} catch (FileOpenException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" を開けません。\n"
			"ファイルがロックされているか、アクセス権がありません。"
		);
	} catch (OutputException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" の保存に失敗しました。"
		);
	}

	if (!errorMessage.isNull()) { // エラー発生時
		QMessageBox::warning( // エラーメッセージを出す
			this, tr("フィルタ保存エラー"), errorMessage,
			QMessageBox::Ok | QMessageBox::Default, 0, 0
		);
		return false;
	}

	setTitle( // タイトルバー文字列変更
		QDir::convertSeparators(editor->getFilePath()) + tr(
			" - フィルタ編集"
		)
	);

	return true;
}

// 可視化()
void FilterEditForm::showEvent(QShowEvent* event)
{
	emit visibilityChanged(true);
}

// ダイアログクローズ()
void FilterEditForm::closeEvent(QCloseEvent* event)
{
	event->accept();
	emit visibilityChanged(false);
}

// 編集領域マウス移動()
void FilterEditForm::editorMouseMoved(int xPos, int yPos)
{
	const Filter* filter = &editor->getFilter();
	int filterWidth = filter->getXLength();
	int filterHeight = filter->getYLength();
	int editorWidth = editor->width();
	int editorHeight = editor->height();

	// 4象限座標計算
	int shiftedX = xPos - (editorWidth / 2 - 1);
	// int shiftedY = 1 - (yPos - (editorHeight / 2 - 1));
	int shiftedY = yPos - (editorHeight / 2 - 1);
	// フィルタサイズと表示サイズの差を加味
	if (shiftedX >= 0) {
		shiftedX = shiftedX * filterWidth / editorWidth;
	} else {
		shiftedX = (shiftedX - 1) * filterWidth / editorWidth + 1;
	}
	if (shiftedY >= 0) {
		shiftedY = shiftedY * filterHeight / editorHeight;
	} else {
		shiftedY = (shiftedY - 1) * filterHeight / editorHeight + 1;
	}

	// 論理座標計算
	double logicalX = (double)shiftedX / filterWidth;
	double logicalY = (double)shiftedY / filterHeight;

	// 周波数・偏角計算
	double frequency = sqrt(logicalX * logicalX + logicalY * logicalY);
	double angle;
	if (logicalX != 0.0 || logicalY != 0.0) {
		angle = atan2(logicalY, logicalX) * 180.0 / ieq::PI;
	} else {
		angle = 0.0;
	}

	// フィルタ座標計算
	int filterX = (shiftedX + filterWidth) % filterWidth;
	int filterY = (shiftedY + filterHeight) % filterHeight;

	// 係数取得
	double coefficient = filter->getCoefficient(filterX, filterY).real();

	// ステータスラベル文字列設定
	QString message("");
	QString formatter;
	message += tr("周波数: ");
	message += formatter.sprintf("%.3f", frequency);
	message += tr(" r/pix   ");
	message += tr("偏角: ");
	message += formatter.sprintf("%.1f", angle);
	message += tr(" 度   ");
	message += tr("係数: ");
	message += formatter.sprintf("%.2f", coefficient);
	statusLabel->setText(message);
}

// 編集領域マウスクリック()
void FilterEditForm::editorMousePressed(int xPos, int yPos)
{
	const Filter* filter = &editor->getFilter();
	int filterWidth = filter->getXLength();
	int filterHeight = filter->getYLength();
	int editorWidth = editor->width();
	int editorHeight = editor->height();

	// 4象限座標計算
	int shiftedX = xPos - (editorWidth / 2 - 1);
	// int shiftedY = 1 - (yPos - (editorHeight / 2 - 1));
	int shiftedY = yPos - (editorHeight / 2 - 1);
	// フィルタサイズと表示サイズの差を加味
	if (shiftedX >= 0) {
		shiftedX = shiftedX * filterWidth / editorWidth;
	} else {
		shiftedX = (shiftedX - 1) * filterWidth / editorWidth + 1;
	}
	if (shiftedY >= 0) {
		shiftedY = shiftedY * filterHeight / editorHeight;
	} else {
		shiftedY = (shiftedY - 1) * filterHeight / editorHeight + 1;
	}

	// 論理座標計算
	double logicalX = (double)shiftedX / filterWidth;
	double logicalY = (double)shiftedY / filterHeight;

	// 周波数・偏角計算
	double frequency = sqrt(logicalX * logicalX + logicalY * logicalY);
	double angle;
	if (logicalX != 0.0 || logicalY != 0.0) {
		angle = atan2(logicalY, logicalX) * 180.0 / ieq::PI;
		if (angle < 0) {
			angle += 180.0;
		}
	} else {
		angle = 0.0;
	}

	// スピンボックス値設定
	startFrequencyBox->setValue((int)(floor(frequency * 1000)));
	endFrequencyBox->setValue((int)(ceil(frequency * 1000)));
	startAngleBox->setValue((int)(floor(angle * 10)));
	endAngleBox->setValue((int)(ceil(angle * 10)));
}

// 編集領域マウスアウト()
void FilterEditForm::editorMouseExited()
{
	statusLabel->setText("");
}

// 新規作成ボタンクリック()
void FilterEditForm::createButtonClicked()
{
	if (!checkSaving()) {
		return;
	}

	editor->initialize(); // 初期化
	meanOfRangeLabel->setText( // 編集領域平均係数表示
		QString().sprintf("=  %.2f", editor->getMeanOfRange())
	);
	setTitle(tr("(新規フィルタ) - フィルタ編集")); // タイトルバー文字列変更
}

// 読込ボタンクリック()
void FilterEditForm::loadButtonClicked()
{
	if (!checkSaving()) {
		return;
	}

	loadFilter(); // フィルタ読込
}

// 上書き保存ボタンクリック()
void FilterEditForm::saveButtonClicked()
{
	saveFilter();
}

// 新規保存ボタンクリック()
void FilterEditForm::saveAsButtonClicked()
{
	saveAsFilter();
}

// 増ボタンクリック()
void FilterEditForm::increaseButtonClicked()
{
	editor->vary(varianceBox->value() / 100.0);
	QString mean;
	mean.sprintf("=  %.2f", editor->getMeanOfRange());
	meanOfRangeLabel->setText(mean);
}

// 減ボタンクリック()
void FilterEditForm::decreaseButtonClicked()
{
	editor->vary(-(varianceBox->value() / 100.0));
	QString mean;
	mean.sprintf("=  %.2f", editor->getMeanOfRange());
	meanOfRangeLabel->setText(mean);
}

// 変位ボックス値変更()
void FilterEditForm::varianceBoxChanged()
{
	int value = varianceBox->value();

	// スピンボックスとスライダの同期を取る
	if (200 - value != varianceSlider->value()) {
		varianceSlider->setValue(200 - value);
	}
}

// 変位スライダ値変更()
void FilterEditForm::varianceSliderChanged()
{
	int value = varianceSlider->value();

	// スライダとスピンボックスの同期を取る
	if (200 - value != varianceBox->value()) {
		varianceBox->setValue(200 - value);
	}
}

// 開始周波数ボックス値変更()
void FilterEditForm::startFrequencyBoxChanged()
{
	int value = startFrequencyBox->value();

	// スピンボックスとスライダの同期を取る
	if (value != startFrequencySlider->value()) {
		startFrequencySlider->setValue(value);
	}

	// 開始と終了の調整をする
	if (value > endFrequencyBox->value()) {
		endFrequencyBox->setValue(value);
		return; // 二重設定防止用
	}

	editor->setFrequencyRange(value/1000.0, endFrequencyBox->value()/1000.0);

	meanOfRangeLabel->setText(
		QString().sprintf("=  %.2f", editor->getMeanOfRange())
	);
}

// 終了周波数ボックス値変更()
void FilterEditForm::endFrequencyBoxChanged()
{
	int value = endFrequencyBox->value();

	// スピンボックスとスライダの同期を取る
	if (value != endFrequencySlider->value()) {
		endFrequencySlider->setValue(value);
	}

	// 開始と終了の調整をする
	if (value < startFrequencyBox->value()) {
		startFrequencyBox->setValue(value);
		return;
	}

	editor->setFrequencyRange(startFrequencyBox->value()/1000.0, value/1000.0);

	meanOfRangeLabel->setText(
		QString().sprintf("=  %.2f", editor->getMeanOfRange())
	);
}

// 開始角ボックス値変更()
void FilterEditForm::startAngleBoxChanged()
{
	int value = startAngleBox->value();

	// スピンボックスとスライダの同期を取る
	if (value != startAngleSlider->value()) {
		startAngleSlider->setValue(value);
	}

	// 開始と終了の調整をする
	if (value > endAngleBox->value()) {
		endAngleBox->setValue(value);
		return;
	}

	editor->setAngleRange(value / 10.0, endAngleBox->value() / 10.0);

	meanOfRangeLabel->setText(
		QString().sprintf("=  %.2f", editor->getMeanOfRange())
	);
}

// 終了角ボックス値変更()
void FilterEditForm::endAngleBoxChanged()
{
	int value = endAngleBox->value();

	// スピンボックスとスライダの同期を取る
	if (value != endAngleSlider->value()) {
		endAngleSlider->setValue(value);
	}

	// 開始と終了の調整をする
	if (value < startAngleBox->value()) {
		startAngleBox->setValue(value);
		return;
	}

	editor->setAngleRange(startAngleBox->value() / 10.0, value / 10.0);

	meanOfRangeLabel->setText(
		QString().sprintf("=  %.2f", editor->getMeanOfRange())
	);
}

// 色付け選択ボックストグル()
void FilterEditForm::coloringSelectorToggled(bool isOn)
{
	editor->setColoring(isOn);
}

// GUI部品生成()
void FilterEditForm::createParts()
{
	editor = new FilterEditor(256, 256, this);

	frequencyGroup = new QGroupBox(tr("周波数帯"), this);
	angleGroup = new QGroupBox(tr("方向帯"), this);
	varianceGroup = new QGroupBox(tr("係数増減"), this);
	meanGroup = new QGroupBox(tr("編集領域の平均係数"), this);

	createButton = new QPushButton(tr("新規作成(&N)"), this);
	loadButton = new QPushButton(tr("ファイル読込(&O)"), this);
	saveButton = new QPushButton(tr("上書き保存(&S)"), this);
	saveAsButton = new QPushButton(tr("新規保存(&A)"), this);

	applyButton = new QPushButton(tr("フィルタ適用(&F)"), this);
	registerButton = new QPushButton(tr("フィルタ登録(&R)"), this);
	registerButton->setEnabled(false);

	// varyButton = new QPushButton(tr("実  行"), varianceGroup);
	increaseButton = new QPushButton(tr("増(&+)"), varianceGroup);
	decreaseButton = new QPushButton(tr("減(&-)"), varianceGroup);

	varianceSlider = new QSlider(
		0, 200, 20, 100, QSlider::Vertical, varianceGroup
	);

	startFrequencySlider = new QSlider(
		0, 708, 100, 0, QSlider::Horizontal, frequencyGroup
	);
	endFrequencySlider = new QSlider(
		0, 708, 100, 0, QSlider::Horizontal, frequencyGroup
	);
	startAngleSlider = new QSlider(
		0, 1800, 150, 0, QSlider::Horizontal, angleGroup
	);
	endAngleSlider = new QSlider(
		0, 1800, 150, 1800, QSlider::Horizontal, angleGroup
	);

	varianceBox = new RationalSpinBox(3, 2, 0, 200, 100, 1, varianceGroup);

	startFrequencyBox = new RationalSpinBox(3, 3, 0, 708, 0, 1, frequencyGroup);
	endFrequencyBox = new RationalSpinBox(3, 3, 0, 708, 0, 1, frequencyGroup);
	startAngleBox = new RationalSpinBox(4, 1, 0, 1800, 0, 1, angleGroup);
	endAngleBox = new RationalSpinBox(4, 1, 0, 1800, 1800, 1, angleGroup);

	coloringSelector = new QCheckBox(tr("編集領域色付け"), this);
	coloringSelector->setChecked(true);

	meanOfRangeLabel = new QLabel(meanGroup);

	statusLabel = new QLabel(this);
	statusBar = new QStatusBar(this);
	statusBar->setSizeGripEnabled(false);
	statusBar->addWidget(statusLabel, 1);
}

// GUI部品配置()
void FilterEditForm::arrangeParts()
{
	// 編集ウィジェットの配置
	editor->move(4, 4);

	// メニューボタンの配置
	const int fileMenuWidth = 94;
	const int fileMenuHeight = 24;
	const int fileMenuLeft = 270;
	// 30ずつ下げる
	createButton->setGeometry(fileMenuLeft, 17, fileMenuWidth, fileMenuHeight);
	loadButton->setGeometry(fileMenuLeft, 47, fileMenuWidth, fileMenuHeight);
	saveButton->setGeometry(fileMenuLeft, 77, fileMenuWidth, fileMenuHeight);
	saveAsButton->setGeometry(fileMenuLeft, 107, fileMenuWidth,fileMenuHeight);

	applyButton->setGeometry(fileMenuLeft, 177, fileMenuWidth, fileMenuHeight);
	registerButton->setGeometry(fileMenuLeft,207,fileMenuWidth,fileMenuHeight);

	// 周波数帯指定部品の配置
	frequencyGroup->setGeometry(3, 272, 236, 79);
	startFrequencySlider->setTickInterval(100);
	startFrequencySlider->setTickmarks(QSlider::Below);
	startFrequencySlider->setGeometry(12, 20, 92, 25);
	startFrequencyBox->setGeometry(110, 20, 56, 22);
	QLabel* startFrequencyLabel = new QLabel(tr("r/pix  から"), frequencyGroup);
	startFrequencyLabel->setGeometry(172, 20, 55, 22);
	endFrequencySlider->setTickInterval(100);
	endFrequencySlider->setTickmarks(QSlider::Above);
	endFrequencySlider->setGeometry(12, 45, 92, 25);
	endFrequencyBox->setGeometry(110, 48, 56, 22);
	QLabel* endFrequencyLabel = new QLabel(tr("r/pix  まで"), frequencyGroup);
	endFrequencyLabel->setGeometry(172, 48, 55, 22);

	// 方向帯指定部品の配置
	angleGroup->setGeometry(3, 357, 236, 79);
	startAngleSlider->setTickInterval(150);
	startAngleSlider->setTickmarks(QSlider::Below);
	startAngleSlider->setGeometry(12, 20, 92, 25);
	startAngleBox->setGeometry(110, 20, 56, 22);
	QLabel* startAngleLabel = new QLabel(tr(" 度    から"), angleGroup);
	startAngleLabel->setGeometry(172, 20, 55, 22);
	endAngleSlider->setTickInterval(150);
	endAngleSlider->setTickmarks(QSlider::Above);
	endAngleSlider->setGeometry(12, 45, 92, 25);
	endAngleBox->setGeometry(110, 48, 56, 22);
	QLabel* endAngleLabel = new QLabel(tr(" 度    まで"), angleGroup);
	endAngleLabel->setGeometry(172, 48, 55, 22);

	// 係数増減部品の配置
	varianceGroup->setGeometry(244, 272, 125, 135);
	varianceSlider->setTickInterval(20);
	varianceSlider->setTickmarks(QSlider::Both);
	varianceSlider->setGeometry(12, 21, 30, 72);
	varianceBox->setGeometry(54, 46, 56, 22);
	QLabel* varianceLabel = new QLabel(tr("増減幅"), varianceGroup);
	varianceLabel->setGeometry(58, 22, 60, 22);
	varianceLabel = new QLabel(tr("ずつ"), varianceGroup);
	varianceLabel->setGeometry(72, 74, 45, 22);
	// varyButton->setGeometry(27, 103, 76, 24);
	increaseButton->setGeometry(12, 103, 47, 24);
	decreaseButton->setGeometry(66, 103, 47, 24);

	// 平均係数部品の配置
	meanGroup->setGeometry(244, 413, 125, 43);
	meanOfRangeLabel->setGeometry(41, 16, 60, 22);

	// 色付け選択チェックボックスの配置
	coloringSelector->setGeometry(9, 439, 200, 22);

	// ステータスバーの配置
	statusBar->setGeometry(0, 459, 372, 25);
}

// フォーカスチェーン設定()
void FilterEditForm::setFocusChain()
{
	// タブ移動とクリックによるフォーカス移動を設定
	createButton->setFocusPolicy(QWidget::StrongFocus);
	loadButton->setFocusPolicy(QWidget::StrongFocus);
	saveButton->setFocusPolicy(QWidget::StrongFocus);
	saveAsButton->setFocusPolicy(QWidget::StrongFocus);
	applyButton->setFocusPolicy(QWidget::StrongFocus);
	registerButton->setFocusPolicy(QWidget::StrongFocus);
	// varyButton->setFocusPolicy(QWidget::StrongFocus);
	increaseButton->setFocusPolicy(QWidget::StrongFocus);
	decreaseButton->setFocusPolicy(QWidget::StrongFocus);
	varianceSlider->setFocusPolicy(QWidget::StrongFocus);
	startFrequencySlider->setFocusPolicy(QWidget::StrongFocus);
	endFrequencySlider->setFocusPolicy(QWidget::StrongFocus);
	startAngleSlider->setFocusPolicy(QWidget::StrongFocus);
	endAngleSlider->setFocusPolicy(QWidget::StrongFocus);
	varianceBox->setFocusPolicy(QWidget::StrongFocus);
	startFrequencyBox->setFocusPolicy(QWidget::StrongFocus);
	endFrequencyBox->setFocusPolicy(QWidget::StrongFocus);
	startAngleBox->setFocusPolicy(QWidget::StrongFocus);
	endAngleBox->setFocusPolicy(QWidget::StrongFocus);
	coloringSelector->setFocusPolicy(QWidget::StrongFocus);

	// 自動デフォルトプッシュボタン設定
	createButton->setAutoDefault(true);
	loadButton->setAutoDefault(true);
	saveButton->setAutoDefault(true);
	saveAsButton->setAutoDefault(true);
	applyButton->setAutoDefault(true);
	registerButton->setAutoDefault(true);
	// varyButton->setAutoDefault(true);
	increaseButton->setAutoDefault(true);
	decreaseButton->setAutoDefault(true);

	// タブ移動順設定
	QWidget::setTabOrder(createButton, loadButton);
	QWidget::setTabOrder(loadButton, saveButton);
	QWidget::setTabOrder(saveButton, saveAsButton);
	QWidget::setTabOrder(saveAsButton, applyButton);
	QWidget::setTabOrder(applyButton, registerButton);
	QWidget::setTabOrder(registerButton, startFrequencySlider);
	QWidget::setTabOrder(startFrequencySlider, startFrequencyBox);
	QWidget::setTabOrder(startFrequencyBox, endFrequencySlider);
	QWidget::setTabOrder(endFrequencySlider, endFrequencyBox);
	QWidget::setTabOrder(endFrequencyBox, startAngleSlider);
	QWidget::setTabOrder(startAngleSlider, startAngleBox);
	QWidget::setTabOrder(startAngleBox, endAngleSlider);
	QWidget::setTabOrder(endAngleSlider, endAngleBox);
	QWidget::setTabOrder(endAngleBox, varianceSlider);
	QWidget::setTabOrder(varianceSlider, varianceBox);
	// QWidget::setTabOrder(varianceBox, varyButton);
	// QWidget::setTabOrder(varyButton, coloringSelector);
	QWidget::setTabOrder(varianceBox, increaseButton);
	QWidget::setTabOrder(increaseButton, decreaseButton);
	QWidget::setTabOrder(decreaseButton, coloringSelector);
	// QWidget::setTabOrder(coloringSelector, createButton); // これはNG
}

// イベント接続()
void FilterEditForm::connectEvent()
{
	connect(
		editor, SIGNAL(mouseMoved(int, int)),
		this, SLOT(editorMouseMoved(int, int))
	);
	connect(
		editor, SIGNAL(mousePressed(int, int)),
		this, SLOT(editorMousePressed(int, int))
	);
	connect(
		editor, SIGNAL(mouseExited()),
		this, SLOT(editorMouseExited())
	);
	connect(
		createButton, SIGNAL(clicked()),
		this, SLOT(createButtonClicked())
	);
	connect(
		loadButton, SIGNAL(clicked()),
		this, SLOT(loadButtonClicked())
	);
	connect(
		saveButton, SIGNAL(clicked()),
		this, SLOT(saveButtonClicked())
	);
	connect(
		saveAsButton, SIGNAL(clicked()),
		this, SLOT(saveAsButtonClicked())
	);
	connect(
		applyButton, SIGNAL(clicked()),
		this, SIGNAL(applyButtonClicked())
	);
	connect(
		registerButton, SIGNAL(clicked()),
		this, SIGNAL(registerButtonClicked())
	);
	// connect(
	// 	varyButton, SIGNAL(clicked()),
	// 	this, SLOT(varyButtonClicked())
	// );
	connect(
		increaseButton, SIGNAL(clicked()),
		this, SLOT(increaseButtonClicked())
	);
	connect(
		decreaseButton, SIGNAL(clicked()),
		this, SLOT(decreaseButtonClicked())
	);
	connect(
		varianceBox, SIGNAL(valueChanged(int)),
		this, SLOT(varianceBoxChanged())
	);
	connect(
		varianceSlider, SIGNAL(valueChanged(int)),
		this, SLOT(varianceSliderChanged())
	);
	connect(
		startFrequencyBox, SIGNAL(valueChanged(int)),
		this, SLOT(startFrequencyBoxChanged())
	);
	connect(
		startFrequencySlider, SIGNAL(valueChanged(int)),
		startFrequencyBox, SLOT(setValue(int))
	);
	connect(
		endFrequencyBox, SIGNAL(valueChanged(int)),
		this, SLOT(endFrequencyBoxChanged())
	);
	connect(
		endFrequencySlider, SIGNAL(valueChanged(int)),
		endFrequencyBox, SLOT(setValue(int))
	);
	connect(
		startAngleBox, SIGNAL(valueChanged(int)),
		this, SLOT(startAngleBoxChanged())
	);
	connect(
		startAngleSlider, SIGNAL(valueChanged(int)),
		startAngleBox, SLOT(setValue(int))
	);
	connect(
		endAngleBox, SIGNAL(valueChanged(int)),
		this, SLOT(endAngleBoxChanged())
	);
	connect(
		endAngleSlider, SIGNAL(valueChanged(int)),
		endAngleBox, SLOT(setValue(int))
	);
	connect(
		coloringSelector, SIGNAL(toggled(bool)),
		this, SLOT(coloringSelectorToggled(bool))
	);
}

void FilterEditForm::setTitle(const QString& title)
{
	ieq::setWindowTitle(this, title);
#if defined(QT_NON_COMMERCIAL) && defined(Q_WS_WIN)
	topData()->caption = title;
#endif
}


// テストドライバ
#ifdef UNIT_TEST
#include <qapplication.h>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	QTextCodec* codec = QTextCodec::codecForName("Shift-JIS");
	QTextCodec::setCodecForTr(codec);

	FilterEditForm form;
	app.setMainWidget(&form);
	form.show();

	return app.exec();
}
#endif
