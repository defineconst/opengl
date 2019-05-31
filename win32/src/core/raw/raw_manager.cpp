#include "raw_manager.h"
#include "../cfg/cfg_manager.h"
//  STB IMAGE ����ͼƬ��
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../third/stb_image.h"
#include "../third/assimp/postprocess.h"
#include "../third/assimp/Importer.hpp"
#include "../third/assimp/scene.h"

//  ԭʼ��������·��
const std::array<std::string, RawManager::kRawTypeEnum> RawManager::RAWDATA_REF = {
    {
        "res/raw/head.db",
        "res/raw/mesh.db",
        "res/raw/image.db",
        "res/raw/program.db",
        "res/raw/material.db",
        "res/raw/manifest.txt",
    }
};

//  �ļ���׺��������
const std::array<std::vector<std::string>, RawManager::kImportTypeEnum> RawManager::SUFFIX_MAP = {
    { 
        { ".obj", ".fbx" }, 
        { ".png", ".jpg" }, 
        { ".program" },
        { ".mtl" },
    }
};

void RawManager::Init()
{
    std::ifstream is;
    //  ͷ����Ϣ
    is.open(RAWDATA_REF[kRAW_HEAD], std::ios::binary);
    ASSERT_LOG(is, "��ȡԭʼ����ʧ��!: {0}", RAWDATA_REF[kRAW_HEAD]);

    RawHead::Head head;
    is.read((char *)&head, sizeof(RawHead::Head));

    _rawHead.mMeshList.resize(head.mMeshLength);
    is.read((char *)_rawHead.mMeshList.data(), head.mMeshLength * sizeof(RawHead::Info));

    _rawHead.mImageList.resize(head.mImageLength);
    is.read((char *)_rawHead.mImageList.data(), head.mImageLength * sizeof(RawHead::Info));

    _rawHead.mProgramList.resize(head.mProgramLength);
    is.read((char *)_rawHead.mProgramList.data(), head.mProgramLength * sizeof(RawHead::Info));

    _rawHead.mMaterialList.resize(head.mMaterialLength);
    is.read((char *)_rawHead.mMaterialList.data(), head.mMaterialLength * sizeof(RawHead::Info));

    is.close();

    ClearRawData();
    ClearResData();
}

void RawManager::BegImport(bool clear)
{
    if (clear)
    {
        for (auto & path : RAWDATA_REF)
        {
            std::remove(RAWDATA_REF[kRAW_HEAD].c_str());
        }
    }
    if (!file_tool::IsFileExists(RAWDATA_REF[kRAW_HEAD]))
    {
        std::ofstream os(RAWDATA_REF[kRAW_HEAD], std::ios::binary);
        std::fill_n(std::ostream_iterator<char>(os),
                       sizeof(RawHead::Head), '\0');
        os.close();
    }
    Init();
}

void RawManager::EndImport()
{
    //  д������ͷ�ļ�
    RawHead::Head head;
    head.mMeshLength = _rawHead.mMeshList.size();
    head.mImageLength = _rawHead.mImageList.size();
    head.mProgramLength = _rawHead.mProgramList.size();
    head.mMaterialLength = _rawHead.mMaterialList.size();

    std::ofstream os(RAWDATA_REF[kRAW_HEAD], std::ios::binary);
    os.write((const char *)&head, sizeof(RawHead::Head));
    os.write((const char *)_rawHead.mMeshList.data(), head.mMeshLength * sizeof(RawHead::Info));
    os.write((const char *)_rawHead.mImageList.data(), head.mImageLength * sizeof(RawHead::Info));
    os.write((const char *)_rawHead.mProgramList.data(), head.mProgramLength * sizeof(RawHead::Info));
    os.write((const char *)_rawHead.mMaterialList.data(), head.mMaterialLength * sizeof(RawHead::Info));
    os.close();

    os.open(RAWDATA_REF[kRAW_MANIFEST]);
    for (const auto & pair : _rawManifestMap)
    {
        os << SFormat("[{0}]={1}\n", pair.first, pair.second);
    }
    os.close();

    _rawManifestMap.clear();
}

void RawManager::Import(const std::string & url)
{
    ImportTypeEnum type = kImportTypeEnum;
    auto name = string_tool::QueryFileSuffix(url);
    for (auto i = 0u; i != SUFFIX_MAP.size(); ++i)
    {
        if (std::find(SUFFIX_MAP.at(i).begin(), 
                      SUFFIX_MAP.at(i).end(), name) != SUFFIX_MAP.at(i).end())
        {
            type = (ImportTypeEnum)i;
        }
    }
    ASSERT_LOG(type != kImportTypeEnum, "�������Դ��ʽ�Ƿ�!: {0}", url);
    Import(url, type);
}

bool RawManager::LoadRaw(const std::string & name)
{
    if (LoadRaw(name, RawTypeEnum::kRAW_MESH)) { return true; }
    if (LoadRaw(name, RawTypeEnum::kRAW_IMAGE)) { return true; }
    if (LoadRaw(name, RawTypeEnum::kRAW_PROGRAM)) { return true; }
    if (LoadRaw(name, RawTypeEnum::kRAW_MATERIAL)) { return true; }
    return false;
}

