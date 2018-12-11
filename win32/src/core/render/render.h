#pragma once

#include "../include.h"

class Shader;

class Render {
public:
    enum RenderQueueFlag {
        //  ����
        kBACKGROUND,
        //  ����
        kGEOMETRY,
        //  ͸��
        kOPATCIY,
        //  ����
        kOVERLAY,
        //  ������ֵ
        MAX,
    };

    struct Command {
        Shader * mShader;
        std::function<void()> mCallFn;
    };

public:
    Render();
    ~Render();

    void PostCommand(Shader * shader, const std::function<void ()> & callfn);

    void DoRender();

private:
    std::vector<std::vector<Command>> _renderQueue;
};