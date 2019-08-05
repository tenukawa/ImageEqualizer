// �錾���e : �t�B���^�ҏW�N���X

#ifndef IEQ_FILTER_EDITOR
#define IEQ_FILTER_EDITOR

#ifndef IEQ
#include "ieq.h"
#endif

#ifndef IEQ_FILTER
#include "ieqfilter.h"
#endif

#include <qimage.h>
#include <qpixmap.h>
#include <qwidget.h>

class ieq::FilterEditor : public QWidget {
	Q_OBJECT
public:
	FilterEditor(
		int width, int height, QWidget* parent = 0, const char* name = 0
	);
	~FilterEditor();
	void loadFilter(const ieq::String& filePath);     // �t�B���^�Ǎ�
	void saveFilter(const ieq::String& filePath);     // �t�B���^�ۑ�
	void setColoring(bool coloring);                  // �F�t���ݒ�
	void setFrequencyRange(double start, double end); // ���g���ѐݒ�
	void setAngleRange(double start, double end);     // �����ѐݒ�
	void vary(double variance);                       // �������s
	void initialize();                                // ������
	void repaint(bool rangeUpdated, bool varied);     // �ĕ`��
	double getMeanOfRange() const;                    // �ҏW�̈敽�όW���Q��
	const ieq::Filter& getFilter() const;             // �t�B���^�Q��
	const ieq::String& getFilePath() const;           // �t�@�C���p�X�Q��
	int getSaveCount() const;                         // �ۑ��J�E���g�Q��
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
	ieq::String filePath;         // �ҏW���t�B���^�̃t�@�C���p�X
	int saveCount;                // �ۑ��J�E���^(����O�ɕۑ���������\����)
	bool coloring;                // �ҏW�̈�F�t���t���O(�^:�F�t��, �U:�F�Ȃ�)
	ieq::Filter* logicalFilter;   // �_���t�B���^
	double startFrequency;        // �ҏW���g���ъJ�n���g��
	double endFrequency;          // �ҏW���g���яI�����g��
	double startAngle;            // �ҏW�����ъJ�n�p(start declination)
	double endAngle;              // �ҏW�����яI���p
	bool rangeUpdated;            // �ҏW�̈�X�V�t���O
	bool varied;                  // �����t���O
	QImage* physicalFilter;       // �t�B���^�C���[�W
	QPixmap* rangeMap;            // �ҏW�̈�`��o�b�t�@

	void updatePhysicalFilter();              // �t�B���^�C���[�W�X�V
	void drawRangeBorder(QPainter& painter);  // �ҏW�̈�g���`��
	void updateRangeMap();                    // �ҏW�̈�`��o�b�t�@�X�V
	void resetBuffer();                       // �o�b�t�@�Đݒ�
	void setPhysicalFilterColorTable();       // �t�B���^�J���[�e�[�u���ݒ�
};

#endif
