// �錾���e : ���O��ԁE�^��`�E�萔�E�C�����C���֐�

// IEQ���ʓ����w�b�_
#ifndef IEQ
#define IEQ

#include <boost/multi_array.hpp>
#include <complex>
#include <list>
#include <map>
#include <string.h>
#include <math.h>
#include <qwidget.h>
#include <qstring.h>
#include <qtextcodec.h>

namespace ieq {
	extern const double PI;    // �~����
	class RationalSpinBox;     // �����X�s���{�b�N�X
	class Filter;              // �t�B���^
	class FilterEditor;        // �t�B���^�ҏW
//	class FilterController;    // a controller has an editor and a viewer
	class FilterEditForm;      // �t�B���^�ҏW�t�H�[��
	class FilterEntry;         // �t�B���^�o�^��� (�񎟊J��)
	class FilterEnterDialog;   // �t�B���^�o�^�_�C�A���O (�񎟊J��)
	class FilterManager;       // �t�B���^�Ǘ� (�񎟊J��)
	class Image;               // �摜
	class ImageEditor;         // �摜�ҏW
//	class ImageController;     // a controller has an editor and a viewer
	class SpectrumViewer;      // �X�y�N�g���\��
	class PowerSpectrumViewer; // �p���[�X�y�N�g���\��
	class MainWindow;          // ���C���E�B���h�E
	typedef QString String;    // ������
	typedef std::vector<int> IntVector;   // �����x�N�^
	typedef std::complex<double> Complex; // ���f��
	typedef boost::multi_array<ieq::Complex, 2> Matrix;   // �s��
	typedef std::list<ieq::Image*> ImageList;             // �摜���X�g
	typedef std::list<ieq::FilterEntry*> FilterEntryList; // �o�^�t�B���^���X�g
	typedef std::map<ieq::String, ieq::FilterEntry*> FilterEntryMap;  // �}�b�v

	// �؂�̂�(����: �؂�̂đΏۂ̐��l, �����_�ȉ��̌���)
	inline double rationalFloor(double value, int downPlaces)
	{
		double bias = pow(10, downPlaces);
		return floor(value * bias) / bias;
	}

	// �؂�グ(����: �؂�グ�Ώۂ̐��l, �����_�ȉ��̌���)
	inline double rationalCeil(double value, int downPlaces)
	{
		double bias = pow(10, downPlaces);
		return ceil(value * bias) / bias;
	}

	// �l�C�e�B�u�����R�[�h��Unicode�ϊ�()
	inline ieq::String toUnicode(
		const char* encodedString, const char* encoding = "sjis"
	)
	{
		const char* enc = encoding;
		if (strcmp(enc, "sjis") == 0 || strcmp(enc, "Shift_JIS") == 0) {
			enc = "Shift-JIS";
		}

	#if defined(Q_WS_WIN)
		if (strcmp(enc, "Shift-JIS") == 0) {
			return QString::fromLocal8Bit(encodedString);
		}
	#endif

		QTextCodec* codec = QTextCodec::codecForName(encoding);
		return codec->toUnicode(encodedString);
	}

	// �E�B���h�E�^�C�g���ݒ�()
	extern void setWindowTitle(QWidget* window, const ieq::String& title);

	// �t�B���^�����O�W�����P�x�ϊ�()
	inline int toLuminosity(double cof, int defaultLumin, int limitMax)
	{
		int luminosity; // �P�x

		// �����`�F�b�N
		if (defaultLumin <= 0 || defaultLumin >= limitMax || cof < 0.0) {
			return -1;
		}

		// �P�x�v�Z
		double wk = defaultLumin * cof;
		luminosity = (int)(
			(limitMax * wk) / (limitMax - defaultLumin + wk) + 0.5
		);

		return (luminosity <= limitMax) ? luminosity : limitMax;
	}
}

#endif
