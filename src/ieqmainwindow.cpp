// �N���X�� : ���C���E�B���h�E

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
	loadSettings();         // �ݒ�Ǎ�
	createActions();        // �A�N�V��������
	createMenuBar();        // ���j���[�o�[����
	createToolBar();        // �c�[���o�[����
	createCentralWidget();  // �����̈搶��
	createStatusBar();      // �X�e�[�^�X�o�[����
	setTitle(tr("Image Equalizer"));
	setGeometry(windowLeft, windowTop, windowWidth, windowHeight);
}

MainWindow::~MainWindow()
{
	// Qt�ɂ��GUI���i�I�u�W�F�N�g�̎������
}

// �E�B���h�E�N���[�Y����()
void MainWindow::closeEvent(QCloseEvent* event)
{
	// �t�B���^�ۑ��m�F
	if (filterForm != 0 && !filterForm->checkSaving()) {
		event->ignore();
		return;
	}

	// �摜�ۑ��m�F
	if (!checkSaving()) {
		event->ignore();
		return;
	}

	saveSettings();  // �ݒ�ۑ�
	event->accept();
}

// �u�J���v�A�N�V�������s()
void MainWindow::openActivated()
{
	if (!checkSaving()) {
		return;
	}

	loadImage(); // �摜�̓Ǎ�
}

// �u����v�A�N�V�������s()
void MainWindow::closeActivated()
{
	if (editor == 0) {
		return;
	}
	if (!checkSaving()) {
		return;
	}

	delete editor; // �摜�ҏW�̈��������ăX�N���[���̈悩�珜��
	editor = 0;

	closeAction->setEnabled(false);
	saveAction->setEnabled(false);
	saveAsAction->setEnabled(false);
	showPowerSpectrumAction->setEnabled(false);

	resolutionLabel->clear(); // ��f���\�����N���A
	setTitle(tr("Image Equalizer"));
	updatePowerSpectrum(); // �p���[�X�y�N�g���X�V
}

// �u�㏑���ۑ��v�A�N�V�������s()
void MainWindow::saveActivated()
{
	saveImage();
}

// �u���O��t���ĕۑ��v�A�N�V�������s()
void MainWindow::saveAsActivated()
{
	saveAsImage();
}

// �u���ɖ߂��v�A�N�V�������s()
void MainWindow::undoActivated()
{
	if (editor == 0 || !editor->isUndoingEnabled()) {
		return;
	}

	editor->undo();

	if (!editor->isUndoingEnabled()) { // ����ȏ�O�̏�Ԃɖ߂��Ȃ��Ȃ�����
		undoAction->setEnabled(false);
	}

	updatePowerSpectrum();
}

// �u�c�[���o�[�v�\���A�N�V�����g�O��()
void MainWindow::showToolBarToggled(bool isOn)
{
	// �\���E��\���ؑ�
	if (isOn && toolBar->isHidden()) {
		toolBar->show();
	} else if (!isOn && toolBar->isVisible()) {
		toolBar->hide();
	}
}

// �u�X�e�[�^�X�o�[�v�\���A�N�V�����g�O��()
void MainWindow::showStatusBarToggled(bool isOn)
{
	// �\���E��\���ؑ�
	if (isOn && statusBar()->isHidden()) {
		statusBar()->show();
	} else if (!isOn && statusBar()->isVisible()) {
		statusBar()->hide();
	}
}

// �u�T�C�h�o�[�v�\���A�N�V�����g�O��()
// �񎟊J���p
void MainWindow::showSideBarToggled(bool isOn)
{
}

// �u�p���[�X�y�N�g���v�\���A�N�V�����g�O��()
void MainWindow::showPowerSpectrumToggled(bool isOn)
{
	updatePowerSpectrum();
}

