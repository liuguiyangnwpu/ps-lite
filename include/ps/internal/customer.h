/**
 *  Copyright (c) 2015 by Contributors
 */
#ifndef PS_INTERNAL_CUSTOMER_H_
#define PS_INTERNAL_CUSTOMER_H_
#include <mutex>
#include <vector>
#include <utility>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>
#include <memory>
#include "ps/internal/message.h"
#include "ps/internal/threadsafe_queue.h"

namespace ps {

/**
 * \brief The object for communication.
 *
 * As a sender, a customer tracks the responses for each request sent.
 *
 * It has its own receiving thread which is able to process any message received
 * from a remote node with `msg.meta.customer_id` equal to this customer's id
 */
class Customer {
public:
    using RecvHandle = std::function<void(const Message& recved)>;
    Customer(int id, const RecvHandle& recv_handle);
    ~Customer();
    int id() { return id_; }
    int NewRequest(int recver);
    void WaitRequest(int timestamp);
    int NumResponse(int timestamp);
    void AddResponse(int timestamp, int num = 1);
    void Accept(const Message& recved) { recv_queue_.Push(recved); }

private:
    void Receiving();

    int id_;

    RecvHandle recv_handle_;
    ThreadsafeQueue<Message> recv_queue_;
    std::unique_ptr<std::thread> recv_thread_;

    std::mutex tracker_mu_;
    std::condition_variable tracker_cond_;
    std::vector<std::pair<int, int>> tracker_;

    DISALLOW_COPY_AND_ASSIGN(Customer);
};

}  // namespace ps
#endif  // PS_INTERNAL_CUSTOMER_H_
