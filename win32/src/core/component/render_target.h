#pragma once

#include "component.h"
#include "../asset/bitmap.h"
#include "../asset/bitmap_cube.h"

class Bitmap;
class BitmapCube;

class RenderTarget : public Component {
public:
	enum AttachmentType {
		kCOLOR		= GL_COLOR_ATTACHMENT0,
		kDEPTH		= GL_DEPTH_ATTACHMENT,
		kSTENCIL	= GL_DEPTH_STENCIL_ATTACHMENT,
	};

	enum TextureType {
		k2D				= GL_TEXTURE_2D,
		k3D_RIGHT		= GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		k3D_LEFT		= GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1,
		k3D_TOP			= GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2,
		k3D_BOTTOM		= GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3,
		k3D_FRONT		= GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4,
		k3D_BACK		= GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5,
	};
	
public:
	static Bitmap * Create2DTexture(const std::uint32_t w, 
									const std::uint32_t h, 
									AttachmentType attachment);

	static BitmapCube * Create3DTexture(const std::uint32_t w, 
										const std::uint32_t h,
										AttachmentType attachment);

	RenderTarget();
	~RenderTarget();
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;

	void BindAttachment(AttachmentType attachment, TextureType type, int texid);
	void CloseDraw() { glDrawBuffer(GL_NONE); }
	void CloseRead() { glReadBuffer(GL_NONE); }
	void Beg();
	void End();

private:
	GLuint _fbo;
};