// �u�t�B���^�ҏW�v�A�N�V�����g�O��()
void MainWindow::editFilterToggled(bool isOn)
{
	if (isOn && filterForm == 0) { // ���񏈗�
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

// �u�t�B���^�Ǘ��v�A�N�V�������s()
// �񎟊J���p
void MainWindow::manageFilterActivated()
{
}

// �u�o�[�W�������v�A�N�V�������s()
void MainWindow::aboutActivated()
{
	// QMessageBox::about();
	QMessageBox box(this);

	box.setCaption(tr("Image Equalizer �̃o�[�W�������"));
	box.setTextFormat(RichText);
	box.setText(
		tr(
			"<h2>Image Equalizer v0.10<br>"
			"<small> - �摜�̎��g���t�B���^�����O�\�t�g</small></h2>"

			"<p>Copyright &copy; 2005 Niekawa &lt;schole@nifty.com&gt;</p>"

			"<p>�{�v���O�����̓t���[�E�\�t�g�E�F�A�ł��B"
			"���Ȃ��́AFree Software Foundation�����\����"
			"GNU ��ʌ��L�g�p�����́u�o�[�W����2�v������"
			"����ȍ~�̊e�o�[�W�����̒����炢���ꂩ��I�����A"
			"���̃o�[�W��������߂�����ɏ]���Ė{�v���O������"
			"�ĔЕz�܂��͕ύX���邱�Ƃ��ł��܂��B</p>"

			"<p>�{�v���O�����͗L�p�Ƃ͎v���܂����A"
			"�Еz�ɂ������ẮA�s�ꐫ�y�ѓ���ړI�K�����ɂ��Ă�"
			"�Öق̕ۏ؂��܂߂āA�����Ȃ�ۏ؂��s�Ȃ��܂���B"
			"�ڍׂɂ��Ă�GNU ��ʌ��L�g�p�����������ǂ݂��������B</p>"

			"<p>���Ȃ��́A�{�v���O�����ƈꏏ��GNU ��ʌ��L�g�p������"
			"�ʂ����󂯎���Ă���͂��ł��B�����łȂ��ꍇ�́A"
			"Free Software Foundation, Inc., 675 Mass Ave, Cambridge, "
			"MA 02139, USA �֎莆�������Ă��������B</p>"
		)
	);
	box.setIconPixmap(QPixmap::fromMimeSource("logo.xpm"));
	box.exec();
}

// �񎟊J���p
// void MainWindow::filterEntryListBoxOnItem(QListBoxItem* item)
// {
// }

// �񎟊J���p
// void MainWindow::filterEntryListBoxClicked(QListBoxItem* item)
// {
// }

// �摜�ҏW�̈�}�E�X�ړ�()
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
	posInfo += tr("�P�x: ");
	posInfo += formatter.sprintf("%d", editor->getLuminosity(xPos, yPos));
	statusLabel->setText(posInfo);
}

// �摜�ҏW�̈�}�E�X�A�E�g()
void MainWindow::editorMouseExited()
{
	statusLabel->clear(); // �X�e�[�^�X���x�����N���A
}

// �t�B���^�K�p�{�^���N���b�N()
void MainWindow::filterApplyButtonClicked()
{
	if (filterForm == 0) {
		return;
	}

	// �摜���ǂݍ��܂�Ă��Ȃ��ꍇ�͓Ǎ�
	if (editor == 0) {
		if (!loadImage()) {
			return;
		}
	}

	// �t�B���^�K�p
	try {
		editor->applyFilter(filterForm->getFilter());
	} catch (bad_alloc) {
		QMessageBox::critical( // �G���[���b�Z�[�W���o��
			this, tr("�G���["),
			tr("���������s�����Ă��邽�߁A�����ł��܂���B"),
			QMessageBox::Ok | QMessageBox::Default, 0, 0
		);
		return;
	}

	if (editor->isUndoingEnabled()) { // ���ɖ߂���ꍇ
		undoAction->setEnabled(true);
	}

	updatePowerSpectrum(); // �p���[�X�y�N�g���X�V
}

// �t�B���^�o�^�{�^���N���b�N()
// �񎟊J���p
void MainWindow::filterRegisterButtonClicked()
{
}

// �t�B���^�t�H�[�������ύX()
void MainWindow::filterFormVisibilityChanged(bool isVisible)
{
	// �u�t�B���^�ҏW�v�A�N�V�����̃I���E�I�t�ؑ�
	if (isVisible && !editFilterAction->isOn()) {
		editFilterAction->setOn(true);
	} else if (!isVisible && editFilterAction->isOn()) {
		editFilterAction->setOn(false);
	}
}

// �p���[�X�y�N�g�������ύX()
void MainWindow::powerSpectrumVisibilityChanged(bool isVisible)
{
	if (showPowerSpectrumAction->isEnabled()) {
		showPowerSpectrumAction->setOn(isVisible);
	}
}

// �p���[�X�y�N�g���}�E�X�ړ�()
void MainWindow::powerSpectrumMouseMoved(
	int xPos, int yPos, const ieq::String& message
)
{
	if (statusBar()->isVisible()) {
		statusLabel->setText(message);
	}
}

// �p���[�X�y�N�g���}�E�X�A�E�g()
void MainWindow::powerSpectrumMouseExited()
{
	statusLabel->clear();
}

// �񎟊J���p
// void MainWindow::filterSelectionActivated(int index)
// {
// }

// �ݒ�Ǎ�()
void MainWindow::loadSettings()
{
	// �f�t�H���g�ݒ�
	entryFilePath = QString::null;
	lastImageDir = QString::null;
	lastFilterDir = QString::null;
	supportingImageTypes = tr(
		"�|�[�^�u�� �O���[�}�b�v (*.pgm);;Windows �r�b�g�}�b�v (*.bmp)"
	);
	suppottingFilterTypes = tr("��Ԏ��g���̈�t�B���^ (*.sff)");
	undoingLimit = 8;
	windowWidth = 640;
	windowHeight = 480;

    QWidget* desktop = QApplication::desktop(); // �f�X�N�g�b�v���̎擾
    windowLeft = (desktop->width() - windowWidth) / 2;
    windowTop = (desktop->height() - windowHeight) / 2 + 30;
}

// �ݒ�ۑ�()
void MainWindow::saveSettings()
{
}

// �A�N�V��������()
void MainWindow::createActions()
{
	// �u�J���v�A�N�V����
	openAction = new QAction(tr("�J��(&O)..."), tr("Ctrl+O"), this);
	openAction->setIconSet(QPixmap::fromMimeSource("open.xpm"));
	openAction->setToolTip(tr("�J�� (Ctrl+O)"));
	openAction->setStatusTip(tr("�摜�t�@�C�����J���܂��B"));
	connect(openAction, SIGNAL(activated()), this, SLOT(openActivated()));

	// �u����v�A�N�V����
	closeAction = new QAction(tr("����(&C)"), 0, this);
	closeAction->setToolTip(tr("����"));
	closeAction->setStatusTip(tr("�ҏW���̉摜����܂��B"));
	closeAction->setEnabled(false);
	connect(closeAction, SIGNAL(activated()), this, SLOT(closeActivated()));

	// �u�㏑���ۑ��v�A�N�V����
	saveAction = new QAction(tr("�㏑���ۑ�(&S)"), tr("Ctrl+S"), this);
	saveAction->setIconSet(QPixmap::fromMimeSource("save.xpm"));
	saveAction->setToolTip(tr("�㏑���ۑ� (Ctrl+S)"));
	saveAction->setStatusTip(tr("�ҏW���̉摜��ۑ����܂��B"));
	saveAction->setEnabled(false);
	connect(saveAction, SIGNAL(activated()), this, SLOT(saveActivated()));

	// �u���O��t���ĕۑ��v�A�N�V����
	saveAsAction = new QAction(tr("���O��t���ĕۑ�(&A)..."), 0, this);
	saveAsAction->setToolTip(tr("���O��t���ĕۑ�"));
	saveAsAction->setStatusTip(tr("�ҏW���̉摜��ʂ̃t�@�C���֕ۑ����܂��B"));
	saveAsAction->setEnabled(false);
	connect(saveAsAction, SIGNAL(activated()), this, SLOT(saveAsActivated()));

	// �u�I���v�A�N�V����
	quitAction = new QAction(tr("�I��(&X)"), 0, this);
	quitAction->setToolTip(tr("�I��"));
	quitAction->setStatusTip(tr("Image Equalizer ���I�����܂��B"));
	connect(quitAction, SIGNAL(activated()), this, SLOT(close()));

	// �u���ɖ߂��v�A�N�V����
	undoAction = new QAction(tr("���ɖ߂�(&U)"), tr("Ctrl+Z"), this);
	undoAction->setIconSet(QPixmap::fromMimeSource("undo.xpm"));
	undoAction->setToolTip(tr("���ɖ߂� (Ctrl+Z)"));
	undoAction->setStatusTip(tr("���O�̑�������ɖ߂��܂��B"));
	undoAction->setEnabled(false);
	connect(undoAction, SIGNAL(activated()), this, SLOT(undoActivated()));

	// �u�c�[���o�[�v�\���A�N�V����
	showToolBarAction = new QAction(tr("�c�[���o�[(&T)"), 0, this);
	showToolBarAction->setToggleAction(true);
	showToolBarAction->setToolTip(tr("�c�[���o�[�̕\��/��\��"));
	showToolBarAction->setStatusTip(
		tr("�c�[���o�[�̕\��/��\����؂�ւ��܂��B")
	);
	showToolBarAction->setOn(true);
	connect(
		showToolBarAction, SIGNAL(toggled(bool)),
		this, SLOT(showToolBarToggled(bool))
	);

	// �u�T�C�h�o�[�v�\���A�N�V����
	showSideBarAction = new QAction(tr("�T�C�h�o�[(&F)"), 0, this);
	showSideBarAction->setToggleAction(true);
	showSideBarAction->setToolTip(tr("�T�C�h�o�[�̕\��/��\��"));
	showSideBarAction->setStatusTip(
		tr("�T�C�h�o�[�̕\��/��\����؂�ւ��܂��B")
	);
	showSideBarAction->setOn(false);
	showSideBarAction->setEnabled(false);
	connect(
		showSideBarAction, SIGNAL(toggled(bool)),
		this, SLOT(showSideBarToggled(bool))
	);

	// �u�X�e�[�^�X �o�[�v�\���A�N�V����
	showStatusBarAction = new QAction(tr("�X�e�[�^�X �o�[(&I)"), 0, this);
	showStatusBarAction->setToggleAction(true);
	showStatusBarAction->setToolTip(tr("�X�e�[�^�X �o�[�̕\��/��\��"));
	showStatusBarAction->setStatusTip(
		tr("�X�e�[�^�X �o�[�̕\��/��\����؂�ւ��܂��B")
	);
	showStatusBarAction->setOn(true);
	connect(
		showStatusBarAction, SIGNAL(toggled(bool)),
		this, SLOT(showStatusBarToggled(bool))
	);

	// �u�p���[�X�y�N�g���v�\���A�N�V����
	showPowerSpectrumAction = new QAction(tr("�p���[�X�y�N�g��(&P)"), 0, this);
	showPowerSpectrumAction->setIconSet(QPixmap::fromMimeSource("power.xpm"));
	showPowerSpectrumAction->setToggleAction(true);
	showPowerSpectrumAction->setOn(false);
	showPowerSpectrumAction->setToolTip(tr("�p���[�X�y�N�g���̕\��/��\��"));
	showPowerSpectrumAction->setStatusTip(
		tr("�p���[�X�y�N�g���̕\��/��\����؂�ւ��܂��B")
	);
	showPowerSpectrumAction->setEnabled(false);
	connect(
		showPowerSpectrumAction, SIGNAL(toggled(bool)),
		this, SLOT(showPowerSpectrumToggled(bool))
	);

	// �u�t�B���^�ҏW�v�A�N�V����
	editFilterAction = new QAction(tr("�t�B���^�ҏW(&E)"), 0, this);
	editFilterAction->setIconSet(QPixmap::fromMimeSource("filter.xpm"));
	editFilterAction->setToggleAction(true);
	editFilterAction->setToolTip(tr("�t�B���^�ҏW�t�H�[���̕\��/��\��"));
	editFilterAction->setStatusTip(
		tr("�t�B���^�ҏW�t�H�[���̕\��/��\����؂�ւ��܂��B")
	);
	editFilterAction->setOn(false);
	connect(
		editFilterAction, SIGNAL(toggled(bool)),
		this, SLOT(editFilterToggled(bool))
	);

	// �u�t�B���^�Ǘ��v�A�N�V����
	manageFilterAction = new QAction(tr("�t�B���^�Ǘ�(&M)..."), 0, this);
	manageFilterAction->setToolTip(tr("�t�B���^�Ǘ�"));
	manageFilterAction->setStatusTip(tr("�t�B���^�̓o�^�����Ǘ����܂��B"));
	manageFilterAction->setEnabled(false);
	connect(
		manageFilterAction, SIGNAL(activated()),
		this, SLOT(manageFilterActivated())
	);

	// �u�o�[�W�������v�A�N�V����
	aboutAction = new QAction(tr("�o�[�W�������(&A)..."), 0, this);
	aboutAction->setToolTip(tr("�o�[�W�������"));
	aboutAction->setStatusTip(
		tr("���쌠����уo�[�W�����Ɋւ������\�����܂��B")
	);
	connect(aboutAction, SIGNAL(activated()), this, SLOT(aboutActivated()));

	// �uQt �ɂ��āv�A�N�V����
	aboutQtAction = new QAction(tr("&Qt �ɂ���..."), 0, this);
	aboutQtAction->setToolTip(tr("Qt �ɂ���"));
	aboutQtAction->setStatusTip(tr("Qt ���C�u�����Ɋւ������\�����܂��B"));
	connect(aboutQtAction, SIGNAL(activated()), qApp, SLOT(aboutQt()));
}

// ���j���[�o�[����()
void MainWindow::createMenuBar()
{
	// �u�t�@�C���v���j���[
	fileMenu = new QPopupMenu(this);
	openAction->addTo(fileMenu);
	closeAction->addTo(fileMenu);
	fileMenu->insertSeparator();
	saveAction->addTo(fileMenu);
	saveAsAction->addTo(fileMenu);
	fileMenu->insertSeparator();
	quitAction->addTo(fileMenu);

	// �u�ҏW�v���j���[
	editMenu = new QPopupMenu(this);
	undoAction->addTo(editMenu);

	// �u�\���v���j���[
	viewMenu = new QPopupMenu(this);
	showPowerSpectrumAction->addTo(viewMenu);
	viewMenu->insertSeparator();
	showToolBarAction->addTo(viewMenu);
	showSideBarAction->addTo(viewMenu);
	showStatusBarAction->addTo(viewMenu);

	// �u�t�B���^�v���j���[
	filterMenu = new QPopupMenu(this);
	editFilterAction->addTo(filterMenu);
	manageFilterAction->addTo(filterMenu);

	// �u�w���v�v���j���[
	helpMenu = new QPopupMenu(this);
	aboutAction->addTo(helpMenu);
	aboutQtAction->addTo(helpMenu);

	menuBar()->insertItem(tr("�t�@�C��(&F)"), fileMenu);
	menuBar()->insertItem(tr("�ҏW(&E)"), editMenu);
	menuBar()->insertItem(tr("�\��(&V)"), viewMenu);
	menuBar()->insertItem(tr("�t�B���^(&I)"), filterMenu);
	menuBar()->insertItem(tr("�w���v(&H)"), helpMenu);
}

// �c�[���o�[����()
void MainWindow::createToolBar()
{
	toolBar = new QToolBar(tr("�c�[���o�["), this);
	openAction->addTo(toolBar);
	saveAction->addTo(toolBar);
	toolBar->addSeparator();
	undoAction->addTo(toolBar);
	toolBar->addSeparator();
	showPowerSpectrumAction->addTo(toolBar);
	editFilterAction->addTo(toolBar);
	toolBar->setMovingEnabled(false);
}

// �����̈搶��()
void MainWindow::createCentralWidget()
{
	imageScrollViewer = new QScrollView(this);
	setCentralWidget(imageScrollViewer);
}

// �X�e�[�^�X�o�[����()
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
	statusBar()->addWidget(statusLabel, 1);  // �L�k����
	statusBar()->addWidget(progressBar);     // �L�k�Ȃ�
	statusBar()->addWidget(resolutionLabel); // �L�k�Ȃ�
}

