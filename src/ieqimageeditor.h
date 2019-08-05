// �錾���e : �摜�ҏW�N���X

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
	void loadImage(const ieq::String& filePath);   // �摜�Ǎ�
	void saveImage(const ieq::String& filePath, const char* format); // �摜�ۑ�
	void applyFilter(const ieq::Filter& filter);   // �t�B���^�K�p
	void undo();                                   // ���ɖ߂�
	int getLuminosity(int x, int y) const;         // �P�x�Q��
	int getXLength() const;                        // x������f���Q��
	int getYLength() const;                        // y������f���Q��
	const ieq::Matrix& getFrequencyDomain() const; // ���g���̈�Q��
	const ieq::String& getFilePath() const;        // �t�@�C���p�X�Q��
	int getSaveCount() const;                      // �ۑ��J�E���g�Q��
	bool isUndoingEnabled() const;                 // �A���h�D���s�\������
	virtual void setTitle(const QString& title);

signals:
	void mouseMoved(int xPos, int yPos);    // �}�E�X�ړ�
	void mousePressed(int xPos, int yPos);  // �}�E�X�N���b�N
	void mouseExited();                     // �}�E�X�A�E�g

protected:
	void paintEvent(QPaintEvent* event);      // �`��
	void mouseMoveEvent(QMouseEvent* event);  // �}�E�X�ړ��C�x���g�n���h��
	void mousePressEvent(QMouseEvent* event); // �}�E�X�N���b�N�C�x���g�n���h��
	void leaveEvent(QEvent* event);           // �}�E�X�A�E�g�C�x���g�n���h��

private:
	ieq::String filePath;           // �ҏW���摜�̃t�@�C���p�X
	int saveCount;                  // �ۑ��J�E���^(����O�ɕۑ���������\����)
	int undoCountDirection;         // �A���h�D���ɕۑ��J�E���^���������鐔
	QImage* physicalImage;          // �����摜
	ieq::Image* logicalImage;       // �_���摜
	ieq::ImageList imageStack;      // �摜�ҏW�X�^�b�N(�_���摜�̃R���e�i)
	int undoingLimit;               // �A���h�D������

	Matrix dummyMatrix;             // �_�~�[�s��

	void updatePhysicalImage();     // �����摜�X�V
	void updateViewer();            // �\���̈�X�V
	void initializeImageStack();    // �ҏW�X�^�b�N������
};

#endif
