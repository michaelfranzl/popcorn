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

#include "client.h"
#include <QDir>
#include <QUrl>
#include <QSslCipher>
#include <QSslKey>
#include <QSslConfiguration>
#include <QHostAddress>

Client::Client(QObject *parent, QString id, QString location) :
    QObject(parent)
{
    qDebug() << "Level1 [Client::initialize]:" << id << location;
    m_id = id;
    m_socket = NULL;

    m_buffer = QByteArray();
    m_dataSize = 0;
    m_writtenCounter = 0;
    m_readCounter = 0;

    m_binaryMode = false;
    m_fileMode = false;
    m_location = location;
}

Client::~Client() {
     qDebug() << "Level1 [Client::~Client]:" << m_id;
}

int Client::connectToServer(QString host, qint64 port) {
    qDebug() << "Level0 [Client::connectToServer] Connecting to" << host << port;
    m_socket->connectToHost(host, port);
    return m_socket->socketDescriptor();
}


void Client::startClientEncryption() {
    qDebug() << "Level1 [Client::startClientEncryption]" << m_id;
    m_socket->startClientEncryption();
}


void Client::startServerEncryption() {
    qDebug() << "Level1 [Client::startServerEncryption]" << m_id;
    m_socket->startServerEncryption();
}


QString Client::getPeerAddress() {
    QString peerAddress = m_socket->peerAddress().toString();
    qDebug() << "Level1 [Client::getPeerAddress]" << m_id << peerAddress;
    return peerAddress;
}

int Client::getState() {
    qDebug() << "Level1 [Client::getState]" << m_id;
    return (int)m_socket->state();
}


QString Client::createSocket(bool is_server, int sd) {
    qDebug() << "Level1 [Client::createSocket] begin" << m_id << is_server << sd;

    m_socket = new QSslSocket(this);

    /*
    connect(m_socket, &QSslSocket::encrypted, this, &Client::onSocketEncrypted);
    connect(m_socket, &QSslSocket::sslErrors, this, &Client::onSocketSslErrors);
    connect(m_socket, &QSslSocket::encryptedBytesWritten, this, &Client::onEncryptedBytesWritten);
    connect(m_socket, &QSslSocket::modeChanged, this, &Client::onModeChanged);
    connect(m_socket, &QSslSocket::bytesWritten, this, &Client::onBytesWritten);
    connect(m_socket, &QSslSocket::readyRead, this, &Client::onReadyRead);
    connect(m_socket, &QSslSocket::stateChanged, this, &Client::onSocketStateChange);
    */
    connect(m_socket, SIGNAL(encrypted()), this, SLOT(onSocketEncrypted()));
    connect(m_socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSocketSslErrors(QList<QSslError>)));
    connect(m_socket, SIGNAL(encryptedBytesWritten(qint64)), this, SLOT(onEncryptedBytesWritten(qint64)));
    connect(m_socket, SIGNAL(modeChanged(QSslSocket::SslMode)), this, SLOT(onModeChanged(QSslSocket::SslMode)));
    connect(m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(onBytesWritten(qint64)));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChange(QAbstractSocket::SocketState)));
    connect(m_socket, SIGNAL(connected()), this, SLOT(onSocketConnected()));

    if (is_server)
        // must come after singal connections
        m_socket->setSocketDescriptor(sd);

    if (m_location == "wan") {
        QString application_filepath = QCoreApplication::applicationFilePath();
        QString application_path = QFileInfo(application_filepath).canonicalPath();

        bool result = m_socket->addCaCertificates(application_path + "/certs/*.pem", QSsl::Pem, QRegExp::WildcardUnix);
        qDebug() << "Level1 [Client::createSocket] RESULT OF CERTS" << application_path + "/certs/*.pem" << result;
    }

    qDebug() << "Level1 [Client::createSocket] end" << m_id << is_server << m_socket->peerAddress();
    return m_socket->peerAddress().toString();
}



void Client::stop() {
    qDebug() << "Level0 [Client::stop]" << m_id;
    unsetFileMode();
    m_buffer.resize(0);
    m_socket->close();
    m_socket->deleteLater();
}


qint64 Client::writePlain(QString cmd) {
    qint64 written_bytes = m_socket->write(cmd.toUtf8());
    qDebug() << "Level4 [Client::writePlain] " << m_socket->socketDescriptor() << "======>" << cmd.trimmed();
    return written_bytes;
}


void Client::doFlush() {
    qDebug() << "Level2 [Client::doFlush]";
    m_socket->flush();
}


QString Client::getMessage() {
    qDebug() << "Level1 [Client::getMessage]";
    return QString::fromLatin1(m_buffer.toHex());
}