// �摜�ۑ��m�F()
bool MainWindow::checkSaving()
{
	if (editor == 0 || editor->getSaveCount() == 0) {
		return true;
	}

	// ���b�Z�[�W�{�b�N�X����
	QString message("");
	QString filePath = editor->getFilePath();
	if (filePath.isEmpty()) {
		message += tr("(�V�K�摜)");
	} else {
		message += QDir::convertSeparators(filePath);
	}
	message += tr(" �͕ύX����Ă��܂��B�ۑ����܂���?");
	QMessageBox box(
		tr("Image Equalizer"), message, QMessageBox::Warning,
		QMessageBox::Yes | QMessageBox::Default,
		QMessageBox::No,
		QMessageBox::Cancel | QMessageBox::Escape
	);
	box.setButtonText(QMessageBox::Yes, tr("�͂�(&Y)"));
	box.setButtonText(QMessageBox::No, tr("������(&N)"));
	box.setButtonText(QMessageBox::Cancel, tr("�L�����Z��"));

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

// �摜���J��()
bool MainWindow::loadImage()
{
	QString filePath = QFileDialog::getOpenFileName(
		QString::null, supportingImageTypes,
		0, 0, tr("�J��")
	);

	if (filePath.isEmpty()) { // �L�����Z���I����
		return false;
	}

	// �Ǎ����s
	QString errorMessage;
	ImageEditor* newEditor = new ImageEditor(undoingLimit, this);
	try {
		newEditor->loadImage(filePath);
	} catch (NoSuchFileException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" ��������܂���B\n"
			"�t�@�C�������m�F���Ă��������B"
		);
	} catch (InputException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" �̓Ǎ��Ɏ��s���܂����B\n"
			"�Ή��ł��Ȃ��t�@�C���`�����w�肳�ꂽ�\��������܂��B\n"
			"�܂��́A�A�N�Z�X�����Ȃ����A�t�@�C�������b�N����Ă��܂��B"
		);
	}

	if (!errorMessage.isNull()) { // �G���[������
		delete newEditor;
		QMessageBox::warning( // �G���[���b�Z�[�W���o��
			this, tr("�摜�Ǎ��G���["), errorMessage,
			QMessageBox::Ok | QMessageBox::Default, 0, 0
		);
		return false;
	}

	// �ǂݍ��ݐ�����
	if (editor != 0) {
		delete editor; // �O�̉摜�ҏW�̈��������ăX�N���[���̈悩�珜��
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

	// �𑜓x�\���X�V
	QString resolution("");
	QString formatter;
	resolution += formatter.sprintf(" %d", editor->getXLength());
	resolution += tr(" x ");
	resolution += formatter.sprintf("%d ", editor->getYLength());
	resolutionLabel->setText(resolution);

	// �^�C�g���o�[������X�V
	setTitle(
		QDir::convertSeparators(editor->getFilePath()) + tr(
			" - Image Equalizer"
		)
	);

	updatePowerSpectrum(); // �p���[�X�y�N�g���X�V

	return true;
}

