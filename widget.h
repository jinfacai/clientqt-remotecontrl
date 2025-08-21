#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFile>
#include <QByteArray>
#include <QString>
#include <QVector>
#include <list>
#include "ClientSocket.h"
#include "Command.h"
#include "Packet.h"
#include "CQueue.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void connectToServer();
    void onSendButtonClicked();
    void selectFile();
    void serveFile();

    // ClientSocket 信号
    void onConnectionStateChanged(bool connected);
    void onPacketReceived(const CPacket& packet);
    void onNetworkError(const QString& error);

private:
    void sendTextMessage(const QString& text);
    void sendFile(const QString& path);
    QString formatFileSize(quint64 bytes);
    void clearSlotUI(int idx);
    void processAsyncQueue();
    int findFreeSlotIndex() const;
    void setSlotReceivingUI(int idx, const QString& filename, bool resetProgress);
    void checkWaitingFiles();

private:
    Ui::Widget *ui;
    ClientSocket* m_networkClient;
    QString selectedFilePath;

    // 多文件槽接收状态
    struct FileSlot {
        QString fileName;
        QByteArray buffer;
        bool active = false;
    };
    QVector<FileSlot> m_slots;
    int m_currentReceivingSlot = -1;

    // 兼容简单用法的缓冲（用于非多槽路径）
    QString m_incomingFileName;
    QByteArray m_incomingFileData;

    // 同步与异步：list + CQueue
    std::list<CPacket> m_syncPackets; // 同步立即处理
    PacketQueue m_asyncQueue;         // 异步耗时处理
    class QTimer* m_asyncTimer = nullptr;

    // 文件等待队列
    QStringList m_waitingFiles;
};

#endif // WIDGET_H
