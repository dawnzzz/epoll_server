#pragma once

#include <cstdint>


#pragma pack(push, 1)
struct TLVHeader {
    uint32_t type;
    uint32_t length;
};
#pragma pack(pop)

const uint32_t TLV_HEADER_LENGTH = sizeof(TLVHeader);
const uint32_t MAX_TLV_LENGTH = 4096;  // 最大允许的TLV包长度

enum TLVHEADER_TYPE {
    STRING,
};

