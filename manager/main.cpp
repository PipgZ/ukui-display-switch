/* main.cpp
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#include <stdio.h>
#include <string>
#include <QApplication>
#include <QTextCodec>
#include <QResource>
#include <QTranslator>
#include <QLocale>
#include <QDateTime>
#include <QDebug>
#include <QStandardPaths>
#include <QProcess>
#include <QScreen>
#include <QDesktopWidget>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xrandr.h>
#include "display-switch/displayswitch.h"
#include "xeventmonitor.h"
#include <QFile>
#include <QtConcurrent/QtConcurrentRun>

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QFile file("/tmp/ukui-greeter.log");
    file.open(QIODevice::WriteOnly|QIODevice::Append);
    Q_UNUSED(context)
    QDateTime dateTime = QDateTime::currentDateTime();
    QByteArray time = QString("[%1] ").arg(dateTime.toString("MM-dd hh:mm:ss.zzz")).toLocal8Bit();
    QByteArray localMsg = msg.toLocal8Bit();
    file.write(localMsg);
    file.write("\n");
    file.close();
    switch(type) {
    case QtDebugMsg:

        fprintf(stderr, "%s Debug: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    case QtInfoMsg:
        fprintf(stderr, "%s Info: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        break;
#endif
    case QtWarningMsg:
        fprintf(stderr, "%s Warnning: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "%s Critical: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s Fatal: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        abort();
    }

}

void waitMonitorsReady()
{
    int n;

    QScreen *screen = QApplication::primaryScreen();
    while(n <= 0 && screen->geometry().x() <= 0 && screen->geometry().y() <= 0
          && screen->geometry().width() <= 0 && screen->geometry().height() <= 0){
        XRRGetMonitors(QX11Info::display(), QX11Info::appRootWindow(), false, &n);
        if(n == -1)
            qWarning() << "info: [main.cpp][waitMonitorsReady]: get monitors failed";
        else if(n > 0){

        }
        //启动xrandr，打开视频输出， 自动设置最佳分辨率
        QProcess enableMonitors;
        enableMonitors.start("xrandr --auto");
        enableMonitors.waitForFinished(-1);
    }
}

void x11_get_screen_size(int *width,int *height)
{
    Display* display;

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot connect to X server %s/n", "simey:0");
        exit (-1);
    }
    int screen_num;

    screen_num = DefaultScreen(display);

    *width = DisplayWidth(display, screen_num);
    *height = DisplayHeight(display, screen_num);
    XCloseDisplay(display);

}

int main(int argc, char *argv[])
{
    //qInstallMessageHandler(outputMessage);

    int width = 0, height = 0;
    x11_get_screen_size(&width,&height);

if(width>=2560){
#if(QT_VERSION>=QT_VERSION_CHECK(5,6,0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
}

    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(true);

    QResource::registerResource("image.qrc");

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    XEventMonitor::instance()->start();

    QWidget w;
    DisplaySwitch ds(&w);
    QObject::connect(XEventMonitor::instance(), SIGNAL(keyRelease(QString)),
                     &ds, SLOT(onGlobalKeyRelease(QString)));

    return a.exec();
}
