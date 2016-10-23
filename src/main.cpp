#include "QTime"

#include "QFile"
#include "QTextStream"
#include <QCoreApplication>
#include "SmartHomeServer.h"
#include "IPv4OcfDeviceController.h"
#include "WebSocketServer.h"
#include <sys/time.h>



uint64_t get_current_ms(){
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    return milliseconds;
}


int main(int argc, char *argv[])
{

    QCoreApplication a(argc, argv);
    SmartHomeServer server;
    IPv4OcfDeviceController ocf(&server);
    ocf.start();

    WebSocketServer ws(&server);

    return a.exec();
}

