#include "udpserver.h"

UdpServer::UdpServer(QObject *parent) :
    QUdpSocket(parent)
{
    connect(this, &UdpServer::readyRead, this, &UdpServer::onReadyRead);
}



UdpServer::~UdpServer() {
    qDebug() << "Level0 [UdpServer::~UdpServer]";
}


bool UdpServer::start(quint16 port, QAbstractSocket::BindMode bindmode) {
    bool success;
    success = bind(port, bindmode);
    qDebug() << "Level5 [UdpServer:start] listening on port" << port << success;
    return success;
}

void UdpServer::stop() {
    qDebug() << "Level0 [UdpServer::stop] closing";
    close();
}

void UdpServer::onReadyRead() {
    QByteArray ba;
    QHostAddress ip;
    //QString::fromLatin1(ba.toHex());
    ba.resize(pendingDatagramSize());
    readDatagram(ba.data(), ba.size(), &ip);
    emit udpDatagramReceived(QString::fromLatin1(ba.toHex()), ip.toString());
    //return QString::fromLatin1(ba.toHex()), ip.toString()
}

qint64 UdpServer::sendDatagramFromHex(QString hex, QString host, qint64 port) {
    qDebug() << "Level2 [UdpServer::sendDatagram] Writing UDP datagram to" << host << port;
    QByteArray ba = QByteArray::fromHex(hex.toLatin1());
    qint64 bytes_sent = writeDatagram(ba, ba.length(), QHostAddress(host), port);
    return bytes_sent;
}
