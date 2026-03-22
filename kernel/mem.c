void* memcpy(void* dest, const void* src, unsigned long n) {
    // n = Number of bytes

    unsigned char* d = dest;
    const unsigned char* s = src;

    // Iterate n times, copying the byte in s into the same index in d
    for (unsigned long i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}
