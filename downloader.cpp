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

#include "downloader.h"

Downloader::Downloader(QObject *parent, QNetworkAccessManager * manager, QString id, QString path, QString filename) :
    QObject(parent)
{
    qDebug() << "Level3 [Downloader::initialize]" << id << path << filename;
    m_manager = manager;
    m_id = id;
    m_path = path;
    m_filename = filename;
}

void Downloader::get() {
    qDebug() << "Level0 [Downloader::get]" << m_id << QUrl(m_path + "/" + m_filename);
    m_reply = m_manager->get(QNetworkRequest(QUrl(m_path + "/" + m_filename)));
    connect(m_reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(m_reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(replyProgress(qint64, qint64)));
}


void Downloader::replyProgress(qint64 bytesReceived, qint64 bytesTotal) {
    qDebug() << "Level3 [Downloader::replyProgress]:"<< m_id << bytesReceived << bytesTotal;
    emit progress(m_id, bytesReceived, bytesTotal);
}

void Downloader::replyFinished() {
    if( m_reply->error() ) {
        qDebug() << "Level1 [Downloader::replyFinished] Error:"<< m_id << m_reply->errorString();
        emit error(m_id, m_reply->errorString());

    } else {
        QDir().mkpath(jail_working_path + "/downloads");
        QFile *file = new QFile(jail_working_path + "/downloads/" + m_filename);
        if(file->open(QFile::WriteOnly)) {
            QByteArray data = m_reply->readAll();
            if (data.length() == 0) {
                emit error(m_id, "readZeroBytes");
            }
            file->write(data);
            file->close();
        } else {
            emit error(m_id, "couldNotOpenFile");
            qDebug() << "Level1 [Downloader::replyFinished] Error: could not open file" << jail_working_path + "/" + m_filename;
        }
        file->deleteLater();
        emit saved(m_id);
    }
    m_reply->deleteLater();
    this->deleteLater();
}
