#include "TestUI.h"
#include <QApplication>
#include <QGroupBox>
#include <QFile>
#include <QTextCursor>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QDateTime>

TestUI::TestUI(QWidget *parent)
    : QWidget(parent)
    , m_networkClient(new ClientSocket(this))
{
    setWindowTitle("Packet & NetworkClient 测试界面");
    resize(600, 500);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 连接区域
    QGroupBox* connectionGroup = new QGroupBox("连接设置");
    QHBoxLayout* connectionLayout = new QHBoxLayout(connectionGroup);

    m_ipEdit = new QLineEdit("127.0.0.1");
    m_portEdit = new QLineEdit("8080");
    m_connectButton = new QPushButton("连接");
    m_disconnectButton = new QPushButton("断开");
    m_disconnectButton->setEnabled(false);

    connectionLayout->addWidget(new QLabel("IP:"));
    connectionLayout->addWidget(m_ipEdit);
    connectionLayout->addWidget(new QLabel("端口:"));
    connectionLayout->addWidget(m_portEdit);
    connectionLayout->addWidget(m_connectButton);
    connectionLayout->addWidget(m_disconnectButton);

    mainLayout->addWidget(connectionGroup);

    // 状态显示
    m_statusLabel = new QLabel("未连接");
    mainLayout->addWidget(m_statusLabel);

    // 文本发送区域
    QGroupBox* textGroup = new QGroupBox("文本消息");
    QVBoxLayout* textLayout = new QVBoxLayout(textGroup);

    m_messageEdit = new QTextEdit();
    m_messageEdit->setMaximumHeight(100);
    m_sendTextButton = new QPushButton("发送文本");

    textLayout->addWidget(m_messageEdit);
    textLayout->addWidget(m_sendTextButton);

    mainLayout->addWidget(textGroup);

    // 文件发送区域
    QGroupBox* fileGroup = new QGroupBox("文件发送");
    QVBoxLayout* fileLayout = new QVBoxLayout(fileGroup);

    QHBoxLayout* filePathLayout = new QHBoxLayout();
    m_filePathEdit = new QLineEdit();
    m_filePathEdit->setPlaceholderText("选择要发送的文件...");
    m_selectFileButton = new QPushButton("选择文件");
    m_sendFileButton = new QPushButton("发送文件");

    filePathLayout->addWidget(m_filePathEdit);
    filePathLayout->addWidget(m_selectFileButton);
    filePathLayout->addWidget(m_sendFileButton);

    fileLayout->addLayout(filePathLayout);

    mainLayout->addWidget(fileGroup);

    // 日志显示区域
    QGroupBox* logGroup = new QGroupBox("通信日志");
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);

    m_logEdit = new QTextEdit();
    m_logEdit->setReadOnly(true);

    logLayout->addWidget(m_logEdit);

    mainLayout->addWidget(logGroup);

    // 连接信号槽
    connect(m_connectButton, &QPushButton::clicked, this, &TestUI::onConnectButtonClicked);
    connect(m_disconnectButton, &QPushButton::clicked, this, &TestUI::onDisconnectButtonClicked);
    connect(m_sendTextButton, &QPushButton::clicked, this, &TestUI::onSendTextButtonClicked);
    connect(m_selectFileButton, &QPushButton::clicked, this, &TestUI::onSelectFileButtonClicked);
    connect(m_sendFileButton, &QPushButton::clicked, this, &TestUI::onSendFileButtonClicked);

    // 连接客户端信号
    connect(m_networkClient, &ClientSocket::connectionStateChanged, this, &TestUI::onConnectionStateChanged);
    connect(m_networkClient, &ClientSocket::packetReceived, this, &TestUI::onPacketReceived);
    connect(m_networkClient, &ClientSocket::errorOccurred, this, &TestUI::onNetworkError);

    logMessage("测试界面已初始化");
}

TestUI::~TestUI()
{
}

void TestUI::onConnectButtonClicked()
{
    QString ip = m_ipEdit->text().trimmed();
    QString portStr = m_portEdit->text().trimmed();

    if (ip.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "IP地址不能为空");
        return;
    }

    bool ok;
    quint16 port = portStr.toUShort(&ok);
    if (!ok || port == 0) {
        QMessageBox::warning(this, "输入错误", "端口号必须是有效的数字");
        return;
    }

    logMessage(QString("正在连接到 %1:%2...").arg(ip).arg(port));

    if (m_networkClient->connectToServer(ip, port)) {
        m_connectButton->setEnabled(false);
        m_disconnectButton->setEnabled(true);
        updateStatus("正在连接...");
    } else {
        logMessage("连接启动失败");
    }
}

void TestUI::onDisconnectButtonClicked()
{
    m_networkClient->disconnectFromServer();
    m_connectButton->setEnabled(true);
    m_disconnectButton->setEnabled(false);
    updateStatus("已断开连接");
    logMessage("已断开连接");
}

