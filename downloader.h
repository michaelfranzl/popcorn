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

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QNetworkReply>
#include <QFile>
#include <QDir>
#include <QSettings>

extern QSettings *settings;
extern QString jail_working_path;

class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(QObject *parent = 0, QNetworkAccessManager *manager = 0, QString id = "", QString path = "", QString filename = "");
    ~Downloader();

private:
    QString m_id;
    QString m_path;
    QString m_filename;
    QFile * m_file;
    QNetworkReply * m_reply;
    QNetworkAccessManager * m_manager;

signals:
    void saved(QString filename);
    void progress(qint64 bytesReceived, qint64 bytesTotal);
    void error(QString errormsg);


public slots:
    void get();
    void abort();

private slots:
    void replyFinished();
    void replyProgress(qint64 bytesReceived, qint64 bytesTotal);

};

#endif // DOWNLOADER_H