int Client::setMessage(QString hex) {
    qDebug() << "Level1 [Client::setMessage]";
    m_buffer.resize(0);
    m_buffer.append(QByteArray::fromHex(hex.toLatin1()));
    return m_buffer.length();
}


QVariantMap Client::setFileMode(QString type, QString filepath, qint64 pos) {
    qDebug() << "Level1 [Client::setFileMode]" << m_socket->socketDescriptor() << type << filepath << pos;
    QVariantMap info;

    if (m_fileMode == true) {
        info.insert("status", "Error");
        info.insert("info", "alreadyInFileMode");
        return info;
    }

    m_fileMode = true;
    m_fileModeType = type; // direction

    if (filepath.contains("..")) {
        // protect against sneaky people
        info.insert("status", "Error");
        info.insert("info", "FileContainsDotDot");
        return info;
    }

    filepath = QDir().cleanPath(filepath);
    qDebug() << "Level1 [Client::setFileMode] clean path" << filepath;

    if (m_fileModeType == "send") {
        // make sure it is really in the jail
        if (settings->value("fileread_jailed").toString() == "true") {
            filepath.prepend(jail_working_path);
        }
    } else {
        // receiving: we can put it anywhere in the working dir
        filepath.prepend(jail_working_path);
    }

    m_file = new QFile(filepath);

    if (m_fileModeType == "send" && !m_file->exists()) {
        info.insert("status", "Error");
        info.insert("info", "FileNotExistOrNotInJail");
        qDebug() << "Level1 [Client::setFileMode] done" << info;
        return info;
    }

    QIODevice::OpenMode open_mode;
    if (m_fileModeType == "receive") {
        open_mode = QIODevice::ReadWrite;
    } else {
        open_mode = QIODevice::ReadOnly;
    }

    if (!m_file->open(open_mode)) {
        info.insert("status", "Error");
        info.insert("info", "cannotOpen");
        qDebug() << "Level1 [Client::setFileMode] done" << info;
        return info;
    }

    m_file->seek(pos);
    info.insert("status", "OK");
    info.insert("size", m_file->size());
    info.insert("pos", m_file->pos());
    qDebug() << "Level1 [Client::setFileMode] done" << info;
    return info;
}


void Client::unsetFileMode() {
    qDebug() << "Level2 [Client::unsetFileMode]" << m_id << "closing file, mode is" << m_fileModeType;
    if (m_fileMode == false)
        return;

    m_file->close();
    m_file->deleteLater();
    m_fileModeType = "";
    m_fileMode = false;
}


qint64 Client::writeBinary(qint64 chunksize) {
    qDebug() << "Level1 [Client::writeBinary]" << m_id << "chunksize=" << chunksize;
    if (m_fileMode) {
        m_writtenCounter += m_socket->write(m_file->read(chunksize));
        qDebug() << "Level1 [Client::writeBinary] fileMode. Wrote total" << m_writtenCounter << "now at file POS" << m_file->pos();

    } else {
        m_writtenCounter += m_socket->write(m_buffer);
        qDebug() << "Level1 [Client::writeBinary] messageMode. Wrote" << m_buffer.length() << "total" << m_writtenCounter;
    }
    return m_writtenCounter;
}


void Client::onBytesWritten(qint64 size) {
    qDebug() << "Level1 [Client::onBytesWritten]" << m_id << "nbytes_now=" << size;
    emit bytesWritten(size);
}


void Client::onReadyRead() {
    QByteArray ba;
    ba = m_socket->readAll();
    m_readCounter += ba.length();

    if (!m_binaryMode) {
        // COMMAND LINE MODE
        QString str = QString::fromUtf8(ba.trimmed());
        QStringList cmds = str.split("\n");
        for (int k = 0; k < cmds.length(); k++) {
            qDebug() << "Level1 [Client::onReadyRead]" << m_id << "<=====" << cmds.at(k);
            emit readPlain(cmds.at(k));
        }

    } else {
        // BINARY MODE
        if (m_fileMode) {
            if (m_fileModeType == "receive") {
                qDebug() << "Level1 [Client::onReadyRead]" << m_id << "WRITING" << ba.size() << "TO FILE" << m_file->size() << "at POS" << m_file->pos();
                m_file->write(ba);
            } else {
                QString str = QString::fromUtf8(ba.trimmed());
                QStringList cmds = str.split("\n");
                for (int k = 0; k < cmds.length(); k++) {
                    qDebug() << "Level1 [Client::onReadyRead]" << m_id << "FILE TRANSFER FEEDBACK         <=====" << cmds.at(k);
                    emit readBinaryFeedback(cmds.at(k));
                }

            }
        } else {
            // JS message
            m_buffer.append(ba);
        }

        qDebug() << "Level1 [Client::onReadyRead]" << m_id << "         <====== now" << ba.length() << "total" << m_readCounter;
        emit readBinary(ba.length());
    }
}


