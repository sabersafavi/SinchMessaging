import java.nio.ByteBuffer

class SinchMessageCodec : MessageCodec {

    companion object {
        private const val MAX_HEADERS = 63
        private const val MAX_HEADER_SIZE = 1023
        private const val MAX_PAYLOAD_SIZE = 256 * 1024
    }

    override fun encode(message: Message): ByteArray {
        validateMessage(message)

        val headerSizes = message.headers.flatMap { (name, value) ->
            listOf(name.toByteArray(Charsets.UTF_8).size.toShort(), value.toByteArray(Charsets.UTF_8).size.toShort())
        }

        val headerContentSize = message.headers.entries.sumOf { (name, value) ->
            name.toByteArray(Charsets.UTF_8).size + value.toByteArray(Charsets.UTF_8).size
        }

        val totalSize = 1 + headerSizes.size * 2 + headerContentSize + message.payload.size
        val buffer = ByteBuffer.allocate(totalSize)

        buffer.put(message.headers.size.toByte())

        headerSizes.forEach { buffer.putShort(it) }

        message.headers.forEach { (name, value) ->
            buffer.put(name.toByteArray(Charsets.UTF_8))
            buffer.put(value.toByteArray(Charsets.UTF_8))
        }

        buffer.put(message.payload)

        return buffer.array()
    }

    override fun decode(data: ByteArray): Message {
        val buffer = ByteBuffer.wrap(data)

        val headerCount = buffer.get().toInt() and 0xFF
        if (headerCount > MAX_HEADERS) {
            throw HeaderCountExceededException("Header count exceeds the maximum allowed ($MAX_HEADERS).")
        }

        val headerSizes = ShortArray(headerCount * 2) {
            if (buffer.remaining() < 2) {
                throw IncompleteHeaderSizeException("Incomplete header size data.")
            }
            buffer.short
        }

        val headers = mutableMapOf<String, String>()
        for (i in 0 until headerCount) {
            val nameSize = headerSizes[i * 2].toInt()
            val valueSize = headerSizes[i * 2 + 1].toInt()

            if (buffer.remaining() < nameSize + valueSize) {
                throw IncompleteHeaderDataException("Incomplete header data.")
            }

            val nameBytes = ByteArray(nameSize)
            buffer.get(nameBytes)
            val name = String(nameBytes, Charsets.UTF_8)

            val valueBytes = ByteArray(valueSize)
            buffer.get(valueBytes)
            val value = String(valueBytes, Charsets.UTF_8)

            headers[name] = value
        }

        val payload = ByteArray(buffer.remaining())
        buffer.get(payload)

        return Message(headers, payload)
    }

    private fun validateMessage(message: Message) {
        if (message.headers.size > MAX_HEADERS) {
            throw HeaderCountExceededException("Maximum $MAX_HEADERS headers allowed.")
        }
        if (message.payload.size > MAX_PAYLOAD_SIZE) {
            throw PayloadSizeExceededException("Maximum payload size of $MAX_PAYLOAD_SIZE bytes allowed.")
        }
        for ((name, value) in message.headers) {
            if (name.length > MAX_HEADER_SIZE || value.length > MAX_HEADER_SIZE) {
                throw HeaderSizeExceededException("Header name and value must be <= $MAX_HEADER_SIZE bytes.")
            }
        }
    }
}

// Custom Exception Classes
class HeaderCountExceededException(message: String) : IllegalArgumentException(message)
class PayloadSizeExceededException(message: String) : IllegalArgumentException(message)
class HeaderSizeExceededException(message: String) : IllegalArgumentException(message)
class IncompleteHeaderSizeException(message: String) : IllegalArgumentException(message)
class IncompleteHeaderDataException(message: String) : IllegalArgumentException(message)