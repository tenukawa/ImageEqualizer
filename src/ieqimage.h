// 宣言内容 : 画像クラス

#ifndef IEQ_IMAGE
#define IEQ_IMAGE

#ifndef IEQ
#include "ieq.h"
#endif

#include <fftw.h>

class ieq::Image {
public:
	Image(const ieq::Matrix& spaceDomain);
	Image(const ieq::Image& sourceImage);
	~Image();
	void setSpaceDomain(const ieq::Matrix& spaceDomain); // 空間領域設定
	void setFrequencyDomain(                             // 周波数領域設定
		const ieq::Matrix& frequencyDomain
	);
	const ieq::Matrix& getSpaceDomain() const;           // 空間領域参照
	const ieq::Complex& getSpaceDomain(int x, int y) const; // 不要?
	const ieq::Matrix& getFrequencyDomain() const;       // 周波数領域参照
	int getXLength() const;                              // x方向サイズ参照
	int getYLength() const;                              // y方向サイズ参照

private:
	ieq::Matrix spaceDomain;     // 空間領域
	ieq::Matrix frequencyDomain; // 周波数領域
	fftwnd_plan ftForwardPlan;   // 順解析プラン
	fftwnd_plan ftInversePlan;   // 逆解析プラン
};

#endif
