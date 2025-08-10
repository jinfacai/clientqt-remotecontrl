#ifndef TESTUI_H
#define TESTUI_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QProgressBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include "ClientSocket.h"
#include "Packet.h"
#include "Command.h"
#include <memory>

class TestUI : public QWidget
{
    Q_OBJECT

public:
    explicit TestUI(QWidget *parent = nullptr);
    ~TestUI();

private slots:
    // 连接相关
    void onConnectButtonClicked();
    void onDisconnectButtonClicked();

    // 发送相关
    void onSendTextButtonClicked();
    void onSelectFileButtonClicked();
    void onSendFileButtonClicked();

    // NetworkClient信号处理
    void onConnectionStateChanged(bool connected);
    void onPacketReceived(const CPacket& packet);
    void onNetworkError(const QString& error);

private:
    // UI组件
    QLineEdit* m_ipEdit;
    QLineEdit* m_portEdit;
    QPushButton* m_connectButton;
    QPushButton* m_disconnectButton;

    QTextEdit* m_messageEdit;
    QPushButton* m_sendTextButton;

    QLineEdit* m_filePathEdit;
    QPushButton* m_selectFileButton;
    QPushButton* m_sendFileButton;

    QTextEdit* m_logEdit;
    QLabel* m_statusLabel;

    // 网络客户端
    ClientSocket* m_networkClient;

    // 文件相关
    QString m_selectedFilePath;

    // 辅助函数
    void logMessage(const QString& message);
    void updateStatus(const QString& status);
    void sendTextMessage(const QString& text);
    void sendFileMessage(const QString& filePath);
};

#endif // TESTUI_H
