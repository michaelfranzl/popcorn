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

#ifndef JSAPI_H
#define JSAPI_H

#include <QMap>
#include <QTimer>
#include <QSettings>
#include <QDebug>
#include <QSound>
#include <QShortcut>
#include <QKeySequence>
#include <QSystemTrayIcon>
#include <QCryptographicHash>
#include <QUdpSocket>

#include "jsapi.h"
#include "mainwindow.h"
#include "message.h"
#include "client.h"
#include "server.h"
#include "process_manager.h"
#include "downloader.h"

#ifdef Q_OS_WIN
    #include <windows.h>
    #include <winuser.h>
    #include <tchar.h>
    #include <direct.h>
    #include <psapi.h>
#endif

#ifdef Q_OS_LINUX
    #include <signal.h>
    //#include <QX11Info>
    #include <X11/Xlib.h>

    //#include <X11/Xutil.h>
    #include <sys/resource.h>
    // apt-get install libxss-dev
    #include <X11/extensions/scrnsaver.h>
    #undef Bool
#endif

#ifdef Q_OS_MACX
#include <unistd.h>
    #include <sys/resource.h>
    #include <ApplicationServices/ApplicationServices.h>
#endif

#include <QChar>

#ifdef Q_OS_LINUX
extern Display *display;
#endif

extern QSettings *settings;
extern QString application_path;
extern QString home_path;
extern QString jail_working_path;

extern "C" {
 void randombytes(char *, qint64);
}

class MainWindow;
class Server;

class JsApi : public QObject
{
    Q_OBJECT

public:
    explicit JsApi(MainWindow *parent = 0);
    MainWindow *m_mainWindow;
    ProcessManager *m_process;

private:
    // variables
    QUdpSocket *m_udpServer;
    Server *m_server;
    QMap<QString, Client*> m_clients;
    QMap<QString, QVariantMap> m_bubbleParams;
    qint64 m_bubbleParamID;
    qint64 m_lastLocalClientID;
    QByteArray *m_lastUdpMessage;
    QHostAddress *m_lastUdpIp;

    QCryptographicHash *m_fileHashCrypto;
    QFile *m_fileHashFile;

    bool m_server_udp_started;
    bool m_server_tcp_started;

    // methods

    
signals:

private slots:
    void onUdpDatagramReceived();
    void bubbleUp(QString label, QVariantMap option = QVariantMap());
    void onNewSocketDescriptor(qintptr sd);
    
public slots:
    // public slot methods are exposed to JavaScript
    void setConfiguration(QString key, QVariant val);
    QVariant getConfiguration(QString key);
    void clearConfiguration(QString key);
    QVariantMap getBubbleParams(int id);
    void windowMinimize();

    // directory operations
    bool dirMake(QString path_rel);
    bool dirRemove(QString path_rel);
    QStringList ls(QString jail_type, QString path_rel, QStringList filters);

    // file operations
    qint64 fileSize(QString infilepath_abs_or_rel);
    bool fileRename(QString infilepath_rel, QString outfilepath_rel);
    qint64 fileWrite(QString jail_type, QString filepath_rel, QString content, int open_mode = 2);
    QString fileRead(QString jail_type, QString filepath_rel);
    bool fileCopy(QString infilepath_abs_or_rel, QString outfilepath_rel);
    bool fileExists(QString jail_type, QString filepath_rel);
    bool fileRemove(QString jail_type, QString filepath_rel);

    QVariantMap fileHashSetup(QString path, int type = 1);
    QVariantMap fileHashDo(qint64 chunksize = 1000000);
    QVariantMap fileHashDone();

    double getIdleTime();
    QString getSystemUserName();
    QString getSystemComputerName();
    QStringList getMyLocalIpAddresses();
    QVariant clientAction(QString action, QVariantMap options = QVariantMap());
    QString getHash(QString string, int type = 1); // MD5 default
    QString getHashFromHexStr(QString string_hex, int type = 1);
    qint64 sendUdpMessage(QVariantMap msg, QString host, qint64 port);
    QVariantMap getUdpMessage();
    void showTrayMessage(QString title, QString msg, int type = 0, int delay = 1000);
    void setTrayToolTip(QString tip);
    void playSound(QString name);
    QVariantMap getStaticPaths();
    void showOptionsDialog();
    QString runUnzip(QString id, QString zip_filepath_rel, QString extract_dir_rel, bool detach = false);
#ifdef Q_OS_WIN
    void JsApi::runUpgrader(QString id, QString source_dir, QString dest_dir, bool detach = true);
#endif
    void downloadFile(QString id, QString path, QString filename = "");
    QString getVersion();
    void shutdown();
    void restart();
    double getMemUsage();
    bool getMinimizedStatus();
    void clearMemoryCaches();
    QString printDebug(QByteArray input);
    void debug(QString str);
    QVariantMap getOsInfo();
    void windowSetFlags(int flgs);
    int windowGetState();
    void windowSetState(int stat);
    void windowActivate();
    void windowRaise();
    void windowClose();
    void windowShow();
    void windowShowNormal();
    int getQtVersion();
    QString getRandomBytes(int len);
    bool startUdpServer(quint16 port);
    bool startTcpServer(quint16 port);
    QStringList getCmdlineArgs();

    // regular slots
    void onOptionsDialogAccepted();
    void onTrayMessageClicked();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onProcessFinished(QString id, QVariantMap contents);
    void onProcessError(QString id, QProcess::ProcessError err);
    void onFileDownloaded(QString id);
    void onFileDownloadError(QString id, QString errormsg);
    void onFileDownloadProgress(QString id, qint64 bytesReceived, qint64 bytesTotal);
};

#endif // JSAPI_H
