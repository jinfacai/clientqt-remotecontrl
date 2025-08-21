#include "widget.h"
#include "ui_widget.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QIcon>
#include <QPixmap>
#include <QDebug>
#include <QTimer>

#include "ClientSocket.h"
#include "Command.h"
#include "Packet.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_networkClient(new ClientSocket(this))
{
    ui->setupUi(this);
    setWindowTitle("Chat Client (Not Connected)");

    // 网络客户端信号
    connect(m_networkClient, &ClientSocket::connectionStateChanged, this, &Widget::onConnectionStateChanged);
    connect(m_networkClient, &ClientSocket::packetReceived, this, &Widget::onPacketReceived);
    connect(m_networkClient, &ClientSocket::errorOccurred, this, &Widget::onNetworkError);

    // UI 按钮
    connect(ui->serveButton, &QPushButton::clicked, this, &Widget::connectToServer);
    connect(ui->sendButton, &QPushButton::clicked, this, &Widget::onSendButtonClicked);
    connect(ui->selectfileButton, &QPushButton::clicked, this, &Widget::selectFile);
    connect(ui->servefileButton, &QPushButton::clicked, this, &Widget::serveFile);

    // 图标
    QIcon icon(":/icons/Iconka-Cat-Commerce-Client.ico");
    setWindowIcon(icon);
    QPixmap iconPix(":/icons/Treetog-I-Documents.ico");

    //滚动条
    ui->filescrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->serveBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->clientTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 隐藏三个槽位
    auto hideSlot = [&](int idx){
        switch (idx) {
        case 0:
            ui->filenamelabel1->setVisible(false);
            ui->filesizelabel1->setVisible(false);
            ui->progressBar1->setVisible(false);
            ui->acceptButton1->setVisible(false);
            ui->saveasButton1->setVisible(false);
            ui->refuseButton1->setVisible(false);
            ui->fileicolabel1->setVisible(false);
            ui->fileicolabel1->setPixmap(iconPix);
            break;
        case 1:
            ui->filenamelabel2->setVisible(false);
            ui->filesizelabel2->setVisible(false);
            ui->progressBar2->setVisible(false);
            ui->acceptButton2->setVisible(false);
            ui->saveasButton2->setVisible(false);
            ui->refuseButton2->setVisible(false);
            ui->fileicolabel2->setVisible(false);
            ui->fileicolabel2->setPixmap(iconPix);
            break;
        case 2:
            ui->filenamelabel3->setVisible(false);
            ui->filesizelabel3->setVisible(false);
            ui->progressBar3->setVisible(false);
            ui->acceptButton3->setVisible(false);
            ui->saveasButton3->setVisible(false);
            ui->refuseButton3->setVisible(false);
            ui->fileicolabel3->setVisible(false);
            ui->fileicolabel3->setPixmap(iconPix);
            break;
        }
    };
    hideSlot(0); hideSlot(1); hideSlot(2);

    // 槽1按钮行为
    connect(ui->acceptButton1, &QPushButton::clicked, this, [this](){
        m_currentReceivingSlot = 0;
        ui->progressBar1->setVisible(true);
        ui->acceptButton1->setVisible(false);
        ui->saveasButton1->setVisible(false);
        ui->refuseButton1->setVisible(true);
        //保存默认路径
        QString defaultPath = QDir(QDir::homePath()).filePath(m_slots[0].fileName);
        QFile f(defaultPath);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(m_slots[0].buffer);
            f.close();
            QMessageBox::information(this, "保存成功", QString("文件已保存到: %1").arg(defaultPath));
        } else {
            QMessageBox::warning(this, "保存失败", QString("无法写入: %1").arg(defaultPath));
        }
    });
    connect(ui->saveasButton1, &QPushButton::clicked, this, [this](){
        // 不改变任何UI状态（保持当前状态）
        //另存为路径
        QString path = QFileDialog::getSaveFileName(this, "另存为", m_slots[0].fileName);
        if (path.isEmpty()) return;
        QFile f(path);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(m_slots[0].buffer);
            f.close();
            QMessageBox::information(this, "保存成功", QString("文件已保存到: %1").arg(path));
            // 保存成功后，隐藏accept和saveas按钮，显示refuse按钮和进度条
            ui->acceptButton1->setVisible(false);
            ui->saveasButton1->setVisible(false);
            ui->refuseButton1->setVisible(true);
            ui->progressBar1->setVisible(true);
        } else {
            QMessageBox::warning(this, "保存失败", QString("无法写入: %1").arg(path));
        }
    });
    connect(ui->refuseButton1, &QPushButton::clicked, this, [this](){
        m_slots[0] = FileSlot{};
        clearSlotUI(0);
    });

    // 槽2按钮行为
    connect(ui->acceptButton2, &QPushButton::clicked, this, [this](){
        m_currentReceivingSlot = 1;
        ui->progressBar2->setVisible(true);
        ui->acceptButton2->setVisible(false);
        ui->saveasButton2->setVisible(false);
        ui->refuseButton2->setVisible(true);
        QString defaultPath = QDir(QDir::homePath()).filePath(m_slots[1].fileName);
        QFile f(defaultPath);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(m_slots[1].buffer);
            f.close();
            QMessageBox::information(this, "保存成功", QString("文件已保存到: %1").arg(defaultPath));
        } else {
            QMessageBox::warning(this, "保存失败", QString("无法写入: %1").arg(defaultPath));
        }
    });
    connect(ui->saveasButton2, &QPushButton::clicked, this, [this](){
        // 不改变任何UI状态（保持当前状态）

        QString path = QFileDialog::getSaveFileName(this, "另存为", m_slots[1].fileName);
        if (path.isEmpty()) return;
        QFile f(path);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(m_slots[1].buffer);
            f.close();
            QMessageBox::information(this, "保存成功", QString("文件已保存到: %1").arg(path));
            // 保存成功后，隐藏accept和saveas按钮，显示refuse按钮和进度条
            ui->acceptButton2->setVisible(false);
            ui->saveasButton2->setVisible(false);
            ui->refuseButton2->setVisible(true);
            ui->progressBar2->setVisible(true);
        } else {
            QMessageBox::warning(this, "保存失败", QString("无法写入: %1").arg(path));
        }
    });
    connect(ui->refuseButton2, &QPushButton::clicked, this, [this](){
        m_slots[1] = FileSlot{};
        clearSlotUI(1);
    });

    // 槽3按钮行为
    connect(ui->acceptButton3, &QPushButton::clicked, this, [this](){
        m_currentReceivingSlot = 2;
        ui->progressBar3->setVisible(true);
        ui->acceptButton3->setVisible(false);
        ui->saveasButton3->setVisible(false);
        ui->refuseButton3->setVisible(true);
        QString defaultPath = QDir(QDir::homePath()).filePath(m_slots[2].fileName);
        QFile f(defaultPath);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(m_slots[2].buffer);
            f.close();
            QMessageBox::information(this, "保存成功", QString("文件已保存到: %1").arg(defaultPath));
        } else {
            QMessageBox::warning(this, "保存失败", QString("无法写入: %1").arg(defaultPath));
        }
    });
    connect(ui->saveasButton3, &QPushButton::clicked, this, [this](){
        // 不改变任何UI状态（保持当前状态）

        QString path = QFileDialog::getSaveFileName(this, "另存为", m_slots[2].fileName);
        if (path.isEmpty()) return;
        QFile f(path);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(m_slots[2].buffer);
            f.close();
            QMessageBox::information(this, "保存成功", QString("文件已保存到: %1").arg(path));
            // 保存成功后，隐藏accept和saveas按钮，显示refuse按钮和进度条
            ui->acceptButton3->setVisible(false);
            ui->saveasButton3->setVisible(false);
            ui->refuseButton3->setVisible(true);
            ui->progressBar3->setVisible(true);
        } else {
            QMessageBox::warning(this, "保存失败", QString("无法写入: %1").arg(path));
        }
    });
    connect(ui->refuseButton3, &QPushButton::clicked, this, [this](){
        m_slots[2] = FileSlot{};
        clearSlotUI(2);
    });

    // 初始化槽数据
    m_slots.resize(3);

    // 异步队列消费器（Qt端CQueue）
    m_asyncTimer = new QTimer(this);
    m_asyncTimer->setInterval(10);//定时器时间间隔10ms
    connect(m_asyncTimer, &QTimer::timeout, this, &Widget::processAsyncQueue);
    m_asyncTimer->start();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::connectToServer()
{
    const QString host = ui->serveipLineEdit->text().trimmed();
    const QString portStr = ui->portLineEdit->text().trimmed();
    bool ok = false;
    quint16 port = portStr.toUShort(&ok);
    if (host.isEmpty() || !ok || port == 0) {
        QMessageBox::warning(this, "输入错误", "请输入有效的服务器地址和端口");
        return;
    }
    m_networkClient->connectToServer(host, port);
}