bool RawManager::LoadRaw(const std::string & name, RawTypeEnum type)
{
    //  ��������
    std::vector<RawHead::Info> * list[] = {
        nullptr,
        &_rawHead.mMeshList,
        &_rawHead.mImageList,
        &_rawHead.mProgramList,
        &_rawHead.mMaterialList,
    };
    
    auto it = std::find(list[type]->begin(), 
                        list[type]->end(), name);
    if (it == list[type]->end()) { return false; }
    
    //  ���غ���
    void (RawManager::*func[])(std::ifstream &, const std::string &) = {
        nullptr,
        &RawManager::LoadRawMesh,
        &RawManager::LoadRawImage,
        &RawManager::LoadRawProgram,
        &RawManager::LoadRawMaterial,
    };

    std::ifstream is(RAWDATA_REF[type], std::ios::binary);
    ASSERT_LOG(is, "�Ҳ����ļ�: {0}", RAWDATA_REF[type]);
    is.seekg(it->mByteOffset, std::ios::beg);
    (this->*func[type])(is, name);
    ASSERT_LOG((uint)is.tellg() - it->mByteOffset == it->mByteLength, 
        "LengthErr. Type: {0}. Name: {1}. Length: {2}. ReadLength: {3}.", 
        type, name, it->mByteLength, (uint)is.tellg() - it->mByteOffset);
    is.close();
    return true;
}

void RawManager::FreeRaw(const std::string & name)
{
    if (FreeRaw(name, kRAW_MESH)) return;
    if (FreeRaw(name, kRAW_IMAGE)) return;
    if (FreeRaw(name, kRAW_PROGRAM)) return;
    if (FreeRaw(name, kRAW_MATERIAL)) return;
}

bool RawManager::FreeRaw(const std::string & name, RawTypeEnum type)
{
    switch (type)
    {
    case RawManager::kRAW_MESH:
        {
            auto it = _rawMeshMap.find(name);
            if (it != _rawMeshMap.end())
            {
                delete[] it->second.mIndexs;
                delete[] it->second.mVertexs;
                _rawMeshMap.erase(it);
                return true;
            }
        }
        break;
    case RawManager::kRAW_IMAGE:
        {
            auto it = _rawImageMap.find(name);
            if (it != _rawImageMap.end())
            {
                stbi_image_free(it->second.mData);
                _rawImageMap.erase(it);
                return true;
            }
        }
        break;
    case RawManager::kRAW_PROGRAM:
        {
            auto it = _rawProgramMap.find(name);
            if (it != _rawProgramMap.end())
            {
                delete[] it->second.mData;
                _rawProgramMap.erase(it);
                return true;
            }
        }
        break;
    case RawManager::kRAW_MATERIAL:
        {
            auto it = _rawMaterialMap.find(name);
            if (it != _rawMaterialMap.end())
            {
                _rawMaterialMap.erase(it);
                return true;
            }
        }
        break;
    }
    return false;
}

void RawManager::FreeRes(const std::string & name)
{
    auto it = _resObjectMap.find(name);
    if (it != _resObjectMap.end())
    {
        delete it->second; _resObjectMap.erase(it);
    }
}

void RawManager::FreeRes(const GLRes * res)
{
    auto it = std::find_if(_resObjectMap.begin(), _resObjectMap.end(), [&](const auto & pair)
        {
            return pair.second == res;
        });
    if (it != _resObjectMap.end())
    {
        FreeRes(it->first);
    }
}

void RawManager::Import(const std::string & url, ImportTypeEnum type)
{
    switch (type)
    {
    case RawManager::kIMPORT_MODEL: ImportModel(url); break;
    case RawManager::kIMPORT_IMAGE: ImportImage(url); break;
    case RawManager::kIMPORT_PROGRAM: ImportProgram(url); break;
    case RawManager::kIMPORT_MATERIAL: ImportMaterial(url); break;
    default: ASSERT_LOG(false, "�������Դ��ʽ�Ƿ�!: {0}, {1}", type, url); break;
    }
}

