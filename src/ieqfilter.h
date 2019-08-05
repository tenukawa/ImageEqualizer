// 宣言内容 : フィルタクラス

#ifndef IEQ_FILTER
#define IEQ_FILTER

#ifndef IEQ
#include "ieq.h"
#endif

#ifndef IEQ_EXCEPTION
#include "ieqexception.h"
#endif

class ieq::Filter {
public:
	Filter();
	Filter(const ieq::Matrix& coefficient);
	~Filter();
	void load(const ieq::String& filePath); // 読込
	void setCoefficient(int x, int y, const ieq::Complex& value); // 係数設定
	void save(const ieq::String& filePath) const; // 保存
	const ieq::Complex& getCoefficient(int x, int y) const; // 係数参照
	int getXLength() const; // x方向サイズ参照
	int getYLength() const; // y方向サイズ参照

	static const char* const fileHeader; // ファイルヘッダ
	static const int fileHeaderLength;   // ファイルヘッダのバイト数

private:
	ieq::Matrix coefficient;        // 係数行列
};

#endif
