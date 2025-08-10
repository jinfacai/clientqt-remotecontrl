#ifndef PACKET_H
#define PACKET_H
#pragma once
#include <QObject>
#include <QByteArray>
#include <QString>
#include <QDebug>
#include <cstdint>
#include <cstddef>
#include <cstring>


class CPacket : public QObject
{
    Q_OBJECT

public:
    // 数据包结构：sHead sLength sCmd strData sSum
    // sLength = sizeof(sCmd) + sizeof(strData) + sizeof(sSum)
    explicit CPacket(QObject *parent = nullptr);

    // 构造数据包
    CPacket(uint16_t nCmd, const uint8_t* pData, size_t nSize, QObject *parent = nullptr);

    // 解析数据包
    CPacket(const uint8_t* pData, size_t& nSize, QObject *parent = nullptr);

    // 拷贝构造函数
    CPacket(const CPacket& pack);

    // 析构函数
    ~CPacket();

    // 赋值运算符
    CPacket& operator=(const CPacket& pack);

    // 获取数据包大小
    int Size() const;

    // 获取数据包数据
    const char* Data() const;

    // 获取命令
    uint16_t getCmd() const { return sCmd; }

    // 获取数据
    const QByteArray& getData() const { return strData; }

    // 设置数据
    void setData(const QByteArray& data);

    // 设置命令
    void setCmd(uint16_t cmd);

#pragma pack(push, 1)

private:
    uint16_t sHead;     // 2字节 - 包头标识 0xFEFF
    uint32_t sLength;   // 4字节 - 数据长度
    uint16_t sCmd;      // 2字节 - 命令
    uint16_t sSum;      // 2字节 - 校验和
    QByteArray strData; // 数据内容
    QByteArray strOut;  // 输出缓冲区

    // 计算校验和
    uint16_t calculateChecksum() const;

    // 验证数据包完整性
    bool validatePacket(const uint8_t* pData, size_t nSize) const;
};

#pragma pack(pop)
#endif // PACKET_H
