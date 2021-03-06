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

#include "jsapi.h"
#include <QWebFrame>
#include <QCryptographicHash>
#include <QSysInfo>
#include <QSslCipher>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QDataStream>

JsApi::JsApi(MainWindow *parent) :
    QObject(parent)
{
    m_mainWindow = parent;

#ifdef Q_OS_MAC
    QShortcut *showOptionsShortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_O), m_mainWindow);
#else
    QShortcut *showOptionsShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), m_mainWindow);
#endif
    connect(showOptionsShortcut, &QShortcut::activated, this, &JsApi::showOptionsDialog);
}


QObject * JsApi::createUdpServer() {
    QUdpSocket * s = new UdpServer(this);
    return s;
}

QObject * JsApi::createTcpServer() {
    TcpServer * tcps = new TcpServer(this);
    return tcps;
}

int JsApi::getQtVersion() {
    return QT_VERSION;
}

QString JsApi::getAppName() {
    return APPNAME;
}

QString JsApi::getRandomBytes(int len) {
    QByteArray buf = QByteArray();
    buf.resize(len);
    randombytes((char*)buf.data(), len);
    QString result = QString(buf.toHex());
    return result;
}

QVariantMap JsApi::getOsInfo() {
    QVariantMap map;
#ifdef Q_OS_WIN
    map.insert("windowsVersion", QSysInfo::windowsVersion());
#endif
#ifdef Q_OS_MAC
    map.insert("macVersion", QSysInfo::macVersion());
#endif
    return map;
}

bool JsApi::windowGetMinimizedStatus() {
    return m_mainWindow->windowState().testFlag(Qt::WindowMinimized);
}

void JsApi::clearMemoryCaches() {
    QWebSettings::globalSettings()->clearMemoryCaches();
}

double JsApi::getMemUsage() {
#ifdef Q_OS_LINUX
    struct rusage rusage;
    getrusage( RUSAGE_SELF, &rusage );
    return rusage.ru_maxrss;
#endif
#ifdef Q_OS_MACX
    struct rusage rusage;
    getrusage( RUSAGE_SELF, &rusage );
    return rusage.ru_maxrss;
#endif
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS pMemCountr;
    bool result = GetProcessMemoryInfo(
                GetCurrentProcess(),
                &pMemCountr,
                sizeof(pMemCountr)
                );
    return pMemCountr.PeakWorkingSetSize;
#endif
}

void JsApi::restart() {
    if (m_mainWindow->windowState().testFlag(Qt::WindowMinimized) == true) {
        settings->setValue("minified_state", "true");
    } else {
        settings->setValue("minified_state", "false");
    }
    settings->sync();

    QProcess proc;
    QStringList used_args = QApplication::arguments();
    used_args.removeFirst(); // executable name
    proc.startDetached(QCoreApplication::applicationFilePath(), used_args);
    qApp->exit(0);
}

void JsApi::shutdown() {
    qDebug() << "Level1 [JsApi::shutdown] Called";
    if (m_mainWindow->windowState().testFlag(Qt::WindowMinimized) == true) {
        settings->setValue("minified_state", "true");
    } else {
        settings->setValue("minified_state", "false");
    }
    qApp->exit(0);
}

QObject *JsApi::createDatabase(QString label) {
    Database *db = new Database(label, this);
    m_mainWindow->webView->page()->mainFrame()->addToJavaScriptWindowObject(label, db, QWebFrame::AutoOwnership);
    return db;
}

QObject *JsApi::createDownloader(QString label, QString path, QString filename) {
    Downloader *dl = new Downloader(this, m_mainWindow->m_network_manager, label, path, filename);
    return dl;
}

QObject *JsApi::runUnzip(QString zip_filepath_rel, QString extract_dir_rel, bool detach) {
    qDebug() << "Level1 [JsApi::runUnzip]" << zip_filepath_rel << extract_dir_rel << detach;

#ifdef Q_OS_WIN
    QString unzip_binary_name = application_path + "/unzip.exe";
    if (!QFile(unzip_binary_name).exists()) return (QObject *)NULL;
#else
    QString unzip_binary_name = "unzip";
#endif

    if (extract_dir_rel.contains("..")) return (QObject *)NULL;

    QString zip_filepath_abs = jail_working_path + "/" + zip_filepath_rel;
    QString destdir_abs = jail_working_path + "/" + extract_dir_rel;

    QStringList args;
    args.append("-o"); // overwrite
    args.append(zip_filepath_abs);
    args.append("-d"); // extractdir
    args.append(destdir_abs);

    ProcessManager * p = new ProcessManager(this, unzip_binary_name, args, detach);
    return p;
}

