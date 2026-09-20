#ifndef BFC_MD5_H
#define BFC_MD5_H

#include <string>
#include <cstdint>

class MD5 {
public:
    MD5();
    void update(const unsigned char* input, std::size_t length);
    void finalize();
    std::string hexdigest() const;
private:
    void transform(const uint8_t block[64]);
    static void encode(uint8_t* output, const uint32_t* input, std::size_t len);
    static void decode(uint32_t* output, const uint8_t* input, std::size_t len);
    uint32_t state[4], count[2];
    uint8_t buffer[64], digest[16];
    bool finalized;
};
#endif
