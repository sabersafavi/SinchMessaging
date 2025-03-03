class Message(val headers: Map<String, String>, val payload: ByteArray) {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as Message

        if (headers != other.headers) return false
        if (!payload.contentEquals(other.payload)) return false

        return true
    }

    override fun hashCode(): Int {
        var result = headers.hashCode()
        result = 31 * result + payload.contentHashCode()
        return result
    }
}