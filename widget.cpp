#include "widget.h"
#include "ui_widget.h"
#include <QIcon>
#include <QMessageBox>
#include <QTcpSocket>
#include <QRegularExpression>
#include <QHostAddress>
#include <QRegularExpressionValidator>
#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QCloseEvent>
#include <QTcpServer>
#include <QHostAddress>
#include <QDebug>
#include <QFileInfo>
#include <QtEndian>
#include <QTextBrowser>
#include <QTimer>
#include <QNetworkProxy>

#define MY_PROTOCOOL_VERSION 1 //当前协议版本
#define MSG_TYPE_TEXT 1 //文本消息
#define MSG_TYPE_FILE 2 //文件消息
#define BUFFER_SIZE 4096
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , socket(new QTcpSocket(this))
    , serverSocket(nullptr)
    , blockSize(0)
{
    ui->setupUi(this);
    // 设置窗口图标
    QIcon icon(":/icons/Iconka-Cat-Commerce-Client.ico");
    this->setWindowIcon(icon);
    ui->filescrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);//文件栏隐藏滑动条，超出时显示
    ui->serveBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);//服务器消息栏隐藏滑动条，超出时显示
    ui->clientTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);//客户发送消息栏隐藏滑动条，超出时显示
    ui->progressBar1->setVisible(false);//文件1进度隐藏，有文件传输时显示
    ui->progressBar2->setVisible(false);//文件2进度隐藏，有文件传输时显示
    ui->progressBar3->setVisible(false);//文件3进度隐藏，有文件传输时显示
    ui->acceptButton1->setVisible(false);//接受按键1隐藏，有文件传输时显示
    ui->saveasButton1->setVisible(false);//另存为按键1隐藏，有文件传输时显示
    ui->refuseButton1->setVisible(false);//拒绝按键1隐藏，有文件传输时显示
    ui->acceptButton2->setVisible(false);//接受按键2隐藏，有文件传输时显示
    ui->saveasButton2->setVisible(false);//另存为按键2隐藏，有文件传输时显示
    ui->refuseButton2->setVisible(false);//拒绝按键2隐藏，有文件传输时显示
    ui->acceptButton3->setVisible(false);//接受按键3隐藏，有文件传输时显示
    ui->saveasButton3->setVisible(false);//另存为按键3隐藏，有文件传输时显示
    ui->refuseButton3->setVisible(false);//拒绝按键3隐藏，有文件传输时显示
    ui->filenamelabel1->setVisible(false);//文件名称1隐藏，有文件传输时显示
    ui->filenamelabel2->setVisible(false);//文件名称2隐藏，有文件传输时显示
    ui->filenamelabel3->setVisible(false);//文件名称3隐藏，有文件传输时显示
    QPixmap iconPix1(":/icons/Treetog-I-Documents.ico");
    ui->fileicolabel1->setPixmap(iconPix1);
    ui->fileicolabel1->setVisible(false);//文件图标1隐藏，有文件传输时显示
    QPixmap iconPix2(":/icons/Treetog-I-Documents.ico");
    ui->fileicolabel2->setPixmap(iconPix2);
    ui->fileicolabel2->setVisible(false);//文件图标2隐藏，有文件传输时显示
    QPixmap iconPix3(":/icons/Treetog-I-Documents.ico");
    ui->fileicolabel3->setPixmap(iconPix3);
    ui->fileicolabel3->setVisible(false);//文件图标3隐藏，有文件传输时显示
    ui->filesizelabel1->setVisible(false);//文件大小1隐藏，有文件传输时显示
    ui->filesizelabel2->setVisible(false);//文件大小2隐藏，有文件传输时显示
    ui->filesizelabel3->setVisible(false);//文件大小3隐藏，有文件传输时显示
    connect(ui->acceptButton1, &QPushButton::clicked, this, &Widget::onacceptButton1clicked);//acceptButton1被点击
    connect(ui->acceptButton2, &QPushButton::clicked, this, &Widget::onacceptButton2clicked);//acceptButton2被点击
    connect(ui->acceptButton3, &QPushButton::clicked, this, &Widget::onacceptButton3clicked);//acceptButton3被点击
    connect(ui->refuseButton1, &QPushButton::clicked, this, &Widget::onrefuseButton1clicked);//refuseButton1被点击
    connect(ui->refuseButton2, &QPushButton::clicked, this, &Widget::onrefuseButton2clicked);//refuseButton2被点击
    connect(ui->refuseButton3, &QPushButton::clicked, this, &Widget::onrefuseButton3clicked);//refuseButton3被点击
    connect(ui->saveasButton1, &QPushButton::clicked, this, &Widget::onsaveasButton1clicked);//saveasButton1被点击
    connect(ui->saveasButton2, &QPushButton::clicked, this, &Widget::onsaveasButton2clicked);//saveasButton2被点击
    connect(ui->saveasButton3, &QPushButton::clicked, this, &Widget::onsaveasButton3clicked);//saveasButton3被点击
    connect(socket, &QTcpSocket::connected, this, &Widget::onConnected);//与服务器socket连接，连接成功
    //connect(socket, &QTcpSocket::disconnected, this, &Widget::onDisconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &Widget::onError);//与服务器socket连接，连接失败报错
    connect(socket, &QTcpSocket::readyRead, this, &Widget::onSocketReadyRead);//与服务器读取信息连接
    connect(socket, &QTcpSocket::errorOccurred,this, &Widget::onSendButtonError);//发送服务器消息失败报错
    connect(socket, &QTcpSocket::errorOccurred,this, &Widget::onSendFileError);//发送文件消息失败报错
    connect(ui->serveButton, &QPushButton::clicked, this, &Widget::connectToServer);//当连接服务器被点击时
    connect(ui->selectfileButton, &QPushButton::clicked,this, &Widget::selectFile);//选择文件被点击
    connect(ui->servefileButton,  &QPushButton::clicked,this, &Widget::serveFile);//发送文件被点击
    connect(ui->sendButton, &QPushButton::clicked, this, &Widget::onSendButtonClicked);//发送消息被点击
    fileSlotsOccupied.resize(3, false);//固定 3 个传输槽,初始化槽位占用状态为 false（未占用）
    m_pendingFileDatas.resize(3);//初始化3个待处理文件的数据结构，用于存储对应的文件接收信息
    // 定义3个FileSlot结构体，分别关联对应的UI控件和文件传输状态变量
    FileSlot fileSlotArray[3] = {
        {
            currentFileName1, totalFileSize1, currentFile1, currentFileSize1, isFilePending1,
            ui->fileicolabel1, ui->filenamelabel1, ui->filesizelabel1,
            ui->progressBar1, ui->acceptButton1, ui->saveasButton1, ui->refuseButton1
        },
        {
            currentFileName2, totalFileSize2, currentFile2, currentFileSize2, isFilePending2,
            ui->fileicolabel2, ui->filenamelabel2, ui->filesizelabel2,
            ui->progressBar2, ui->acceptButton2, ui->saveasButton2, ui->refuseButton2
        },
        {
            currentFileName3, totalFileSize3, currentFile3, currentFileSize3, isFilePending3,
            ui->fileicolabel3, ui->filenamelabel3, ui->filesizelabel3,
            ui->progressBar3, ui->acceptButton3, ui->saveasButton3, ui->refuseButton3
        }
    };
    // 将数组转换为 QVector 方便动态管理
    fileSlots = QVector<FileSlot>(fileSlotArray, fileSlotArray + 3);
    // 统一连接信号
    for (int i = 0; i < 3; ++i) {
        connect(fileSlots[i].acceptButton, &QPushButton::clicked, this, [this, i]() { handleAccept(i); });
        connect(fileSlots[i].refuseButton, &QPushButton::clicked, this, [this, i]() { handleRefuse(i); });
        connect(fileSlots[i].saveasButton, &QPushButton::clicked, this, [this, i]() { handleSaveAs(i); });
    }
}

