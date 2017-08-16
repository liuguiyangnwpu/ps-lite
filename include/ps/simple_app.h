/**
 *  Copyright (c) 2015 by Contributors
 */
#ifndef PS_SIMPLE_APP_H_
#define PS_SIMPLE_APP_H_
#include <string>
#include "ps/internal/message.h"
#include "ps/internal/postoffice.h"
namespace ps {

/**
  the format for a received request or reponse for \ref SimpleApp
 */
struct SimpleData {
/* the int head */
    int head;
/* the string body */
    std::string body;
/* sender's node id */
    int sender;
/* the associated timestamp */
    int timestamp;
};

/**
  a simple app
 *
 * It provides basic communcation with a pair of int (head) and string (body)
 */
class SimpleApp {
public:
/**
 constructor
* @param app_id the app id, should match with the remote node app with which this app
* is communicated
*/
    explicit SimpleApp(int app_id);

/* deconstructor */
    virtual ~SimpleApp() { delete obj_; obj_ = nullptr; }

/**
 send a request to a remote node
*
* \param req_head request head
* \param req_body request body
* \param recv_id remote node id
*
* @return the timestamp of this request
*/
    virtual inline int Request(int req_head, const std::string& req_body, int recv_id);

/**
 wait until a request is finished
*
* \param timestamp
*/
    virtual inline void Wait(int timestamp) { obj_->WaitRequest(timestamp); }


/**
 send back a response for a request
* \param recv_req the received request
* \param the response body
*/
    virtual inline void Response(const SimpleData& recv_req, const std::string& res_body = "");

/**
 the handle to proces a received request/respoonse
*
* \param recved the received request or response
* \param app this pointer
*/
    using Handle = std::function<void(const SimpleData& recved, SimpleApp* app)>;

/**
 set the request handle
* \param request_handle the request handle
*/
    virtual inline void set_request_handle(const Handle& request_handle) {
        CHECK(request_handle) << "invalid request handle";
        request_handle_ = request_handle;
    }

/**
 set the response handle
* \param response_handle the response handle
*/
    virtual inline void set_response_handle(const Handle& response_handle) {
        CHECK(response_handle) << "invalid response handle";
        response_handle_ = response_handle;
    }

/**
 returns the customer
*/
    virtual inline Customer* get_customer() { return obj_; }

protected:
/* empty construct */
    inline SimpleApp() : obj_(nullptr) {
        request_handle_ = [](const SimpleData& recved, SimpleApp* app) {
            app->Response(recved);
        };
        response_handle_ = [](const SimpleData& recved, SimpleApp* app) { };
    }

/* process a received message */
    virtual inline void Process(const Message& msg);

/* ps internal object */
    Customer* obj_;

private:
/* request handle */
    Handle request_handle_;
/* request handle */
    Handle response_handle_;
};

////////////////////////////////////////////////////////////////////////////////

inline SimpleApp::SimpleApp(int app_id) : SimpleApp() {
    using namespace std::placeholders;
    obj_ = new Customer(app_id, std::bind(&SimpleApp::Process, this, _1));
}

inline int SimpleApp::Request(int req_head, const std::string& req_body, int recv_id) {
    // setup message
    Message msg;
    msg.meta.head = req_head;
    if (req_body.size()) msg.meta.body = req_body;
    int ts = obj_->NewRequest(recv_id);
    msg.meta.timestamp = ts;
    msg.meta.request = true;
    msg.meta.simple_app = true;
    msg.meta.customer_id = obj_->id();

    // send
    for (int r : Postoffice::Get()->GetNodeIDs(recv_id)) {
        msg.meta.recver = r;
        Postoffice::Get()->van()->Send(msg);
    }
    return ts;
}

inline void SimpleApp::Response(const SimpleData& req, const std::string& res_body) {
    // setup message
    Message msg;
    msg.meta.head = req.head;
    if (res_body.size()) msg.meta.body = res_body;
    msg.meta.timestamp = req.timestamp;
    msg.meta.request = false;
    msg.meta.simple_app = true;
    msg.meta.customer_id = obj_->id();
    msg.meta.recver = req.sender;

    // send
    Postoffice::Get()->van()->Send(msg);
}


inline void SimpleApp::Process(const Message& msg) {
    SimpleData recv;
    recv.sender    = msg.meta.sender;
    recv.head      = msg.meta.head;
    recv.body      = msg.meta.body;
    recv.timestamp = msg.meta.timestamp;
    if (msg.meta.request) {
        CHECK(request_handle_);
        request_handle_(recv, this);
    } else {
        CHECK(response_handle_);
        response_handle_(recv, this);
    }
}

}  // namespace ps
#endif  // PS_SIMPLE_APP_H_
