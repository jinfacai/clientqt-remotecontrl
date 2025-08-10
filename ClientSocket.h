#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QByteArray>
#include <QQueue>
#include "Packet.h"
#include "Command.h"

class ClientSocket : public QObject{
    Q_OBJECT
public:
    explicit ClientSocket(QObject* parent = nullptr);//防止隐式转换
    ~ClientSocket();

    //连接到服务器
    bool connectToServer(const QString&host, quint16 port);

    //断开连接
    void disconnectFromServer();

    //发送数据包
    bool sendPacket(const CPacket& packet);

    //发送命令
    //bool sendCommand(std::unique_ptr<CCommand> command);
    bool sendCommand(uint16_t cmd, const QByteArray& data);

    // 发送文本消息
    bool sendTextMessage(const QString& message);

    // 发送文件
    bool sendFile(const QString& filename, const QByteArray& fileData);

    //检查连接状态
    bool isConnected() const;

    // 获取最后错误信息
    QString getLastError() const;

signals:
    // 连接状态变化信号
    void connectionStateChanged(bool connected);

    // 接收到数据包信号
    void packetReceived(const CPacket& packet);

    // 接收到文本消息信号
    void textMessageReceived(const QString& message);

    // 接收到文件信号
    void fileReceived(const QString& filename, const QByteArray& fileData);

    // 错误信号
    void errorOccurred(const QString& error);

private slots:
    // 处理连接状态变化
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

    // 处理数据接收
    void onReadyRead();

    // 处理重连定时器
    void onReconnectTimer();

private:
    QTcpSocket* m_socket;
    QTimer* m_reconnectTimer;
    QByteArray m_receiveBuffer;
    QQueue<CPacket> m_sendQueue;

    QString m_host;
    quint16 m_port;
    bool m_autoReconnect;
    int m_reconnectInterval;

    // 处理接收到的数据
    void processReceivedData();

    // 处理接收到的命令
    void handleCommand(std::unique_ptr<CCommand> command);

    // 发送队列中的数据包
    void sendQueuedPackets();

    // 设置自动重连
    void setupAutoReconnect();

    // 停止自动重连
    void stopAutoReconnect();
};

#endif // CLIENTSOCKET_H
