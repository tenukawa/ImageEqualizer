// ÉÇÉWÉÖÅ[Éãñº : mainä÷êî

#include "ieqmainwindow.h"

#include <qapplication.h>
#include <qtextcodec.h>
#include <qpixmap.h>

using namespace ieq;

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	QTextCodec* codec = QTextCodec::codecForName("Shift-JIS");
	QTextCodec::setCodecForTr(codec);

	MainWindow window;
	app.setMainWidget(&window);
	window.setIcon(QPixmap::fromMimeSource("logosmall.xpm"));
	window.show();

	return app.exec();
}