void Client::setBinary(qint64 size) {
    qDebug() << "Level2 [Client::setBinary]" << m_id;
    m_dataSize = size;
    m_writtenCounter = 0;
    m_readCounter = 0;
    m_binaryMode = true;    
}


void Client::unsetBinary() {
    qDebug() << "Level2 [Client::unsetBinary]" << m_id;
    QByteArray tmp = m_socket->readAll(); // empty buffer
    m_buffer.resize(0);
    m_dataSize = 0;
    m_writtenCounter = 0;
    m_readCounter = 0;
    m_binaryMode = false;
}


void Client::onSocketStateChange(QAbstractSocket::SocketState state) {
    qDebug() << "Level2 [Client::onSocketStateChange]" << m_id << state << m_socket->peerAddress();
    emit socketStateChange((int)state);
}


void Client::resume() {
    qDebug() << "Level1 [Client::resume]" << m_id;
    m_socket->resume();

}

void Client::onEncryptedBytesWritten(qint64 size) {
    emit encryptedBytesWritten(size);
}


void Client::onModeChanged(QSslSocket::SslMode mode) {
    qDebug() << "Level1 [Client::onModeChanged]" << m_id << mode;
    emit modeChanged((int)mode);
}


void Client::onSocketSslErrors(QList<QSslError> errors) {
    qDebug() << "Level1 [Client::onSocketSslErrors]" << m_id << errors;
    QVariantMap map;
    for (int i = 0; i < errors.length(); i++) {
        map.insert(QString::number(i), errors.at(i).errorString());
    }
    emit socketErrors(map);
}

void Client::onSocketEncrypted() {
    qDebug() << "Level1 [Client::onSocketEncrypted]" << m_id;
    emit socketEncrypted();
}

void Client::doIgnoreSslErrors() {
    qDebug() << "Level1 [Client::doIgnoreSslErrors]" << m_id;
    m_socket->ignoreSslErrors();
}


QVariantMap Client::getInfo() {
    QVariantMap result;

    QVariantMap certs;
    QVariantMap sessionCipher;

    QList<QSslCertificate> cert_chain = m_socket->peerCertificateChain();
    for (int i = 0; i < cert_chain.length(); i++) {
        QVariantMap obj;
        QSslCertificate crt = cert_chain.at(i);
        QSslKey pubkey = crt.publicKey();

        obj.insert("digest", QString(crt.digest().toHex()));
        obj.insert("effectiveDate", crt.effectiveDate().toString("yyyyMMddHHmmss"));
        obj.insert("expiryDate", crt.expiryDate().toString("yyyyMMddHHmmss"));
        obj.insert("serialNumber", QString(crt.serialNumber().toHex()));
        obj.insert("pem", QString(crt.toPem()));
        obj.insert("text", crt.toText());
        obj.insert("version", QString(crt.version()));
        obj.insert("publicKeyLength", pubkey.length());
        obj.insert("publicKeyDer", QString(pubkey.toDer().toHex()));
        obj.insert("publicKeyPem", QString(pubkey.toPem()));
        obj.insert("publicKeyAlgorithm", (int)pubkey.algorithm());
        obj.insert("subjectOrganization", crt.subjectInfo(QSslCertificate::Organization));
        obj.insert("subjectCommonName", crt.subjectInfo(QSslCertificate::CommonName));
        obj.insert("subjectLocalityName", crt.subjectInfo(QSslCertificate::LocalityName));
        obj.insert("subjectOrganizationalUnitName", crt.subjectInfo(QSslCertificate::OrganizationalUnitName));
        obj.insert("subjectCountryName", crt.subjectInfo(QSslCertificate::CountryName));
        obj.insert("subjectStateOrProvinceName", crt.subjectInfo(QSslCertificate::StateOrProvinceName));

        certs.insert(QString::number(i), obj);
    }

    QSslCipher c = m_socket->sessionCipher();
    sessionCipher.insert("name", c.name());
    sessionCipher.insert("authenticationMethod", c.authenticationMethod());
    sessionCipher.insert("encryptionMethod", c.encryptionMethod());
    sessionCipher.insert("keyExchangeMethod", c.keyExchangeMethod());
    sessionCipher.insert("protocolString", c.protocolString());
    sessionCipher.insert("supportedBits", c.supportedBits());
    sessionCipher.insert("usedBits", c.usedBits());

    result.insert("certs", certs);
    result.insert("sessionCipher", sessionCipher);
    return result;
}


