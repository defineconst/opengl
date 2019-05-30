#pragma once

#include "../include.h"
#include "gl_mesh.h"
#include "gl_program.h"
#include "gl_material.h"
#include "gl_texture2d.h"

class RawManager {
public:
    enum ImportTypeEnum {
        kIMPORT_MODEL,
        kIMPORT_IMAGE,
        kIMPORT_PROGRAM,
        kIMPORT_MATERIAL,
        kImportTypeEnum,
    };

    enum RawPathEnum {
        kRAW_PATH_HEAD,
        kRAW_PATH_MESH,
        kRAW_PATH_IMAGE,
        kRAW_PATH_PROGRAM,
        kRAW_PATH_MATERIAL,
        kRAW_PATH_LISTING,
        kRawPathEnum,
    };

    enum RawTypeEnum {
        kRAW_TYPE_MESH,
        kRAW_TYPE_IMAGE,
        kRAW_TYPE_PROGRAM,
        kRAW_TYPE_MATERIAL,
        kRawTypeEnum,
    };

    struct RawMesh {
        uint mIndexLength;
        uint mVertexLength;
        uint           * mIndexs;
        GLMesh::Vertex * mVertexs;
    };

    struct RawImage {
        uint mW, mH;
        uint mFormat;
        uint mByteLength;
        uchar *mData;
    };

    struct RawProgram {
        uchar * mData;
        uint mPassLength;
        uint mVSByteLength;
        uint mGSByteLength;
        uint mFSByteLength;
    };

    struct RawMaterial {
        struct Texture {
            char mName[RAW_NAME_LEN];
            char mTexture[RAW_NAME_LEN];
        };
        uint mShininess;
        char mMesh[RAW_NAME_LEN];
        char mProgram[RAW_NAME_LEN];
        Texture mTextures[MTLTEX2D_LEN];
    };

    //  ԭʼ����ͷ����Ϣ
    struct RawHead {
        struct Head {
            uint mMeshLength;
            uint mImageLength;
            uint mProgramLength;
            uint mMaterialLength;
        };

        struct Info {
            char mName[RAW_NAME_LEN];
            uint mByteOffset;
            uint mByteLength;
            Info(const char * name, uint offset, uint length)
                : mByteOffset(offset), mByteLength(length)
            { 
                memcpy(mName, name, sizeof(mName));
            }

            bool operator==(const std::string & name) const
            {
                return name == mName;
            }

            Info() { }
        };
        std::vector<Info> mMeshList;
        std::vector<Info> mImageList;
        std::vector<Info> mProgramList;
        std::vector<Info> mMaterialList;
    };

    static const std::array<std::string, kRawPathEnum> RAWDATA_REF;

    static const std::array<std::vector<std::string>, kImportTypeEnum> SUFFIX_MAP;

public:
    void Init();
    void BegImport();
    void EndImport();
    void Import(const std::string & url);

    //  ��ԭʼ���ݼ��ص��ڴ�
    bool LoadRaw(const std::string & name);
    bool LoadRaw(const std::string & name, RawTypeEnum type);
    //  ��ԭʼ���ݴ��ڴ�ж��
    void FreeRaw(const std::string & name);
    bool FreeRaw(const std::string & name, RawTypeEnum type);

    //  ͨ��ԭʼ���ݹ������
    template <class T>
    T * LoadRes(const std::string & name)
    {
        auto it = _resObjectMap.find(name);
        if (it != _resObjectMap.end())
        {
            ASSERT_LOG(dynamic_cast<T *>(it->second) != nullptr, 
                "Res Type Not Match! {0}, {1}", name, typeid(T).name());
            return reinterpret_cast<T *>(it->second);
        }
        //  û���ڻ�����, ��ԭʼ���ݼ�����Դ
        GLRes * res = nullptr;
        if (std::any_ofv(_rawHead.mMeshList.begin(), _rawHead.mMeshList.end(), name)) 
        {
            res = LoadResMesh(name);
        }
        else if (std::any_ofv(_rawHead.mImageList.begin(), _rawHead.mImageList.end(), name))
        {
            res = LoadResImage(name);
        }
        else if (std::any_ofv(_rawHead.mProgramList.begin(), _rawHead.mProgramList.end(), name))
        {
            res = LoadResProgram(name);
        }
        else if (std::any_ofv(_rawHead.mMaterialList.begin(), _rawHead.mMaterialList.end(), name)) 
        {
            res = LoadResMaterial(name);
        }
        ASSERT_LOG(res != nullptr, "res not found! {0}", name);

        ASSERT_LOG(dynamic_cast<T *>(res) != nullptr, "Res Type Not Match! {0}, {1}, {2}", name, typeid(T).name(), typeid(*res).name());
        return reinterpret_cast<T *>(res);
    }

    //  ���ٶ���, ����ԭʼ����
    void FreeRes(const std::string & name);
    void FreeRes(const GLRes * res);

private:
    void Import(const std::string & url, ImportTypeEnum type);
    void ImportModel(const std::string & url);
    void ImportImage(const std::string & url);
    void ImportProgram(const std::string & url);
    void ImportMaterial(const std::string & url);

    //  ����ԭʼ���ݵ��ڴ�
    void LoadRawMesh(std::ifstream & is, const std::string & name);
    void LoadRawImage(std::ifstream & is, const std::string & name);
    void LoadRawProgram(std::ifstream & is, const std::string & name);
    void LoadRawMaterial(std::ifstream & is, const std::string & name);

    //  ��ԭʼ���ݴ�����Դ
    GLRes * LoadResMesh(const std::string & name);
    GLRes * LoadResImage(const std::string & name);
    GLRes * LoadResProgram(const std::string & name);
    GLRes * LoadResMaterial(const std::string & name);

    //  ��������ԭʼ����
    void ClearRawData();

    //  Ϊ������������
    std::string BuildName(const uchar * data, const uint len);

private:
    RawHead _rawHead;
    //  ��Դ����
    std::map<std::string, GLRes *> _resObjectMap;
    //  ԭʼ��Դ����
    std::map<std::string, RawMesh> _rawMeshMap;
    std::map<std::string, RawImage> _rawImageMap;
    std::map<std::string, RawProgram> _rawProgramMap;
    std::map<std::string, RawMaterial> _rawMaterialMap;
    //  ��Դ��Ӧ��url
    std::map<std::string, std::string> _rawListingMap;
};