void RawManager::ImportModel(const std::string & url)
{
    std::function<void(aiNode * node, const aiScene * scene, const std::string & directory)> LoadNode;
    std::function<void(aiMesh * mesh, const aiScene * scene, const std::string & directory)> LoadImage;
    std::function<void(aiMesh * mesh, std::vector<GLMesh::Vertex> & vertexs, std::vector<uint> & indexs)> LoadMesh;

    LoadImage = [&, this](aiMesh * mesh, const aiScene * scene, const std::string & directory)
    {
        aiString textureURL;
        auto aiMaterial = scene->mMaterials[mesh->mMaterialIndex];
        for (auto i = 0; i != aiMaterial->GetTextureCount(aiTextureType_DIFFUSE); ++i)
        {
            aiMaterial->GetTexture(aiTextureType_DIFFUSE, i, &textureURL);
            auto fullpath = directory + std::string(textureURL.C_Str());
            auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
            ImportImage(string_tool::Replace(urlpath, "\\", "/"));
        }

        if (0 != aiMaterial->GetTextureCount(aiTextureType_REFLECTION))
        {
            aiMaterial->GetTexture(aiTextureType_REFLECTION, 0, &textureURL);
            auto fullpath = directory + std::string(textureURL.C_Str());
            auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
            ImportImage(string_tool::Replace(urlpath, "\\", "/"));
        }

        if (0 != aiMaterial->GetTextureCount(aiTextureType_SPECULAR))
        {
            aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &textureURL);
            auto fullpath = directory + std::string(textureURL.C_Str());
            auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
            ImportImage(string_tool::Replace(urlpath, "\\", "/"));
        }

        if (0 != aiMaterial->GetTextureCount(aiTextureType_HEIGHT))
        {
            aiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &textureURL);
            auto fullpath = directory + std::string(textureURL.C_Str());
            auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
            ImportImage(string_tool::Replace(urlpath, "\\", "/"));
        }
    };

    LoadMesh = [&, this](aiMesh * mesh, std::vector<GLMesh::Vertex> & vertexs, std::vector<uint> & indexs)
    {
        auto indexBase = vertexs.size();
        for (auto i = 0; i != mesh->mNumVertices; ++i)
        {
            GLMesh::Vertex vertex;
            //	position
            vertex.v.x = mesh->mVertices[i].x;
            vertex.v.y = mesh->mVertices[i].y;
            vertex.v.z = mesh->mVertices[i].z;
            //	normal
            vertex.n.x = mesh->mNormals[i].x;
            vertex.n.y = mesh->mNormals[i].y;
            vertex.n.z = mesh->mNormals[i].z;
            //  color
            vertex.c.r = 1.0f;
            vertex.c.g = 1.0f;
            vertex.c.b = 1.0f;
            vertex.c.a = 1.0f;
            //	tan
            vertex.tan.x = mesh->mTangents[i].x;
            vertex.tan.y = mesh->mTangents[i].y;
            vertex.tan.z = mesh->mTangents[i].z;
            //	bitan
            vertex.bitan.x = mesh->mBitangents[i].x;
            vertex.bitan.y = mesh->mBitangents[i].y;
            vertex.bitan.z = mesh->mBitangents[i].z;
            //	uv
            vertex.uv.x = mesh->mTextureCoords[0][i].x;
            vertex.uv.y = mesh->mTextureCoords[0][i].y;
            vertexs.push_back(vertex);
        }
        if (!indexs.empty())
        {
            indexs.push_back((uint)~0);
        }
        for (auto i = 0; i != mesh->mNumFaces; ++i)
        {
            std::transform(mesh->mFaces[i].mIndices,
                           mesh->mFaces[i].mIndices + mesh->mFaces[i].mNumIndices,
                           std::back_inserter(indexs), [&](const auto & v) { return v + indexBase; });
        }
    };

    LoadNode = [&, this](aiNode * node, const aiScene * scene, const std::string & directory)
    {
        std::vector<uint>           indexs;
        std::vector<GLMesh::Vertex> vertexs;
        for (auto i = 0; i != node->mNumMeshes; ++i)
        {
            LoadMesh(scene->mMeshes[node->mMeshes[i]], vertexs, indexs);

            LoadImage(scene->mMeshes[node->mMeshes[i]], scene, directory);
        }
        RawMesh rawMesh;
        //  ��������
        auto indexByteLength = indexs.size() * sizeof(uint);
        rawMesh.mIndexLength = indexs.size();
        rawMesh.mIndexs = new uint[rawMesh.mIndexLength];
        memcpy(rawMesh.mIndexs, indexs.data(), indexByteLength);
        //  ��������
        auto vertexByteLength = vertexs.size() * sizeof(GLMesh::Vertex);
        rawMesh.mVertexLength = vertexs.size();
        rawMesh.mVertexs = new GLMesh::Vertex[rawMesh.mVertexLength];
        memcpy(rawMesh.mVertexs, vertexs.data(), vertexByteLength);

        //  ��������
        auto length = indexByteLength + vertexByteLength;
        auto buffer = new uchar[length];
        memcpy(buffer                  , rawMesh.mIndexs, indexByteLength);
        memcpy(buffer + indexByteLength, rawMesh.mVertexs, vertexByteLength);
        auto name = BuildName(buffer, length);
        delete[] buffer;

        //  Write File
        std::ofstream os(RAWDATA_REF[kRAW_MESH], std::ios::binary | std::ios::app);
        ASSERT_LOG(os, "Import Model Failed. {0}", RAWDATA_REF[kRAW_MESH]);
        auto byteOffset = file_tool::GetFileLength(os);
        os.write((const char *)&rawMesh.mIndexLength, sizeof(uint));
        os.write((const char *)&rawMesh.mVertexLength, sizeof(uint));
        os.write((const char *)rawMesh.mIndexs, sizeof(uint)            * rawMesh.mIndexLength);
        os.write((const char *)rawMesh.mVertexs, sizeof(GLMesh::Vertex) * rawMesh.mVertexLength);
        _rawHead.mMeshList.emplace_back(name.c_str(), (uint)byteOffset, (uint)os.tellp() - byteOffset);
        os.close();

        //  �����嵥
        _rawManifestMap.insert(std::make_pair(name, url));

        for (auto i = 0; i != node->mNumChildren; ++i)
        {
            LoadNode(node->mChildren[i], scene, directory);
        }
    };

    Assimp::Importer importer;
    auto scene = importer.ReadFile(url, aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_FlipUVs);
    ASSERT_LOG(nullptr != scene, "Error URL: {0}", url);
    ASSERT_LOG(nullptr != scene->mRootNode, "Error URL: {0}", url);
    for (auto i = 0; i != scene->mRootNode->mNumChildren; ++i)
    {
        LoadNode(scene->mRootNode->mChildren[i], scene, url.substr(0, 1 + url.find_last_of('/')));
    }
}