Widget::~Widget()
{
    delete ui;
}

//ip是否是ipv4或者ipv6
bool isValidIp(const QString &ip) {
    if (ip.trimmed().isEmpty()) return false;
    static const QRegularExpression re(
        R"(^((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)$)"//判断255.255.255.255
        );
    return re.match(ip).hasMatch();//是否匹配成功
}


//判断ip port是否正确
bool Widget::validateInput(QString& ip, quint16& port)
{

    QString ipInput = ui->serveipLineEdit->text();//读取IP地址

    QString portInput = ui->portLineEdit->text();//读取端口号

    // 检查 IP 地址是否为空
    if (ipInput.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入有效的 IP 地址");
        return false;
    }
    // 检查 IP 地址格式是否正确
    if (!isValidIp(ipInput)) {
        QMessageBox::warning(this, "输入错误", "请输入有效的 IP 地址（IPv4 格式）");
        return false;
    }
    // 检查端口号是否为空
    if (portInput.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入有效的端口号");
        return false;
    }

    bool ok;
    port = portInput.toUShort(&ok);//ushort<=65535

    // 检查端口号是否有效
    if (!ok || port < 1) {
        QMessageBox::warning(this, "输入错误",
                             "端口号必须是 1 到 65535 之间的有效整数");
        return false;
    }
    // 将有效的 IP 地址赋值给传入的引用变量
    ip = ipInput;
    return true;
}

//连接到服务器
void Widget::connectToServer()
{
    QString ip;
    quint16 port;
    // 验证输入的 IP 地址和端口号
    if (!validateInput(ip, port)) {
        return;
    }
    //QTcpSocket *socket = new QTcpSocket(this);

    // 禁用代理（一定要在连接之前设置）
    socket->setProxy(QNetworkProxy::NoProxy);

    // 开始连接
    //socket->connectToHost(ip, port);

    // 尝试连接到指定的服务器地址和端口
    socket->connectToHost(ip, port);//接受并解析
    //qt才有的
}
// 处理与服务器连接成功的事件
void Widget::onConnected()
{
    // 弹出消息框提示连接成功
    QMessageBox::information(this, "连接成功", "已成功连接到服务器");
    //socket->disconnectFromHost();关闭连接
}

void Widget::onDisconnected()
{
    // 可添加断开连接后的处理逻辑
}

// 处理与服务器连接时发生的错误
void Widget::onError(QAbstractSocket::SocketError socketError)
{
    QString errorMessage;
    switch (socketError) {
    case QAbstractSocket::ConnectionRefusedError:
        errorMessage = "连接被拒绝，请检查服务器是否正在运行。";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorMessage = "远程主机已关闭连接。";
        break;
    case QAbstractSocket::HostNotFoundError:
        errorMessage = "未找到主机，请检查 IP 地址是否正确。";
        break;
    case QAbstractSocket::SocketTimeoutError:
        errorMessage = "连接超时，请检查网络连接。";
        break;
    default:
        errorMessage = "发生未知错误：" + socket->errorString();
        break;
    }
    // 弹出消息框显示错误信息
    QMessageBox::critical(this, "连接失败", errorMessage);
}
// 选择要发送的文件
void Widget::selectFile()
{
    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "请选择要发送的文件",
        QString(),             // 默认路径
        "All Files (*)"    // 过滤器
        );
    // 如果用户没有选择文件，则返回
    if (fileName.isEmpty())
        return;
    selectedFilePath = fileName; // 保存选择的文件路径

    QString baseName = QFileInfo(fileName).fileName();//显示文件名+拓展名
    ui->fileBrowser->setText(baseName);//在文件browser中显示文件名
}
// 计算校验和函数（与服务端逻辑一致）
quint64 calculateChecksum(const QByteArray &data, size_t len) {
    quint64 sum = 0;
    // 保证len不超过data.size()
    size_t length = qMin(len, static_cast<size_t>(data.size()));

    for (size_t i = 0; i < length; ++i) {
        sum += static_cast<quint8>(data[i]);
    }
    return sum;
}

