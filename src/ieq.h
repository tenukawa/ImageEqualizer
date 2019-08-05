// 宣言内容 : 名前空間・型定義・定数・インライン関数

// IEQ共通内部ヘッダ
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
	extern const double PI;    // 円周率
	class RationalSpinBox;     // 少数スピンボックス
	class Filter;              // フィルタ
	class FilterEditor;        // フィルタ編集
//	class FilterController;    // a controller has an editor and a viewer
	class FilterEditForm;      // フィルタ編集フォーム
	class FilterEntry;         // フィルタ登録情報 (二次開発)
	class FilterEnterDialog;   // フィルタ登録ダイアログ (二次開発)
	class FilterManager;       // フィルタ管理 (二次開発)
	class Image;               // 画像
	class ImageEditor;         // 画像編集
//	class ImageController;     // a controller has an editor and a viewer
	class SpectrumViewer;      // スペクトル表示
	class PowerSpectrumViewer; // パワースペクトル表示
	class MainWindow;          // メインウィンドウ
	typedef QString String;    // 文字列
	typedef std::vector<int> IntVector;   // 整数ベクタ
	typedef std::complex<double> Complex; // 複素数
	typedef boost::multi_array<ieq::Complex, 2> Matrix;   // 行列
	typedef std::list<ieq::Image*> ImageList;             // 画像リスト
	typedef std::list<ieq::FilterEntry*> FilterEntryList; // 登録フィルタリスト
	typedef std::map<ieq::String, ieq::FilterEntry*> FilterEntryMap;  // マップ

	// 切り捨て(引数: 切り捨て対象の数値, 小数点以下の桁数)
	inline double rationalFloor(double value, int downPlaces)
	{
		double bias = pow(10, downPlaces);
		return floor(value * bias) / bias;
	}

	// 切り上げ(引数: 切り上げ対象の数値, 小数点以下の桁数)
	inline double rationalCeil(double value, int downPlaces)
	{
		double bias = pow(10, downPlaces);
		return ceil(value * bias) / bias;
	}

	// ネイティブ文字コード→Unicode変換()
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

	// ウィンドウタイトル設定()
	extern void setWindowTitle(QWidget* window, const ieq::String& title);

	// フィルタリング係数→輝度変換()
	inline int toLuminosity(double cof, int defaultLumin, int limitMax)
	{
		int luminosity; // 輝度

		// 引数チェック
		if (defaultLumin <= 0 || defaultLumin >= limitMax || cof < 0.0) {
			return -1;
		}

		// 輝度計算
		double wk = defaultLumin * cof;
		luminosity = (int)(
			(limitMax * wk) / (limitMax - defaultLumin + wk) + 0.5
		);

		return (luminosity <= limitMax) ? luminosity : limitMax;
	}
}

#endif