void Widget::onSendButtonClicked()
{
    const QString text = ui->clientTextEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        QMessageBox::information(this, "发送失败", "文本内容为空");
        return;
    }
    sendTextMessage(text);
    ui->clientTextEdit->clear();
    const QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->serveBrowser->append(QString("[%1]\n[我] %2").arg(currentTime, text));
}

void Widget::selectFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "请选择要发送的文件");
    if (fileName.isEmpty()) return;
    selectedFilePath = fileName;
    ui->fileBrowser->setText(QFileInfo(fileName).fileName());
}

void Widget::serveFile()
{
    if (selectedFilePath.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择文件");
        return;
    }
    sendFile(selectedFilePath);
    ui->fileBrowser->clear();
}

void Widget::onConnectionStateChanged(bool connected)
{
    setWindowTitle(connected ? "Chat Client (Connected)" : "Chat Client (Not Connected)");
}

void Widget::onNetworkError(const QString& error)
{
    QMessageBox::warning(this, "网络错误", error);
}

void Widget::onPacketReceived(const CPacket& packet)
{
    // 同步与异步双路径
    m_syncPackets.push_back(packet);
    m_asyncQueue.push(static_cast<size_t>(packet.getCmd()), packet);

    for (const CPacket& p : m_syncPackets) {
        switch (static_cast<CCommand::Type>(p.getCmd())) {
        case CCommand::Type::TEXT_MESSAGE: {
            const QString text = QString::fromUtf8(p.getData());
            const QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            ui->serveBrowser->append(QString("[%1]\n[对方] %2").arg(currentTime, text));
            break;
        }
        case CCommand::Type::FILE_START: {
            const QString filename = QString::fromUtf8(p.getData());
            if (m_slots.size() < 3) m_slots.resize(3);
            int idx = findFreeSlotIndex();
            if (idx < 0) {
                // 没有空闲槽位，加入等待队列
                m_waitingFiles.append(filename);
                ui->serveBrowser->append(QString("[系统] 文件 %1 加入等待队列，等待空闲槽位...").arg(filename));
                break;
            }
            m_currentReceivingSlot = idx;
            m_slots[idx].fileName = filename;
            m_slots[idx].buffer.clear();
            m_slots[idx].active = true;
            setSlotReceivingUI(idx, filename, true);
            const QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            ui->serveBrowser->append(QString("[%1]\n[对方] 发送文件：%2").arg(currentTime, filename));
            break;
        }
        case CCommand::Type::FILE_DATA: {
            int idx = m_currentReceivingSlot;
            if (idx >= 0 && idx < m_slots.size() && m_slots[idx].active) {
                m_slots[idx].buffer.append(p.getData());
                // 更新所有槽位的进度显示
                if (idx == 0) {
                    ui->filesizelabel1->setText(QString("已接收：%1").arg(formatFileSize(static_cast<quint64>(m_slots[idx].buffer.size()))));
                    ui->progressBar1->setVisible(true);
                    int value = qMin(99, (m_slots[idx].buffer.size() % 100) + 1);
                    ui->progressBar1->setValue(value);
                } else if (idx == 1) {
                    ui->filesizelabel2->setText(QString("已接收：%1").arg(formatFileSize(static_cast<quint64>(m_slots[idx].buffer.size()))));
                    ui->progressBar2->setVisible(true);
                    int value = qMin(99, (m_slots[idx].buffer.size() % 100) + 1);
                    ui->progressBar2->setValue(value);
                } else if (idx == 2) {
                    ui->filesizelabel3->setText(QString("已接收：%1").arg(formatFileSize(static_cast<quint64>(m_slots[idx].buffer.size()))));
                    ui->progressBar3->setVisible(true);
                    int value = qMin(99, (m_slots[idx].buffer.size() % 100) + 1);
                    ui->progressBar3->setValue(value);
                }
            }
            break;
        }
        case CCommand::Type::FILE_COMPLETE: {
            int idx = m_currentReceivingSlot;
            if (idx >= 0 && idx < m_slots.size() && m_slots[idx].active) {
                // 更新所有槽位的完成状态
                if (idx == 0) {
                    ui->progressBar1->setValue(100);
                    ui->progressBar1->setVisible(false);
                    ui->filesizelabel1->setText(QString("接收完成：%1").arg(formatFileSize(static_cast<quint64>(m_slots[idx].buffer.size()))));
                    // 文件接收完成后的初始状态：隐藏进度条，显示三个按钮
                    ui->acceptButton1->setVisible(true);
                    ui->saveasButton1->setVisible(true);
                    ui->refuseButton1->setVisible(true);
                } else if (idx == 1) {
                    ui->progressBar2->setValue(100);
                    ui->progressBar2->setVisible(false);
                    ui->filesizelabel2->setText(QString("接收完成：%1").arg(formatFileSize(static_cast<quint64>(m_slots[idx].buffer.size()))));
                    // 文件接收完成后的初始状态：隐藏进度条，显示三个按钮
                    ui->acceptButton2->setVisible(true);
                    ui->saveasButton2->setVisible(true);
                    ui->refuseButton2->setVisible(true);
                } else if (idx == 2) {
                    ui->progressBar3->setValue(100);
                    ui->progressBar3->setVisible(false);
                    ui->filesizelabel3->setText(QString("接收完成：%1").arg(formatFileSize(static_cast<quint64>(m_slots[idx].buffer.size()))));
                    // 文件接收完成后的初始状态：隐藏进度条，显示三个按钮
                    ui->acceptButton3->setVisible(true);
                    ui->saveasButton3->setVisible(true);
                    ui->refuseButton3->setVisible(true);
                }
            }
            break;
        }
        case CCommand::Type::TEST_CONNECT: {
            ui->serveBrowser->append("[系统] 测试连接 OK");
            break;
        }
        default:
            break;
        }
    }
    m_syncPackets.clear();
}

