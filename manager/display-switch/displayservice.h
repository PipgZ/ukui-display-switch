/* displayservice.h
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
#ifndef DISPLAYSERVICE_H
#define DISPLAYSERVICE_H

#include <QObject>
#include <QVector>
#include <QMap>

enum DisplayMode{
    DISPLAY_MODE_ORI        = 1,
    DISPLAY_MODE_CLONE      = 2,
    DISPLAY_MODE_EXTEND     = 3,
    DISPLAY_MODE_ONLY_OUT   = 4
};

class DisplayService : public QObject
{
    Q_OBJECT
public:
    explicit DisplayService(QObject *parent = nullptr);
    bool switchDisplayMode(DisplayMode targetMode);

private:
    // 屏幕的刷新频率
    class  DispResolution
    {
    public:
         DispResolution() {}
         DispResolution(int w, int h, double r): width(w), height(h), flushHz(r){}
    public:
         int width, height;
         double flushHz;
    };

    void getMonitors();
    // flag = true, 从index正向找到第一个正整数
    // flag = false, 从index反向找到第一个正整数
    // 没有找到则返回 0
    QString getNumberFromStr(const QString &str, int index, const bool flag);
    QString createSuitableResolution(); // clone模式下 当没有适合各个屏幕的分辨率时  就创造一个最合适的分辨率

    QMap<QString, QVector<QString>> monitors;
    QStringList monitorNames;
};

#endif // DISPLAYSERVICE_H
