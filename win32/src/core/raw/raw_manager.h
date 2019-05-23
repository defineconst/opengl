#pragma once

#include "../include.h"
#include "gl_mesh.h"
#include "gl_program.h"
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

    enum RawTypeEnum {
        kRAW_HEAD,
        kRAW_MESH,
        kRAW_IMAGE,
        kRAW_PROGRAM,
        kRAW_MATERIAL,
        kRAW_MD5TOURL,
        kRawTypeEnum,
    };

    struct RawMesh {
        uint mIndexLength;
        uint mVertexLength;
        uint *  mIndexs;
        float * mVertexs;
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
            char mName[MTL_TEX_NAME_LEN];
            char mTexture[MD5_LEN];
        };
        uint mShininess;
        char mMesh[MD5_LEN];
        char mProgram[MD5_LEN];
        Texture mTextures[MTL_TEX2D_LEN];
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
            char mMD5[MD5_LEN];
            uint mByteOffset;
            uint mByteLength;
            Info(const char * md5, uint offset, uint length)
                : mByteOffset(offset), mByteLength(length)
            { 
                memcpy(mMD5, md5, sizeof(mMD5));
            }

            bool operator==(const std::string & md5) const
            {
                return md5 == mMD5;
            }

            Info() { }
        };
        std::vector<Info> mMeshList;
        std::vector<Info> mImageList;
        std::vector<Info> mProgramList;
        std::vector<Info> mMaterialList;
    };

    static const std::array<std::string, kRawTypeEnum> RAWDATA_REF;

    static const std::array<std::vector<std::string>, kImportTypeEnum> SUFFIX_MAP;

public:
    void Init();
    void BegImport();
    void EndImport();
    void Import(const std::string & url);

    //  ��ԭʼ���ݼ��ص��ڴ�
    bool LoadRaw(const std::string & key);
    bool LoadRaw(const std::string & key, RawTypeEnum type);
    //  ��ԭʼ���ݴ��ڴ�ж��
    void FreeRaw(const std::string & key);
    bool FreeRaw(const std::string & key, RawTypeEnum type);

    //  ͨ��ԭʼ���ݹ������
    template <class T>
    T & LoadRes(const std::string & key) const;
    //  ���ٶ���, ����ԭʼ����
    void FreeRes(const GLRes * res);
    void FreeRes(const std::string & key);

private:
    void Import(const std::string & url, ImportTypeEnum type);
    void ImportModel(const std::string & url);
    void ImportImage(const std::string & url);
    void ImportProgram(const std::string & url);
    void ImportMaterial(const std::string & url);

    //  ����ԭʼ���ݵ��ڴ�
    void LoadRawMesh(std::ifstream & istream, const std::string & key);
    void LoadRawImage(std::ifstream & istream, const std::string & key);
    void LoadRawProgram(std::ifstream & istream, const std::string & key);
    void LoadRawMaterial(std::ifstream & istream, const std::string & key);

    //  ��������ԭʼ����
    void ClearRawData();

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
    std::map<std::string, std::string> _rawMD5ToURLMap;
};