void RawManager::ImportImage(const std::string & url)
{
    RawImage rawImage;
    rawImage.mData = stbi_load(url.c_str(),
        (int *)&rawImage.mW, 
        (int *)&rawImage.mH,   
        (int *)&rawImage.mFormat, 0);
    ASSERT_LOG(rawImage.mData != nullptr, "URL: {0}", url);

    rawImage.mByteLength = rawImage.mW * rawImage.mH * rawImage.mFormat;

    switch (rawImage.mFormat)
    {
    case 1: rawImage.mFormat = GL_RED; break;
    case 3: rawImage.mFormat = GL_RGB; break;
    case 4: rawImage.mFormat = GL_RGBA; break;
    }

    auto name = BuildName(rawImage.mData, rawImage.mByteLength);

    //  Write File
    std::ofstream os(RAWDATA_REF[kRAW_IMAGE], std::ios::binary | std::ios::app);
    ASSERT_LOG(os, "Import Image Failed. {0}", RAWDATA_REF[kRAW_IMAGE]);
    auto byteOffset = file_tool::GetFileLength(os);
    os.write((const char *)&rawImage.mW, sizeof(uint));
    os.write((const char *)&rawImage.mH, sizeof(uint));
    os.write((const char *)&rawImage.mFormat, sizeof(uint));
    os.write((const char *)&rawImage.mByteLength, sizeof(uint));
    os.write((const char *)rawImage.mData, rawImage.mByteLength);
    _rawHead.mImageList.emplace_back(name.c_str(), (uint)byteOffset, (uint)os.tellp() - byteOffset);
    os.close();

    //  �����嵥
    _rawManifestMap.insert(std::make_pair(name, url));
}

