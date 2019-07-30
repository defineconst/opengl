#pragma once

#include "pipe.h"

//  GBuffer��Ⱦ�ܵ�
class PipeGBuffer : public Pipe {
public:
    virtual void OnAdd(Renderer * renderer, PipeState * state) override;
    virtual void OnDel(Renderer * renderer, PipeState * state) override;
    virtual void OnUpdate(Renderer * renderer, PipeState * state) override;
};