void TestUI::onSendTextButtonClicked()
{
    QString text = m_messageEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        QMessageBox::information(this, "提示", "请输入要发送的文本");
        return;
    }

    sendTextMessage(text);
    m_messageEdit->clear();
}

void TestUI::onSelectFileButtonClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择要发送的文件");
    if (!filePath.isEmpty()) {
        m_selectedFilePath = filePath;
        m_filePathEdit->setText(QFileInfo(filePath).fileName());
        logMessage(QString("已选择文件: %1").arg(QFileInfo(filePath).fileName()));
    }
}

void TestUI::onSendFileButtonClicked()
{
    if (m_selectedFilePath.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要发送的文件");
        return;
    }

    sendFileMessage(m_selectedFilePath);
}

void TestUI::onConnectionStateChanged(bool connected)
{
    if (connected) {
        updateStatus("已连接");
        logMessage("连接成功");
        m_connectButton->setEnabled(false);
        m_disconnectButton->setEnabled(true);
    } else {
        updateStatus("未连接");
        logMessage("连接断开");
        m_connectButton->setEnabled(true);
        m_disconnectButton->setEnabled(false);
    }
}

void TestUI::onPacketReceived(const CPacket& packet)
{
    uint16_t cmd = packet.getCmd();
    QByteArray data = packet.getData();

    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    switch (cmd) {
    case 1: // CMD_CHAT_MESSAGE - 文本消息
    {
        QString text = QString::fromUtf8(data);
        logMessage(QString("[%1] 收到文本: %2").arg(currentTime, text));
    }
    break;
    case 2: // CMD_FILE_START - 文件开始
    {
        QString filename = QString::fromUtf8(data);
        // 仅使用文件名，避免路径穿越
        filename = QFileInfo(filename).fileName();
        logMessage(QString("[%1] 收到文件开始: %2").arg(currentTime, filename));
    }
    break;
    case 3: // CMD_FILE_DATA - 文件数据
        logMessage(QString("[%1] 收到文件数据块，大小: %2 字节").arg(currentTime).arg(data.size()));
        break;
    case 4: // CMD_FILE_COMPLETE - 文件结束
        logMessage(QString("[%1] 收到文件结束消息").arg(currentTime));
        break;
    case 1981: // CMD_TEST_CONNECT - 测试连接
    {
        QString response = QString::fromUtf8(data);
        logMessage(QString("[%1] 收到测试连接响应: %2").arg(currentTime, response));
    }
    break;
    default:
        logMessage(QString("[%1] 收到未知命令: %2").arg(currentTime).arg(cmd));
        break;
    }
}

void TestUI::onNetworkError(const QString& error)
{
    logMessage(QString("网络错误: %1").arg(error));
    QMessageBox::critical(this, "网络错误", error);
}

void TestUI::sendTextMessage(const QString& text)
{
    if (!m_networkClient->isConnected()) {
        QMessageBox::warning(this, "发送失败", "未连接到服务器");
        return;
    }

    // 直接使用命令接口发送文本消息
    if (m_networkClient->sendCommand(static_cast<quint16>(CCommand::Type::TEXT_MESSAGE), text.toUtf8())) {
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        logMessage(QString("[%1] 发送文本: %2").arg(currentTime, text));
    } else {
        QMessageBox::critical(this, "发送失败", "无法发送消息到服务器");
    }
}

void TestUI::sendFileMessage(const QString& filePath)
{
    if (!m_networkClient->isConnected()) {
        QMessageBox::warning(this, "发送失败", "未连接到服务器");
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "错误", "无法打开文件");
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    QString fileName = QFileInfo(filePath).fileName();

    // 发送文件开始
    if (!m_networkClient->sendCommand(static_cast<quint16>(CCommand::Type::FILE_START), fileName.toUtf8())) {
        QMessageBox::critical(this, "发送失败", "无法发送文件开始到服务器");
        return;
    }
    // 分块发送数据
    const int chunkSize = 1024;
    for (int i = 0; i < fileData.size(); i += chunkSize) {
        QByteArray chunk = fileData.mid(i, chunkSize);
        if (!m_networkClient->sendCommand(static_cast<quint16>(CCommand::Type::FILE_DATA), chunk)) {
            QMessageBox::critical(this, "发送失败", "文件数据发送中断");
            return;
        }
    }
    // 发送完成
    if (m_networkClient->sendCommand(static_cast<quint16>(CCommand::Type::FILE_COMPLETE), QByteArray())) {
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        logMessage(QString("[%1] 发送文件: %2 (%3 字节)").arg(currentTime, fileName).arg(fileData.size()));
    } else {
        QMessageBox::critical(this, "发送失败", "无法发送文件到服务器");
    }
}

void TestUI::logMessage(const QString& message)
{
    m_logEdit->append(message);
    QTextCursor cursor = m_logEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logEdit->setTextCursor(cursor);
}

void TestUI::updateStatus(const QString& status)
{
    m_statusLabel->setText(status);
}