#ifdef Q_OS_WIN
QObject *JsApi::runUpgrader(bool detach) {
    qDebug() << "Level1 [JsApi::runUpgrader]" << detach;

    QStringList args;
    args.append(jail_working_path + "/newbinary");
    args.append(application_path);

    ProcessManager * p = new ProcessManager(this, jail_working_path + "upgrader/upgrader.exe", args, detach);
    return p;
}
#endif

QByteArray JsApi::mapToByteArray(QVariantMap map) {
    QByteArray ba = QByteArray();
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << map;
    return ba;
}

QVariantMap JsApi::byteArrayToMap(QByteArray ba) {
    QDataStream stream(&ba, QIODevice::ReadOnly);
    QVariantMap map;
    stream >> map;
    return map;
}

QString JsApi::mapToHex(QVariantMap map) {
    return mapToByteArray(map).toHex();
}

QVariantMap JsApi::hexToMap(QString hex) {
    QByteArray ba = QByteArray::fromHex(hex.toLatin1());
    return byteArrayToMap(ba);
}

void JsApi::showOptionsDialog() {
    m_mainWindow->optionsDialog->loadSettings();
    m_mainWindow->optionsDialog->show();
}

void JsApi::onOptionsDialogAccepted() {
    emit optionsDialogAccepted();
}

void JsApi::onTrayMessageClicked() {
    emit trayMessageClicked();
}

void JsApi::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    qDebug() << "Level1 [JsApi::onTrayIconActivated]" << reason;
    emit trayIconActivated(reason);
}

void JsApi::showTrayMessage(QString title, QString msg, int type, int delay) {
    qDebug() << "Level1 [JsApi::showTrayMessage]" << title << msg;
    QSystemTrayIcon::MessageIcon sti = (QSystemTrayIcon::MessageIcon)type;
    m_mainWindow->m_trayIcon->showMessage(title, msg, sti, delay);
}

void JsApi::setTrayToolTip(QString tip) {
    qDebug() << "Level1 [JsApi::setTrayToolTip]";
    m_mainWindow->m_trayIcon->setToolTip(tip);
}

QVariant JsApi::getConfiguration(QString key) {
    return settings->value(key);
}

void JsApi::clearConfiguration(QString key) {
    if (
            // protected for security reasons
            key == "jail_working" ||
            key == "webinspector" ||
            key == "url" ||
            key == "fileread_jailed"
            )
        return;
    settings->remove(key);
}

void JsApi::setConfiguration(QString key, QVariant val) {
    if (
            // protected for security reasons
            key == "jail_working" ||
            key == "webinspector" ||
            key == "url" ||
            key == "fileread_jailed"
            )
        return;
    settings->setValue(key, val);
}

void JsApi::printDebug(QByteArray input) {
    printf("PRINT DEBUG %s", input.data());
}


QVariantMap JsApi::fileHashSetup(QString path, int hashtype) {
    QVariantMap map;
    qDebug() << "Level2 [JsApi::fileHashSetup]" << path << hashtype;

    if (path.contains("..")) {
        // protect against sneaky people
        map.insert("status", "Error");
        map.insert("info", "pathContainsDotDot");
        return map;
    }
    if (settings->value("fileread_jailed").toString() == "true") {
        path.prepend(jail_working_path);
    }
    m_fileHashFile = new QFile(path);
    if (!m_fileHashFile->exists()) {
        map.insert("status", "Error");
        map.insert("info", "fileNotExisting");
        return map;
    }
    if (!m_fileHashFile->open(QIODevice::ReadOnly)) {
        map.insert("status", "Error");
        map.insert("info", "fileCannotOpen");
        return map;
    }

    m_fileHashCrypto = new QCryptographicHash((QCryptographicHash::Algorithm)hashtype);
    m_fileHashCrypto->reset();
    map.insert("status", "OK");
    map.insert("size", m_fileHashFile->size());
    return map;
}

