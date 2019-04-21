#pragma once

#include "../include.h"

namespace glsl_tool {
    template <class T>
    constexpr uint UBOTypeSize()
    {
        if constexpr (std::is_same<std::remove_cv_t<T>, glm::mat4>::value)
        {
            return 64;
        }
        else if constexpr (std::is_same<std::remove_cv_t<T>, glm::vec4>::value ||
                           std::is_same<std::remove_cv_t<T>, glm::vec3>::value)
        {
            return 16;
        }
        else if constexpr (std::is_same<std::remove_cv_t<T>, float>::value ||
                           std::is_same<std::remove_cv_t<T>, iint>::value ||
                           std::is_same<std::remove_cv_t<T>, uint>::value ||
                           std::is_same<std::remove_cv_t<T>, int>::value)
        {
            return 4;
        }
        else
        {
            static_assert(false);
        }
    }

    template <class T>
    constexpr uint UBOTypeAlig()
    {
        return sizeof(T) > 16 ? 16
            : sizeof(T) > 8 ? 16
            : sizeof(T) > 4 ? 8
            : 4;
    }

    //  ���� std140 ����, ������ƫ����
    template <class T>
    constexpr uint UBOOffsetBase(uint base)
    {
        constexpr auto alig = UBOTypeAlig<T>();
        base = (base + alig - 1) / alig * alig;
        return base;
    }

    //  ���� std 140 ����, �����ƫ����
    template <class T>
    constexpr uint UBOOffsetFill(uint base)
    {
        constexpr auto size = UBOTypeSize<T>();
        return UBOOffsetBase<T>(base) + size;
    }
}