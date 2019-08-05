// �錾���e : ��O�N���X

#ifndef IEQ_EXCEPTION
#define IEQ_EXCEPTION

namespace ieq {
	class Exception { // ��O
	};
	class IOException : public Exception { // ���o�͗�O
	};
	class NoSuchFileException : public IOException { // �t�@�C���s��
	};
	class NoSuchDirectoryException : public IOException { // �f�B���N�g���s��
	};
	class FileOpenException : public IOException { // �t�@�C���I�[�v����O
	};
	class InputException : public IOException { // ���͗�O
	};
	class OutputException : public IOException { // �o�͗�O
	};
	class FormatException : public IOException { // �t�H�[�}�b�g��O
	};
	class UnknownFormatException : public FormatException { // �t�H�[�}�b�g�s��
	};
	class InvalidFormatException: public FormatException { // �t�H�[�}�b�g�s��
	};
}

#endif
