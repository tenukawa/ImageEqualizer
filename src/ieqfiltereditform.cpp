// �N���X�� : �t�B���^�ҏW�t�H�[��

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
	createParts();   // GUI���i����
	arrangeParts();  // GUI���i�z�u
	setFocusChain(); // �t�H�[�J�X�`�F�[���ݒ�
	connectEvent();  // �C�x���g�ڑ�
	setTitle(tr("(�V�K�t�B���^) - �t�B���^�ҏW"));
	QString mean;
	mean.sprintf("=  %.2f", editor->getMeanOfRange());
	meanOfRangeLabel->setText(mean); // �ҏW�̈敽�όW���\��
}

FilterEditForm::~FilterEditForm()
{
	// Qt�ɂ��GUI���i�I�u�W�F�N�g�̎������
}

// �t�B���^�Q��()
const Filter& FilterEditForm::getFilter() const
{
	return editor->getFilter();
}

// �t�@�C���p�X�Q��()
const String& FilterEditForm::getFilePath() const
{
	return editor->getFilePath();
}

// �ۑ��m�F()
bool FilterEditForm::checkSaving()
{
	if (editor->getSaveCount() == 0) {
		return true;
	}

	// ���b�Z�[�W�{�b�N�X����
	QString message("");
	QString filePath = editor->getFilePath();
	if (filePath.isEmpty()) {
		message += tr("(�V�K�t�B���^)");
	} else {
		message += QDir::convertSeparators(filePath);
	}
	message += tr(" �͕ύX����Ă��܂��B�ۑ����܂���?");
	QMessageBox box(
		tr("�t�B���^�ҏW"), message, QMessageBox::Warning,
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

// �t�B���^�Ǎ�()
bool FilterEditForm::loadFilter()
{
	QString filePath = QFileDialog::getOpenFileName(
		QString::null, FilterEditForm::getFilterFileTypes(),
		0, 0, tr("�t�B���^���J��")
	);

	if (filePath.isEmpty()) { // �L�����Z���I����
		return false;
	}

	QString errorMessage;
	try {
		editor->loadFilter(filePath); // �t�B���^�Ǎ��̎��s
	} catch (NoSuchFileException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" ��������܂���B\n"
			"�t�@�C�������m�F���Ă��������B"
		);
	} catch (FileOpenException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" ���J���܂���B\n"
			"�t�@�C�������b�N����Ă��邩�A�A�N�Z�X��������܂���B"
		);
	} catch (InputException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" �̓Ǎ��Ɏ��s���܂����B"
		);
	} catch (UnknownFormatException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" ���J���܂���B\n"
			"���Ή��̃t�@�C���`���ł��B"
		);
	} catch (InvalidFormatException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" ���J���܂���B\n"
			"�t�@�C���`���Ɍ�肪����܂��B"
		);
	}

	if (!errorMessage.isNull()) { // �G���[������
		QMessageBox::warning( // �G���[���b�Z�[�W���o��
			this, tr("�t�B���^�Ǎ��G���["), errorMessage,
			QMessageBox::Ok | QMessageBox::Default, 0, 0
		);
		return false;
	}

	meanOfRangeLabel->setText( // �ҏW�̈敽�όW���\��
		QString().sprintf("=  %.2f", editor->getMeanOfRange())
	);
	setTitle( // �^�C�g���o�[������ύX
		QDir::convertSeparators(editor->getFilePath()) + tr(
			" - �t�B���^�ҏW"
		)
	);

	return true;
}

// �t�B���^�ۑ�()
bool FilterEditForm::saveFilter()
{
	QString filePath = editor->getFilePath();
	if (filePath.isEmpty()) { // (�V�K�t�B���^) �̏ꍇ
		return saveAsFilter();
	}

	QString errorMessage;
	try {
		editor->saveFilter(filePath); // �t�B���^�ۑ��̎��s
	} catch (NoSuchDirectoryException) {
		return saveAsFilter();
	} catch (FileOpenException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" ���J���܂���B\n"
			"�t�@�C�������b�N����Ă��邩�A�A�N�Z�X��������܂���B"
		);
	} catch (OutputException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" �̕ۑ��Ɏ��s���܂����B"
		);
	}

	if (!errorMessage.isNull()) { // �G���[������
		QMessageBox::warning( // �G���[���b�Z�[�W���o��
			this, tr("�t�B���^�ۑ��G���["), errorMessage,
			QMessageBox::Ok | QMessageBox::Default, 0, 0
		);
		return false;
	}

	return true;
}