void RawManager::ImportProgram(const std::string & url)
{
    //  ����Shader
    const auto ParseShader = [&](
        std::stringstream & is, 
        const char * endflag, 
        std::string & buffer)
    {
        std::string line;
        while (std::getline(is, line))
        {
            if (string_tool::IsEqualSkipSpace(line, endflag))
            {
                break;
            }
            buffer.append(line);
            buffer.append("\n");
        }
        ASSERT_LOG(string_tool::IsEqualSkipSpace(line, endflag), "EndFlag Error: {0}", endflag);
    };

    //  ����Pass
    const auto ParsePass = [&](
        std::stringstream & is, 
        const char * endFlag,
        std::string & vBuffer, 
        std::string & gBuffer, 
        std::string & fBuffer, 
        GLProgram::PassAttr * passAttr)
    {
        std::string line;
        while (std::getline(is, line))
        {
            if (!string_tool::IsPrint(line)) { continue; }

            if (string_tool::IsEqualSkipSpace(line, endFlag))
            {
                break;
            }

            if (string_tool::IsEqualSkipSpace(line, "CullFace") 
                || string_tool::IsEqualSkipSpace(line, "BlendMode")
                || string_tool::IsEqualSkipSpace(line, "DepthTest")
                || string_tool::IsEqualSkipSpace(line, "DepthWrite")
                || string_tool::IsEqualSkipSpace(line, "StencilTest")
                || string_tool::IsEqualSkipSpace(line, "RenderQueue")
                || string_tool::IsEqualSkipSpace(line, "RenderType")
                || string_tool::IsEqualSkipSpace(line, "DrawType")
                || string_tool::IsEqualSkipSpace(line, "PassName"))
            {
                ASSERT_LOG(passAttr != nullptr, "����Pass���Դ���: {0}, {1}", endFlag, line);
                std::stringstream ss;
                std::string word;
                ss.str(line);
                ss >> word;
                
                if (word == "CullFace")
                {
                    ss >> word;
                    if (word == "Front")                    { passAttr->vCullFace = GL_FRONT; }
                    else if (word == "Back")                { passAttr->vCullFace = GL_BACK; }
                    else if (word == "FrontBack")           { passAttr->vCullFace = GL_FRONT_AND_BACK; }
                    else { ASSERT_LOG(false, "����Pass���Դ���: {0}, {1}", word, line); }
                }
                else if (word == "BlendMode")
                {
                    ss >> word;
                    if (word == "Zero")                     { passAttr->vBlendSrc = GL_ZERO; }
                    else if (word == "One")                 { passAttr->vBlendSrc = GL_ONE; }
                    else if (word == "SrcColor")            { passAttr->vBlendSrc = GL_SRC_COLOR; }
                    else if (word == "SrcAlpha")            { passAttr->vBlendSrc = GL_SRC_ALPHA; }
                    else if (word == "DstAlpha")            { passAttr->vBlendSrc = GL_DST_ALPHA; }
                    else if (word == "OneMinusSrcColor")    { passAttr->vBlendSrc = GL_ONE_MINUS_SRC_COLOR; }
                    else if (word == "OneMinusSrcAlpha")    { passAttr->vBlendSrc = GL_ONE_MINUS_SRC_ALPHA; }
                    else if (word == "OneMinusDstAlpha")    { passAttr->vBlendSrc = GL_ONE_MINUS_DST_ALPHA; }
                    else { ASSERT_LOG(false, "����Pass���Դ���: {0}, {1}", word, line); }

                    ss >> word;
                    if (word == "Zero")                     { passAttr->vBlendDst = GL_ZERO; }
                    else if (word == "One")                 { passAttr->vBlendDst = GL_ONE; }
                    else if (word == "SrcColor")            { passAttr->vBlendDst = GL_SRC_COLOR; }
                    else if (word == "SrcAlpha")            { passAttr->vBlendDst = GL_SRC_ALPHA; }
                    else if (word == "DstAlpha")            { passAttr->vBlendDst = GL_DST_ALPHA; }
                    else if (word == "OneMinusSrcColor")    { passAttr->vBlendDst = GL_ONE_MINUS_SRC_COLOR; }
                    else if (word == "OneMinusSrcAlpha")    { passAttr->vBlendDst = GL_ONE_MINUS_SRC_ALPHA; }
                    else if (word == "OneMinusDstAlpha")    { passAttr->vBlendDst = GL_ONE_MINUS_DST_ALPHA; }
                    else { ASSERT_LOG(false, "����Pass���Դ���: {0}, {1}", word, line); }
                }
                else if (word == "DepthTest")               { passAttr->bDepthTest = true; }
                else if (word == "DepthWrite")              { passAttr->bDepthWrite = true; }
                else if (word == "StencilTest")
                {
                    ss >> word;
                    if (word == "Keep")             { passAttr->vStencilOpFail = GL_KEEP; }
                    else if (word == "Zero")        { passAttr->vStencilOpFail = GL_ZERO; }
                    else if (word == "Incr")        { passAttr->vStencilOpFail = GL_INCR; }
                    else if (word == "Decr")        { passAttr->vStencilOpFail = GL_DECR; }
                    else if (word == "Invert")      { passAttr->vStencilOpFail = GL_INVERT; }
                    else if (word == "Replace")     { passAttr->vStencilOpFail = GL_REPLACE; }
                    else { ASSERT_LOG(false, "����Pass���Դ���: {0}, {1}", word, line); }

                    ss >> word;
                    if (word == "Keep")             { passAttr->vStencilOpZFail = GL_KEEP; }
                    else if (word == "Zero")        { passAttr->vStencilOpZFail = GL_ZERO; }
                    else if (word == "Incr")        { passAttr->vStencilOpZFail = GL_INCR; }
                    else if (word == "Decr")        { passAttr->vStencilOpZFail = GL_DECR; }
                    else if (word == "Invert")      { passAttr->vStencilOpZFail = GL_INVERT; }
                    else if (word == "Replace")     { passAttr->vStencilOpZFail = GL_REPLACE; }
                    else { ASSERT_LOG(false, "����Pass���Դ���: {0}, {1}", word, line); }

                    ss >> word;
                    if (word == "Keep")             { passAttr->vStencilOpZPass = GL_KEEP; }
                    else if (word == "Zero")        { passAttr->vStencilOpZPass = GL_ZERO; }
                    else if (word == "Incr")        { passAttr->vStencilOpZPass = GL_INCR; }
                    else if (word == "Decr")        { passAttr->vStencilOpZPass = GL_DECR; }
                    else if (word == "Invert")      { passAttr->vStencilOpZPass = GL_INVERT; }
                    else if (word == "Replace")     { passAttr->vStencilOpZPass = GL_REPLACE; }
                    else { ASSERT_LOG(false, "����Pass���Դ���: {0}, {1}", word, line); }

                    ss >> word;
                    if (word == "Never")            { passAttr->vStencilFunc = GL_NEVER; }
                    else if (word == "Less")        { passAttr->vStencilFunc = GL_LESS; }
                    else if (word == "Equal")       { passAttr->vStencilFunc = GL_EQUAL; }
                    else if (word == "Greater")     { passAttr->vStencilFunc = GL_GREATER; }
                    else if (word == "NotEqual")    { passAttr->vStencilFunc = GL_NOTEQUAL; }
                    else if (word == "Gequal")      { passAttr->vStencilFunc = GL_GEQUAL; }
                    else if (word == "Always")      { passAttr->vStencilFunc = GL_ALWAYS; }
                    else { ASSERT_LOG(false, "����Pass���Դ���: {0}, {1}", word, line); }

                    ss >> word;
                    passAttr->vStencilMask = std::stoi(word);

                    ss >> word;
                    passAttr->vStencilRef = std::stoi(word);
                }
                else if (word == "PassName")
                {
                    ss >> word;
                    memcpy(passAttr->mPassName, word.c_str(), word.size());
                }
                else if (word == "RenderQueue")
                {
                    ss >> word;
                    if (word == "Background")       { passAttr->vRenderQueue = 0; }
                    else if (word == "Geometric")   { passAttr->vRenderQueue = 1; }
                    else if (word == "Opacity")     { passAttr->vRenderQueue = 2; }
                    else if (word == "Top")         { passAttr->vRenderQueue = 3; }
                    else { ASSERT_LOG(false, "����Pass���Դ���: {0}, {1}", word, line); }
                }
                else if (word == "RenderType")
                {
                    ss >> word;
                    if (word == "Light")            { passAttr->vRenderType = 0; }
                    else if (word == "Shadow")      { passAttr->vRenderType = 1; }
                    else if (word == "Forward")     { passAttr->vRenderType = 2; }
                    else if (word == "Deferred")    { passAttr->vRenderType = 3; }
                    else { ASSERT_LOG(false, "����Pass���Դ���: {0}, {1}", word, line); }
                }
                else if (word == "DrawType")
                {
                    ss >> word;
                    if (word == "Instance")         { passAttr->vDrawType = 0; }
                    else if (word == "Vertex")      { passAttr->vDrawType = 1; }
                    else if (word == "Index")       { passAttr->vDrawType = 2; }
                    else { ASSERT_LOG(false, "����Pass���Դ���: {0}, {1}", word, line); }
                }
                else { ASSERT_LOG(false, "����Pass���Դ���: {0}, {1}", word, line); }
            }
            else if (string_tool::IsEqualSkipSpace(line, "VShader Beg"))
            {
                ParseShader(is, "VShader End", vBuffer);
            }
            else if (string_tool::IsEqualSkipSpace(line, "GShader Beg"))
            {
                ParseShader(is, "GShader End", gBuffer);
            }
            else if (string_tool::IsEqualSkipSpace(line, "FShader Beg"))
            {
                ParseShader(is, "FShader End", fBuffer);
            }
            else
            {
                vBuffer.append(line);
                vBuffer.append("\n");
                gBuffer.append(line);
                gBuffer.append("\n");
                fBuffer.append(line);
                fBuffer.append("\n");
            }
        }
        ASSERT_LOG(string_tool::IsEqualSkipSpace(line, endFlag), "EndFlag Error: {0}", endFlag);
    };

    //  ����Include
    const auto ParseInclude = [](const std::string & word)
    {
        auto pos = word.find_last_of(' ');
        ASSERT_LOG(pos != std::string::npos, "Include Error: {0}", word);
        auto url = word.substr(pos + 1);

        std::ifstream is(url);
        ASSERT_LOG(is, "Include URL Error: {0}", url);

        std::string data;
        std::string line;
        while (std::getline(is, line))
        {
            data.append(line);
            data.append("\n");
        }
        is.close();
        return data;
    };

    //  �滻Include
    const auto OpenProgramFile = [&](const std::string & url)
    {
        std::string line,data;
        std::ifstream is(url);
        ASSERT_LOG(is, "URL Error: {0}", url);
        while (std::getline(is, line))
        {
            if (string_tool::IsEqualSkipSpace(line, "#include"))
            {
                data.append(ParseInclude(line));
                data.append("\n");
            }
            else
            {
                data.append(line);
                data.append("\n");
            }
        }
        is.close();

        std::stringstream ss;
        ss.str(data);
        return std::move(ss);
    };

    //  ����GL Program����
    auto is = OpenProgramFile(url);
    std::string line;
    std::string vCommonBuffer;
    std::string gCommonBuffer;
    std::string fCommonBuffer;
    std::vector<std::tuple<
        std::string,
        std::string,
        std::string,
        GLProgram::PassAttr>> passs;
    while (std::getline(is, line))
    {
        if (string_tool::IsEqualSkipSpace(line, "Common Beg"))
        {
            ParsePass(is, "Common End", vCommonBuffer, gCommonBuffer, fCommonBuffer, nullptr);
        }
        else if (string_tool::IsEqualSkipSpace(line, "Pass Beg"))
        {
            decltype(passs)::value_type pass;
            ParsePass(is, "Pass End",
                std::get<0>(pass),  std::get<1>(pass),
                std::get<2>(pass), &std::get<3>(pass));
            passs.push_back(pass);
        }
    }

    //  ����GL Program����
    std::string vBuffer, gBuffer, fBuffer;
    for (auto i = 0; i != passs.size(); ++i)
    {
        vBuffer.append(std::get<0>(passs.at(i)));
        gBuffer.append(std::get<1>(passs.at(i)));
        fBuffer.append(std::get<2>(passs.at(i)));
    }

    //  д��GL Program����
    RawProgram rawProgram   = { 0 };
    rawProgram.mPassLength  = passs.size();
    if (!vBuffer.empty())
    {
        vCommonBuffer.append(vBuffer);
        rawProgram.mVSByteLength = vCommonBuffer.size();
    }
    if (!gBuffer.empty())
    {
        gCommonBuffer.append(gBuffer);
        rawProgram.mGSByteLength = gCommonBuffer.size();
    }
    if (!fBuffer.empty())
    {
        fCommonBuffer.append(fBuffer);
        rawProgram.mFSByteLength = fCommonBuffer.size();
    }

    auto byteLength = rawProgram.mPassLength * sizeof(GLProgram::PassAttr)
                    + rawProgram.mVSByteLength
                    + rawProgram.mGSByteLength
                    + rawProgram.mFSByteLength;
    rawProgram.mData = new uchar[byteLength];

    auto ptr = rawProgram.mData;
    for (auto i = 0; i != passs.size(); ++i)
    {
        memcpy(ptr, &std::get<3>(passs.at(i)), sizeof(GLProgram::PassAttr));
        ptr += sizeof(GLProgram::PassAttr);
    }
    if (rawProgram.mVSByteLength != 0)
    {
        memcpy(ptr, vCommonBuffer.data(), rawProgram.mVSByteLength);
        ptr += rawProgram.mVSByteLength;
    }
    if (rawProgram.mGSByteLength != 0)
    {
        memcpy(ptr, gCommonBuffer.data(), rawProgram.mGSByteLength);
        ptr += rawProgram.mGSByteLength;
    }
    if (rawProgram.mFSByteLength != 0)
    {
        memcpy(ptr, fCommonBuffer.data(), rawProgram.mFSByteLength);
        ptr += rawProgram.mFSByteLength;
    }
    ASSERT_LOG(ptr - rawProgram.mData == byteLength, "");

    auto name = BuildName(rawProgram.mData, byteLength);

    //  Write File
    std::ofstream os(RAWDATA_REF[kRAW_PROGRAM], std::ios::binary | std::ios::app);
    ASSERT_LOG(os, "Import Program Failed. {0}", RAWDATA_REF[kRAW_PROGRAM]);
    auto byteOffset = file_tool::GetFileLength(os);
    os.write((const char *)&rawProgram.mPassLength, sizeof(uint));
    os.write((const char *)&rawProgram.mVSByteLength, sizeof(uint));
    os.write((const char *)&rawProgram.mGSByteLength, sizeof(uint));
    os.write((const char *)&rawProgram.mFSByteLength, sizeof(uint));
    os.write((const char *)rawProgram.mData, rawProgram.mVSByteLength
        + rawProgram.mGSByteLength + rawProgram.mFSByteLength
        + rawProgram.mPassLength * sizeof(GLProgram::PassAttr));
    _rawHead.mProgramList.emplace_back(name.c_str(), (uint)byteOffset, (uint)os.tellp() - byteOffset);
    os.close();

    //  �����嵥
    _rawManifestMap.insert(std::make_pair(name, url));
}

