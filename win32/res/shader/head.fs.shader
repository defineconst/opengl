//	frag
#version 410 core

uniform mat4 mvp_;
uniform mat4 mv_;
uniform vec3 camera_pos_;
uniform vec3 camera_eye_;
uniform sampler2D texture_;
uniform sampler2D normal_;

struct LightSpot_ {
	vec3 mPos;
	vec3 mDir;
	vec3 mColor;
	float mMin;
	float mMax;
	float mMinCone;
	float mMaxCone;
};

struct LightPoint_ {
	vec3 mPos;
	vec3 mColor;
	float mMin;
	float mMax;
};

uniform Light_ 
{
	float mAmbient;
	int mSpotNum;
	int mPointNum;
	LightSpot_ mSpots[32];
	LightPoint_ mPoints[32];
} light_;

in vec2 vuv_;
in vec4 vpos_mv_;
in vec4 vpos_mvp_;

out vec4 color_;