// �摜���㏑���ۑ�()
bool MainWindow::saveImage()
{
	if (editor == 0) {
		return true;
	}

	QString filePath = editor->getFilePath();
	if (filePath.isEmpty()) { // (�V�K�摜) �̏ꍇ
		return saveAsImage();
	}

	// �ۑ��`���擾
	const char* format; // �ۑ��`��
	QString ext = QFileInfo(filePath).extension(false); // �g���q�擾
	if (!ext.isEmpty() && ext.lower() == "pgm") {        // pgm�̏ꍇ
		format = "PGM";
	} else if (!ext.isEmpty() && ext.lower() == "bmp") { // bmp�̏ꍇ
		format = "BMP";
	} else { // ��L�ȊO�̊g���q�̏ꍇ�A�܂��͊g���q���Ȃ��ꍇ
		format = QImage::imageFormat(filePath); // ���ۂ̌`�����擾
		if (format == 0) { // �`���s���̏ꍇ
			format = "PPM";
		}
	}

	// �ۑ����s
	QString errorMessage;
	try {
		editor->saveImage(filePath, format);
	} catch (NoSuchDirectoryException) {
		return saveAsImage();
	} catch (OutputException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" �̕ۑ��Ɏ��s���܂����B\n"
			"�Ή��ł��Ȃ��t�@�C���`�����w�肳�ꂽ�\��������܂��B\n"
			"�܂��́A�A�N�Z�X�����Ȃ����A�t�@�C�������b�N����Ă��܂��B"
		);
	}

	if (!errorMessage.isNull()) { // �G���[������
		QMessageBox::warning( // �G���[���b�Z�[�W���o��
			this, tr("�摜�ۑ��G���["), errorMessage,
			QMessageBox::Ok | QMessageBox::Default, 0, 0
		);
		return false;
	}

	return true;
}

