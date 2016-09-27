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

#ifndef CLIENT_H
#define CLIENT_H


#include <QSslSocket>
#include <QCryptographicHash>
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QTemporaryFile>
#include <QCoreApplication>
#include <QFileInfo>
#include <QSettings>

#include "message.h"


extern QString jail_working_path;
extern QSettings *settings;

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject *parent, QString id, QString location);




    //variables


    QSslSocket *m_socket;

    // variables
    QString m_id;

private:
    // methods

    // member variables
    QByteArray m_buffer;

    QString m_location;

    qint64 m_writtenCounter;
    qint64 m_readCounter;
    qint64 m_dataSize;
    bool m_binaryMode;

    // file stuff
    bool m_fileMode;
    QString m_fileModeType;
    QFile *m_file;

signals:
    void bytesWritten(qint64 size);
    void readPlain(QString cmd);
    void readBinaryFeedback(QString cmd);
    void readBinary(qint64 bytes);
    void socketStateChange(int state);
    void encryptedBytesWritten(qint64 size);
    void modeChanged(int mode);
    void socketErrors(QVariantMap map);
    void socketEncrypted();

private slots:
    void onReadyRead();
    void onSocketStateChange(QAbstractSocket::SocketState state);
    void onBytesWritten(qint64 size);

    void onEncryptedBytesWritten(qint64 size);
    void onSocketSslErrors(QList<QSslError> errors);
    void onSocketEncrypted();
    void onModeChanged(QSslSocket::SslMode mode);

public slots:
    int connectToServer(QString host, qint64 port);
    void stop();
    int setMessage(QVariantMap msg);
    QVariantMap getMessage();
    qint64 writePlain(QString cmd);
    qint64 writeBinary(qint64 chunksize);
    void setBinary(qint64 size);
    void unsetBinary();
    void doFlush();
    QVariantMap setFileMode(QString type, QString fileName, qint64 pos = 0);
    void unsetFileMode();
    bool createSocket(bool is_server = false, int sd = 0);
    QString getPeerAddress();
    QVariantMap getInfo();
    int getState();

    void resume();
    void doIgnoreSslErrors();
    void startClientEncryption();
    void startServerEncryption();

    
};

#endif // CLIENT_H