void Widget::processAsyncQueue()
{
    PacketQueueItem item;
    int maxPerTick = 50;
    while (maxPerTick-- > 0 && m_asyncQueue.pop(item)) {
        switch (static_cast<CCommand::Type>(item.nOperator)) {
        case CCommand::Type::TEXT_MESSAGE:
            break;
        case CCommand::Type::FILE_DATA:
            // 可扩展：落盘/校验
            break;
        case CCommand::Type::FILE_COMPLETE:
            // 可扩展：通知/历史记录
            break;
        default:
            break;
        }
    }
}

void Widget::sendTextMessage(const QString& text)
{
    m_networkClient->sendCommand(static_cast<quint16>(CCommand::Type::TEXT_MESSAGE), text.toUtf8());
}

void Widget::sendFile(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "发送失败", "无法打开文件");
        return;
    }
    const QByteArray data = f.readAll();
    f.close();

    m_networkClient->sendCommand(static_cast<quint16>(CCommand::Type::FILE_START), QFileInfo(path).fileName().toUtf8());
    const int chunk = 1024;
    for (int i = 0; i < data.size(); i += chunk) {
        m_networkClient->sendCommand(static_cast<quint16>(CCommand::Type::FILE_DATA), data.mid(i, chunk));
    }
    m_networkClient->sendCommand(static_cast<quint16>(CCommand::Type::FILE_COMPLETE), QByteArray());

    const QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->serveBrowser->append(QString("[%1]\n[我] 发送了文件：%2").arg(currentTime, QFileInfo(path).fileName()));
}

