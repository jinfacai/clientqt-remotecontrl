#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QFile>
#include <QTimer>
#include <QVector>
#include <QMap>
#include <QDir>
#include <boost/crc.hpp>
#include <QDebug>
#include <QMutex>
#include <QQueue>
#include <QPair>
#include <QThread>
#include <QSharedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

#define MY_PROTOCOL_VERSION 1
#define MSG_TYPE_TEXT 1
#define MSG_TYPE_FILE_START 6
#define MSG_TYPE_FILE_CHUNK 5
#define MSG_TYPE_FILE_END 7
#define MSG_TYPE_ACK 3
#define MSG_TYPE_ID_ASSIGN 8
#define MSG_TYPE_REFUSE 9
#define CHUNK_SIZE 32768

#pragma pack(push, 1)
struct PacketHeader {
    quint8  version;
    quint8  msg_type;
    quint32 datalen;
    quint32 filename_len;
    quint64 file_size;
    quint32 msg_id;
    quint32 chunk_index;
    quint32 chunk_count;
    quint32 sender_id;
    quint32 crc32;
};
#pragma pack(pop)

// CRC32工具函数声明（放在类外！）
quint32 calculateHeaderCRC32(const PacketHeader& header, const QByteArray& data = QByteArray());
bool verifyHeaderCRC32(const PacketHeader& header, const QByteArray& data = QByteArray());

struct FileRecvInfo {
    QString filename;
    quint64 filesize;
    quint32 chunk_count;
    QFile* file;
    QVector<bool> received;
    quint32 received_chunks;
    bool accepted;
    QString saveasPath;
    bool end_received;
    QString tempFilePath;
    bool acceptProgressInitialized;
    QMap<quint32, QByteArray> pendingChunks;
    QMutex mutex;
    QQueue<QPair<quint32, QByteArray>> chunkQueue;
    QThread* workerThread = nullptr;
    FileRecvInfo() : filesize(0), chunk_count(0), file(nullptr), received_chunks(0), accepted(false), end_received(false), acceptProgressInitialized(false), workerThread(nullptr) {}
};

struct FileSendTask {
    QFile* file;
    quint32 msg_id;
    quint32 chunk_count;
    quint32 current_chunk;
    QVector<bool> acked;
    QTimer* timer;
    QString filename;
    quint64 filesize;
    FileSendTask() : file(nullptr), msg_id(0), chunk_count(0), current_chunk(0), timer(nullptr), filesize(0) {}
};

struct TextSendTask {
    QString text;
    quint32 msg_id;
    QTimer* timer;
    int retryCount;
    TextSendTask() : msg_id(0), timer(nullptr), retryCount(0) {}
};

struct FileNameSendTask {
    QString filename;
    quint32 msg_id;
    QTimer* timer;
    int retryCount;
    FileNameSendTask() : msg_id(0), timer(nullptr), retryCount(0) {}
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

signals:
    void fileChunkWritten(quint32 msg_id, quint32 chunk_index);

private slots:
    void connectToServer();
    void onConnected();
    void onError(QAbstractSocket::SocketError socketError);
    void onSocketReadyRead();
    void onSendButtonClicked();
    void selectFile();
    void serveFile();
    //void onSendFileError(QAbstractSocket::SocketError socketError);

    // 文件槽按钮
    void onacceptButton1clicked();
    void onacceptButton2clicked();
    void onacceptButton3clicked();
    void onrefuseButton1clicked();
    void onrefuseButton2clicked();
    void onrefuseButton3clicked();
    void onsaveasButton1clicked();
    void onsaveasButton2clicked();
    void onsaveasButton3clicked();

    void handleAccept(int slotIndex);
    void handleSaveAs(int slotIndex);
    void handleRefuse(int slotIndex);
    void onFileChunkWritten(quint32 msg_id, quint32 chunk_index);

private:
    QString formatFileSize(quint64 bytes);
    void sendTextMessage(const QString& text);
    void sendFile(const QString& path);
    void sendFileChunk(QFile& file, quint32 msg_id, quint32 chunk_index, quint32 chunk_count, quint32 sender_id);
    void sendAck(quint32 msg_id, quint32 chunk_index);

    void handleTextMessage(const PacketHeader& header);
    void handleFileStart(const PacketHeader& header);
    void handleFileChunk(const PacketHeader& header);
    void handleFileEnd(const PacketHeader& header);
    void handleAck(const PacketHeader& header);
    void handleIdAssign(const PacketHeader& header);

    void updateFileSlot(int slot, const QString& filename, quint64 filesize, int progress);
    void clearFileSlot(int slot);

    int findFreeSlot();
    int findSlotByMsgId(quint32 msg_id);
    void checkForCompletion(quint32 msg_id);

    quint32 nextMsgId();
    quint32 clientId = 0;
    quint32 lastMsgId = 0;

    void initAcceptProgress(int slotIndex, FileRecvInfo& info);
    QString elideFileName(const QString& name, int maxLen = 20);

    // 新增：自动拒绝消息辅助函数声明
    void sendRefuseMessage(quint32 msg_id);

private:
    Ui::Widget *ui;
    QTcpSocket* socket;
    QString selectedFilePath;
    QMap<quint32, QSharedPointer<FileRecvInfo>> recvFiles; // msg_id -> FileRecvInfo
    QVector<quint32> slotMsgIds; // 槽位对应的msg_id
    QMap<quint32, FileSendTask> sendTasks;
    QMap<quint32, TextSendTask> textSendTasks;
    QMap<quint32, FileNameSendTask> fileNameSendTasks;
    void sendNextChunk(quint32 msg_id);
    void resendCurrentChunk(quint32 msg_id);
    void resendTextMessage(quint32 msg_id);
    void resendFileName(quint32 msg_id);
};


#endif // WIDGET_H
