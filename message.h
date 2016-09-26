/*
 * popcorn (c) 2016 Michael Franzl
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QSslSocket>
#include <QHostAddress>
#include <QDebug>
#include <QDataStream>

class Message : public QObject
{
    Q_OBJECT

private:
    //variables

    //methods
    QVariantMap deserialize(QByteArray data);
    QByteArray serialize(QVariantMap map);


public:
    explicit Message(QObject *parent = 0, QByteArray ba = QByteArray());
    explicit Message(QObject *parent = 0, QVariantMap map = QVariantMap());
    QVariantMap toMap();
    QByteArray toByteArray();
    QByteArray m_bodySerialized;
    
signals:
    
public slots:
    
};

#endif // MESSAGE_H