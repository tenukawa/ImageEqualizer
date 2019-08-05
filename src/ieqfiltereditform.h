// �錾���e : �t�B���^�ҏW�t�H�[���N���X

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
	const ieq::Filter& getFilter() const;   // �t�B���^�Q��
	const ieq::String& getFilePath() const; // �t�@�C���p�X�Q��
	bool checkSaving();                     // �ۑ��m�F
	virtual void setTitle(const QString& title);

	static ieq::String getFilterFileTypes() // �Ή��t�@�C���^�C�v�Q��
	{
		return tr("��Ԏ��g���̈�t�B���^ (*.sff)");
	}

signals:
	void applyButtonClicked();               // �t�B���^�K�p�{�^���N���b�N
	void registerButtonClicked();            // �t�B���^�o�^�{�^���N���b�N
	void visibilityChanged(bool visibility); // �����ύX

protected:
	void showEvent(QShowEvent* event);       // ����
	void closeEvent(QCloseEvent* event);     // �_�C�A���O�N���[�Y

private slots: // �C�x���g�n���h��
	void editorMouseMoved(int xPos, int yPos);   // �ҏW�̈�}�E�X�ړ�
	void editorMousePressed(int xPos, int yPos); // �ҏW�̈�}�E�X�N���b�N
	void editorMouseExited();        // �ҏW�̈�}�E�X�A�E�g
	void createButtonClicked();      // �V�K�쐬�{�^���N���b�N
	void loadButtonClicked();        // �Ǎ��{�^���N���b�N
	void saveButtonClicked();        // �㏑���ۑ��{�^���N���b�N
	void saveAsButtonClicked();      // �V�K�ۑ��{�^���N���b�N
	void increaseButtonClicked();    // ���{�^���N���b�N
	void decreaseButtonClicked();    // ���{�^���N���b�N
	void varianceBoxChanged();       // �ψʃ{�b�N�X�l�ύX
	void varianceSliderChanged();    // �ψʃX���C�_�l�ύX
	void startFrequencyBoxChanged(); // �J�n���g���{�b�N�X�l�ύX
	void endFrequencyBoxChanged();   // �I�����g���{�b�N�X�l�ύX
	void startAngleBoxChanged();     // �J�n�p�{�b�N�X�l�ύX
	void endAngleBoxChanged();       // �I���p�{�b�N�X�l�ύX
	void coloringSelectorToggled(bool isOn); // �F�t���I���{�b�N�X�g�O��

private:
	ieq::FilterEditor* editor;      // �ҏW�̈�
	QGroupBox* frequencyGroup;      // ���g���ѐݒ�O���[�v
	QGroupBox* angleGroup;          // �����ѐݒ�O���[�v
	QGroupBox* varianceGroup;       // �ψʐݒ�O���[�v
	QGroupBox* meanGroup;           // ���όW���O���[�v
	QPushButton* createButton;      // �V�K�쐬�{�^��
	QPushButton* loadButton;        // �Ǎ��{�^��
	QPushButton* saveButton;        // �㏑���ۑ��{�^��
	QPushButton* saveAsButton;      // �V�K�ۑ��{�^��
	QPushButton* applyButton;       // �t�B���^�K�p�{�^��
	QPushButton* registerButton;    // �t�B���^�o�^�{�^��
	QPushButton* increaseButton;    // ���{�^��
	QPushButton* decreaseButton;    // ���{�^��
	QSlider* varianceSlider;        // �������ݒ�X���C�_
	QSlider* startFrequencySlider;  // �J�n���g���ݒ�X���C�_
	QSlider* endFrequencySlider;    // �I�����g���ݒ�X���C�_
	QSlider* startAngleSlider;      // �J�n�p�ݒ�X���C�_
	QSlider* endAngleSlider;        // �I���p�ݒ�X���C�_
	ieq::RationalSpinBox* varianceBox;        // �������ݒ�X�s���{�b�N�X
	ieq::RationalSpinBox* startFrequencyBox;  // �J�n���g���ݒ�X�s���{�b�N�X
	ieq::RationalSpinBox* endFrequencyBox;    // �I�����g���ݒ�X�s���{�b�N�X
	ieq::RationalSpinBox* startAngleBox;      // �J�n�p�ݒ�X�s���{�b�N�X
	ieq::RationalSpinBox* endAngleBox;        // �I���p�ݒ�X�s���{�b�N�X
	QCheckBox* coloringSelector;    // �F�t���I���`�F�b�N�{�b�N�X
	QLabel* meanOfRangeLabel;       // �ҏW�̈敽�όW���\�����x��
	QStatusBar* statusBar;          // �X�e�[�^�X�o�[
	QLabel* statusLabel;            // �X�e�[�^�X���x��

	void createParts();     // GUI���i����
	void arrangeParts();    // GUI���i�z�u
	void setFocusChain();   // �t�H�[�J�X�`�F�[���ݒ�
	void connectEvent();    // �C�x���g�ڑ�
	bool loadFilter();      // �t�B���^�Ǎ�
	bool saveFilter();      // �t�B���^�㏑���ۑ�
	bool saveAsFilter();    // �t�B���^�V�K�ۑ�
};
#endif
