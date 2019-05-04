#pragma once

#include "../res/bitmap.h"
#include "../res/bitmap_cube.h"
#include "../tools/debug_tool.h"

class RenderBuffer {
public:
    RenderBuffer(const std::uint32_t w, const std::uint32_t h, int fmt)
    {
        _w = w; _h = h; _format = fmt;
        glGenRenderbuffers(1, &_GLID);
        glBindRenderbuffer(GL_RENDERBUFFER, _GLID);
        glRenderbufferStorage(GL_RENDERBUFFER, fmt, w, h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        ASSERT_LOG(glGetError() == 0, "Create RBO Error");
    }

    ~RenderBuffer()
    {
        glDeleteRenderbuffers(1, &_GLID);
    }
    
    uint GetGLID() const { return _GLID; }
private:
    std::uint32_t _w;
    std::uint32_t _h;
    int _format;
    uint _GLID;
};

class RenderTarget {
public:
    enum BindType {
        kALL = GL_FRAMEBUFFER,
        kDRAW = GL_DRAW_FRAMEBUFFER,
        kREAD = GL_READ_FRAMEBUFFER,
        kNONE = GL_NONE,
    };

	enum AttachmentType {
		kCOLOR0 = GL_COLOR_ATTACHMENT0,
		kCOLOR1 = GL_COLOR_ATTACHMENT0 + 1,
		kCOLOR2 = GL_COLOR_ATTACHMENT0 + 2,
		kCOLOR3 = GL_COLOR_ATTACHMENT0 + 3,
		kCOLOR4 = GL_COLOR_ATTACHMENT0 + 4,
		kCOLOR5 = GL_COLOR_ATTACHMENT0 + 5,
		kCOLOR6 = GL_COLOR_ATTACHMENT0 + 6,
		kDEPTH  = GL_DEPTH_ATTACHMENT,
		kSTENCIL = GL_DEPTH_STENCIL_ATTACHMENT,
	};

	enum TextureType {
		k2D				= GL_TEXTURE_2D,
		k3D		        = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	};

public:
    static RenderBuffer * CreateBuffer(const std::uint32_t w, const std::uint32_t h, AttachmentType attachment, int fmt);
	static RenderTexture2D * CreateTexture2D(const std::uint32_t w, const std::uint32_t h, AttachmentType attachment, int texfmt, int rawfmt, int pixtype);
	static RenderTexture3D * CreateTexture3D(const std::uint32_t w, const std::uint32_t h, AttachmentType attachment, int texfmt, int rawfmt, int pixtype);

    static void Bind(BindType bindType, const RenderTarget & rt);
    static void Bind(BindType bindType);

    static void BindAttachment(BindType bindType, AttachmentType attachment, uint buffer);
    static void BindAttachment(BindType bindType, AttachmentType attachment, TextureType type, uint texture);
    static void BindAttachment(BindType bindType, AttachmentType attachment, TextureType type, const RenderTexture2D * texture);
    static void BindAttachment(BindType bindType, AttachmentType attachment, TextureType type, const RenderTexture3D * texture);


    RenderTarget();
    ~RenderTarget();

    void Start(BindType bindType = BindType::kALL);
    void Ended();
    uint GetGLID() const;

    void BindAttachment(AttachmentType attachment, uint buffer);
    void BindAttachment(AttachmentType attachment, TextureType type, uint texture);
    void BindAttachment(AttachmentType attachment, TextureType type, RenderTexture2D * texture2D);
    void BindAttachment(AttachmentType attachment, TextureType type, RenderTexture3D * texture3D);

private:
	uint _fbo;

    BindType _bindType;
};