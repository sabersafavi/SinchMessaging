fun main() {
    val codec = SinchMessageCodec()
    val headers = mapOf("Content-Type" to "application/json", "X-Request-Id" to "12345")
    val payload = "{\"key\":\"value\"}".toByteArray(Charsets.UTF_8)
    val message = Message(headers, payload)

    val encoded = codec.encode(message)
    val decoded = codec.decode(encoded)

    println(decoded.headers)
    println(String(decoded.payload))
    println(message == decoded)
}