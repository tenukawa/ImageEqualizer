// 宣言内容 : 例外クラス

#ifndef IEQ_EXCEPTION
#define IEQ_EXCEPTION

namespace ieq {
	class Exception { // 例外
	};
	class IOException : public Exception { // 入出力例外
	};
	class NoSuchFileException : public IOException { // ファイル不在
	};
	class NoSuchDirectoryException : public IOException { // ディレクトリ不在
	};
	class FileOpenException : public IOException { // ファイルオープン例外
	};
	class InputException : public IOException { // 入力例外
	};
	class OutputException : public IOException { // 出力例外
	};
	class FormatException : public IOException { // フォーマット例外
	};
	class UnknownFormatException : public FormatException { // フォーマット不明
	};
	class InvalidFormatException: public FormatException { // フォーマット不正
	};
}

#endif
