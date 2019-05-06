#pragma once

#include "../include.h"

#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
//  ��������, ����ִ��
//  ����������, ��ӡ��־, �жϳ���
#define ASSERT_LOG(cond, fmt, ...)                      \
{                                                       \
    if (!(cond))                                        \
    {                                                   \
        std::cout << "Line: " << __LINE__     << " "    \
                  << "Func: " << __FUNCTION__ << " "    \
                  << SFormat(fmt, __VA_ARGS__)          \
                  << std::endl;                         \
        abort();                                        \
    }                                                   \
}

//  ��������, ���غ�������
//  ����������, �жϳ���
#define ASSERT_RET(cond, ...)	                        \
{                                                       \
    ASSERT_LOG(cond,"");                                \
    return __VA_ARGS__;                                 \
}

//  ��������, ���غ�������
//  ����������, ����ִ��
#define CHECK_RET(cond, ...)	                        \
{                                                       \
    if (!(cond))                                        \
    {                                                   \
        return __VA_ARGS__;                             \
    }                                                   \
}
#else
#define ASSERT_LOG
#define ASSERT_RET
#define CHECK_RET
#endif