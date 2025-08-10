#include "ClientSocket.h"



ClientSocket::ClientSocket(QObject* parent)
    :QObject(parent)
    ,m_socket(nullptr)
    ,m_reconnectTimer(nullptr)
    ,m_port(0)
    ,m_autoReconnect(false)
    ,m_reconnectInterval(5000)
{
    m_socket = new QTcpSocket(this);
    //连接信号槽
    connect(m_socket,&QTcpSocket::connected,this,&ClientSocket::onConnected);
    connect(m_socket,&QTcpSocket::disconnected,this,&ClientSocket::onDisconnected);
    connect(m_socket,QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this,&ClientSocket::onError);
    connect(m_socket,&QTcpSocket::readyRead,this,&ClientSocket::onReadyRead);
    //创建重连定时器
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer,&QTimer::timeout,this,&ClientSocket::onReconnectTimer);
}

ClientSocket::~ClientSocket(){
    disconnectFromServer();
}

bool ClientSocket::connectToServer(const QString& host, quint16 port){
    if(m_socket->state() == QAbstractSocket::ConnectedState){
        qWarning()<<"Already connected to server";
    }
    m_host = host;
    m_port = port;
    qDebug()<<"Connecting to server:"<<host<<":"<<port;
    m_socket->connectToHost(host,port);
    return true;
}

void ClientSocket::disconnectFromServer(){
    stopAutoReconnect();
    if(m_socket->state() != QAbstractSocket::UnconnectedState){
        m_socket->disconnectFromHost();
    }
    m_receiveBuffer.clear();
    m_sendQueue.clear();
}

void ClientSocket::stopAutoReconnect(){
    m_reconnectTimer->stop();
}

bool ClientSocket::isConnected()const{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

QString ClientSocket::getLastError()const{
    return m_socket->errorString();
}

void ClientSocket::onConnected(){
    qDebug()<<"Connected to server successfully";
    emit connectionStateChanged(true);
    // 发送队列中的数据包
    sendQueuedPackets();
}

void ClientSocket::onDisconnected(){
    qDebug()<<"Disconnected from server";
    emit connectionStateChanged(false);
    //启动自动重连
    if(m_autoReconnect && !m_host.isEmpty()){
        setupAutoReconnect();
    }
}

void ClientSocket::setupAutoReconnect(){
    if(!m_reconnectTimer->isActive()){
        m_reconnectTimer->start(m_reconnectInterval);
    }
}

void ClientSocket::onError(QAbstractSocket::SocketError error){
    QString errorMsg = m_socket->errorString();
    qWarning()<<"Socket error:"<< error << "-"<<errorMsg;
    emit errorOccurred(errorMsg);
}

bool ClientSocket::sendPacket(const CPacket& packet){
    if(!isConnected()){
        qWarning()<<"Not connected to server, packet queued";
        m_sendQueue.enqueue(packet);//packet加入队列
        return false;
    }
    QByteArray data(packet.Data(),packet.Size());
    qint64 bytesWrriten = m_socket->write(data);
    if(bytesWrriten == -1){
        qWarning()<<"Failed to send packet,error:"<<m_socket->errorString();
        return false;
    }
    qDebug() << "Sent packet, cmd:" << packet.getCmd() << "size:" << packet.Size() << "bytes";
    return true;
}

bool ClientSocket::sendCommand(uint16_t cmd, const QByteArray& data){
    CPacket packet(cmd,reinterpret_cast<const uint8_t*>(data.data()), data.size());
    return sendPacket(packet);
}

void ClientSocket::onReadyRead(){
    //读取所有可用数据
    QByteArray newData = m_socket->readAll();
    m_receiveBuffer.append(newData);
    qDebug()<<"Received"<<newData.size()<<"bytes, buffer size:"<<m_receiveBuffer.size();
    //处理接收到的数据
    processReceivedData();
}

void ClientSocket::processReceivedData(){
    while(m_receiveBuffer.size() >= 8){
        size_t remainingSize = m_receiveBuffer.size();
        CPacket packet(reinterpret_cast<const uint8_t*>(m_receiveBuffer.data()), remainingSize);
        if(remainingSize == 0){
            //解析失败，继续重试
            m_receiveBuffer.remove(0,1);
            continue;
        }
        //移除已处理的数据包
        int processedSize = m_receiveBuffer.size() - remainingSize;
        if(processedSize>0){
            m_receiveBuffer.remove(0, processedSize);
        }
        // 发送数据包接收信号
        emit packetReceived(packet);
    }
}

void ClientSocket::sendQueuedPackets()
{
    while (!m_sendQueue.isEmpty()) {
        CPacket packet = m_sendQueue.dequeue();
        sendPacket(packet);
    }
}


void ClientSocket::onReconnectTimer()
{
    if (!m_autoReconnect || m_host.isEmpty()) {
        return;
    }

    qDebug() << "Attempting to reconnect to server...";
    m_socket->connectToHost(m_host, m_port);
}