QVariantMap JsApi::fileHashDo(qint64 chunksize) {
    qDebug() << "Level2 [JsApi::fileHashDo]" << chunksize;
    QVariantMap map;
    QByteArray buf = m_fileHashFile->read(chunksize);
    qint64 read_bytes = buf.length();
    m_fileHashCrypto->addData(buf.data(), read_bytes);
    map.insert("status", "OK");
    map.insert("read_bytes", read_bytes);
    map.insert("file_pos", m_fileHashFile->pos());
    return map;
}

QVariantMap JsApi::fileHashDone() {
    QVariantMap map;
    QString result = QString(m_fileHashCrypto->result().toHex());
    m_fileHashFile->close();
    map.insert("status", "OK");
    map.insert("result", result);
    return map;
}

QString JsApi::getHash(QString string, int type) {
    QString result;
    QByteArray ba;
    QByteArray h;
    ba = string.toLatin1();
    h = QCryptographicHash::hash(ba, (QCryptographicHash::Algorithm)type);
    result = QString(h.toHex());
    return result;
}

QString JsApi::getHashFromHexStr(QString string_hex, int type) {
    QString result;
    QByteArray ba;
    QByteArray h;
    ba = QByteArray().fromHex(string_hex.toLatin1());
    h = QCryptographicHash::hash(ba, (QCryptographicHash::Algorithm)type);
    result = QString(h.toHex());
    return result;
}

void JsApi::debug(QString str) {
    qDebug() << str;
}

QObject * JsApi::createClient(QString id, QString location) {
    Client * c = new Client(this, id, location);
    return c;
}

void JsApi::playSound(QString name) {
    qDebug() << "Level1 [JsApi::playSound]";
#ifdef Q_OS_LINUX
    QStringList args;
    args.append(name);
    ProcessManager *p = new ProcessManager(this, "aplay", args);
    p->run();
#else
    QSound::play(name);
#endif
}


QStringList JsApi::getCmdlineArgs() {
    return QApplication::arguments();
}


QVariantMap JsApi::getStaticPaths() {
    QVariantMap info;
    info.insert("home_path", home_path);
    info.insert("settings_path", settings->fileName());
    info.insert("application_path", application_path);
    return info;
}

void JsApi::windowSetFlags(int flgs) {
    m_mainWindow->setWindowFlags(Qt::WindowFlags(flgs));
}

int JsApi::windowGetState() {
    return m_mainWindow->windowState();
}

void JsApi::windowSetState(int stat) {
    Qt::WindowState state = (Qt::WindowState)stat;
    m_mainWindow->setWindowState(state);
}

void JsApi::windowActivate() {
    m_mainWindow->activateWindow();
}

void JsApi::windowRaise() {
    m_mainWindow->raise();
}

void JsApi::windowClose() {
    m_mainWindow->close();
}

void JsApi::windowShow() {
    m_mainWindow->show();
}

void JsApi::windowShowNormal() {
    m_mainWindow->showNormal();
}

void JsApi::windowMinimize() {
    m_mainWindow->setWindowState(Qt::WindowMinimized);
    settings->setValue("minified_state", "true");
}

QString JsApi::getVersion() {
    return m_mainWindow->m_version;
}

/* ----------- Generic File System API begin ----------------- */

qint64 JsApi::fileSize(QString infilepath_abs_or_rel) {
    QString infilepath_abs;
    if (infilepath_abs_or_rel.contains("..")) return -4;
    if (settings->value("fileread_jailed").toString() == "true") {
        infilepath_abs = jail_working_path + infilepath_abs_or_rel;
    } else {
        infilepath_abs = infilepath_abs_or_rel;
    }
    QFile f(infilepath_abs);
    if (!f.exists()) return -1;
    qint64 size = f.size();
    f.close();
    return size;
}

bool JsApi::fileRename(QString infilepath_rel, QString outfilepath_rel) {
    if (infilepath_rel.contains("..")) return false;
    if (outfilepath_rel.contains("..")) return false;
    return QFile().rename(jail_working_path + infilepath_rel, jail_working_path + outfilepath_rel);
}

