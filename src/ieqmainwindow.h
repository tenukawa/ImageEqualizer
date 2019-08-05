// �錾���e : ���C���E�B���h�E�N���X

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

// �񎟊J���p
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
	void closeEvent(QCloseEvent* event);       // �E�B���h�E�N���[�Y����

private slots: // �C�x���g�n���h��
	void openActivated();         // �u�J���v�A�N�V�������s
	void closeActivated();        // �u����v�A�N�V�������s
	void saveActivated();         // �u�㏑���ۑ��v�A�N�V�������s
	void saveAsActivated();       // �u���O��t���ĕۑ��v�A�N�V�������s
	void undoActivated();         // �u���ɖ߂��v�A�N�V�������s
	void showToolBarToggled(bool isOn); // �u�c�[���o�[�v�\���A�N�V�����g�O��
	void showStatusBarToggled(    // �u�X�e�[�^�X�o�[�v�\���A�N�V�����g�O��
		bool isOn
	);
	void showSideBarToggled(bool isOn); // �u�T�C�h�o�[�v�\���A�N�V�����g�O��
	void showPowerSpectrumToggled( // �u�p���[�X�y�N�g���v�\���A�N�V�����g�O��
		bool isOn
	);
	void editFilterToggled(bool isOn); // �u�t�B���^�ҏW�v�A�N�V�����g�O��
	void manageFilterActivated();      // �u�t�B���^�Ǘ��v�A�N�V�������s
	void aboutActivated();             // �u�o�[�W�������v�A�N�V�������s
	void editorMouseMoved(int xPos, int yPos); // �摜�ҏW�̈�}�E�X�ړ�
	void editorMouseExited();                  // �摜�ҏW�̈�}�E�X�A�E�g
	void filterApplyButtonClicked();           // �t�B���^�K�p�{�^���N���b�N
	void filterRegisterButtonClicked();        // �t�B���^�o�^�{�^���N���b�N
	void filterFormVisibilityChanged(    // �t�B���^�t�H�[�������ύX
		bool isVisible
	);
	void powerSpectrumVisibilityChanged( // �p���[�X�y�N�g�������ύX
		bool isVisible
	);
	void powerSpectrumMouseMoved(        // �p���[�X�y�N�g���}�E�X�ړ�
		int xPos, int yPos, const ieq::String& message
	);
	void powerSpectrumMouseExited();     // �p���[�X�y�N�g���}�E�X�A�E�g

// 	�񎟊J���p
// 	void hideSideBarButtonClicked();
// 	void editFillterButtonClicked();
// 	void manageFillterButtonClicked();
// 	void filterEntryListBoxOnItem(QListBoxItem* item);
// 	void filterEntryListBoxClicked(QListBoxItem* item);
// 	void filterSelectionActivated(int index);

private:
	void loadSettings();         // �ݒ�Ǎ�
	void saveSettings();         // �ݒ�ۑ�
	void createActions();        // �A�N�V��������
	void createMenuBar();        // ���j���[�o�[����
	void createToolBar();        // �c�[���o�[����
	void createCentralWidget();  // �����̈搶��
	void createStatusBar();      // �X�e�[�^�X�o�[����
	void updatePowerSpectrum();  // �p���[�X�y�N�g���X�V
	bool loadImage();            // �摜���J��
	bool saveImage();            // �摜���㏑���ۑ�
	bool saveAsImage();          // �摜�𖼑O��t���ĕۑ�
	bool checkSaving();          // �摜�ۑ��m�F

	ieq::String entryFilePath;      // �t�B���^�o�^�t�@�C���̃p�X
	ieq::String lastImageDir;       // �Ō�ɎQ�Ƃ����摜�̃f�B���N�g���p�X
	ieq::String lastFilterDir;      // �Ō�ɎQ�Ƃ����t�B���^�̃f�B���N�g���p�X
	ieq::String supportingImageTypes;  // �T�|�[�g����摜�̃t�@�C���^�C�v
	ieq::String suppottingFilterTypes; // �T�|�[�g����t�B���^�̃t�@�C���^�C�v
	int undoingLimit;               // �A���h�D������
	int windowLeft;                 // �E�B���h�E���[��x���W
	int windowTop;                  // �E�B���h�E��[��y���W
	int windowWidth;                // �E�B���h�E�̕�
	int windowHeight;               // �E�B���h�E�̍���

	QPopupMenu* fileMenu;    // �u�t�@�C���v���j���[
	QPopupMenu* editMenu;    // �u�ҏW�v���j���[
	QPopupMenu* viewMenu;    // �u�\���v���j���[
	QPopupMenu* filterMenu;  // �u�t�B���^�v���j���[
	QPopupMenu* helpMenu;    // �u�w���v�v���j���[

	QToolBar* toolBar;       // �c�[���o�[

	QAction* openAction;                // �u�J���v�A�N�V����
	QAction* closeAction;               // �u����v�A�N�V����
	QAction* saveAction;                // �u�㏑���ۑ��v�A�N�V����
	QAction* saveAsAction;              // �u���O��t���ĕۑ��v�A�N�V����
	QAction* quitAction;                // �u�I���v�A�N�V����
	QAction* undoAction;                // �u���ɖ߂��v�A�N�V����
	QAction* showToolBarAction;         // �u�c�[���o�[�v�\���A�N�V����
	QAction* showStatusBarAction;       // �u�X�e�[�^�X�o�[�v�\���A�N�V����
	QAction* showSideBarAction;         // �u�T�C�h�o�[�v�\���A�N�V����
	QAction* showPowerSpectrumAction;   // �u�p���[�X�y�N�g���v�\���A�N�V����
	QAction* editFilterAction;          // �u�t�B���^�ҏW�v�A�N�V����
	QAction* manageFilterAction;        // �u�t�B���^�Ǘ��v�A�N�V����
	QAction* aboutAction;               // �u�o�[�W�������v�A�N�V����
	QAction* aboutQtAction;             // �uQt�ɂ��āv�A�N�V����

	QScrollView* imageScrollViewer;          // �摜�\���X�N���[���̈�
	ieq::ImageEditor* editor;                // �摜�ҏW�̈�
	ieq::FilterEditForm* filterForm;         // �t�B���^�t�H�[��
	ieq::PowerSpectrumViewer* powerSpectrum; // �p���[�X�y�N�g��

// 	�񎟊J���p
// 	QSplitter* centralSplitter;
// 	QVBox* sideBar;
// 	QPushButton* hideSideBarButton;
// 	QPushButton* editFillterButton;
// 	QPushButton* manageFillterButton;
// 	QListBox* filterEntryListBox;
// 	ieq::FilterManager* manager;
// 	const ieq::FilterEntryList* entryList;
// 	ieq::IntVector filterSelectionIds;

	QLabel* statusLabel;       // �X�e�[�^�X�o�[�̃��x��
	QProgressBar* progressBar; // �v���O���X�o�[
	QLabel* resolutionLabel;   // �𑜓x(��f��)�\�����x��
};

#endif