QString Widget::formatFileSize(quint64 bytes)
{
    const quint64 kb = 1024ULL;
    const quint64 mb = 1024ULL * kb;
    const quint64 gb = 1024ULL * mb;
    if (bytes >= gb) return QString::number(static_cast<double>(bytes) / gb, 'f', 2) + " GB";
    if (bytes >= mb) return QString::number(static_cast<double>(bytes) / mb, 'f', 2) + " MB";
    if (bytes >= kb) return QString::number(static_cast<double>(bytes) / kb, 'f', 2) + " KB";
    return QString::number(bytes) + " B";
}

void Widget::clearSlotUI(int idx)
{
    switch (idx) {
    case 0:
        ui->filenamelabel1->clear();
        ui->filesizelabel1->clear();
        ui->progressBar1->setValue(0);
        ui->filenamelabel1->setVisible(false);
        ui->filesizelabel1->setVisible(false);
        ui->progressBar1->setVisible(false);
        ui->acceptButton1->setVisible(false);
        ui->saveasButton1->setVisible(false);
        ui->refuseButton1->setVisible(false);
        ui->fileicolabel1->setVisible(false);
        break;
    case 1:
        ui->filenamelabel2->clear();
        ui->filesizelabel2->clear();
        ui->progressBar2->setValue(0);
        ui->filenamelabel2->setVisible(false);
        ui->filesizelabel2->setVisible(false);
        ui->progressBar2->setVisible(false);
        ui->acceptButton2->setVisible(false);
        ui->saveasButton2->setVisible(false);
        ui->refuseButton2->setVisible(false);
        ui->fileicolabel2->setVisible(false);
        break;
    case 2:
        ui->filenamelabel3->clear();
        ui->filesizelabel3->clear();
        ui->progressBar3->setValue(0);
        ui->filenamelabel3->setVisible(false);
        ui->filesizelabel3->setVisible(false);
        ui->progressBar3->setVisible(false);
        ui->acceptButton3->setVisible(false);
        ui->saveasButton3->setVisible(false);
        ui->refuseButton3->setVisible(false);
        ui->fileicolabel3->setVisible(false);
        break;
    }

    // 检查等待队列，如果有等待的文件就自动开始接收
    checkWaitingFiles();
}