// 发送选择的文件到服务器
void Widget::serveFile()
{
    if (selectedFilePath.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择文件");
        return;
    }

    QFile file(selectedFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "错误", "无法打开文件");
        return;
    }

    // 读取整个文件内容
    QByteArray fileData = file.readAll();
    file.close();
    int fileSize = fileData.size();
    QByteArray fileName = QFileInfo(file).fileName().toUtf8();
    int filenameLen = fileName.size();
    //文件为空
    if (fileSize == 0) {
        QMessageBox::information(this, "提示", "文件内容为空，无需发送");
        return;
    }
    FileHeader filehead = {
        .version = MY_PROTOCOOL_VERSION,
        .msg_type = MSG_TYPE_FILE,
        .filename_len = static_cast<uint32_t>(qToBigEndian(filenameLen)),
        .file_size = static_cast<uint64_t>(qToBigEndian(fileSize)),
        .text_size = 0
    };
    socket->write(reinterpret_cast<const char*>(&filehead), sizeof(FileHeader));
    //定义数据包总大小
    int totalSize = sizeof(PacketHeader) + filenameLen + fileSize;
    if(totalSize >= BUFFER_SIZE){
        int currentSize = 0;
        int dataLen = BUFFER_SIZE - sizeof(PacketHeader) - filenameLen;
        //构造包头
        PacketHeader packetheader = {
            .data_len = static_cast<uint32_t>(qToBigEndian(dataLen)),
            .sSum = 0
        };
        QByteArray buffer[BUFFER_SIZE];
        int reSize = totalSize - currentSize;
        while(currentSize < totalSize){
            QByteArray packet;
            //包头
            packet.append(reinterpret_cast<const char*>(&packetheader), sizeof(PacketHeader));
            //文件名
            packet.append(fileName, filenameLen);
            //文件数据
            packet.append(fileData+currentSize,dataLen);
            // 计算并设置校验和
            quint64 checksum = calculateChecksum(packet,packet.size());
            checksum = static_cast<uint64_t>(qToBigEndian(checksum));
            // 将校验和写入包头部（假设PacketHeader中有checksum字段）
            memcpy(reinterpret_cast<char*>(&packetheader) +
                       offsetof(PacketHeader, sSum), &checksum, sizeof(checksum));
            memcpy(packet.data(), &packetheader, sizeof(PacketHeader));
            //写入缓存区
            buffer[0].append(packet);
            // 发送数据包
            qint64 bytesWritten = socket->write(buffer[0]);
            if (bytesWritten != packet.size()) {
                // 发送失败
                qWarning() << "发送失败：仅发送了" << bytesWritten << "字节，预期" << packet.size() << "字节";
                qWarning() << "Socket 错误：" << socket->errorString();
                return;
            }
            currentSize +=  dataLen;
            //如果buffersize剩下空间比剩余数据大
            //直接传输剩余数据
            if(dataLen > reSize){
                dataLen = reSize;
            }
            //继续传输datalen大小
            else{
                continue;
            }

        }

    }
    else{
        //int currentSize = 0;
        int dataLen = fileSize;
        //构造包头
        PacketHeader packetheader = {
            .data_len = static_cast<uint32_t>(qToBigEndian(dataLen)),
            .sSum = 0
        };
        QByteArray buffer[BUFFER_SIZE];
        QByteArray packet;
        //包头
        packet.append(reinterpret_cast<const char*>(&packetheader), sizeof(PacketHeader));
        //文件名
        packet.append(fileName, filenameLen);
        //文本数据
        packet.append(fileData,dataLen);
        // 计算并设置校验和
        quint64 checksum = calculateChecksum(packet, packet.size());
        checksum = static_cast<uint64_t>(qToBigEndian(checksum));
        // 将校验和写入包头部（假设PacketHeader中有checksum字段）
        memcpy(reinterpret_cast<char*>(&packetheader) +
                   offsetof(PacketHeader, sSum), &checksum, sizeof(checksum));
        memcpy(packet.data(), &packetheader, sizeof(PacketHeader));
        //写入缓存区
        buffer[0].append(packet);
        // 发送数据包
        qint64 bytesWritten = socket->write(buffer[0]);
        //检查连接状态
        if (socket->state() != QAbstractSocket::ConnectedState) {
            QMessageBox::warning(this, "连接错误", "未连接到服务器");
            return;
        }
        if (bytesWritten != packet.size()) {
            // 发送失败
            qWarning() << "发送失败：仅发送了" << bytesWritten << "字节，预期" << packet.size() << "字节";
            qWarning() << "Socket 错误：" << socket->errorString();
            return;
        }

    }
    // 最后一包发送完毕后，更新 UI 提示
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QString justName = QFileInfo(selectedFilePath).fileName();
        ui->serveBrowser->append(QString("[我] 发送了文件：%1").arg(justName));
        ui->fileBrowser->clear();
        QMessageBox::information(this, "完成", "文件已全部发送");
    } else {
        onSendFileError(socket->error());
    }
}
// 处理文件发送过程中发生的错误
void Widget::onSendFileError(QAbstractSocket::SocketError socketError)
{
    QString errorMessage;
    switch (socketError) {
    case QAbstractSocket::ConnectionRefusedError:
        errorMessage = "连接被拒绝，可能服务器未运行或端口未开放。";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorMessage = "远程主机已关闭连接，无法完成文件发送。";
        break;
    case QAbstractSocket::HostNotFoundError:
        errorMessage = "未找到主机，请检查 IP 地址是否正确。";
        break;
    case QAbstractSocket::SocketTimeoutError:
        errorMessage = "连接超时，请检查网络连接或服务器响应情况。";
        break;
    case QAbstractSocket::NetworkError:
        errorMessage = "网络错误，请检查网络连接是否正常。";
        break;
    case QAbstractSocket::AddressInUseError:
        errorMessage = "地址正在使用中，请检查端口是否被占用。";
        break;
    case QAbstractSocket::SocketAccessError:
        errorMessage = "没有足够的权限进行操作，请检查程序权限。";
        break;
    case QAbstractSocket::SocketResourceError:
        errorMessage = "系统资源不足，无法完成操作。";
        break;
    default:
        errorMessage = "发生未知错误：" + socket->errorString();
        break;
    }
    QMessageBox::critical(this, QStringLiteral("文件发送失败"), QStringLiteral("文件发送失败：%1").arg(errorMessage));
}
// 处理窗口关闭事件
void Widget::closeEvent(QCloseEvent *event)
{
    auto st = socket->state();
    // 只有在“已连接”或“正在连接”这两种状态下，才尝试断开并弹提示
    if (st == QAbstractSocket::ConnectedState || st == QAbstractSocket::ConnectingState) {
        // 优雅断开
        socket->disconnectFromHost();
        // 等待连接断开，最多等待 2000 毫秒
        if (!socket->waitForDisconnected(2000)) {
            socket->abort();//强制关闭套接字连接
        }

        // 给用户一个断开连接的提示
        QMessageBox::information(this,"已断开","已与服务器断开连接");
    }
    // 如果根本没连上（UnconnectedState），就直接关窗，不弹提示也不调用断开
    event->accept();//当前事件已经被处理完毕
}
// 处理客户端发送文本消息给服务器
void Widget::onSendButtonClicked()
{
    //获取用户输入的文本
    QString txt = ui->clientTextEdit->toPlainText().trimmed();
    if (txt.isEmpty()) {
        QMessageBox::information(this, "提示", "请输入非空消息");
        return;
    }
    QByteArray textData = txt.toUtf8();
    int txtsize = textData.size();
    if (textData.size() > BUFFER_SIZE) { return; }
    FileHeader filehead;
    filehead.version = MY_PROTOCOOL_VERSION;
    filehead.msg_type = MSG_TYPE_TEXT;
    filehead.filename_len = 0;
    filehead.file_size = 0;
    filehead.text_size = qToBigEndian(static_cast<quint64>(txtsize));  // 64位网络字节序
    QByteArray headBuffer(reinterpret_cast<const char*>(&filehead), sizeof(FileHeader));
   // headbuffer.append(reinterpret_cast<const char*>(&filehead), sizeof(FileHeader));
    qint64 bytesWritten = socket->write(headBuffer);
    if (bytesWritten != headBuffer.size()) {
        QMessageBox::warning(this, "发送错误",
                             QString("文件头发送失败: 预期%1字节, 实际%2字节")
                                 .arg(headBuffer.size()).arg(bytesWritten));
        return;
    }
    //定义总大小
    int totalSize = sizeof(PacketHeader) +  txtsize;
    if(totalSize >= BUFFER_SIZE){
        int currentSize = 0;
        int dataLen = BUFFER_SIZE - sizeof(PacketHeader);
        //构造包头
        PacketHeader packetheader = {
            .data_len = static_cast<uint32_t>(qToBigEndian(dataLen)),
            .sSum = 0
        };
        QByteArray buffer[BUFFER_SIZE];
        int reSize = totalSize - currentSize;
        while(currentSize < totalSize){
            QByteArray packet;
            //包头
            packet.append(reinterpret_cast<const char*>(&packetheader), sizeof(PacketHeader));
            //文本数据
            packet.append(textData+currentSize,dataLen);
            // 计算并设置校验和
            quint64 checksum = calculateChecksum(packet, packet.size());
            checksum = static_cast<uint64_t>(qToBigEndian(checksum));
            // 将校验和写入包头部（假设PacketHeader中有checksum字段）
            memcpy(reinterpret_cast<char*>(&packetheader) +
                       offsetof(PacketHeader, sSum), &checksum, sizeof(checksum));
            memcpy(packet.data(), &packetheader, sizeof(PacketHeader));
            //写入缓存区
            buffer[0].append(packet);
            // 发送数据包
            qint64 bytesWritten = socket->write(buffer[0]);
            //检查连接状态
            if (socket->state() != QAbstractSocket::ConnectedState) {
                QMessageBox::warning(this, "连接错误", "未连接到服务器");
                return;
            }
            if (bytesWritten != packet.size()) {
                // 发送失败
                qWarning() << "发送失败：仅发送了" << bytesWritten << "字节，预期" << packet.size() << "字节";
                qWarning() << "Socket 错误：" << socket->errorString();
                return;
            }
            currentSize +=  dataLen;
            //如果buffersize剩下空间比剩余数据大
            //直接传输剩余数据
            if(dataLen > reSize){
                dataLen = reSize;
            }
            //继续传输datalen大小
            else{
                continue;
            }

        }

    }
    else{
        //int currentSize = 0;
        int dataLen = txtsize;
        //构造包头
        PacketHeader packetheader = {
            .data_len = static_cast<uint32_t>(qToBigEndian(dataLen)),
            .sSum = 0
        };
        QByteArray buffer[BUFFER_SIZE];
        QByteArray packet;
        //包头
        packet.append(reinterpret_cast<const char*>(&packetheader), sizeof(PacketHeader));
        //文本数据
        packet.append(textData,dataLen);
        // 计算并设置校验和
        quint64 checksum = calculateChecksum(packet, packet.size());
        checksum = static_cast<uint64_t>(qToBigEndian(checksum));
        // 将校验和写入包头部（假设PacketHeader中有checksum字段）
        memcpy(reinterpret_cast<char*>(&packetheader) +
                   offsetof(PacketHeader, sSum), &checksum, sizeof(checksum));
        memcpy(packet.data(), &packetheader, sizeof(PacketHeader));
        //写入缓存区
        buffer[0].append(packet);
        // 发送数据包
        qint64 bytesWritten = socket->write(buffer[0]);
        //检查连接状态
        if (socket->state() != QAbstractSocket::ConnectedState) {
            QMessageBox::warning(this, "连接错误", "未连接到服务器");
            return;
        }

    }
    // 更新UI
    ui->clientTextEdit->clear();
    ui->serveBrowser->append(QString("[我] %1").arg(txt));
}
// 处理发送消息过程中发生的错误
void Widget::onSendButtonError(QAbstractSocket::SocketError socketError)
{
    QString errorMessage;
    switch (socketError) {
    case QAbstractSocket::ConnectionRefusedError:
        errorMessage = "连接被拒绝，可能服务器未运行或端口未开放。";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorMessage = "远程主机已关闭连接，无法发送消息。";
        break;
    case QAbstractSocket::HostNotFoundError:
        errorMessage = "未找到主机，请检查 IP 地址是否正确。";
        break;
    case QAbstractSocket::SocketTimeoutError:
        errorMessage = "连接超时，请检查网络连接或服务器响应情况。";
        break;
    case QAbstractSocket::NetworkError:
        errorMessage = "网络错误，请检查网络连接是否正常。";
        break;
    case QAbstractSocket::AddressInUseError:
        errorMessage = "地址正在使用中，请检查端口是否被占用。";
        break;
    case QAbstractSocket::SocketAccessError:
        errorMessage = "没有足够的权限进行操作，请检查程序权限。";
        break;
    case QAbstractSocket::SocketResourceError:
        errorMessage = "系统资源不足，无法完成操作。";
        break;
    default:
        errorMessage = "发生未知错误：" + socket->errorString();
        break;
    }
    QMessageBox::critical(this, QStringLiteral("发送失败"), QStringLiteral("消息发送失败：%1").arg(errorMessage));
}
//文件处理
void Widget::handleFile(QString& currentFileName,qint64& totalFileSize,bool& isFilePending,QLabel* fileIconLabel,
                        QLabel* fileNameLabel,QLabel* fileSizeLabel,QPushButton* acceptButton,QPushButton* saveAsButton,
                        QPushButton* refuseButton,QProgressBar* progressBar,QTextBrowser* serveBrowser) {
    if (!isFilePending) {
        // 隐藏所有组件（可用于清理残留状态）
        fileIconLabel->setVisible(false);
        fileNameLabel->setVisible(false);
        fileSizeLabel->setVisible(false);
        acceptButton->setVisible(false);
        saveAsButton->setVisible(false);
        refuseButton->setVisible(false);
        progressBar->setVisible(false);
        return;
    }

    // 显示文件基本信息
    fileIconLabel->setVisible(true);
    fileNameLabel->setText(currentFileName);
    fileNameLabel->setVisible(true);

    //格式化文件大小并显示
    QString fileSizeText;
    double size = static_cast<double>(totalFileSize);
    if (size >= 1024.0 * 1024.0) {
        fileSizeText = QString("%.2f MB").arg(size / (1024.0 * 1024.0));
    } else if (size >= 1024.0) {
        fileSizeText = QString("%.2f KB").arg(size / 1024.0);
    } else {
        fileSizeText = QString("%1 B").arg(static_cast<quint64>(size));
    }
    fileSizeLabel->setText(QString("大小：%1").arg(fileSizeText));
    fileSizeLabel->setVisible(true);

    // 显示操作按钮
    acceptButton->setVisible(true);
    saveAsButton->setVisible(true);
    refuseButton->setVisible(true);

    // 初始化进度条
    progressBar->setMaximum(static_cast<int>(totalFileSize));
    progressBar->setValue(0); // 初始进度为0
    progressBar->setVisible(false); // 后续在点击"接受"后再显示

    // 更新 serveBrowser显示信息
    serveBrowser->append(QString("[服务器] 发送了文件：%1（大小：%2）").arg(currentFileName, fileSizeText));

}

