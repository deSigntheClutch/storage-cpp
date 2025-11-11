
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <hiredis/hiredis.h>
#include <boost/json.hpp>

namespace json = boost::json;

class RedisPublisher {
public:
    RedisPublisher(const std::string& host = "127.0.0.1", int port = 6379)
        : ctx_(nullptr) {
        ctx_ = redisConnect(host.c_str(), port);
        if (!ctx_ || ctx_->err) {
            if (ctx_) {
                std::cerr << "Connection error: " << ctx_->errstr << std::endl;
                redisFree(ctx_);
                ctx_ = nullptr;
            } else {
                std::cerr << "Connection error: can't allocate redis context\n";
            }
            throw std::runtime_error("Failed to connect to Redis");
        }
        std::cout << "Publisher connected to Redis\n";
    }

    ~RedisPublisher() {
        if (ctx_) {
            redisFree(ctx_);
            std::cout << "Publisher disconnected\n";
        }
    }

    bool publish(const std::string& channel, const std::string& message) {
        redisReply* reply = (redisReply*)redisCommand(
            ctx_,
            "PUBLISH %s %s",
            channel.c_str(),
            message.c_str()
        );

        if (!reply) {
            std::cerr << "PUBLISH failed: no reply\n";
            return false;
        }

        if (reply->type == REDIS_REPLY_ERROR) {
            std::cerr << "PUBLISH error: " << reply->str << "\n";
            freeReplyObject(reply);
            return false;
        }

        // reply->integer contains the number of subscribers that received the message
        int subscribers = reply->integer;
        std::cout << "Message sent to " << subscribers << " subscriber(s)\n";

        freeReplyObject(reply);
        return true;
    }

    bool publishJson(const std::string& channel, const json::object& data) {
        std::string message = json::serialize(data);
        return publish(channel, message);
    }

private:
    redisContext* ctx_;
};

int main() {
    try {
        RedisPublisher publisher;

        std::string channel = "news:updates";

        // 发布普通文本消息
        std::cout << "\n=== Publishing text messages ===\n";
        publisher.publish(channel, "Hello, Redis Pub/Sub!");
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 发布 JSON 消息
        std::cout << "\n=== Publishing JSON messages ===\n";
        
        json::object news1 = {
            {"id", 1},
            {"title", "Breaking News"},
            {"content", "Redis is awesome!"},
            {"timestamp", std::time(nullptr)}
        };
        publisher.publishJson(channel, news1);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        json::object news2 = {
            {"id", 2},
            {"title", "Tech Update"},
            {"content", "C++ is powerful!"},
            {"timestamp", std::time(nullptr)}
        };
        publisher.publishJson(channel, news2);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 持续发布消息
        std::cout << "\n=== Publishing periodic messages (Press Ctrl+C to stop) ===\n";
        int counter = 0;
        while (true) {
            json::object msg = {
                {"id", ++counter},
                {"title", "Periodic Update"},
                {"content", "Message #" + std::to_string(counter)},
                {"timestamp", std::time(nullptr)}
            };
            
            if (publisher.publishJson(channel, msg)) {
                std::cout << "Published message #" << counter << "\n";
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