void RawManager::ImportMaterial(const std::string & url)
{
    std::ifstream is(url);
    ASSERT_LOG(is, "URL: {0}", url);

    RawMaterial rawMaterial = { 0 };
    is >> rawMaterial.mShininess;
    is >> rawMaterial.mMesh;
    is >> rawMaterial.mProgram;
    for (auto & texture : rawMaterial.mTextures)
    {
        is >> texture.mName;
        is >> texture.mTexture;
        if (is.eof()) break;
    }
    is.close();

    auto name = BuildName((uchar *)&rawMaterial, sizeof(RawMaterial));

    //  Write File
    std::ofstream os(RAWDATA_REF[kRAW_MATERIAL], std::ios::binary | std::ios::app);
    ASSERT_LOG(os, "Import Material Failed. {0}", RAWDATA_REF[kRAW_MATERIAL]);
    auto byteOffset = file_tool::GetFileLength(os);
    os.write((const char *)&rawMaterial, sizeof(RawMaterial));
    _rawHead.mMaterialList.emplace_back(name.c_str(), (uint)byteOffset, (uint)os.tellp() - byteOffset);
    os.close();

    //  �����嵥
    _rawManifestMap.insert(std::make_pair(name, url));
}

void RawManager::LoadRawMesh(std::ifstream & is, const std::string & name)
{
    RawMesh rawMesh = { 0 };
    is.read((char *)&rawMesh, sizeof(uint) + sizeof(uint));
    rawMesh.mIndexs = new uint[rawMesh.mIndexLength];
    is.read((char *)rawMesh.mIndexs, sizeof(uint) * rawMesh.mIndexLength);
    rawMesh.mVertexs = new GLMesh::Vertex[rawMesh.mVertexLength];
    is.read((char *)rawMesh.mVertexs, sizeof(GLMesh::Vertex) * rawMesh.mVertexLength);

    _rawMeshMap.insert(std::make_pair(name, rawMesh));
}

