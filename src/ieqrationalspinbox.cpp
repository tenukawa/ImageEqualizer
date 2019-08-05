// クラス名 : 小数スピンボックス

#include "ieqrationalspinbox.h"

#include <qspinbox.h>
#include <qwidget.h>
#include <qvalidator.h>
#include <qlineedit.h>
#include <math.h>
#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#endif

ieq::RationalSpinBox::RationalSpinBox(
	unsigned precision, int downPlaces,
	int minValue, int maxValue, int value, int step,
	QWidget* parent, const char* name
) : QSpinBox(minValue, maxValue, step, parent, name) 
{
	QRegExp rationalExp("-?[0-9]+\\.?[0-9]*"); // 小数正規表現
	setValidator(new QRegExpValidator(rationalExp, this)); // 入力チェッカ設定
	this->precision = precision;
	this->downPlaces = downPlaces;
	editor()->setAlignment(AlignRight); // 表示を右寄せに設定
	setValue(value);
}

ieq::RationalSpinBox::~RationalSpinBox()
{
	// Qtによるオブジェクトツリーの自動解放
}

// int値→表示テキスト変換()
QString ieq::RationalSpinBox::mapValueToText(int value)
{
	int seisu;    // 整数部
	int shousu;   // 小数部
	QString text;

	if (downPlaces >= 0) {
		int divisor = (int)pow(10.0, (double)downPlaces);
		seisu = abs(value) / divisor;
		shousu = abs(value) % divisor;
		if (shousu != 0) {
			if (value >= 0) {
				text.sprintf("%d.%0*d", seisu, downPlaces, shousu);
			} else {
				text.sprintf("-%d.%0*d", seisu, downPlaces, shousu);
			}
		} else { // 整数部のみの場合
			if (value >= 0) {
				text.sprintf("%d", seisu);
			} else {
				text.sprintf("-%d", seisu);
			}
		}
	} else {
		seisu = value * (int)pow(10.0, (double)-downPlaces);
		if (value >= 0) {
			text.sprintf("%d", seisu);
		} else {
			text.sprintf("-%d", seisu);
		}
	}

	return text;
}

// 入力テキスト→int値変換()
int ieq::RationalSpinBox::mapTextToValue(bool* ok)
{
	double wk;
	int value = minValue();
	if (sscanf(text().latin1(), "%lf", &wk) == 1) {
		// 入力値を整数化し、有効でない桁を切り捨てる
		wk *= pow(10.0, (double)downPlaces);
		value = (int)wk % (int)pow(10.0, (double)precision);
		*ok = true;
	} else { // 入力が浮動小数点数形式でない場合
		*ok = false;
	}

	return value;
}



#ifdef UNIT_TEST
#include <qapplication.h>
// 単体テストドライバ
int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	// ieq::RationalSpinBox box(3, 3, 0, 500, 0, 1);
	ieq::RationalSpinBox box(3, 2, -100, 100, -100, 1);
	box.setMinimumSize(50, 30);
	box.setMaximumSize(50, 30);

	app.setMainWidget(&box);
	box.show();

	return app.exec();
}
#endif
