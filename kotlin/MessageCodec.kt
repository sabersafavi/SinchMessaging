interface MessageCodec {
    fun encode(message: Message): ByteArray
    fun decode(data: ByteArray): Message
}