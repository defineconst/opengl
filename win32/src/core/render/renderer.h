#pragma once

#include "../include.h"
#include "render_type.h"

class Pipe;
class PipeState;

class Renderer {
public:
    Renderer();
    ~Renderer();

    //  ���ؾ���ջ
    MatrixStack &   GetMatrixStack();
    //  ����1֡������
    uint            GetVertexCount();
    //  ����1֡��Ⱦ��
    uint            GetDrawCount();
    //  ��Ⱦһ֡
    void            RenderOnce();
    //  ��ӹܵ�
    void            AddPipe(Pipe * pipe);
    //  �������
    void            ClearCommands();
    //  ���еƹ�����
    void            SortLightCommands();
    //  Bind ϵ����. �ύ������Ӱ��״̬.
    void Bind(const CameraCommand * command);
    bool Bind(const GLProgram * program, uint pass);
    //  Post ϵ����. �ύ���ݲ�Ӱ��״̬.
    void Post(const glm::mat4   * model);
    void Post(const GLMaterial  * material);
    void Post(const LightCommand * command);
    void Post(const CommandEnum     type, const RenderCommand * command);
    void Post(const DrawTypeEnum    draw, const FragTypeEnum frag, const GLMesh * mesh);

private:
    PipeState *         _state;
    std::vector<Pipe *> _pipes;
};
