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

#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QDir>
#include <QStringList>

class ProcessManager : public QObject
{
    Q_OBJECT
public:
    explicit ProcessManager(QObject *parent = 0, QString app = "", QStringList args = QStringList(), bool detach = false, QString working_dir = QDir::rootPath(), qint64 timeout = 10000);


private:
    QProcess * m_proc;
    QString m_app;
    QStringList m_args;
    bool m_detach;
    QString m_working_dir;
    qint64 m_timeout;
    QString m_buffer_stdout;
    QString m_buffer_stderr;
    
signals:
    void started();
    void finished(QVariantMap info);
    void error(QProcess::ProcessError err);
    
public slots:
    void run();

    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onError (QProcess::ProcessError err);
    void onReadyReadStandardError ();
    void onReadyReadStandardOutput ();
};

#endif // PROCESS_MANAGER_H
