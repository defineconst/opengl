#pragma once

#include "../include.h"
#include "render_type.h"

class Pipe;
class PipeState;

class Renderer {
public:
    Renderer();
    ~Renderer();

    MatrixStack &   GetMaterialStack();
    uint            GetVertexCount();
    uint            GetDrawCount();
    void            RenderOnce();
    void            AddPipe(Pipe * pipe);

    //  Bind ϵ����. �ύ������Ӱ��״̬.
    void Bind(const CameraCommand * command);
    bool Bind(const GLProgram     * program, uint pass);

    //  Post ϵ����. �ύ���ݲ�Ӱ��״̬.
    void Post(const Light *         light);
    void Post(const glm::mat4  *    model);
    void Post(const GLMaterial *    material);
    void Post(const CommandEnum     type,       const RenderCommand * command);
    void Post(const DrawTypeEnum    type,       const GLMesh *           mesh);

private:
    PipeState *         _state;
    std::vector<Pipe *> _pipes;
};
