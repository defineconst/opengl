#pragma once

extern "C" {
#include "md5.h"
}

#include <string>

struct MD5Ret {
    char str[20];
    MD5Ret()
    {
        memset(str, 0, sizeof(str));
    }
};

inline std::string MD5(const void * data, const unsigned int len)
{
    char md5[16];
    MD5_CTX  ctx;
    MD5Beg(&ctx);
    MD5Run(&ctx, (unsigned char *)data, len);
    MD5End(&ctx, (unsigned char *)md5);
    return std::string(md5, sizeof(md5));
}

