#include "chat.h"

int main(int argc, char *argv[])
{
#ifdef RTP_SOCKETTYPE_WINSOCK
	WSADATA dat;
	WSAStartup(MAKEWORD(2, 2), &dat);
#endif

	QApplication a(argc, argv);
	Chat w;
	w.show();
	return a.exec();

#ifdef RTP_SOCKETTYPE_WINSOCK
	WSACleanup();
#endif // RTP_SOCKETTYPE_WINSOCK
}
