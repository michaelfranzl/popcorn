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

#include "message.h"


Message::Message(QObject *parent, QByteArray ba) :
    QObject(parent)
{
    qDebug() << "Level2 [Message] got ByteArray length" << ba.length();
    m_bodySerialized = ba;
}

Message::Message(QObject *parent, QVariantMap map) :
    QObject(parent)
{
    qDebug() << "Level2 [Message] got map" << map;
    m_bodySerialized = serialize(map);
}

QByteArray Message::toByteArray() {
    qDebug() << "Level2 [Message::toByteArray]";
    return m_bodySerialized;
}

QVariantMap Message::toMap() {
    qDebug() << "Level2 [Message::toMap]";
    return deserialize(m_bodySerialized);
}

QByteArray Message::serialize(QVariantMap body) {
    QByteArray ba = QByteArray();
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << body;
    return ba;
}

QVariantMap Message::deserialize(QByteArray ba) {
    QDataStream stream(&ba, QIODevice::ReadOnly);
    QVariantMap map;
    stream >> map;
    return map;
}
