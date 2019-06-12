#pragma once

#include "../include.h"
#include "gl_mesh.h"
#include "gl_program.h"
#include "gl_material.h"
#include "gl_texture2d.h"

class RawManager {
public:
    enum ImportTypeEnum {
        kIMPORT_MODEL,      //  导入模型
        kIMPORT_IMAGE,      //  导入图片
        kIMPORT_PROGRAM,    //  导入着色器
        kIMPORT_MATERIAL,   //  导入材质
        kImportTypeEnum,
    };

    enum RawTypeEnum {
        kRAW_MESH,          //  网格
        kRAW_IMAGE,         //  图片
        kRAW_PROGRAM,       //  着色器
        kRAW_MATERIAL,      //  材质
        kRawTypeEnum,
    };

    class ManifestSlot {
    public:
        uint mByteOffset;
        uint mByteLength;
        std::string mName;
        RawTypeEnum mType;

        ManifestSlot() = default;

        ManifestSlot(
            uint byteOffset, 
            uint byteLength, 
            const RawTypeEnum & type,
            const std::string & name)
            : mByteOffset(byteOffset)
            , mByteLength(byteLength)
            , mType(type),mName(name)
        { }

        bool operator==(const std::string & name) const
        {
            return name == mName;
        }
    };

    class Raw {
    protected:
        Raw() {}
    public:
        virtual ~Raw() {}
        virtual void Serialize(std::ofstream & os) = 0;
        virtual void Deserialize(std::ifstream & is) = 0;
    };

    class RawMesh : public Raw {
    public:
        std::vector<uint>           mIndexs;
        std::vector<GLMesh::Vertex> mVertexs;
        
        SERIALIZE_BEG
            SERIALIZE_POD_ARRAY(mIndexs);
            SERIALIZE_POD_ARRAY(mVertexs);
        SERIALIZE_END

        DESERIALIZE_BEG
            DESERIALIZE_POD_ARRAY(mIndexs);
            DESERIALIZE_POD_ARRAY(mVertexs);
        DESERIALIZE_END
    };

    class RawImage : public Raw {
    public:
        uint mW, mH;
        uint mFormat;
        std::string mData;

        SERIALIZE_BEG
            SERIALIZE_POD(mW);
            SERIALIZE_POD(mH);
            SERIALIZE_POD(mFormat);
            SERIALIZE_POD_ARRAY(mData);
        SERIALIZE_END

        DESERIALIZE_BEG
            DESERIALIZE_POD(mW);
            DESERIALIZE_POD(mH);
            DESERIALIZE_POD(mFormat);
            DESERIALIZE_POD_ARRAY(mData);
        DESERIALIZE_END
    };

    class RawProgram : public Raw {
    public:
        std::vector<GLProgram::Pass> mPasss;
        std::string mVSBuffer;
        std::string mGSBuffer;
        std::string mFSBuffer;

        SERIALIZE_BEG
            SERIALIZE_POD_ARRAY(mPasss);
            SERIALIZE_POD_ARRAY(mVSBuffer);
            SERIALIZE_POD_ARRAY(mGSBuffer);
            SERIALIZE_POD_ARRAY(mFSBuffer);
        SERIALIZE_END

        DESERIALIZE_BEG
            DESERIALIZE_POD_ARRAY(mPasss);
            DESERIALIZE_POD_ARRAY(mVSBuffer);
            DESERIALIZE_POD_ARRAY(mGSBuffer);
            DESERIALIZE_POD_ARRAY(mFSBuffer);
        DESERIALIZE_END
    };

    class RawMaterial : public Raw {
    public:
        struct Item {
            GLMaterial::Item::TypeEnum mType;
            std::string mKey;
            std::string mValStr;
            float       mValNum;

            void Serialize(std::ofstream & os)
            {
                auto size = (uint)mKey.size();
                os.write((const char *)&mType, sizeof(mType));
                os.write((const char *)&size,  sizeof(uint));
                os.write((const char *)mKey.data(), size);
                switch (mType)
                {
                case GLMaterial::Item::TypeEnum::kNUMBER:
                    {
                        os.write((const char *)&mValNum, sizeof(float));
                    }
                    break;
                case GLMaterial::Item::TypeEnum::kTEX2D:
                case GLMaterial::Item::TypeEnum::kTEX3D:
                    {
                        size = (uint)mValStr.size();
                        os.write((const char *)&size, sizeof(uint));
                        os.write((const char *)mValStr.data(), size);
                    }
                    break;
                }
            }

