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

#include <QSettings>
#include <QDebug>
#include <QCommandLineParser>

#include "mainwindow.h"

QSettings *settings;
QString application_path;
QString home_path;
QString jail_working_path;


#ifdef Q_OS_LINUX
Display *display;
#endif


void logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QString pathLog = home_path + "/" APPNAME ".log";
    QFile f(pathLog);
    f.open(QIODevice::Append);
    QDateTime datetime = QDateTime::currentDateTime();
    QString timestamp = datetime.toString("yyyyMMddHHmmss");
    f.write(timestamp.toLatin1());
    f.write(" ");
    f.write(QString(msg).toLatin1());
    f.write("\r\n");
    f.close();
}


void logToStdout(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QByteArray localMsg = QString(msg).toLocal8Bit();

    int threshold = settings->value("log_threshold").toInt();
    int level;

    if (localMsg.startsWith("Level")) {
        level = localMsg.mid(5, 1).toInt();
        if (level <= threshold) {
            printf("%s\n", localMsg.constData());
            fflush(stdout);
        }
        return;
    }
}


void noLog(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
}

void purgeLogfile() {
    qDebug() << "Truncating Logfile";
    QString pathLog = home_path + "/" APPNAME ".log";
    QFile f(pathLog);
    f.open(QFile::WriteOnly|QFile::Truncate);
    f.close();
}


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QApplication::setApplicationName("popcorn");
    QApplication::setApplicationVersion(QString::number(VERSION_MAJOR) + "." + QString::number(VERSION_MINOR) + "." + QString::number(VERSION_PATCH));

#ifdef Q_OS_LINUX
    home_path = QDir::homePath() + "/.config/" APPNAME "/";
#else
    home_path = QDir::homePath() + "/" APPNAME "/";
#endif
    QDir().mkpath(home_path);

    QCommandLineOption ini_file_option("c", "Configuration file to use", "ini_file", home_path + APPNAME ".ini");
    QCommandLineOption index_file_option("i", "Index file (.html) to load", "index_file");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(ini_file_option);
    parser.addOption(index_file_option);
    parser.process(a);

#ifdef Q_OS_LINUX
    display = XOpenDisplay(NULL);
#endif

    application_path = QFileInfo(QCoreApplication::applicationFilePath()).canonicalPath() + "/";

    QString ini_file = parser.value(ini_file_option);
    settings = new QSettings(ini_file, QSettings::IniFormat);
    qDebug() << "[main]: ini file is" << settings->fileName();

    QString filespath = QDir::homePath() + "/" APPNAME "-files/";

    if (!settings->contains("window_title"))    settings->setValue("window_title", APPNAME " Popup");
    if (!settings->contains("webinspector"))    settings->setValue("webinspector", "false");
    if (!settings->contains("jail_working"))    settings->setValue("jail_working", filespath);
    if (!settings->contains("fileread_jailed")) settings->setValue("fileread_jailed", "true");
    if (!settings->contains("log_to_file"))     settings->setValue("log_to_file", "false");
    if (!settings->contains("log_to_stdout"))   settings->setValue("log_to_stdout", "false");
    if (!settings->contains("context_menu"))    settings->setValue("context_menu", "true");
    if (!settings->contains("log_threshold"))   settings->setValue("log_threshold", 0);
    if (!settings->contains("index_file"))      settings->setValue("index_file", "");

    jail_working_path = settings->value("jail_working").toString();

    QDir().mkpath(jail_working_path);

    if (!settings->value("jail_working").toString().endsWith("/")) {
        jail_working_path.append("/");
        settings->setValue("jail_working", jail_working_path);
    }

    purgeLogfile();

    if (settings->value("log_to_file").toString() == "true") {
        qInstallMessageHandler(logToFile);
    } else if (settings->value("log_to_stdout").toString() == "true") {
        qInstallMessageHandler(logToStdout);
    } else {
        qDebug() << "[main]: Logging disabled. Enable log_to_file or log_to_stdout in ini file.";
        qInstallMessageHandler(noLog);
    }

    settings->setValue("exit", "false"); // this tells us if the program has crashed or exited normally

    MainWindow w;
    w.init(parser.value(index_file_option));
    w.show();

    return a.exec();
}