// �摜�𖼑O��t���ĕۑ�()
bool MainWindow::saveAsImage()
{
	if (editor == 0) {
		return true;
	}

	QString currentFilePath = editor->getFilePath();
	if (currentFilePath.isEmpty() || !QFile::exists(currentFilePath)) {
		currentFilePath = QString::null;
	}

	QString selectedType; // �I�����ꂽ�t�@�C���^�C�v
	QString filePath = QFileDialog::getSaveFileName( // �_�C�A���O�I�[�v��
		currentFilePath, supportingImageTypes,
		0, 0, tr("���O��t���ĕۑ�"), &selectedType
	);
	if (filePath.isEmpty()) { // �L�����Z���I����
		return false;
	}

	// �I�����ꂽ�t�@�C���^�C�v�̊g���q�擾
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
	// �I�����ꂽ�t�@�C���̊g���q�擾
	QString selectedFileExt = QFileInfo(filePath).extension(false);
	// �t�@�C���̊g���q���Ȃ����t�@�C���^�C�v�̊g���q�ƈႤ�ꍇ
	if (selectedFileExt.isEmpty() || selectedFileExt.lower()!=selectedTypeExt) {
		filePath += "." + selectedTypeExt; // �g���q��t��
	}

	if (QFile::exists(filePath)) { // �����̃t�@�C����I�������ꍇ
		// �㏑���̊m�F�_�C�A���O���o��
		QString message = QDir::convertSeparators(filePath) + tr(
			" �͊��ɑ��݂��܂��B\n�㏑�����܂���?"
		);
		QMessageBox box(
			tr("���O��t���ĕۑ�"), message, QMessageBox::Warning,
			QMessageBox::Yes | QMessageBox::Default,
			QMessageBox::No | QMessageBox::Escape, 0
		);
		box.setButtonText(QMessageBox::Yes, tr("�͂�(&Y)"));
		box.setButtonText(QMessageBox::No, tr("������(&N)"));
		int ret = box.exec();
		if (ret != QMessageBox::Yes) { // Yes�ȊO��I�������ꍇ
			return false;
		}
	}

	// �ۑ��`���擾
	const char* format; // �ۑ��`��
	QString ext = QFileInfo(filePath).extension(false); // �g���q�擾
	if (!ext.isEmpty() && ext.lower() == "pgm") {        // pgm�̏ꍇ
		format = "PGM";
	} else if (!ext.isEmpty() && ext.lower() == "bmp") { // bmp�̏ꍇ
		format = "BMP";
	} else { // ��L�ȊO�̊g���q�̏ꍇ�A�܂��͊g���q���Ȃ��ꍇ
		format = QImage::imageFormat(filePath); // ���ۂ̌`�����擾
		if (format == 0) { // �`���s���̏ꍇ
			format = "PPM";
		}
	}

	// �ۑ����s
	QString errorMessage;
	try {
		editor->saveImage(filePath, format);
	} catch (NoSuchDirectoryException) {
		errorMessage = QDir::convertSeparators(
			QFileInfo(filePath).dir(true).absPath()
		) + tr(" �Ƃ������O�̃f�B���N�g����������܂���B");
	} catch (OutputException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" �̕ۑ��Ɏ��s���܂����B\n"
			"�Ή��ł��Ȃ��t�@�C���`�����w�肳�ꂽ�\��������܂��B\n"
			"�܂��́A�A�N�Z�X�����Ȃ����A�t�@�C�������b�N����Ă��܂��B"
		);
	}

	if (!errorMessage.isNull()) { // �G���[������
		QMessageBox::warning( // �G���[���b�Z�[�W���o��
			this, tr("�摜�ۑ��G���["), errorMessage,
			QMessageBox::Ok | QMessageBox::Default, 0, 0
		);
		return false;
	}

	setTitle( // �^�C�g���o�[������ύX
		QDir::convertSeparators(editor->getFilePath()) + tr(
			" - Image Equalizer"
		)
	);

	return true;
}

