/* displayservice.cpp
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
#include "displayservice.h"
#include "monitorwatcher.h"
#include <QException>
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <QVector>
#include <QtCore/qmath.h>

DisplayService::DisplayService(QObject *parent) : QObject(parent)
{

}

bool DisplayService::switchDisplayMode(DisplayMode targetMode)
{
    //获取显示器列表
    getMonitors();

    //如果当前只有一个显示器就什么也不做
    if(monitors.keys().size() < 2) {
        return false;
    }

    QProcess subProcess;
    //模式切换
    try {
        switch(targetMode){
        case DISPLAY_MODE_ORI:
        {
            QString XRANDR_ORIONLY = "xrandr --output " + monitorNames[0] + " --primary --auto";
            for(int i = 1; i < monitorNames.size(); i++)
                XRANDR_ORIONLY = XRANDR_ORIONLY + " --output " + monitorNames[i] + " --off";

            qDebug() << "info: [Class:DisplayService][switchDisplayMode] XRANDR_ORIONLY: " << XRANDR_ORIONLY;
            subProcess.start(XRANDR_ORIONLY, QIODevice::NotOpen);
            break;
        }
        case DISPLAY_MODE_CLONE:
        {
            //查找最佳克隆分辨率
            QString BEST_CLONE_MODE = createSuitableResolution();
            /*
            bool find = false;
            for(auto & mode_0 : monitors[monitorNames[0]]) {
                for(auto & mode_1 : monitors[monitorNames[1]]) {
                    if(mode_0 == mode_1) {
                        BEST_CLONE_MODE = mode_0;
                        find = true;
                        break;
                    }
                }
                if(find)
                    break;
            }
            */
            QString XRANDR_CLONE = "xrandr --output " + monitorNames[0] + " --mode " + BEST_CLONE_MODE + "  --primary --auto ";
            for(int i = 1; i < monitorNames.size(); i++)
                XRANDR_CLONE = XRANDR_CLONE + " --output " + monitorNames[i] + " --mode " + BEST_CLONE_MODE + " --same-as " + monitorNames[0];

             qDebug() << "info: [Class:DisplayService][switchDisplayMode] XRANDR_CLONE: " << XRANDR_CLONE;
             subProcess.start(XRANDR_CLONE, QIODevice::NotOpen);
            break;
        }
        case DISPLAY_MODE_EXTEND:
        {
            QString XRANDR_EXTEND = "xrandr --output " + monitorNames[0] + "  --primary --auto";
            for(int i = 1; i < monitorNames.size(); i++)
                XRANDR_EXTEND = XRANDR_EXTEND + " --output " + monitorNames[i] + " --right-of " + monitorNames[i-1] + " --auto";

            qDebug() << "info: [Class:DisplayService][switchDisplayMode] XRANDR_EXTEND: " << XRANDR_EXTEND;
            subProcess.start(XRANDR_EXTEND, QIODevice::NotOpen);
            break;
        }
        case DISPLAY_MODE_ONLY_OUT:
        {
            QString XRANDR_OUTONLY = "xrandr --output " + monitorNames[0] + " --off --output " + monitorNames[1] + " --primary --auto";

            subProcess.start(XRANDR_OUTONLY);
            qDebug() << "info: [Class:DisplayService][switchDisplayMode] XRANDR_OUTONLY: " << XRANDR_OUTONLY;
            break;
        }
        }
        subProcess.waitForFinished();
        return true;
    } catch(const QException &e) {
        qWarning() << "info: [Class:DisplayService][switchDisplayMode]: " << e.what();
        return false;
    }
}


