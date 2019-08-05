// �錾���e : �t�B���^�N���X

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
	void load(const ieq::String& filePath); // �Ǎ�
	void setCoefficient(int x, int y, const ieq::Complex& value); // �W���ݒ�
	void save(const ieq::String& filePath) const; // �ۑ�
	const ieq::Complex& getCoefficient(int x, int y) const; // �W���Q��
	int getXLength() const; // x�����T�C�Y�Q��
	int getYLength() const; // y�����T�C�Y�Q��

	static const char* const fileHeader; // �t�@�C���w�b�_
	static const int fileHeaderLength;   // �t�@�C���w�b�_�̃o�C�g��

private:
	ieq::Matrix coefficient;        // �W���s��
};

#endif
