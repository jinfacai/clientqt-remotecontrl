#ifndef WIDGET_H
#define WIDGET_H
//#include "packetheader.h"
#include <QWidget>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDataStream>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTextBrowser>
#include <QtGlobal>

// 修正后的协议包头（与服务端完全一致）
#pragma pack(push, 1)
struct FileHeader{
    uint8_t  version;//1字节
    uint8_t  msg_type;//1字节
    uint32_t filename_len;//4字节 文件名长度
    uint64_t file_size;//8字节 整个文件大小
    uint64_t text_size;//8字节 文本消息大小
};//(22)
//协议包头
typedef struct PacketHeader{
    uint32_t data_len;//4字节 当前数据包长度
    uint64_t sSum;//8字节
};//(12)

#pragma pack(pop)

struct FileSlot {
    QString& currentFileName;
    qint64& totalFileSize;
    QFile*& currentFile;
    qint64& currentFileSize;
    bool& isFilePending;
    QLabel* fileIconLabel;
    QLabel* fileNameLabel;
    QLabel* fileSizeLabel;
    QProgressBar* progressBar;
    QPushButton* acceptButton;
    QPushButton* saveasButton;
    QPushButton* refuseButton;
};


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
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void selectFile();
    void serveFile();
    void onSocketReadyRead();
    void onSendButtonClicked();
    void onSendButtonError(QAbstractSocket::SocketError socketError);
    void onSendFileError(QAbstractSocket::SocketError socketError);
    bool validateInput(QString& ip, quint16& port);
    void onacceptButton1clicked();      // 接受文件
    void onsaveasButton1clicked();      // 另存为文件
    void onrefuseButton1clicked();      // 拒绝文件
    void onacceptButton2clicked();      // 接受文件
    void onsaveasButton2clicked();      // 另存为文件
    void onrefuseButton2clicked();      // 拒绝文件
    void onacceptButton3clicked();      // 接受文件
    void onsaveasButton3clicked();      // 另存为文件
    void onrefuseButton3clicked();      // 拒绝文件
    void handleFile(
        QString& currentFileName,
        qint64& totalFileSize,
        bool& isFilePending,
        QLabel* fileIconLabel,
        QLabel* fileNameLabel,
        QLabel* fileSizeLabel,
        QPushButton* acceptButton,
        QPushButton* saveAsButton,
        QPushButton* refuseButton,
        QProgressBar* progressBar,
        QTextBrowser* serveBrowser
        );
    //void handleFileMessage(const QByteArray &dataBody, const PacketHeader &header);
    void handleSaveAs(int slotIndex);
    void handleRefuse(int slotIndex);
    void handleAccept(int slotIndex);
    void handleTextMessage(uint64_t txtsize);
    void handleFileMessage(uint32_t flnamelen, uint64_t flsize);

private:
    Ui::Widget *ui;
    QTcpSocket* socket;
    QTcpServer* tcpServer;
    QTcpSocket* serverSocket;    // 接受客户端（其实就是本程序的 clientSocket）的连接
    qint64 blockSize;
    QString selectedFilePath;
    QByteArray m_receiveBuffer;
    QVector<bool> fileSlotsOccupied;
    QVector<QByteArray> m_pendingFileDatas;
    QVector<FileSlot> fileSlots; // 槽位状态数组
    const quint8 PROTOCOL_VERSION = 1;
    const quint8 MSG_TEXT = 1;
    const quint8 MSG_FILE = 2;
    bool m_isHeaderParsed = false;    // 是否已解析协议头
    PacketHeader m_currentHeader;     // 当前消息的协议头
    QString currentFileName1, currentFileName2, currentFileName3;
    qint64 totalFileSize1 = 0, totalFileSize2 = 0, totalFileSize3 = 0;
    QFile *currentFile1 = nullptr, *currentFile2 = nullptr, *currentFile3 = nullptr;
    qint64 currentFileSize1 = 0, currentFileSize2 = 0, currentFileSize3 = 0;
    bool isFilePending1 = false, isFilePending2 = false, isFilePending3 = false;

protected:
    void closeEvent(QCloseEvent *event) override;
};
#endif // WIDGET_H
