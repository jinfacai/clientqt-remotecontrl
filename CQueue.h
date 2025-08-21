#ifndef CQUEUE_H
#define CQUEUE_H
#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <vector>
#include <chrono>
#include "Packet.h"

// 队列项结构 - 与Linux端保持一致
template<typename T>
struct QueueItem {
    size_t nOperator;     // 操作类型标识
    T Data;               // 数据内容
    std::shared_ptr<void> hEvent;  // 事件句柄

    // 默认构造函数
    QueueItem() : nOperator(0), Data(), hEvent(nullptr) {}

    // 参数化构造函数
    QueueItem(size_t op, const T& data, std::shared_ptr<void> event = nullptr)
        : nOperator(op), Data(data), hEvent(event) {}

    // 拷贝/赋值
    QueueItem(const QueueItem& other) = default;
    QueueItem& operator=(const QueueItem& other) = default;
};

// 线程安全的队列类（纯标准库实现）
template<typename T>
class CQueue {
public:
    CQueue() = default;
    ~CQueue() = default;

    // 入队操作
    void push(const QueueItem<T>& item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(item);
        m_condition.notify_one();
    }

    void push(size_t nOperator, const T& data, std::shared_ptr<void> hEvent = nullptr) {
        push(QueueItem<T>(nOperator, data, hEvent));
    }

    // 出队操作
    bool pop(QueueItem<T>& item) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_queue.empty()) return false;
        item = m_queue.front();
        m_queue.pop();
        return true;
    }

    bool pop(QueueItem<T>& item, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_condition.wait_for(lock, timeout, [this]{ return !m_queue.empty(); })) return false;
        item = m_queue.front();
        m_queue.pop();
        return true;
    }

    // 队列状态
    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    // 清空队列
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_queue.empty()) m_queue.pop();
    }

    // 批量操作
    void pushBatch(const std::vector<QueueItem<T>>& items) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& it : items) m_queue.push(it);
        m_condition.notify_all();
    }

    std::vector<QueueItem<T>> popBatch(size_t maxCount) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<QueueItem<T>> result;
        while (!m_queue.empty() && result.size() < maxCount) {
            result.push_back(m_queue.front());
            m_queue.pop();
        }
        return result;
    }

    // 等待队列非空
    void waitForData() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this]{ return !m_queue.empty(); });
    }

private:
    mutable std::mutex m_mutex;
    std::queue<QueueItem<T>> m_queue;
    std::condition_variable m_condition;
};

using PacketQueue = CQueue<CPacket>;
using PacketQueueItem = QueueItem<CPacket>;

#endif // CQUEUE_H