// 接受文件处理
void Widget::handleAccept(int slotIndex) {
    // 检查槽位索引是否合法
    if (slotIndex < 0 || slotIndex >= 3) return;
    // 通过索引获取对应的文件接收槽的引用，便于后续操作该槽的数据和控件
    FileSlot& slot = fileSlots[slotIndex];
    //判断当前文件接收槽是否空闲
    if (!slot.isFilePending || slot.currentFile) return;
    // 获取当前文件的默认保存路径（假设为当前目录下的同名文件）
    QString savePath = QDir::currentPath() + QDir::separator() + slot.currentFileName;

    // 打开文件进行写入
    QScopedPointer<QFile> file(new QFile(savePath, this));
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "错误", "无法打开文件保存");
        handleRefuse(slotIndex);
        return;
    }
    //将缓存在内存中的文件数据写入对应槽的文件中。
    qint64 written = file->write(m_pendingFileDatas[slotIndex]);
    if (written == -1) {
        QMessageBox::critical(this, "错误", "文件写入失败");
        handleRefuse(slotIndex);
        return;
    }
    // 更新 currentFileSize 和进度条
    slot.currentFileSize = written;
    slot.progressBar->setMaximum(slot.totalFileSize);
    slot.progressBar->setValue(slot.currentFileSize);
    slot.progressBar->setVisible(true); // 显示进度条

    // 延迟 500ms 后自动隐藏进度条
    QTimer::singleShot(500, this, [progressBar = slot.progressBar]() {
        progressBar->setVisible(false);
    });

    // 重置状态
    slot.isFilePending = false;
    slot.totalFileSize = 0;
    slot.currentFileName.clear();
    fileSlotsOccupied[slotIndex] = false; // 释放槽位

    // 隐藏UI组件
    slot.fileIconLabel->setVisible(false);
    slot.fileNameLabel->setVisible(false);
    slot.fileSizeLabel->setVisible(false);
    slot.progressBar->setVisible(false);
    slot.acceptButton->setVisible(false);
    slot.saveasButton->setVisible(false);
    slot.refuseButton->setVisible(false);

    QMessageBox::information(this, "完成", "文件已保存至：\n" + savePath);
}

