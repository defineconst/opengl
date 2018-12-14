#pragma once

#include "../include.h"

class Camera;
class Shader;

class Render {
public:
    //  ��������
    enum QueueType {
        //  ����
        kBACKGROUND,
        //  ����
        kGEOMETRY,
        //  ͸��
        kOPATCIY,
        //  ����
        kOVERLAY,
        //  ������ֵ
        kNONE,
        MAX,
    };

    enum CommandType {
        //  ��Ⱦ
        kRENDER,
        //  ����仯
        kTRANSFORM,
    };

    struct CameraInfo {
        Camera * mCamera;
        size_t mID;

        CameraInfo(): mCamera(nullptr), mID(0)
        { }

        CameraInfo(Camera * camera, size_t id): mCamera(camera), mID(id)
        { }

        bool operator ==(size_t id) const
        {
            return mID == id;
        }

        bool operator !=(size_t id) const
        {
            return mID != id;
        }

        bool operator <(size_t id) const
        {
            return mID < id;
        }

        bool operator >(size_t id) const
        {
            return mID > id;
        }
    };

    //  �任����
    struct CommandTransform {
        const bool mIsPush;
        const glm::mat4 * mMatrix;

        CommandTransform(
            const bool ispush, 
            const glm::mat4 * mat4)
            : mIsPush(ispush), mMatrix(mat4)
        { }

        void operator ()();
    };

    //  ��Ⱦ����
    struct CommandRender {
        //void operator ()();
    };

    struct Command {
        size_t mCameraID;
        Shader * mShader;
        CommandType mType;
        std::function<void()> mCallFn;

        Command(): mCameraID(0), mShader(nullptr)
        { }
    };

public:
    Render();

    ~Render();

    void AddCamera(Camera * camera, size_t id);
    void DelCamera(size_t id);

    void PostCommand(const Command & command);

    void DoRender();

private:
    void RenderObjects(CameraInfo & camera);

private:
    std::vector<CameraInfo> _cameras;

    std::vector<std::vector<Command>> _renderQueue;
};