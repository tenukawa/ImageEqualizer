// �錾���e : �X�y�N�g���\�����ۃN���X

#ifndef IEQ_SPECTRUM_VIEWER
#define IEQ_SPECTRUM_VIEWER

#ifndef IEQ
#include "ieq.h"
#endif

#include <qdialog.h>
#include <qstatusbar.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qimage.h>

class ieq::SpectrumViewer : public QWidget {
	Q_OBJECT
public:
	SpectrumViewer(
		int xSize, int ySize,
		QWidget* parent = 0, const char* name = 0, WFlags flag = WNoAutoErase
	);
	~SpectrumViewer();
	void setFrequencyDomain(                       // ���g���̈�ݒ�
		const ieq::Matrix& frequencyDomain
	);
	const ieq::Matrix& getFrequencyDomain() const; // ���g���̈�Q��
	int getXLength() const;                        // x�����T�C�Y�Q��
	int getYLength() const;                        // y�����T�C�Y�Q��
	virtual void setTitle(const QString& title);

signals:
	void visibilityChanged(bool visibility);  // �����ύX
	void mouseMoved(                          // �}�E�X�ړ�
		int xPos, int yPos, const ieq::String& message
	);
	void mouseExited();                       // �}�E�X�A�E�g

protected:
	virtual int toLuminosity(      // �P�x�Z�o���ۃ��\�b�h
		const Complex& value
	) = 0;
	virtual ieq::String toMessage( // ���b�Z�[�W�ϊ����ۃ��\�b�h
		int xPos, int yPos, const Complex& value
	) = 0;
	void paintEvent(QPaintEvent* event);  // �`��
	void showEvent(QShowEvent* event);    // ����
	void hideEvent(QHideEvent* event);    // �s����
	void mouseMoveEvent(QMouseEvent* event); // �}�E�X�ړ��C�x���g�n���h��
	void leaveEvent(QEvent* event);          // �}�E�X�A�E�g�C�x���g�n���h��

private:
	ieq::Matrix frequencyDomain;        // ���g���̈�
	QImage* drawingBuffer;              // �`��o�b�t�@

	void updateDrawingBuffer();         // �`��o�b�t�@�X�V
	void setDrawingBufferColorTable();  // �`��o�b�t�@�J���[�e�[�u���ݒ�
};

#endif