// 拒绝文件处理
void Widget::handleRefuse(int slotIndex) {
    // 检查槽位索引是否合法
    if (slotIndex < 0 || slotIndex >= 3) return;
    // 通过索引获取对应的文件接收槽的引用，便于后续操作该槽的数据和控件
    FileSlot& slot = fileSlots[slotIndex];
    //判断当前文件接收槽是否空闲
    if (!slot.isFilePending && !slot.currentFile) return;
    // 清理文件资源
    if (slot.currentFile) {
        slot.currentFile->close();
        QFile::remove(slot.currentFile->fileName());
        delete slot.currentFile;
        slot.currentFile = nullptr;
    }

    // 重置状态
    slot.isFilePending = false;
    slot.totalFileSize = 0;
    slot.currentFileName.clear();

    // 隐藏UI组件
    slot.fileIconLabel->setVisible(false);
    slot.fileNameLabel->setVisible(false);
    slot.fileSizeLabel->setVisible(false);
    slot.progressBar->setVisible(false);
    slot.acceptButton->setVisible(false);
    slot.saveasButton->setVisible(false);
    slot.refuseButton->setVisible(false);
}

// 另存为处理
void Widget::handleSaveAs(int slotIndex) {
    // 检查槽位索引是否合法
    if (slotIndex < 0 || slotIndex >= 3) return;
    // 通过索引获取对应的文件接收槽的引用，便于后续操作该槽的数据和控件
    FileSlot& slot = fileSlots[slotIndex];
    //判断当前文件接收槽是否空闲
    if (!slot.isFilePending || slot.currentFile) return;

    QString savePath = QFileDialog::getSaveFileName(this, "另存为", slot.currentFileName);
    if (savePath.isEmpty()) return;

    //尝试写入文件
    QScopedPointer<QFile> file(new QFile(savePath, this));
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "错误", "无法保存文件");
        handleRefuse(slotIndex);
        return;
    }

    // 入内存中的缓存数据
    qint64 written = file->write(m_pendingFileDatas[slotIndex]);
    if (written == -1) {
        QMessageBox::critical(this, "错误", "文件写入失败");
        handleRefuse(slotIndex);
        return;
    }

    // 更新进度条显示逻辑
    slot.currentFileSize = written;
    slot.progressBar->setMaximum(slot.totalFileSize);
    slot.progressBar->setValue(slot.currentFileSize);
    slot.progressBar->setVisible(true);

    // 延迟隐藏进度条（保持与接受操作一致）
    QTimer::singleShot(500, this, [progressBar = slot.progressBar]() {
        progressBar->setVisible(false);
    });

    // 清理槽位状态和UI组件
    slot.isFilePending = false;
    slot.totalFileSize = 0;
    slot.currentFileName.clear();
    fileSlotsOccupied[slotIndex] = false;

    //隐藏UI控件（注意顺序，避免闪烁）
    slot.fileIconLabel->setVisible(false);
    slot.fileNameLabel->setVisible(false);
    slot.fileSizeLabel->setVisible(false);
    slot.acceptButton->setVisible(false);
    slot.saveasButton->setVisible(false);
    slot.refuseButton->setVisible(false);

    //提示保存成功
    QMessageBox::information(this, "完成", "文件已保存至：\n" + savePath);
}

