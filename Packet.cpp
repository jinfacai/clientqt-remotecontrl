#include "Packet.h"
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif
CPacket::CPacket(QObject *parent)
    : QObject(parent), sHead(0), sLength(0), sCmd(0), sSum(0)
{
}

CPacket::CPacket(uint16_t nCmd, const uint8_t* pData, size_t nSize, QObject *parent)
    : QObject(parent)
{
    // nSize = strData.size()
    sHead = 0xFEFF; // 0xFF 0xFE
    sLength = nSize + 2 + 2; // sizeof(sCmd) + sizeof(sSum)
    sCmd = nCmd;

    if (nSize > 0) {
        strData.resize(nSize);
        memcpy(strData.data(), pData, nSize); // pData -> strData
    } else {
        // 记录错误日志（使用标准错误日志，除此以外）
        qWarning() << "Error: Received packet with zero data size (Cmd=" << sCmd << ")";
        strData.clear();
    }

    sSum = calculateChecksum();
}

CPacket::CPacket(const uint8_t* pData, size_t& nSize, QObject *parent)
    : QObject(parent)
{
    // nSize = pack.size
    size_t i = 0;

    // 查找包头
    for (; i < nSize; i++) {
        if (*(uint16_t*)(pData + i) == 0xFEFF) {
            sHead = *(uint16_t*)(pData + i);
            i += 2;
            break;
        }
    }

    // 检查剩余数据是否足够（sLength 4字节 + sCmd 2字节 + sSum 2字节）
    if (i + 4 + 2 + 2 > nSize) {
        qWarning() << "Error: Incomplete packet header (need" << (i + 8) << "bytes, available" << nSize;
        nSize = 0;
        return;
    }

    // 读取数据长度
    //sLength = *(uint32_t*)(pData + i);
    sLength = ntohl(*(uint32_t*)(pData + i));
    i += 4;

    // 如果未完全接收到数据包，返回失败
    if (sLength + i > nSize) {
        qWarning() << "Error: Incomplete packet (need" << (sLength + i) << "bytes, available" << nSize;
        nSize = 0;
        return;
    }

    // 读取命令
    //sCmd = *(uint16_t*)(pData + i);
    sCmd = ntohs(*(uint16_t*)(pData + i));
    i += 2;

    // strData数据读取（有错误）
    if (sLength > 4) {
        strData.resize(sLength - 2 - 2); // sLength - 2 - 2 = sizeof(strData)
        memcpy(strData.data(), pData + i, sLength - 2 - 2);
        i += sLength - 2 - 2;
    }

    // 校验校验和
    //sSum = *(uint16_t*)(pData + i);
    sSum = ntohs(*(uint16_t*)(pData + i));
    uint16_t sum = calculateChecksum();

    if (sum == sSum) {
        nSize -= i; // 移除粘包
        // 修改为剩余未处理数据
        return;
    } else {
        qWarning() << "Error: Checksum mismatch";
        nSize = 0;
    }
}

CPacket::CPacket(const CPacket& pack)
    : QObject(pack.parent())
{
    sHead = pack.sHead;
    sLength = pack.sLength;
    sCmd = pack.sCmd;
    strData = pack.strData;
    sSum = pack.sSum;
}

CPacket::~CPacket()
{
}

CPacket& CPacket::operator=(const CPacket& pack)
{
    if (this != &pack) {  // 防止自赋值
        sHead = pack.sHead;
        sLength = pack.sLength;
        sCmd = pack.sCmd;
        strData = pack.strData; // 深拷贝
        sSum = pack.sSum;
    }
    return *this;  // 返回对象引用
}

int CPacket::Size() const
{
    return sLength + 6; // 数据包总大小
}

const char* CPacket::Data() const
{
    QByteArray& mutableStrOut = const_cast<QByteArray&>(strOut);
    mutableStrOut.resize(Size());
    uint8_t* pData = reinterpret_cast<uint8_t*>(const_cast<char*>(mutableStrOut.data()));

    *reinterpret_cast<uint16_t*>(pData) = sHead;
    pData += 2;

    *reinterpret_cast<uint32_t*>(pData) = htonl(sLength);
    pData += 4;

    *reinterpret_cast<uint16_t*>(pData) = htons(sCmd);
    pData += 2;

    memcpy(pData, strData.data(), strData.size());
    pData += strData.size();

    *reinterpret_cast<uint16_t*>(pData) = htons(sSum);

    return mutableStrOut.data();
}

void CPacket::setData(const QByteArray& data)
{
    strData = data;
    sLength = data.size() + 2 + 2; // sizeof(sCmd) + sizeof(sSum)
    sSum = calculateChecksum();
}

void CPacket::setCmd(uint16_t cmd)
{
    sCmd = cmd;
    sSum = calculateChecksum();
}

uint16_t CPacket::calculateChecksum() const
{
    uint16_t sum = 0;
    for (uint8_t byte : strData) {
        sum += byte;
    }
    return sum;
}

bool CPacket::validatePacket(const uint8_t* pData, size_t nSize) const
{
    if (nSize < 8) { // 最小包大小：2(头) + 4(长度) + 2(命令) + 2(校验和)
        return false;
    }

    // 检查包头
    if (*(uint16_t*)pData != 0xFEFF) {
        return false;
    }

    // 检查数据长度
    uint32_t length = *(uint32_t*)(pData + 2);
    if (length + 6 > nSize) { // 6 = 2(头) + 4(长度)
        return false;
    }

    return true;
}
