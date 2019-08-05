// モジュール名 : ウィンドウタイトル設定

#include "ieq.h"

#if defined(QT_NON_COMMERCIAL) && defined(Q_WS_WIN)
#ifdef UNICODE
#undef UNICODE
#endif
#include <windows.h>
#endif

void ieq::setWindowTitle(QWidget* window, const ieq::String& title)
{
#if defined(QT_NON_COMMERCIAL) && defined(Q_WS_WIN)
	HWND hw = window->winId();
	QString deceiverTitle =
		title +
		"                                                                "
		"                                                                "
		"                                                                "
		"                                                                "
		"                                             [Non-Commercial] - ";
	SetWindowText(hw, (const char*)deceiverTitle.local8Bit());
	// topData()->caption = caption;
	// window->setCaption(
	// 	title +
	// 	"                                                                "
	// 	"                                                                "
	// 	"                                                                "
	// 	"                                                                "
	//	"                                             [Non-Commercial] - "
	// );
#else
	window->setCaption(title);
#endif
}