// 文件1 接受按钮处理
void Widget::onacceptButton1clicked() {
    handleAccept(0);
}
// 文件2 接受按钮处理
void Widget::onacceptButton2clicked() {
    handleAccept(1);
}
// 文件3 接受按钮处理
void Widget::onacceptButton3clicked() {
    handleAccept(2);
}
// 文件1 拒绝按钮处理
void Widget::onrefuseButton1clicked() {
    handleRefuse(0);
}
// 文件2 拒绝按钮处理
void Widget::onrefuseButton2clicked() {
    handleRefuse(1);
}
// 文件3 拒绝按钮处理
void Widget::onrefuseButton3clicked() {
    handleRefuse(2);
}
// 文件1 另存为按钮处理
void Widget::onsaveasButton1clicked() {
    handleSaveAs(0);
}
// 文件2 另存为按钮处理
void Widget::onsaveasButton2clicked() {
    handleSaveAs(1);
}
// 文件3 另存为按钮处理
void Widget::onsaveasButton3clicked() {
    handleSaveAs(2);
}

    // Socket数据可读时触发此函数
void Widget::onSocketReadyRead() {
        // 读取所有新数据并追加到缓冲区
        //QByteArray recvBuffer[BUFFER_SIZE];
        FileHeader filehead;
        QByteArray fileheadData = socket->read(sizeof(FileHeader));
        //报错
        memcpy(&filehead, fileheadData.constData(), sizeof(FileHeader));

        uint8_t verSion = filehead.version;
        uint8_t msgType = filehead.msg_type;
        uint32_t filenameLen = qFromBigEndian(filehead.filename_len);
        uint64_t fileSize = qFromBigEndian(filehead.file_size);
        uint64_t textSize = qFromBigEndian(filehead.text_size);

        // 验证协议版本和消息类型uint32_t filenameLen
        if (verSion != MY_PROTOCOOL_VERSION) {
            qWarning() << "无效的文件协议，丢弃数据";
            return;
        }
        if (msgType != MSG_TYPE_TEXT && msgType != MSG_TYPE_FILE) {
            qWarning() << "无效的消息内容，丢弃数据";
            return;
        }
        if (filenameLen <= 0 || filenameLen > 256) {
            qWarning() << "文件名长度异常：" << filenameLen;
            return;
        }
        if (msgType == MSG_TYPE_FILE && fileSize == 0) {
            qWarning() << "文件内容为空：" << fileSize;
            return;
        }
        if (msgType == MSG_TYPE_TEXT && textSize == 0) {
            qWarning() << "文本消息内容为空：" << textSize;
            return;
        }
        if (msgType == MSG_TYPE_TEXT) {
            handleTextMessage(textSize);
        }
        else if (msgType == MSG_TYPE_FILE) {
            handleFileMessage(filenameLen, fileSize);
        }
        else {
            qDebug() << "未知消息类型:" << msgType;
        }

}