void DisplayService::getMonitors()
{
    QProcess subProcess;
    subProcess.setProgram(QStandardPaths::findExecutable("xrandr"));
    subProcess.setArguments({"-q"});
    subProcess.start(QIODevice::ReadOnly);
    subProcess.waitForFinished();
    QString outputs = subProcess.readAll();
    QStringList lines = outputs.split('\n');
    QString name;
    QVector<QString> modes;
    bool find = false;
    QString lastName;
    monitorNames.clear();
    monitors.clear();
    for(auto & line : lines) {
        if(line.indexOf(" connected") != -1) {   //找显示器名称
            name = line.left(line.indexOf(' '));
            monitorNames.push_back(name);
            if(find)    //又找到了一个显示器，将上一个显示器的信息存到map
                monitors[lastName] = modes;
            find = true;
            lastName = name;
            modes.clear();
        } else {
            if(line.startsWith(' ')) {      //获取分辨率
                QString mode = line.trimmed().split(' ').at(0);
                modes.push_back(mode);
            }
        }
    }
    monitors[name] = modes;     //将最后一个显示器的信息存储到map

}

QString DisplayService::getNumberFromStr(const QString &str, int index, const bool flag)
{
    int dir = flag ? 1 : -1;
    const int &len = str.size();
    QString resStr = "";
    while(index < len && index >= 0 && (str.at(index) > '9' || str.at(index) < '0')) index += dir;
    while(index < len && index >= 0 && str.at(index) <= '9' && str.at(index) >= '0')
    {
        resStr = flag ? resStr + str.at(index) : str.at(index) + resStr;
        index += dir;
    }
    if(resStr.isEmpty())
        return "0";
    return resStr;
}

QString DisplayService::createSuitableResolution()
{
    QVector<DispResolution> dispsVec; // 记录各个屏幕的最适合的分辨率

    QString cmd = "xrandr -q";
    QProcess subProcess;
    subProcess.start(cmd, QIODevice::ReadOnly);
    subProcess.waitForFinished();
    QString outputs = subProcess.readAll();
    QStringList lines = outputs.split('\n');


    DispResolution minDisp(INT32_MAX, INT32_MAX, 60);
    for(int i = 0; i < lines.size();++i)
    {
        const QString &line = lines.at(i);
        if(line.indexOf(" connected") != -1)
        {
            const QString &nextLine = lines.at(++i);
            int x_index = nextLine.indexOf("x");
            int p_index = nextLine.indexOf(".");
            int width = getNumberFromStr(nextLine, x_index, false).toInt();
            int height = getNumberFromStr(nextLine, x_index, true).toInt();
            QString s1 = getNumberFromStr(nextLine, p_index, false);
            QString s2 = getNumberFromStr(nextLine, p_index, true);
            double flushHz = s1.toInt() + s2.toInt() / pow(10, s2.size());
            dispsVec.push_back({width, height, flushHz});
            //qDebug() << "获得第 " << dispsVec.size() << " 个屏幕的最优分辨率: "\
                     << width << " " << height << " " << flushHz;
            minDisp.width = minDisp.width < width ? minDisp.width : width;
            minDisp.height = minDisp.height < height ? minDisp.height : height;
            minDisp.flushHz = minDisp.flushHz < flushHz ? minDisp.flushHz : flushHz;
        }
    }

    //qDebug() << "生成最优分辨率: " << minDisp.width << " " << minDisp.height << " " << minDisp.flushHz;

    cmd = "cvt " + QString::number(minDisp.width) + " "
            + QString::number(minDisp.height) + " "
            + QString::number(minDisp.flushHz);

    subProcess.start(cmd, QIODevice::ReadOnly);
    subProcess.waitForFinished();
    outputs = subProcess.readAll();

    QString resolutionName;
    int begin_index = outputs.indexOf("Modeline ") + 8;
    cmd = "xrandr --newmode " + outputs.mid(begin_index);
    //qDebug() << cmd;
    subProcess.start(cmd, QIODevice::NotOpen);
    subProcess.waitForFinished();

    begin_index = outputs.indexOf("\"") + 1;
    int end_index = outputs.indexOf("\"", begin_index);
    resolutionName = outputs.mid(begin_index, end_index - begin_index);
    //qDebug() << "resolutionName: " << resolutionName;

    getMonitors();
    for(int i = 0;i < monitorNames.size();i++)
    {
        cmd = "xrandr --addmode " + monitorNames.at(i) + " " + resolutionName;
        //qDebug() << cmd;
        subProcess.start(cmd, QIODevice::NotOpen);
        subProcess.waitForFinished();
    }

    return resolutionName;
}