// �p���[�X�y�N�g���X�V()
void MainWindow::updatePowerSpectrum()
{
	bool isEnabled = showPowerSpectrumAction->isEnabled();
	bool isOn = showPowerSpectrumAction->isOn();

	// ������Ԃ���(�����܂���OFF)
	if (powerSpectrum == 0 && (!isEnabled || !isOn)) {
		return;
	}

	if (powerSpectrum == 0 && isEnabled && isOn) { // ������Ԃ��L������ON
		powerSpectrum = new PowerSpectrumViewer(
			256, 256, this, 0, 
			WType_TopLevel | WStyle_Customize | WStyle_DialogBorder |
			WStyle_Title | WStyle_SysMenu | WStyle_Dialog | WNoAutoErase
		);
		powerSpectrum->setTitle(tr("�p���[�X�y�N�g��"));
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

	if (editor != 0 && isEnabled && isOn) { // �摜�����݂��L������ON�̏ꍇ
		if (!isVisible) { // �����Ă��Ȃ��ꍇ
			powerSpectrum->show(); // �\��
		}
		// �\�����e�X�V
		powerSpectrum->setFrequencyDomain(editor->getFrequencyDomain());
	} else { // �摜�����݂��Ȃ��܂��͖����܂���OFF�̏ꍇ
		if (isVisible) { // �����Ă���ꍇ
			powerSpectrum->close();
		}
		// �����Ă��Ȃ��ꍇ�͉������Ȃ�
	}
}

void MainWindow::setTitle(const QString& title)
{
	ieq::setWindowTitle(this, title);
#if defined(QT_NON_COMMERCIAL) && defined(Q_WS_WIN)
	topData()->caption = title;
#endif
}