void Widget::checkWaitingFiles()
{
    // 如果有等待的文件且有空闲槽位，自动开始接收
    if (!m_waitingFiles.isEmpty()) {
        int idx = findFreeSlotIndex();
        if (idx >= 0) {
            QString filename = m_waitingFiles.takeFirst();
            m_currentReceivingSlot = idx;
            m_slots[idx].fileName = filename;
            m_slots[idx].buffer.clear();
            m_slots[idx].active = true;
            setSlotReceivingUI(idx, filename, true);
            ui->serveBrowser->append(QString("[系统] 开始接收等待队列中的文件：%1").arg(filename));
        }
    }
}

int Widget::findFreeSlotIndex() const
{
    for (int i = 0; i < m_slots.size(); ++i) {
        if (!m_slots[i].active) return i;
    }
    return -1;
}

void Widget::setSlotReceivingUI(int idx, const QString& filename, bool resetProgress)
{
    switch (idx) {
    case 0:
        ui->filenamelabel1->setText(filename);
        ui->filesizelabel1->setText("等待数据...");
        ui->filenamelabel1->setVisible(true);
        ui->filesizelabel1->setVisible(true);
        ui->acceptButton1->setVisible(true);
        ui->saveasButton1->setVisible(true);
        ui->refuseButton1->setVisible(true);
        ui->fileicolabel1->setVisible(true);
        ui->progressBar1->setValue(0);
        ui->progressBar1->setVisible(false);
        break;
    case 1:
        ui->filenamelabel2->setText(filename);
        ui->filesizelabel2->setText("等待数据...");
        ui->filenamelabel2->setVisible(true);
        ui->filesizelabel2->setVisible(true);
        ui->acceptButton2->setVisible(true);
        ui->saveasButton2->setVisible(true);
        ui->refuseButton2->setVisible(true);
        ui->fileicolabel2->setVisible(true);
        ui->progressBar2->setValue(0);
        ui->progressBar2->setVisible(false);
        break;
    case 2:
        ui->filenamelabel3->setText(filename);
        ui->filesizelabel3->setText("等待数据...");
        ui->filenamelabel3->setVisible(true);
        ui->filesizelabel3->setVisible(true);
        ui->acceptButton3->setVisible(true);
        ui->saveasButton3->setVisible(true);
        ui->refuseButton3->setVisible(true);
        ui->fileicolabel3->setVisible(true);
        ui->progressBar3->setValue(0);
        ui->progressBar3->setVisible(false);
        break;
    default:
        break;
    }
}
