# Sinch Binary Message Encoding Scheme

This project implements a binary message encoding scheme for a signaling protocol, primarily used for real-time communication applications.

## Message Structure

A message consists of:

* **Headers:** A variable number of name-value pairs (ASCII-encoded strings).
    * Maximum 63 headers.
    * Header name and value are limited to 1023 bytes each.
* **Payload:** A binary payload, limited to 256 KiB.

## Encoding Scheme

1.  **Header Count:** The first byte represents the number of headers (0-63).
2.  **Header Sizes:** A sequence of 2-byte short values, representing the sizes of each header name and value (in bytes). The order is headerNameSize, headerValueSize, headerNameSize, headerValueSize, etc.
3.  **Header Content:** The UTF-8 encoded header names and values are concatenated.
4.  **Payload:** The binary payload is appended directly after the header content.

## Implementation

The implementation is in Kotlin and includes:

* `Message` class: Represents a message with headers and payload.
* `MessageCodec` interface: Defines the `encode` and `decode` methods.
* `SinchMessageCodec` class: Implements the encoding and decoding logic.
* Custom exception classes: For robust error handling.

## Usage

```kotlin
val codec = SinchMessageCodec()
val headers = mapOf("Content-Type" to "application/json", "X-Request-Id" to "12345")
val payload = "{\"key\":\"value\"}".toByteArray(Charsets.UTF_8)
val message = Message(headers, payload)

val encoded = codec.encode(message)
val decoded = codec.decode(encoded)

println(decoded.headers)
println(String(decoded.payload))
println(message == decoded)