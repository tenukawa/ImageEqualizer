// �N���X�� : �t�B���^

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

const char* const Filter::fileHeader = "ieqsff1.0      "; // �t�@�C���w�b�_
const int Filter::fileHeaderLength = 16; // �t�@�C���w�b�_�̃o�C�g��

Filter::Filter()
	: coefficient(extents[256][256])
{
	int i, j;

	// �S������1, �S�������[���ɏ�����
	for (i = 0; i < coefficient.size(); i++) {
		for (j = 0; j < coefficient[i].size(); j++) {
			coefficient[i][j] = Complex(1.0, 0.0); // �����?
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

// �Ǎ�()
void Filter::load(const String& path)
{
	int i, j;

	// �t�@�C���I�[�v��
	QFile fin(path);
	if (!fin.exists()) { // ���݂��Ȃ�
		throw NoSuchFileException();
	}
	if (!fin.open(IO_ReadOnly)) { // �I�[�v���G���[
		throw FileOpenException();
	}

	// ���̓X�g���[���ݒ�
	QDataStream stream(&fin);
	stream.setByteOrder(QDataStream::LittleEndian); // ���g���G���f�B�A������

	// �t�H�[�}�b�g�`�F�b�N
	char headstr[64];
	stream.readRawBytes(headstr, Filter::fileHeaderLength); // �w�b�_�Ǎ�
	if (fin.status() != IO_Ok) { // �G���[
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

	// �T�C�Y�擾
	Q_INT32 rows, cols, filler;
	stream >> rows;   // �s��
	stream >> cols;   // ��
	stream >> filler;
	stream >> filler;
	if (fin.status() != IO_Ok) { // �G���[
		fin.close();
		throw InputException();
	}
	if (stream.atEnd()) { // EOF
		fin.close();
		throw InvalidFormatException();
	}

	// �s��f�[�^�Ǎ�
	Matrix mat(extents[rows][cols]);
	double re, im;    // ����, ����
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			if (stream.atEnd()) { // EOF
				fin.close();
				throw InvalidFormatException();
			}
			stream >> re;
			stream >> im;
			if (fin.status() != IO_Ok) { // �G���[
				fin.close();
				throw InputException();
			}
			mat[i][j] = Complex(re, im);
		}
	}

	// �t�@�C���N���[�Y
	fin.close();

	if (coefficient.size() != rows || coefficient[0].size() != cols) {
		coefficient.resize(extents[rows][cols]); // �T�C�Y���Ⴄ�ꍇ�X�V
	}
	coefficient = mat;
}

// �ۑ�()
void Filter::save(const String& path) const
{
	int i, j;

	// �t�@�C���I�[�v��
	if (!QFileInfo(path).dir(true).exists()) { // �f�B���N�g�������݂��Ȃ�
		throw NoSuchDirectoryException();
	}
	QFile fout(path);
	if (!fout.open(IO_WriteOnly)) { // �I�[�v���G���[
		throw FileOpenException();
	}

	// �o�̓X�g���[���ݒ�
	QDataStream stream(&fout);
	stream.setByteOrder(QDataStream::LittleEndian);

	// ���^��񏑍���
	Q_INT32 rows = coefficient.size();      // �s��
	Q_INT32 cols = coefficient[0].size();   // ��
	Q_INT32 filler = 0;
	stream.writeRawBytes(Filter::fileHeader, Filter::fileHeaderLength);
	stream << rows;
	stream << cols;
	stream << filler;
	stream << filler;
	if (fout.status() != IO_Ok) { // �G���[����
		fout.close();
		throw OutputException();
	}

	// �s��f�[�^������
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			stream << coefficient[i][j].real();
			stream << coefficient[i][j].imag();
			if (fout.status() != IO_Ok) { // �G���[����
				fout.close();
				throw OutputException();
			}
		}
	}

	// �t�@�C���N���[�Y
	fout.close();
}

// �W���ݒ�()
void Filter::setCoefficient(int x, int y, const Complex& value)
{
	coefficient[y][x] = value;
}

// �W���Q��()
const Complex& Filter::getCoefficient(int x, int y) const
{
	return coefficient[y][x];
}

// x�����T�C�Y�Q��
int Filter::getXLength() const
{
	return coefficient[0].size();
}

// y�����T�C�Y�Q��
int Filter::getYLength() const
{
	return coefficient.size();
}



// �P�̃e�X�g�h���C�o
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
	// �e�X�gOK
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
	// �e�X�gOK
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
