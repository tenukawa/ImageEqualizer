// 宣言内容 : 小数スピンボックスクラス

#ifndef IEQ_RATIONAL_SPIN_BOX
#define IEQ_RATIONAL_SPIN_BOX

#ifndef IEQ
#include "ieq.h"
#endif

#include <qspinbox.h>

class ieq::RationalSpinBox : public QSpinBox {
public:
	RationalSpinBox(
		unsigned precision, int downPlaces,
		int minValue, int maxValue, int value, int step = 1,
		QWidget* parent = 0, const char* name = 0
	);
	~RationalSpinBox();

protected:
	QString mapValueToText(int value); // int値→表示テキスト変換
	int mapTextToValue(bool* ok);      // 入力テキスト→int値変換

private:
	unsigned precision;    // 精度(有効数字の桁数)
	int downPlaces;        // 小数点以下の桁数
};

#endif
