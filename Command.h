#ifndef COMMAND_H
#define COMMAND_H
#pragma once
#include <QObject>
#include <QString>
#include <QByteArray>
//#include <memory>
//#include <cstdint>

class CCommand : public QObject{
    Q_OBJECT
public:
    enum class Type : uint16_t{
        TEXT_MESSAGE = 1,
        FILE_START    = 2,
        FILE_DATA     = 3,
        FILE_COMPLETE = 4,
        TEST_CONNECT  = 1981
    };
    Q_ENUM(Type)

    explicit CCommand(QObject* parent = nullptr) : QObject(parent){}
    virtual ~CCommand() = default;
    // 获取命令类型
    virtual Type getType() const = 0;

    // 序列化命令数据
    virtual QByteArray serialize() const = 0;

    // 反序列化命令数据
    virtual bool deserialize(const QByteArray& data) = 0;

    // 工厂方法：根据命令类型创建具体的命令对象
    static std::unique_ptr<CCommand> createCommand(Type type, QObject* parent = nullptr);
};

// 文本消息命令
class TextMessageCommand : public CCommand {
    Q_OBJECT

public:
    explicit TextMessageCommand(QObject* parent = nullptr);
    explicit TextMessageCommand(const QString& message, QObject* parent = nullptr);

    Type getType() const override { return Type::TEXT_MESSAGE; }
    QByteArray serialize() const override;
    bool deserialize(const QByteArray& data) override;

    const QString& getMessage() const { return message_; }
    void setMessage(const QString& message) { message_ = message; }

private:
    QString message_;
};

// 文件开始命令（首包：仅文件名）
class FileStartCommand : public CCommand {
    Q_OBJECT
public:
    explicit FileStartCommand(QObject* parent = nullptr);
    explicit FileStartCommand(const QString& filename, QObject* parent = nullptr);

    Type getType() const override { return Type::FILE_START; }
    QByteArray serialize() const override;
    bool deserialize(const QByteArray& data) override;

    const QString& getFilename() const { return filename_; }
    void setFilename(const QString& f) { filename_ = f; }
private:
    QString filename_;
};

// 文件数据命令（中间数据包：仅数据）
class FileDataCommand : public CCommand {
    Q_OBJECT
public:
    explicit FileDataCommand(QObject* parent = nullptr);
    explicit FileDataCommand(const QByteArray& data, QObject* parent = nullptr);

    Type getType() const override { return Type::FILE_DATA; }
    QByteArray serialize() const override;
    bool deserialize(const QByteArray& data) override;

    const QByteArray& getDataBytes() const { return data_; }
    void setDataBytes(const QByteArray& d) { data_ = d; }
private:
    QByteArray data_;
};

// 文件完成命令
class FileCompleteCommand : public CCommand {
    Q_OBJECT
public:
    explicit FileCompleteCommand(QObject* parent = nullptr);

    Type getType() const override { return Type::FILE_COMPLETE; }
    QByteArray serialize() const override;
    bool deserialize(const QByteArray& data) override;
};

// 测试连接命令
class TestConnectCommand : public CCommand {
    Q_OBJECT
public:
    explicit TestConnectCommand(QObject* parent = nullptr);

    Type getType() const override { return Type::TEST_CONNECT; }
    QByteArray serialize() const override;
    bool deserialize(const QByteArray& data) override;
};


#endif // COMMAND_H
