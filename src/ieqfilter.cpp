// クラス名 : フィルタ

#include "ieqfilter.h"

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <string.h>

#ifdef UNIT_TEST
#include <iostream>
#endif

using namespace boost;
using namespace ieq;
using namespace std;

const char* const Filter::fileHeader = "ieqsff1.0      "; // ファイルヘッダ
const int Filter::fileHeaderLength = 16; // ファイルヘッダのバイト数

Filter::Filter()
	: coefficient(extents[256][256])
{
	int i, j;

	// 全実部を1, 全虚部をゼロに初期化
	for (i = 0; i < coefficient.size(); i++) {
		for (j = 0; j < coefficient[i].size(); j++) {
			coefficient[i][j] = Complex(1.0, 0.0); // 非効率?
		}
	}
}

Filter::Filter(const Matrix& coef)
	: coefficient(coef)
{
}

Filter::~Filter()
{
}

// 読込()
void Filter::load(const String& path)
{
	int i, j;

	// ファイルオープン
	QFile fin(path);
	if (!fin.exists()) { // 存在しない
		throw NoSuchFileException();
	}
	if (!fin.open(IO_ReadOnly)) { // オープンエラー
		throw FileOpenException();
	}

	// 入力ストリーム設定
	QDataStream stream(&fin);
	stream.setByteOrder(QDataStream::LittleEndian); // リトルエンディアン方式

	// フォーマットチェック
	char headstr[64];
	stream.readRawBytes(headstr, Filter::fileHeaderLength); // ヘッダ読込
	if (fin.status() != IO_Ok) { // エラー
		fin.close();
		throw InputException();
	}
	if (stream.atEnd()) { // EOF
		fin.close();
		throw UnknownFormatException();
	}

	if (memcmp(headstr, Filter::fileHeader, Filter::fileHeaderLength) != 0) {
		fin.close();
		throw UnknownFormatException();
	}

	// サイズ取得
	Q_INT32 rows, cols, filler;
	stream >> rows;   // 行数
	stream >> cols;   // 列数
	stream >> filler;
	stream >> filler;
	if (fin.status() != IO_Ok) { // エラー
		fin.close();
		throw InputException();
	}
	if (stream.atEnd()) { // EOF
		fin.close();
		throw InvalidFormatException();
	}

	// 行列データ読込
	Matrix mat(extents[rows][cols]);
	double re, im;    // 実部, 虚部
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			if (stream.atEnd()) { // EOF
				fin.close();
				throw InvalidFormatException();
			}
			stream >> re;
			stream >> im;
			if (fin.status() != IO_Ok) { // エラー
				fin.close();
				throw InputException();
			}
			mat[i][j] = Complex(re, im);
		}
	}

	// ファイルクローズ
	fin.close();

	if (coefficient.size() != rows || coefficient[0].size() != cols) {
		coefficient.resize(extents[rows][cols]); // サイズが違う場合更新
	}
	coefficient = mat;
}

// 保存()
void Filter::save(const String& path) const
{
	int i, j;

	// ファイルオープン
	if (!QFileInfo(path).dir(true).exists()) { // ディレクトリが存在しない
		throw NoSuchDirectoryException();
	}
	QFile fout(path);
	if (!fout.open(IO_WriteOnly)) { // オープンエラー
		throw FileOpenException();
	}

	// 出力ストリーム設定
	QDataStream stream(&fout);
	stream.setByteOrder(QDataStream::LittleEndian);

	// メタ情報書込み
	Q_INT32 rows = coefficient.size();      // 行数
	Q_INT32 cols = coefficient[0].size();   // 列数
	Q_INT32 filler = 0;
	stream.writeRawBytes(Filter::fileHeader, Filter::fileHeaderLength);
	stream << rows;
	stream << cols;
	stream << filler;
	stream << filler;
	if (fout.status() != IO_Ok) { // エラー発生
		fout.close();
		throw OutputException();
	}

	// 行列データ書込み
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			stream << coefficient[i][j].real();
			stream << coefficient[i][j].imag();
			if (fout.status() != IO_Ok) { // エラー発生
				fout.close();
				throw OutputException();
			}
		}
	}

	// ファイルクローズ
	fout.close();
}

// 係数設定()
void Filter::setCoefficient(int x, int y, const Complex& value)
{
	coefficient[y][x] = value;
}

// 係数参照()
const Complex& Filter::getCoefficient(int x, int y) const
{
	return coefficient[y][x];
}

// x方向サイズ参照
int Filter::getXLength() const
{
	return coefficient[0].size();
}

// y方向サイズ参照
int Filter::getYLength() const
{
	return coefficient.size();
}



// 単体テストドライバ
#ifdef UNIT_TEST

int main()
{
	Matrix m(extents[256][256]);
	Filter f1;
	Filter f2(m);
	int x, y;

	for (y = 0; y < f2.getYLength(); y++) {
		for (x = 0; x < f2.getXLength(); x++) {
			f2.setCoefficient(x, y, Complex(5.5, 6.6));
		}
	}

	int f1_cnt = 0;
	int f2_cnt = 0;
/*
	// テストOK
	cout << "f1:\n";
	for (y = 0; y < f1.getYLength(); y++) {
		for (x = 0; x < f1.getXLength(); x++) {
			cout << f1.getCoefficient(x, y).real() << " + "
			<< f1.getCoefficient(x, y).imag() << "i    ";
			f1_cnt++;
		}
	}
	cout << "\nf1_cnt: " << f1_cnt << "\n";
*/
	// テストOK
	cout << "\n\nf2:\n";
	for (y = 0; y < f2.getYLength(); y++) {
		for (x = 0; x < f2.getXLength(); x++) {
			cout << "(" << x << "," << y << ") ";
			cout << f2.getCoefficient(x, y).real() << " + "
			<< f2.getCoefficient(x, y).imag() << "i    ";
			f2_cnt++;
		}
	}
	cout << "\nf2_cnt: " << f2_cnt << "\n";

	return 0;
}
#endif
