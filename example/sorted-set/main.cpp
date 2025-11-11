#include <iostream>
#include <string>
#include <vector>
#include <hiredis/hiredis.h>
#include <boost/json.hpp>

namespace json = boost::json;

void add_student(redisContext* ctx, json::object& student) {
	// Extract grade as score
	int64_t grade = student["grade"].as_int64();

	// Serialize the student object to JSON string
	std::string jsonStr = json::serialize(student);

	// Add to Redis sorted set
	redisReply* reply = (redisReply*)redisCommand(
		ctx,
		"ZADD %s %lld %s",
		"pg:student",   // key
		grade,          // score (grade)
		jsonStr.c_str() // member (JSON string)
	);
	
	if (!reply) {
		std::cerr << "ZADD failed: no reply\n";
		return;
	}
	if (reply->type == REDIS_REPLY_ERROR) {
		std::cerr << "ZADD error: " << reply->str << "\n";
	} else {
		std::cout << "Added student: " << student["name"].as_string() 
		          << " with grade " << grade << std::endl;
	}
	
	freeReplyObject(reply);
}

std::vector<json::object> get_top_k_students(redisContext* ctx, int k) {
	std::vector<json::object> results;
	
	// Get top k students (highest scores) using ZREVRANGE
	redisReply* reply = (redisReply*)redisCommand(
		ctx,
		"ZREVRANGE %s 0 %d",
		"pg:student",  // key
		k - 1          // get top k elements (0-indexed)
	);
	
	if (!reply) {
		std::cerr << "ZREVRANGE failed: no reply\n";
		return results;
	}
	if (reply->type == REDIS_REPLY_ERROR) {
		std::cerr << "ZREVRANGE error: " << reply->str << "\n";
		freeReplyObject(reply);
		return results;
	}
	
	if (reply->type == REDIS_REPLY_ARRAY) {
		for (size_t i = 0; i < reply->elements; ++i) {
			redisReply* member = reply->element[i];
			if (member->str) {
				try {
					// Parse JSON string back to object
					json::value val = json::parse(member->str);
					results.push_back(val.as_object());
				} catch (const std::exception& e) {
					std::cerr << "Error parsing JSON: " << e.what() << std::endl;
				}
			}
		}
	}
	
	freeReplyObject(reply);
	return results;
}

std::vector<json::object> list_all_students(redisContext* ctx) {
	std::vector<json::object> results;
	
	// Get all students with scores using ZRANGE with WITHSCORES
	redisReply* reply = (redisReply*)redisCommand(
		ctx,
		"ZRANGE %s 0 -1 WITHSCORES",
		"pg:student"  // key
	);
	
	if (!reply) {
		std::cerr << "ZRANGE failed: no reply\n";
		return results;
	}
	if (reply->type == REDIS_REPLY_ERROR) {
		std::cerr << "ZRANGE error: " << reply->str << "\n";
		freeReplyObject(reply);
		return results;
	}
	
	if (reply->type == REDIS_REPLY_ARRAY) {
		// Elements come in pairs: member, score, member, score, ...
		for (size_t i = 0; i + 1 < reply->elements; i += 2) {
			redisReply* member = reply->element[i];
			redisReply* score = reply->element[i + 1];
			
			if (member->str) {
				try {
					// Parse JSON string back to object
					json::value val = json::parse(member->str);
					json::object obj = val.as_object();
					
					results.push_back(std::move(obj));
				} catch (const std::exception& e) {
					std::cerr << "Error parsing JSON: " << e.what() << std::endl;
				}
			}
		}
	}
	
	freeReplyObject(reply);
	return results;
}

/****
1. Hset won't dedupes the key-value pairs.
"Alice"   95 {"id":1001,"name":"Alice","class":"Math","grade":95,"age":16}
"Alice"   95 {"id":1001,"name":"Alice","grade":95}
// Store in sorted set with just ID
redisCommand(ctx, "ZADD pg:student %lld %lld", grade, id);
// Store full student data in a hash
redisCommand(ctx, "HSET pg:student:%lld data %s", id, jsonStr.c_str());
****/
int main() {
    const char* host = "127.0.0.1";
    int port = 6379;

    // 连接 Redis
    redisContext* ctx = redisConnect(host, port);
    if (ctx == nullptr || ctx->err) {
        if (ctx) {
            std::cerr << "Connection error: " << ctx->errstr << std::endl;
            redisFree(ctx);
        } else {
            std::cerr << "Connection error: can't allocate redis context\n";
        }
        return 1;
    }

	std::string s[4];
    s[0] = R"({"id":1001,"name":"Alice","grade":95})";
    s[1] = R"({"id":1002,"name":"Bob","grade":93})";
    s[2] = R"({"id":1003,"name":"Cam","grade":94})";
    s[3] = R"({"id":1004,"name":"Dam","grade":93})";

	std::vector<json::object> students;

	for (int i = 0; i < 4; i++) {
		json::value val = json::parse(s[i]);
    	json::object& obj = val.as_object();
		add_student(ctx, obj);
    }

	std::vector<json::object> top3Students = list_all_students(ctx);

	for (auto x : top3Students) {
		std::cout << x["name"].as_string() << "   " <<
		x["grade"].as_int64() << " " << json::serialize(x) << std::endl;
	}

    // 断开
    redisFree(ctx);
    return 0;
}
