// 宣言内容 : パワースペクトル表示クラス

#ifndef IEQ_POWER_SPECTRUM_VIEWER
#define IEQ_POWER_SPECTRUM_VIEWER

#ifndef IEQ
#include "ieq.h"
#endif

#ifndef IEQ_SPECTRUM_VIEWER
#include "ieqspectrumviewer.h"
#endif

class ieq::PowerSpectrumViewer : public ieq::SpectrumViewer {
public:
	PowerSpectrumViewer(
		int xSize, int ySize,
		QWidget* parent = 0, const char* name = 0, WFlags flag = WNoAutoErase
	);
	~PowerSpectrumViewer();
protected:
	virtual int toLuminosity(const Complex& value); // 輝度算出
	virtual ieq::String toMessage(                  // メッセージ変換
		int xPos, int yPos, const Complex& value
	);
};

#endif