QString JsApi::relPathToJailedAbsPath(QString jail_type, QString path_rel) {
    QString result;
    if (path_rel.contains("..")) {
        return "";
    }
    if (jail_type == "working") {
        result = jail_working_path + path_rel;
    } else if (jail_type == "home") {
        result = home_path + path_rel;
    } else if (jail_type == "application") {
        result = application_path + path_rel;
    } else {
        return "";
    }
    return result;
}

qint64 JsApi::fileWrite(QString jail_type, QString filepath_rel, QString content, int open_mode) {
    QString filepath_abs = relPathToJailedAbsPath(jail_type, filepath_rel);
    if (filepath_abs == "") return -1;

    QFile f(filepath_abs);
    if (! f.open((QIODevice::OpenMode)open_mode)) return -1;
    qint64 written_bytes = f.write(content.toUtf8());
    f.close();
    return written_bytes;
}

QString JsApi::fileRead(QString jail_type, QString filepath_rel) {
    QString filepath_abs = relPathToJailedAbsPath(jail_type, filepath_rel);
    if (filepath_abs == "") return "";

    QFile f(filepath_abs);
    if (f.size() > 1000000) return "";
    if (! f.open(QIODevice::ReadOnly)) return "";
    QByteArray contents = f.readAll();
    f.close();
    return QString::fromUtf8(contents);
}


bool JsApi::fileCopy(QString infilepath_abs_or_rel, QString outfilepath_rel) {
    QString infilepath_abs;
    if (infilepath_abs_or_rel.contains("..")) return false;
    if (outfilepath_rel.contains("..")) return false;
    if (settings->value("fileread_jailed").toString() == "true") {
        infilepath_abs = jail_working_path + infilepath_abs_or_rel;
    } else {
        infilepath_abs = infilepath_abs_or_rel;
    }
    return QFile().copy(infilepath_abs, jail_working_path + outfilepath_rel);
}


bool JsApi::fileExists(QString jail_type, QString filepath_rel) {
    QString filepath_abs = relPathToJailedAbsPath(jail_type, filepath_rel);
    if (filepath_abs == "") return -1;

    QFile f(filepath_abs);
    bool result = f.exists();
    f.close();
    return result;
}


bool JsApi::fileRemove(QString jail_type, QString filepath_rel) {
    QString filepath_abs = relPathToJailedAbsPath(jail_type, filepath_rel);
    if (filepath_abs == "") return -1;
    return QDir().remove(filepath_abs);
}

QStringList JsApi::ls(QString jail_type, QString path_rel, QStringList wildcard, int filter) {
    QString path_abs = relPathToJailedAbsPath(jail_type, path_rel);
    if (path_abs == "") return QStringList();

    QDir d(path_abs);
    if (!d.exists()) return QStringList();

    d.setFilter((QDir::Filter)filter);
    return d.entryList(wildcard);
}


bool JsApi::dirRemove(QString jail_type, QString path_rel) {
    QString path_abs = relPathToJailedAbsPath(jail_type, path_rel);
    if (path_abs == "") return false;
    return QDir(path_abs).removeRecursively();
}

bool JsApi::dirMake(QString jail_type, QString path_rel) {
    QString path_abs = relPathToJailedAbsPath(jail_type, path_rel);
    if (path_abs == "") return false;
    return QDir().mkpath(path_abs);
}