            void Deserialize(std::ifstream & is)
            {
                is.read((char *)&mType, sizeof(mType));

                auto size = (uint)mKey.size();
                is.read((char *)&size, sizeof(uint));

                mKey.resize(size);
                is.read((char *)mKey.data(), size);

                switch (mType)
                {
                case GLMaterial::Item::TypeEnum::kNUMBER:
                    {
                        is.read((char *)&mValNum, sizeof(float));
                    }
                    break;
                case GLMaterial::Item::TypeEnum::kTEX2D:
                case GLMaterial::Item::TypeEnum::kTEX3D:
                    {
                        is.read((char *)&size, sizeof(uint));
                        mValStr.resize(size);
                        is.read((char *)mValStr.data(), size);
                    }
                    break;
                }
            }
        };

        std::string mMesh;
        std::string mProgram;
        std::vector<Item> mItems;

        SERIALIZE_BEG
            SERIALIZE_ARRAY(mItems);
            SERIALIZE_POD_ARRAY(mMesh);
            SERIALIZE_POD_ARRAY(mProgram);
        SERIALIZE_END

        DESERIALIZE_BEG
            DESERIALIZE_ARRAY(mItems);
            DESERIALIZE_POD_ARRAY(mMesh);
            DESERIALIZE_POD_ARRAY(mProgram);
        DESERIALIZE_END
    };

    //  清单文件
    static const std::string MANIFEST_SLOT_URL;
    //  资源文件
    static const std::array<std::string, kRawTypeEnum> RAWDATA_URL;
    //  后缀映射
    static const std::array<std::vector<std::string>, kImportTypeEnum> SUFFIX_MAP;

public:
    void Init();
    void BegImport(bool   clear = false);
    void Import(const std::string & url);
    void EndImport();

    //  将原始数据加载到内存
    Raw * LoadRaw(const std::string & name);
    //  将原始数据从内存卸载
    void FreeRaw(const std::string & name);

    //  通过原始数据构造对象
    template <class T>
    T * LoadRes(const std::string & name)
    {
        auto it = _resObjectMap.find(name);
        if (it != _resObjectMap.end())
        {
            ASSERT_LOG(dynamic_cast<T *>(it->second) != nullptr, "Res Type Not Match! {0}, {1}", name, typeid(T).name(), typeid(*it->second).name());
            return reinterpret_cast<T *>(it->second);
        }
        auto res = (GLRes *)nullptr;
        auto raw = std::find(_manifest.begin(), _manifest.end(), name);
        switch (raw->mType)
        {
        case kRAW_MESH: res = LoadResMesh(name); break;
        case kRAW_IMAGE: res = LoadResImage(name); break;
        case kRAW_PROGRAM: res = LoadResProgram(name); break;
        case kRAW_MATERIAL: res = LoadResMaterial(name); break;
        }
        ASSERT_LOG(dynamic_cast<T *>(res) != nullptr, "Res Type Not Match. {0}, {1}, {2}", name, typeid(T).name(), typeid(*res).name());
        return reinterpret_cast<T *>(res);
    }

    //  销毁对象, 保留原始数据
    void FreeRes(const std::string & name);

private:
    void ImportModel(const std::string & url);
    void ImportImage(const std::string & url);
    void ImportProgram(const std::string & url);
    void ImportMaterial(const std::string & url);

    //  从原始数据创建资源
    GLRes * LoadResMesh(const std::string & name);
    GLRes * LoadResImage(const std::string & name);
    GLRes * LoadResProgram(const std::string & name);
    GLRes * LoadResMaterial(const std::string & name);

private:
    //  资源对象
    std::map<std::string, GLRes *> _resObjectMap;
    std::map<std::string, Raw *> _rawObjectMap;
    std::vector<ManifestSlot> _manifest;
};