//处理文本消息
void Widget::handleTextMessage(uint64_t txtsize) {
    PacketHeader packethead;
    qint64 totalsize = static_cast<qint64>(sizeof(PacketHeader) + txtsize);
    qint64 currentsize = BUFFER_SIZE;
    int recvsize = 0;
    if (totalsize > BUFFER_SIZE) {
        while (recvsize < totalsize) {
            QByteArray packtxtData = socket->read(BUFFER_SIZE);
            memcpy(&packethead, packtxtData.constData(), sizeof(PacketHeader));
            uint32_t datalen = qFromBigEndian(packethead.data_len);
            uint64_t checkSum = qFromBigEndian(packethead.sSum);
            //复制数据包
            QByteArray verifyData = packtxtData;
            size_t sSumOffset = offsetof(PacketHeader, sSum);
            //memset(verifyData.constData() + sSumOffset, 0, sizeof(uint64_t));//置零
            memset(verifyData.data() + sSumOffset, 0, sizeof(uint64_t));
            //计算整个数据包校验和
            uint64_t currentcheckSum = calculateChecksum(verifyData.constData(), verifyData.size());
            if (currentcheckSum != checkSum) {
                qDebug() << "Checksum mismatch! Corrupted data.";
            }
            char txtbuffer[BUFFER_SIZE];
            memcpy(txtbuffer, packtxtData.constData() + sizeof(PacketHeader), datalen);
            //uint64_t currentsum = calculateChecksum()
            recvsize += datalen;
            QString text = QString::fromUtf8(txtbuffer, datalen); // UTF-8编码

            ui->serveBrowser->append(QString("[服务器] %1").arg(text));
        }
    }
    else {
        QByteArray packtxtData = socket->read(BUFFER_SIZE);
        memcpy(&packethead, packtxtData.constData(), sizeof(PacketHeader));
        uint32_t datalen = qFromBigEndian(packethead.data_len);
        uint64_t checkSum = qFromBigEndian(packethead.sSum);
        //复制数据包
        QByteArray verifyData = packtxtData;
        size_t sSumOffset = offsetof(PacketHeader, sSum);
        memset(verifyData.data() + sSumOffset, 0, sizeof(uint64_t));//置零
        //计算整个数据包校验和
        uint64_t currentcheckSum = calculateChecksum(verifyData.constData(), verifyData.size());
        if (currentcheckSum != checkSum) {
            qDebug() << "Checksum mismatch! Corrupted data.";
        }
        char txtbuffer[BUFFER_SIZE];
        memcpy(txtbuffer, packtxtData.constData() + sizeof(PacketHeader), datalen);
        QString text = QString::fromUtf8(txtbuffer, datalen); // UTF-8编码
        ui->serveBrowser->append(QString("[服务器] %1").arg(text));
    }
}