bool JsApi::dirCopy(QString dst_path_rel, QString dst_jail_type, QString src_path_abs_or_rel, QString src_jail_type) {

    qDebug() << "Level3 [JsApi::dirCopy]" << dst_path_rel << dst_jail_type << src_path_abs_or_rel << src_jail_type;

    QString dst_path_abs = relPathToJailedAbsPath(dst_jail_type, dst_path_rel);
    qDebug() << "Level3 [JsApi::dirCopy] dst_path_abs" << dst_path_abs;
    if (dst_path_abs == "") return false;

    QString src_path_abs;
    if (settings->value("fileread_jailed").toString() == "true") {
        src_path_abs = relPathToJailedAbsPath(src_jail_type, src_path_abs_or_rel);
        qDebug() << "Level3 [JsApi::dirCopy] src_path_abs" << src_path_abs;
        if (src_path_abs == "") return false;
    } else {
        src_path_abs = src_path_abs_or_rel;
    }

    QDir dst_dir(dst_path_abs);
    if (! dst_dir.exists()) {
        qDebug() << "Level0 [JsApi::dirCopy] dst dir not existing";
        return false;
    }

    QDir src_dir(src_path_abs);
    if (! src_dir.exists()) {
        qDebug() << "Level0 [JsApi::dirCopy] src dir not existing";
        return false;
    }

    bool result;
    foreach (QString subdirpath, src_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString dst_path_new = dst_path_abs + "/" + subdirpath;
        result = QDir().mkpath(dst_path_new); // creates parent dirs
        if (!result) return false;
        qDebug() << "Level0 [JsApi::dirCopy]  creating dir" << result << dst_path_new;
        result = dirCopy(dst_path_rel + "/" + subdirpath,
                         dst_jail_type,
                         src_path_abs_or_rel + "/" + subdirpath,
                         src_jail_type); // recursion
        if (!result) return false;
    }

    foreach (QString fname, src_dir.entryList(QDir::Files)) {
        bool result = QFile::copy(src_path_abs + "/" + fname, dst_path_abs + "/" + fname);
        if (!result) return false;
        qDebug() << "Level0 [JsApi::dirCopy]   copy file" << result << fname;
    }
    return true;
}


/* ----------- Generic File System API end ----------------- */



/* This uses the Operating System libraries to get the time since the
 * user last touched the mouse or keyboard.
 */
double JsApi::getIdleTime() {
#ifdef Q_OS_WIN
    DWORD lastActiveTimestamp;
    LASTINPUTINFO inputInfo;
    inputInfo.cbSize = sizeof(LASTINPUTINFO);
    BOOL success = GetLastInputInfo(&inputInfo);

    if (!success) {
        DWORD err = GetLastError();
        qDebug() << "Level1 [JsApi::getIdleTime] ERROR:" << err;
    } else {
        lastActiveTimestamp = inputInfo.dwTime;
    }
    return lastActiveTimestamp / 1000;
#endif
#ifdef Q_OS_LINUX
    XScreenSaverInfo *info = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo(display, DefaultRootWindow(display), info);
    double idleTime;
    idleTime = info->idle;
    return idleTime / 1000;
#endif
#ifdef Q_OS_MACX
    CFTimeInterval timeSinceLastEvent = CGEventSourceSecondsSinceLastEventType(kCGEventSourceStateHIDSystemState, kCGAnyInputEventType);
    return timeSinceLastEvent;

#endif
}

QString JsApi::getSystemUserName() {
    QString name;
    qDebug() << "Level1 [JsApi::getSystemUserName]: called";
#ifdef Q_OS_WIN
    DWORD bufCharCount = 32767;
    TCHAR infoBuf[32767];
    if ( ! GetUserName(infoBuf, &bufCharCount) ) {
        qDebug() << "Level1 [JsApi::getSystemUserName]: error in WINDOWS API GetUserName";
        return "ERROR_IN_USER_NAME";
    } else {
        qDebug() << "Level1 [JsApi::getSystemUserName]: found out user name" << name;
        name = QString::fromWCharArray(infoBuf);
        return name;
    }
#endif
#ifdef Q_OS_LINUX
    name = QString(getenv("USERNAME"));
    return name;
#endif
#ifdef Q_OS_MACX
    name = QString(getenv("USER"));
    return name.toUtf8();
#endif
}

QString JsApi::getSystemComputerName() {
    QString name;
    qDebug() << "Level1 [JsApi::getSystemComputerName]: called";
#ifdef Q_OS_WIN
    DWORD bufCharCount = 32767;
    TCHAR infoBuf[32767];
    if ( ! GetComputerName(infoBuf, &bufCharCount) ) {
        qDebug() << "Level1 [JsApi::getSystemComputerName]: error in WINDOWS API GetComputerName";
        return "ERROR_IN_COMPUTER_NAME";
    } else {
        name = QString::fromWCharArray(infoBuf);
        qDebug() << "Level1 [JsApi::getSystemComputerName]: found out computer name" << name;
        return name;
    }
#else
    name = QHostInfo::localHostName();
    return name;
#endif
}

QStringList JsApi::getMyLocalIpAddresses() {
    QStringList address_list;
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        QString address_string = address.toString();
        qDebug() << "Level2 [JsApi::getMyLocalIpAddress]: IP" << address_string;
        address_list.append(address.toString());
    }
    return address_list;
}
