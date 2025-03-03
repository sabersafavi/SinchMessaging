#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <stdexcept>
#include <sstream>

class Message {
public:
    std::map<std::string, std::string> headers;
    std::vector<uint8_t> payload;

    bool operator==(const Message& other) const {
        return headers == other.headers && payload == other.payload;
    }
};

class MessageCodec {
public:
    virtual std::vector<uint8_t> encode(const Message& message) const = 0;
    virtual Message decode(const std::vector<uint8_t>& data) const = 0;
    virtual ~MessageCodec() = default;
};

class SinchMessageCodec : public MessageCodec {
public:
    std::vector<uint8_t> encode(const Message& message) const override {
        validateMessage(message);

        std::vector<uint16_t> headerSizes;
        std::vector<uint8_t> headerContent;

        for (const auto& [name, value] : message.headers) {
            std::vector<uint8_t> nameBytes(name.begin(), name.end());
            std::vector<uint8_t> valueBytes(value.begin(), value.end());

            headerSizes.push_back(static_cast<uint16_t>(nameBytes.size()));
            headerSizes.push_back(static_cast<uint16_t>(valueBytes.size()));

            headerContent.insert(headerContent.end(), nameBytes.begin(), nameBytes.end());
            headerContent.insert(headerContent.end(), valueBytes.begin(), valueBytes.end());
        }

        std::vector<uint8_t> encoded;
        encoded.push_back(static_cast<uint8_t>(message.headers.size()));

        for (uint16_t size : headerSizes) {
            encoded.push_back(static_cast<uint8_t>(size & 0xFF));
            encoded.push_back(static_cast<uint8_t>((size >> 8) & 0xFF));
        }

        encoded.insert(encoded.end(), headerContent.begin(), headerContent.end());
        encoded.insert(encoded.end(), message.payload.begin(), message.payload.end());

        return encoded;
    }

    Message decode(const std::vector<uint8_t>& data) const override {
        if (data.empty()) {
            throw std::invalid_argument("Empty data.");
        }

        uint8_t headerCount = data[0];
        if (headerCount > MAX_HEADERS) {
            throw std::invalid_argument("Invalid header count.");
        }

        std::vector<uint16_t> headerSizes;
        size_t offset = 1;

        for (size_t i = 0; i < headerCount * 2; ++i) {
            if (offset + 2 > data.size()) {
                throw std::invalid_argument("Incomplete header size data.");
            }
            uint16_t size = static_cast<uint16_t>(data[offset]) | (static_cast<uint16_t>(data[offset + 1]) << 8);
            headerSizes.push_back(size);
            offset += 2;
        }

        std::map<std::string, std::string> headers;
        for (size_t i = 0; i < headerCount; ++i) {
            if(offset + headerSizes[i*2] + headerSizes[i*2+1] > data.size()){
                throw std::invalid_argument("Incomplete header data");
            }
            std::string name(data.begin() + offset, data.begin() + offset + headerSizes[i * 2]);
            offset += headerSizes[i * 2];
            std::string value(data.begin() + offset, data.begin() + offset + headerSizes[i * 2 + 1]);
            offset += headerSizes[i * 2 + 1];

            headers[name] = value;
        }

        std::vector<uint8_t> payload(data.begin() + offset, data.end());
        return Message{headers, payload};
    }

private:
    void validateMessage(const Message& message) const {
        if (message.headers.size() > MAX_HEADERS) {
            throw std::invalid_argument("Maximum 63 headers allowed.");
        }
        if (message.payload.size() > MAX_PAYLOAD_SIZE) {
            throw std::invalid_argument("Maximum payload size of 256 KiB allowed.");
        }
        for (const auto& [name, value] : message.headers) {
            if (name.length() > MAX_HEADER_SIZE || value.length() > MAX_HEADER_SIZE) {
                throw std::invalid_argument("Header name and value must be <= 1023 bytes.");
            }
        }
    }

    static const uint8_t MAX_HEADERS = 63;
    static const uint16_t MAX_HEADER_SIZE = 1023;
    static const size_t MAX_PAYLOAD_SIZE = 256 * 1024;
};

int main() {
    SinchMessageCodec codec;
    Message message;
    message.headers["Content-Type"] = "application/json";
    message.headers["X-Request-Id"] = "12345";
    std::string payloadStr = "{\"key\":\"value\"}";
    message.payload.assign(payloadStr.begin(), payloadStr.end());

    std::vector<uint8_t> encoded = codec.encode(message);
    Message decoded = codec.decode(encoded);

    if (message == decoded) {
        std::cout << "Encoding and decoding successful!" << std::endl;
    } else {
        std::cout << "Encoding and decoding failed!" << std::endl;
    }

    return 0;
}