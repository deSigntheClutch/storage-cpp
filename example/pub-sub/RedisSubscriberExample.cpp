#include <iostream>
#include <string>
#include <vector>
#include <hiredis/hiredis.h>
#include <boost/json.hpp>

namespace json = boost::json;

class RedisSubscriber {
public:
    RedisSubscriber(const std::string& host = "127.0.0.1", int port = 6379)
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
        std::cout << "Subscriber connected to Redis\n";
    }

    ~RedisSubscriber() {
        if (ctx_) {
            redisFree(ctx_);
            std::cout << "Subscriber disconnected\n";
        }
    }

    bool subscribe(const std::vector<std::string>& channels) {
        // Build SUBSCRIBE command with multiple channels
        std::string command = "SUBSCRIBE";
        for (const auto& channel : channels) {
            command += " " + channel;
        }

        redisReply* reply = (redisReply*)redisCommand(ctx_, command.c_str());
        if (!reply) {
            std::cerr << "SUBSCRIBE failed: no reply\n";
            return false;
        }

        if (reply->type == REDIS_REPLY_ERROR) {
            std::cerr << "SUBSCRIBE error: " << reply->str << "\n";
            freeReplyObject(reply);
            return false;
        }

        freeReplyObject(reply);
        return true;
    }

    void listen() {
        std::cout << "\n=== Listening for messages (Press Ctrl+C to stop) ===\n";
        
        while (true) {
            redisReply* reply;
            
            // Wait for messages (blocking call)
            if (redisGetReply(ctx_, (void**)&reply) != REDIS_OK) {
                std::cerr << "Error getting reply\n";
                break;
            }
            if (!reply) {
                std::cerr << "No reply received\n";
                break;
            }

            // Process the reply
            if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
                std::string messageType = reply->element[0]->str;
                std::string channel = reply->element[1]->str;
                std::string message = reply->element[2]->str;

                if (messageType == "message") {
                    handleMessage(channel, message);
                } else if (messageType == "subscribe") {
                    std::cout << "✓ Subscribed to channel: " << channel << "\n";
                    std::cout << "  Total subscriptions: " << reply->element[2]->integer << "\n";
                }
            }

            freeReplyObject(reply);
        }
    }

private:
    void handleMessage(const std::string& channel, const std::string& message) {
        std::cout << "\n[" << channel << "] Received message:\n";
        
        // Try to parse as JSON
        try {
            json::value val = json::parse(message);
            if (val.is_object()) {
                json::object obj = val.as_object();
                std::cout << "  JSON Message:\n";
                for (const auto& [key, value] : obj) {
                    std::cout << "    " << key << ": " << value << "\n";
                }
            } else {
                std::cout << "  " << message << "\n";
            }
        } catch (...) {
            // Not JSON, print as plain text
            std::cout << "  " << message << "\n";
        }
    }

    redisContext* ctx_;
};

int main() {
    try {
        RedisSubscriber subscriber;

        // Subscribe to one or multiple channels
        std::vector<std::string> channels = {
            "news:updates",
            "alerts:system"
        };

        subscriber.subscribe(channels);
        subscriber.listen();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
