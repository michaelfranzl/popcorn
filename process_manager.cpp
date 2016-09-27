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

#include "process_manager.h"

ProcessManager::ProcessManager(QObject *parent, QString app, QStringList args, bool detach, QString working_dir, qint64 timeout) :
    QObject(parent)
{
    qDebug() << "Level1 [ProcessManager::initialize]" << app << args << detach << working_dir << timeout;

    m_app = app;
    m_args = args;
    m_detach = detach;
    m_working_dir = working_dir;
    m_timeout = timeout;

    m_proc = new QProcess(this);
    connect(m_proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onFinished(int,QProcess::ExitStatus)));
    connect(m_proc, SIGNAL(readyReadStandardError()), this, SLOT(onReadyReadStandardError()));
    connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStandardOutput()));
    connect(m_proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onError(QProcess::ProcessError)));
    qDebug() << "Level1 [ProcessManager::initialize ]done";
}

void ProcessManager::run() {
    qDebug() << "Level1 [ProcessManager::run]";
    if (m_detach) {
        m_proc->startDetached(m_app, m_args);
    } else {
        m_proc->start(m_app, m_args);
    }
}

void ProcessManager::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    qDebug() << "Level0 [ProcessManager::onFinished]" << exitCode << exitStatus;
    QVariantMap std_in_out;
    std_in_out.insert("stdout", m_buffer_stdout);
    std_in_out.insert("stderr", m_buffer_stderr);
    emit finished(std_in_out);
    m_proc->deleteLater();
    qDebug() << "Level1 [ProcessManager::onFinished] cleaned up";
}

void ProcessManager::onError(QProcess::ProcessError err) {
    qDebug() << "Level0 [ProcessManager::error]" << err;
    emit error(err);
}

void ProcessManager::onReadyReadStandardError () {
    QString output = QString::fromUtf8(m_proc->readAllStandardError());
    m_buffer_stderr.append(output);
    qDebug() << "Level1 [ProcessManager::readyReadStandardError]" << output;
}

void ProcessManager::onReadyReadStandardOutput () {
    QString output = QString::fromUtf8(m_proc->readAllStandardOutput());
    m_buffer_stdout.append(output);
    qDebug() << "Level1 [ProcessManager::readyReadStandardOutput]" << output;
}
