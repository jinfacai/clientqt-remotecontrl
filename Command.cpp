#include "Command.h"
#include <QDataStream>

std::unique_ptr<CCommand> CCommand::createCommand(Type type, QObject* parent)
{
    switch (type) {
    case Type::TEXT_MESSAGE:  return std::unique_ptr<CCommand>(new TextMessageCommand(parent));
    case Type::FILE_START:    return std::unique_ptr<CCommand>(new FileStartCommand(parent));
    case Type::FILE_DATA:     return std::unique_ptr<CCommand>(new FileDataCommand(parent));
    case Type::FILE_COMPLETE: return std::unique_ptr<CCommand>(new FileCompleteCommand(parent));
    case Type::TEST_CONNECT:  return std::unique_ptr<CCommand>(new TestConnectCommand(parent));
    default:                  return nullptr;
    }
}

// TextMessageCommand实现
TextMessageCommand::TextMessageCommand(QObject* parent)
    : CCommand(parent)
{
}

TextMessageCommand::TextMessageCommand(const QString& message, QObject* parent)
    : CCommand(parent), message_(message)
{
}

QByteArray TextMessageCommand::serialize() const
{
    // 直接使用UTF-8编码，与Linux端保持一致
    return message_.toUtf8();
}

bool TextMessageCommand::deserialize(const QByteArray& data)
{
    // 直接使用UTF-8解码，与Linux端保持一致
    message_ = QString::fromUtf8(data);
    return true;
}

// FileStartCommand 实现
FileStartCommand::FileStartCommand(QObject* parent)
    : CCommand(parent) {}

FileStartCommand::FileStartCommand(const QString& filename, QObject* parent)
    : CCommand(parent), filename_(filename) {}

QByteArray FileStartCommand::serialize() const {
    return filename_.toUtf8();
}

bool FileStartCommand::deserialize(const QByteArray& data) {
    filename_ = QString::fromUtf8(data);
    return true;
}

// FileDataCommand 实现
FileDataCommand::FileDataCommand(QObject* parent)
    : CCommand(parent) {}

FileDataCommand::FileDataCommand(const QByteArray& data, QObject* parent)
    : CCommand(parent), data_(data) {}

QByteArray FileDataCommand::serialize() const {
    return data_;
}

bool FileDataCommand::deserialize(const QByteArray& data) {
    data_ = data;
    return true;
}

// FileCompleteCommand 实现
FileCompleteCommand::FileCompleteCommand(QObject* parent)
    : CCommand(parent) {}

QByteArray FileCompleteCommand::serialize() const {
    return QByteArray(); // 空负载
}

bool FileCompleteCommand::deserialize(const QByteArray& data) {
    Q_UNUSED(data);
    return true;
}

// TestConnectCommand 实现
TestConnectCommand::TestConnectCommand(QObject* parent)
    : CCommand(parent)
{
}

QByteArray TestConnectCommand::serialize() const {
    // 返回"OK"消息，与Linux端保持一致
    return QByteArray("OK");
}

bool TestConnectCommand::deserialize(const QByteArray& data) {
    Q_UNUSED(data);
    return true;
}