void RawManager::LoadRawImage(std::ifstream & is, const std::string & name)
{
    RawImage rawImage = { 0 };
    is.read((char *)&rawImage.mW, sizeof(uint));
    is.read((char *)&rawImage.mH, sizeof(uint));
    is.read((char *)&rawImage.mFormat, sizeof(uint));
    is.read((char *)&rawImage.mByteLength, sizeof(uint));
    rawImage.mData = new uchar[rawImage.mByteLength];
    is.read((char *)rawImage.mData, rawImage.mByteLength);

    _rawImageMap.insert(std::make_pair(name, rawImage));
}

void RawManager::LoadRawProgram(std::ifstream & is, const std::string & name)
{
    RawProgram rawProgram = { 0 };
    is.read((char *)&rawProgram.mPassLength, sizeof(uint));
    is.read((char *)&rawProgram.mVSByteLength, sizeof(uint));
    is.read((char *)&rawProgram.mGSByteLength, sizeof(uint));
    is.read((char *)&rawProgram.mFSByteLength, sizeof(uint));
    auto byteLength = rawProgram.mPassLength * sizeof(GLProgram::PassAttr)
                    + rawProgram.mVSByteLength
                    + rawProgram.mGSByteLength
                    + rawProgram.mFSByteLength;
    rawProgram.mData = new uchar[byteLength];
    is.read((char *)rawProgram.mData, byteLength);

    _rawProgramMap.insert(std::make_pair(name, rawProgram));
}