// �t�B���^�V�K�ۑ�()
bool FilterEditForm::saveAsFilter()
{
	QString currentFilePath = editor->getFilePath();
	if (currentFilePath.isEmpty() || !QFile::exists(currentFilePath)) {
		currentFilePath = QString::null;
	}

	QString filePath = QFileDialog::getSaveFileName( // �_�C�A���O�I�[�v��
		currentFilePath, FilterEditForm::getFilterFileTypes(),
		0, 0, tr("�t�B���^�̕ۑ�")
	);
	if (filePath.isEmpty()) { // �L�����Z���I����
		return false;
	}

	QString ext = QFileInfo(filePath).extension(false); // �g���q�擾
	if (ext.isEmpty() || ext != QString("sff")) { // �g���q���Ȃ����Ⴄ�ꍇ
		filePath += QString(".sff"); // �g���q��t��
	}

	if (QFile::exists(filePath)) { // �����̃t�@�C����I�������ꍇ
		// �㏑���̊m�F�_�C�A���O���o��
		QString message = QDir::convertSeparators(filePath) + tr(
			" �͊��ɑ��݂��܂��B\n�㏑�����܂���?"
		);
		QMessageBox box(
			tr("�t�B���^�̕ۑ�"), message, QMessageBox::Warning,
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

	QString errorMessage;
	try {
		editor->saveFilter(filePath); // �t�B���^�ۑ��̎��s
	} catch (NoSuchDirectoryException) {
		errorMessage = QDir::convertSeparators(
			QFileInfo(filePath).dir(true).absPath()
		) + tr(" �Ƃ������O�̃f�B���N�g����������܂���B");
	} catch (FileOpenException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" ���J���܂���B\n"
			"�t�@�C�������b�N����Ă��邩�A�A�N�Z�X��������܂���B"
		);
	} catch (OutputException) {
		errorMessage = QDir::convertSeparators(filePath) + tr(
			" �̕ۑ��Ɏ��s���܂����B"
		);
	}

	if (!errorMessage.isNull()) { // �G���[������
		QMessageBox::warning( // �G���[���b�Z�[�W���o��
			this, tr("�t�B���^�ۑ��G���["), errorMessage,
			QMessageBox::Ok | QMessageBox::Default, 0, 0
		);
		return false;
	}

	setTitle( // �^�C�g���o�[������ύX
		QDir::convertSeparators(editor->getFilePath()) + tr(
			" - �t�B���^�ҏW"
		)
	);

	return true;
}

// ����()
void FilterEditForm::showEvent(QShowEvent* event)
{
	emit visibilityChanged(true);
}

// �_�C�A���O�N���[�Y()
void FilterEditForm::closeEvent(QCloseEvent* event)
{
	event->accept();
	emit visibilityChanged(false);
}

// �ҏW�̈�}�E�X�ړ�()
void FilterEditForm::editorMouseMoved(int xPos, int yPos)
{
	const Filter* filter = &editor->getFilter();
	int filterWidth = filter->getXLength();
	int filterHeight = filter->getYLength();
	int editorWidth = editor->width();
	int editorHeight = editor->height();

	// 4�ی����W�v�Z
	int shiftedX = xPos - (editorWidth / 2 - 1);
	// int shiftedY = 1 - (yPos - (editorHeight / 2 - 1));
	int shiftedY = yPos - (editorHeight / 2 - 1);
	// �t�B���^�T�C�Y�ƕ\���T�C�Y�̍�������
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

	// �_�����W�v�Z
	double logicalX = (double)shiftedX / filterWidth;
	double logicalY = (double)shiftedY / filterHeight;

	// ���g���E�Ίp�v�Z
	double frequency = sqrt(logicalX * logicalX + logicalY * logicalY);
	double angle;
	if (logicalX != 0.0 || logicalY != 0.0) {
		angle = atan2(logicalY, logicalX) * 180.0 / ieq::PI;
	} else {
		angle = 0.0;
	}

	// �t�B���^���W�v�Z
	int filterX = (shiftedX + filterWidth) % filterWidth;
	int filterY = (shiftedY + filterHeight) % filterHeight;

	// �W���擾
	double coefficient = filter->getCoefficient(filterX, filterY).real();

	// �X�e�[�^�X���x��������ݒ�
	QString message("");
	QString formatter;
	message += tr("���g��: ");
	message += formatter.sprintf("%.3f", frequency);
	message += tr(" r/pix   ");
	message += tr("�Ίp: ");
	message += formatter.sprintf("%.1f", angle);
	message += tr(" �x   ");
	message += tr("�W��: ");
	message += formatter.sprintf("%.2f", coefficient);
	statusLabel->setText(message);
}

// �ҏW�̈�}�E�X�N���b�N()
void FilterEditForm::editorMousePressed(int xPos, int yPos)
{
	const Filter* filter = &editor->getFilter();
	int filterWidth = filter->getXLength();
	int filterHeight = filter->getYLength();
	int editorWidth = editor->width();
	int editorHeight = editor->height();

	// 4�ی����W�v�Z
	int shiftedX = xPos - (editorWidth / 2 - 1);
	// int shiftedY = 1 - (yPos - (editorHeight / 2 - 1));
	int shiftedY = yPos - (editorHeight / 2 - 1);
	// �t�B���^�T�C�Y�ƕ\���T�C�Y�̍�������
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

	// �_�����W�v�Z
	double logicalX = (double)shiftedX / filterWidth;
	double logicalY = (double)shiftedY / filterHeight;

	// ���g���E�Ίp�v�Z
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

	// �X�s���{�b�N�X�l�ݒ�
	startFrequencyBox->setValue((int)(floor(frequency * 1000)));
	endFrequencyBox->setValue((int)(ceil(frequency * 1000)));
	startAngleBox->setValue((int)(floor(angle * 10)));
	endAngleBox->setValue((int)(ceil(angle * 10)));
}

// �ҏW�̈�}�E�X�A�E�g()
void FilterEditForm::editorMouseExited()
{
	statusLabel->setText("");
}

// �V�K�쐬�{�^���N���b�N()
void FilterEditForm::createButtonClicked()
{
	if (!checkSaving()) {
		return;
	}

	editor->initialize(); // ������
	meanOfRangeLabel->setText( // �ҏW�̈敽�όW���\��
		QString().sprintf("=  %.2f", editor->getMeanOfRange())
	);
	setTitle(tr("(�V�K�t�B���^) - �t�B���^�ҏW")); // �^�C�g���o�[������ύX
}

// �Ǎ��{�^���N���b�N()
void FilterEditForm::loadButtonClicked()
{
	if (!checkSaving()) {
		return;
	}

	loadFilter(); // �t�B���^�Ǎ�
}

// �㏑���ۑ��{�^���N���b�N()
void FilterEditForm::saveButtonClicked()
{
	saveFilter();
}

// �V�K�ۑ��{�^���N���b�N()
void FilterEditForm::saveAsButtonClicked()
{
	saveAsFilter();
}

// ���{�^���N���b�N()
void FilterEditForm::increaseButtonClicked()
{
	editor->vary(varianceBox->value() / 100.0);
	QString mean;
	mean.sprintf("=  %.2f", editor->getMeanOfRange());
	meanOfRangeLabel->setText(mean);
}

// ���{�^���N���b�N()
void FilterEditForm::decreaseButtonClicked()
{
	editor->vary(-(varianceBox->value() / 100.0));
	QString mean;
	mean.sprintf("=  %.2f", editor->getMeanOfRange());
	meanOfRangeLabel->setText(mean);
}

// �ψʃ{�b�N�X�l�ύX()
void FilterEditForm::varianceBoxChanged()
{
	int value = varianceBox->value();

	// �X�s���{�b�N�X�ƃX���C�_�̓��������
	if (200 - value != varianceSlider->value()) {
		varianceSlider->setValue(200 - value);
	}
}

// �ψʃX���C�_�l�ύX()
void FilterEditForm::varianceSliderChanged()
{
	int value = varianceSlider->value();

	// �X���C�_�ƃX�s���{�b�N�X�̓��������
	if (200 - value != varianceBox->value()) {
		varianceBox->setValue(200 - value);
	}
}

// �J�n���g���{�b�N�X�l�ύX()
void FilterEditForm::startFrequencyBoxChanged()
{
	int value = startFrequencyBox->value();

	// �X�s���{�b�N�X�ƃX���C�_�̓��������
	if (value != startFrequencySlider->value()) {
		startFrequencySlider->setValue(value);
	}

	// �J�n�ƏI���̒���������
	if (value > endFrequencyBox->value()) {
		endFrequencyBox->setValue(value);
		return; // ��d�ݒ�h�~�p
	}

	editor->setFrequencyRange(value/1000.0, endFrequencyBox->value()/1000.0);

	meanOfRangeLabel->setText(
		QString().sprintf("=  %.2f", editor->getMeanOfRange())
	);
}

// �I�����g���{�b�N�X�l�ύX()
void FilterEditForm::endFrequencyBoxChanged()
{
	int value = endFrequencyBox->value();

	// �X�s���{�b�N�X�ƃX���C�_�̓��������
	if (value != endFrequencySlider->value()) {
		endFrequencySlider->setValue(value);
	}

	// �J�n�ƏI���̒���������
	if (value < startFrequencyBox->value()) {
		startFrequencyBox->setValue(value);
		return;
	}

	editor->setFrequencyRange(startFrequencyBox->value()/1000.0, value/1000.0);

	meanOfRangeLabel->setText(
		QString().sprintf("=  %.2f", editor->getMeanOfRange())
	);
}

// �J�n�p�{�b�N�X�l�ύX()
void FilterEditForm::startAngleBoxChanged()
{
	int value = startAngleBox->value();

	// �X�s���{�b�N�X�ƃX���C�_�̓��������
	if (value != startAngleSlider->value()) {
		startAngleSlider->setValue(value);
	}

	// �J�n�ƏI���̒���������
	if (value > endAngleBox->value()) {
		endAngleBox->setValue(value);
		return;
	}

	editor->setAngleRange(value / 10.0, endAngleBox->value() / 10.0);

	meanOfRangeLabel->setText(
		QString().sprintf("=  %.2f", editor->getMeanOfRange())
	);
}

// �I���p�{�b�N�X�l�ύX()
void FilterEditForm::endAngleBoxChanged()
{
	int value = endAngleBox->value();

	// �X�s���{�b�N�X�ƃX���C�_�̓��������
	if (value != endAngleSlider->value()) {
		endAngleSlider->setValue(value);
	}

	// �J�n�ƏI���̒���������
	if (value < startAngleBox->value()) {
		startAngleBox->setValue(value);
		return;
	}

	editor->setAngleRange(startAngleBox->value() / 10.0, value / 10.0);

	meanOfRangeLabel->setText(
		QString().sprintf("=  %.2f", editor->getMeanOfRange())
	);
}

// �F�t���I���{�b�N�X�g�O��()
void FilterEditForm::coloringSelectorToggled(bool isOn)
{
	editor->setColoring(isOn);
}

// GUI���i����()
void FilterEditForm::createParts()
{
	editor = new FilterEditor(256, 256, this);

	frequencyGroup = new QGroupBox(tr("���g����"), this);
	angleGroup = new QGroupBox(tr("������"), this);
	varianceGroup = new QGroupBox(tr("�W������"), this);
	meanGroup = new QGroupBox(tr("�ҏW�̈�̕��όW��"), this);

	createButton = new QPushButton(tr("�V�K�쐬(&N)"), this);
	loadButton = new QPushButton(tr("�t�@�C���Ǎ�(&O)"), this);
	saveButton = new QPushButton(tr("�㏑���ۑ�(&S)"), this);
	saveAsButton = new QPushButton(tr("�V�K�ۑ�(&A)"), this);

	applyButton = new QPushButton(tr("�t�B���^�K�p(&F)"), this);
	registerButton = new QPushButton(tr("�t�B���^�o�^(&R)"), this);
	registerButton->setEnabled(false);

	// varyButton = new QPushButton(tr("��  �s"), varianceGroup);
	increaseButton = new QPushButton(tr("��(&+)"), varianceGroup);
	decreaseButton = new QPushButton(tr("��(&-)"), varianceGroup);

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

	coloringSelector = new QCheckBox(tr("�ҏW�̈�F�t��"), this);
	coloringSelector->setChecked(true);

	meanOfRangeLabel = new QLabel(meanGroup);

	statusLabel = new QLabel(this);
	statusBar = new QStatusBar(this);
	statusBar->setSizeGripEnabled(false);
	statusBar->addWidget(statusLabel, 1);
}

// GUI���i�z�u()
void FilterEditForm::arrangeParts()
{
	// �ҏW�E�B�W�F�b�g�̔z�u
	editor->move(4, 4);

	// ���j���[�{�^���̔z�u
	const int fileMenuWidth = 94;
	const int fileMenuHeight = 24;
	const int fileMenuLeft = 270;
	// 30��������
	createButton->setGeometry(fileMenuLeft, 17, fileMenuWidth, fileMenuHeight);
	loadButton->setGeometry(fileMenuLeft, 47, fileMenuWidth, fileMenuHeight);
	saveButton->setGeometry(fileMenuLeft, 77, fileMenuWidth, fileMenuHeight);
	saveAsButton->setGeometry(fileMenuLeft, 107, fileMenuWidth,fileMenuHeight);

	applyButton->setGeometry(fileMenuLeft, 177, fileMenuWidth, fileMenuHeight);
	registerButton->setGeometry(fileMenuLeft,207,fileMenuWidth,fileMenuHeight);

	// ���g���юw�蕔�i�̔z�u
	frequencyGroup->setGeometry(3, 272, 236, 79);
	startFrequencySlider->setTickInterval(100);
	startFrequencySlider->setTickmarks(QSlider::Below);
	startFrequencySlider->setGeometry(12, 20, 92, 25);
	startFrequencyBox->setGeometry(110, 20, 56, 22);
	QLabel* startFrequencyLabel = new QLabel(tr("r/pix  ����"), frequencyGroup);
	startFrequencyLabel->setGeometry(172, 20, 55, 22);
	endFrequencySlider->setTickInterval(100);
	endFrequencySlider->setTickmarks(QSlider::Above);
	endFrequencySlider->setGeometry(12, 45, 92, 25);
	endFrequencyBox->setGeometry(110, 48, 56, 22);
	QLabel* endFrequencyLabel = new QLabel(tr("r/pix  �܂�"), frequencyGroup);
	endFrequencyLabel->setGeometry(172, 48, 55, 22);

	// �����юw�蕔�i�̔z�u
	angleGroup->setGeometry(3, 357, 236, 79);
	startAngleSlider->setTickInterval(150);
	startAngleSlider->setTickmarks(QSlider::Below);
	startAngleSlider->setGeometry(12, 20, 92, 25);
	startAngleBox->setGeometry(110, 20, 56, 22);
	QLabel* startAngleLabel = new QLabel(tr(" �x    ����"), angleGroup);
	startAngleLabel->setGeometry(172, 20, 55, 22);
	endAngleSlider->setTickInterval(150);
	endAngleSlider->setTickmarks(QSlider::Above);
	endAngleSlider->setGeometry(12, 45, 92, 25);
	endAngleBox->setGeometry(110, 48, 56, 22);
	QLabel* endAngleLabel = new QLabel(tr(" �x    �܂�"), angleGroup);
	endAngleLabel->setGeometry(172, 48, 55, 22);

	// �W���������i�̔z�u
	varianceGroup->setGeometry(244, 272, 125, 135);
	varianceSlider->setTickInterval(20);
	varianceSlider->setTickmarks(QSlider::Both);
	varianceSlider->setGeometry(12, 21, 30, 72);
	varianceBox->setGeometry(54, 46, 56, 22);
	QLabel* varianceLabel = new QLabel(tr("������"), varianceGroup);
	varianceLabel->setGeometry(58, 22, 60, 22);
	varianceLabel = new QLabel(tr("����"), varianceGroup);
	varianceLabel->setGeometry(72, 74, 45, 22);
	// varyButton->setGeometry(27, 103, 76, 24);
	increaseButton->setGeometry(12, 103, 47, 24);
	decreaseButton->setGeometry(66, 103, 47, 24);

	// ���όW�����i�̔z�u
	meanGroup->setGeometry(244, 413, 125, 43);
	meanOfRangeLabel->setGeometry(41, 16, 60, 22);

	// �F�t���I���`�F�b�N�{�b�N�X�̔z�u
	coloringSelector->setGeometry(9, 439, 200, 22);

	// �X�e�[�^�X�o�[�̔z�u
	statusBar->setGeometry(0, 459, 372, 25);
}

// �t�H�[�J�X�`�F�[���ݒ�()
void FilterEditForm::setFocusChain()
{
	// �^�u�ړ��ƃN���b�N�ɂ��t�H�[�J�X�ړ���ݒ�
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

	// �����f�t�H���g�v�b�V���{�^���ݒ�
	createButton->setAutoDefault(true);
	loadButton->setAutoDefault(true);
	saveButton->setAutoDefault(true);
	saveAsButton->setAutoDefault(true);
	applyButton->setAutoDefault(true);
	registerButton->setAutoDefault(true);
	// varyButton->setAutoDefault(true);
	increaseButton->setAutoDefault(true);
	decreaseButton->setAutoDefault(true);

	// �^�u�ړ����ݒ�
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
	// QWidget::setTabOrder(coloringSelector, createButton); // �����NG
}

// �C�x���g�ڑ�()
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


// �e�X�g�h���C�o
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
