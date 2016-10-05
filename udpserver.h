#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QUdpSocket>

class UdpServer : public QUdpSocket
{
    Q_OBJECT
public:
    explicit UdpServer(QObject *parent = 0);
    ~UdpServer();

signals:
    void udpDatagramReceived(QString hex, QString ip);

private slots:
    void onReadyRead();

public slots:
    bool start(quint16 port, QAbstractSocket::BindMode bindmode = QAbstractSocket::DontShareAddress);
    void stop();
    qint64 sendDatagramFromHex(QString hex, QString host, qint64 port);
    //QString getHexDatagram();

};

#endif // UDPSERVER_H