void RawManager::LoadRawMaterial(std::ifstream & is, const std::string & name)
{
    RawMaterial rawMaterial = { 0 };
    is.read((char *)&rawMaterial, sizeof(RawMaterial));
 
    _rawMaterialMap.insert(std::make_pair(name, rawMaterial));
}

GLRes * RawManager::LoadResMesh(const std::string & name)
{
    auto it = _rawMeshMap.find(name);
    if (it == _rawMeshMap.end())
    {
        auto res = LoadRaw(name, RawTypeEnum::kRAW_MESH);
        ASSERT_LOG(res, "Not Found Raw Mesh. {0}", name);
        it = _rawMeshMap.find(name);
    }
    auto glMesh = new GLMesh();
    glMesh->Init(
        it->second.mVertexs, it->second.mVertexLength, 
        it->second.mIndexs, it->second.mIndexLength, 
        GLMesh::Vertex::kV_N_C_UV_TAN_BITAN);
    _resObjectMap.insert(std::make_pair(name, glMesh));
    return glMesh;
}

GLRes * RawManager::LoadResImage(const std::string & name)
{
    auto it = _rawImageMap.find(name);
    if (it == _rawImageMap.end())
    {
        auto res = LoadRaw(name, RawTypeEnum::kRAW_IMAGE);
        ASSERT_LOG(res, "Not Found Raw Image. {0}", name);
        it = _rawImageMap.find(name);
    }
    auto glTexture2D = new GLTexture2D();
    glTexture2D->Init(
        it->second.mFormat, it->second.mFormat, 
        GL_UNSIGNED_BYTE, 
        it->second.mW, 
        it->second.mH, 
        it->second.mData);
    _resObjectMap.insert(std::make_pair(name, glTexture2D));
    return glTexture2D;
}

GLRes * RawManager::LoadResProgram(const std::string & name)
{
    auto it = _rawProgramMap.find(name);
    if (it == _rawProgramMap.end())
    {
        auto res = LoadRaw(name, RawTypeEnum::kRAW_PROGRAM);
        ASSERT_LOG(res, "Not Found Raw Program. {0}", name);
        it = _rawProgramMap.find(name);
    }
    auto glProgram = new GLProgram();
    auto pAttr = (GLProgram::PassAttr *)it->second.mData;
    auto vData = (const char *)(pAttr + it->second.mPassLength);
    auto gData = vData + it->second.mVSByteLength;
    auto fData = gData + it->second.mGSByteLength;
    //  PassAttr
    for (auto i = 0; i != it->second.mPassLength; ++i)
    {
        glProgram->AddPassAttr(*pAttr++);
    }
    //  Shader
    glProgram->Init(
        (const char *)vData, it->second.mVSByteLength,
        (const char *)gData, it->second.mGSByteLength,
        (const char *)fData, it->second.mFSByteLength);
    _resObjectMap.insert(std::make_pair(name, glProgram));
    return glProgram;
}

GLRes * RawManager::LoadResMaterial(const std::string & name)
{
    auto it = _rawMaterialMap.find(name);
    if (it == _rawMaterialMap.end())
    {
        auto res = LoadRaw(name, RawTypeEnum::kRAW_MATERIAL);
        ASSERT_LOG(res, "Not Found Raw Material. {0}", name);
        it = _rawMaterialMap.find(name);
    }
    auto glMaterial = new GLMaterial();
    glMaterial->SetShininess((float)it->second.mShininess);
    glMaterial->SetMesh(LoadRes<GLMesh>(it->second.mMesh));
    glMaterial->SetProgram(LoadRes<GLProgram>(it->second.mProgram));
    for (auto i = 0; i != MTLTEX2D_LEN && it->second.mTextures[i].mName[0] != '\0'; ++i)
    {
        glMaterial->SetTexture2D(
            LoadRes<GLTexture2D>(it->second.mTextures[i].mTexture),
            it->second.mTextures[i].mName, RAW_NAME_LEN, i);
    }
    _resObjectMap.insert(std::make_pair(name, glMaterial));
    return glMaterial;
}

void RawManager::ClearRawData()
{
    //  Delete Mesh Raw
    for (auto & rawMesh : _rawMeshMap)
    {
        delete[]rawMesh.second.mIndexs;
        delete[]rawMesh.second.mVertexs;
    }
    _rawMeshMap.clear();

    //  Delete Image Raw
    for (auto & rawImage : _rawImageMap)
    {
        stbi_image_free(rawImage.second.mData);
    }
    _rawImageMap.clear();

    //  Delete Program Raw
    for (auto & rawProgram : _rawProgramMap)
    {
        delete[]rawProgram.second.mData;
    }
    _rawProgramMap.clear();

    //  Delete Material Raw
    _rawMaterialMap.clear();
    
    //  Delete Manifest
    _rawManifestMap.clear();
}

void RawManager::ClearResData()
{
    for (auto & res : _resObjectMap)
    {
        delete res.second;
    }
    _resObjectMap.clear();
}

std::string RawManager::BuildName(const uchar * data, const uint len)
{
    uchar name[RAW_NAME_LEN] = { 0 };
    auto md5 = Code::MD5Encode(data, len);
    return (const char *)number_tool::Conver16((const uchar *)md5.c_str(), md5.size(), name);
}