//处理文件消息
void Widget::handleFileMessage(uint32_t flnamelen, uint64_t flsize) {
    QString filename;
    char flbuffer;
    PacketHeader packethead;
    int totalsize = sizeof(PacketHeader) + flnamelen + flsize;
    int currentsize = BUFFER_SIZE;
    int recvsize = 0;
    if (totalsize > BUFFER_SIZE) {
        while (recvsize < totalsize) {
            QByteArray packtxtData = socket->read(BUFFER_SIZE);
            memcpy(&packethead, packtxtData.constData(), sizeof(PacketHeader));
            uint32_t datalen = qFromBigEndian(packethead.data_len);
            uint64_t checkSum = qFromBigEndian(packethead.sSum);
            //复制数据包
            QByteArray verifyData = packtxtData;
            size_t sSumOffset = offsetof(PacketHeader, sSum);
            memset(verifyData.data() + sSumOffset, 0, sizeof(uint64_t));//置零
            //计算整个数据包校验和
            uint64_t currentcheckSum = calculateChecksum(verifyData.constData(), verifyData.size());
            if (currentcheckSum != checkSum) {
                qDebug() << "Checksum mismatch! Corrupted data.";
            }
            char flname[BUFFER_SIZE];
            memcpy(flname, packtxtData.constData() + sizeof(PacketHeader), flnamelen);
            QString filename = QString::fromUtf8(flname, datalen);
            char flbuffer[BUFFER_SIZE];
            memcpy(flbuffer, packtxtData.constData() + sizeof(PacketHeader) + flnamelen, datalen);
            recvsize += datalen;
            //QString text = QString::fromUtf8(txtbuffer, datalen); // UTF-8编码
            //ui->serveBrowser->append(QString("[服务器] %1").arg(text));
        }
    }
    else {
        //读取数据包
        QByteArray packtxtData = socket->read(BUFFER_SIZE);
        //解析数据包
        memcpy(&packethead, packtxtData.constData(), sizeof(PacketHeader));
        uint32_t datalen = qFromBigEndian(packethead.data_len);
        uint64_t checkSum = qFromBigEndian(packethead.sSum);
        //复制数据包
        QByteArray verifyData = packtxtData;
        size_t sSumOffset = offsetof(PacketHeader, sSum);
        memset(verifyData.data() + sSumOffset, 0, sizeof(uint64_t));//置零
        //计算整个数据包校验和
        uint64_t currentcheckSum = calculateChecksum(verifyData.constData(), verifyData.size());
        if (currentcheckSum != checkSum) {
            qDebug() << "Checksum mismatch! Corrupted data.";
        }
        char flname[BUFFER_SIZE];
        memcpy(flname, packtxtData.constData() + sizeof(PacketHeader), flnamelen);
        QString filename = QString::fromUtf8(flname, datalen);
        char flbuffer[BUFFER_SIZE];
        memcpy(flbuffer, packtxtData.constData() + sizeof(PacketHeader) + flnamelen, datalen);
    }

     // 遍历3个槽位，寻找可用槽位
    for (int slotIndex = 0; slotIndex < 3; ++slotIndex) {
        // 获取槽位状态引用
        bool& isFilePending = (slotIndex == 0) ? isFilePending1 :
                                (slotIndex == 1) ? isFilePending2 : isFilePending3;
        QString& currentFileName = (slotIndex == 0) ? currentFileName1 :
                                     (slotIndex == 1) ? currentFileName2 : currentFileName3;
        qint64& totalFileSize = (slotIndex == 0) ? totalFileSize1 :
                                  (slotIndex == 1) ? totalFileSize2 : totalFileSize3;
        QFile*& currentFile = (slotIndex == 0) ? currentFile1 :
                                (slotIndex == 1) ? currentFile2 : currentFile3;
        qint64& currentFileSize = (slotIndex == 0) ? currentFileSize1 :
                                (slotIndex == 1) ? currentFileSize2 : currentFileSize3;

        // 检查槽位是否可用（未占用、无pending文件、无当前文件）
         if (!fileSlotsOccupied[slotIndex] && !isFilePending && currentFile == nullptr) {
            // 绑定文件信息到槽位
            currentFileName = filename;
            totalFileSize = flsize;
            isFilePending = true;
            fileSlotsOccupied[slotIndex] = true;
            //m_pendingFileDatas[slotIndex] = flbuffer; // 暂存文件内容

            // 获取UI组件指针
            QLabel* fileIconLabel = (slotIndex == 0) ? ui->fileicolabel1 :
                                (slotIndex == 1) ? ui->fileicolabel2 : ui->fileicolabel3;
            QLabel* fileNameLabel = (slotIndex == 0) ? ui->filenamelabel1 :
                                (slotIndex == 1) ? ui->filenamelabel2 : ui->filenamelabel3;
            QLabel* fileSizeLabel = (slotIndex == 0) ? ui->filesizelabel1 :
                                (slotIndex == 1) ? ui->filesizelabel2 : ui->filesizelabel3;
            QPushButton* acceptButton = (slotIndex == 0) ? ui->acceptButton1 :
                                (slotIndex == 1) ? ui->acceptButton2 : ui->acceptButton3;
            QPushButton* saveAsButton = (slotIndex == 0) ? ui->saveasButton1 :
                                 (slotIndex == 1) ? ui->saveasButton2 : ui->saveasButton3;
            QPushButton* refuseButton = (slotIndex == 0) ? ui->refuseButton1 :
                                 (slotIndex == 1) ? ui->refuseButton2 : ui->refuseButton3;
            QProgressBar* progressBar = (slotIndex == 0) ? ui->progressBar1 :
                                 (slotIndex == 1) ? ui->progressBar2 : ui->progressBar3;

         //调用 handleFile 函数来显示文件信息和更新 serveBrowser
         handleFile(
             currentFileName,
             totalFileSize,
             isFilePending,
             fileIconLabel,
             fileNameLabel,
             fileSizeLabel,
             acceptButton,
             saveAsButton,
             refuseButton,
             progressBar,
             ui->serveBrowser // 传递 serveBrowser 指针
              );

             break; // 找到可用槽位后跳出循环
         }
    }
}
