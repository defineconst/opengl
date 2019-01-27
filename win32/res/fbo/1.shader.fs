#version 410 core

uniform vec3 camera_pos_;
uniform vec3 camera_eye_;

struct LightDirect_ {
	vec3 mNormal;
	
	vec3 mAmbient;
	vec3 mDiffuse;
	vec3 mSpecular;
};

struct LightPoint_ {
	vec3 mPosition;
	float mK0, mK1, mK2;
	
	vec3 mAmbient;
	vec3 mDiffuse;
	vec3 mSpecular;
};

struct LightSpot_ {
	vec3 mNormal;
	vec3 mPosition;
	float mK0, mK1, mK2;
	float mOutCone, mInCone;

	vec3 mAmbient;
	vec3 mDiffuse;
	vec3 mSpecular;
};

uniform struct Light_ {
	int mDirectNum;
	int mPointNum;
	int mSpotNum;
	LightDirect_ mDirects[4];
	LightPoint_ mPoints[8];
	LightSpot_ mSpots[8];
} light_;

uniform struct Material_ {
	sampler2D mNormal0;
	sampler2D mNormal1;
	sampler2D mNormal2;
	sampler2D mNormal3;

	sampler2D mDiffuse0;
	sampler2D mDiffuse1;
	sampler2D mDiffuse2;
	sampler2D mDiffuse3;

	sampler2D mSpecular0;
	sampler2D mSpecular1;
	sampler2D mSpecular2;
	sampler2D mSpecular3;
	float mShininess;
	float mFlipUVX;
	float mFlipUVY;
} material_;

in V_OUT_ {
	vec3 mNormal;
    vec4 mMVPPos;
    vec3 mMVPos;
    vec3 mMPos;
    vec2 mUV;
} v_out_;

out vec4 color_;

vec3 CalculateDirect(LightDirect_ light, vec3 fragNormal, vec3 viewNormal)
{
	float diff = dot(fragNormal, -light.mNormal);
	vec3 center = (-light.mNormal + viewNormal) * 0.5;
	float spec = pow(dot(fragNormal, center), material_.mShininess);

	vec3 ambient = light.mAmbient * texture(material_.mDiffuse0, v_out_.mUV).rgb;
	vec3 diffuse = light.mDiffuse * texture(material_.mDiffuse0, v_out_.mUV).rgb * diff;
	vec3 specular = light.mSpecular * texture(material_.mSpecular0, v_out_.mUV).rgb * spec;
	return ambient + diffuse + specular;
}

vec3 CalculatePoint(LightPoint_ light, vec3 fragNormal, vec3 viewNormal)
{
	vec3 lightNormal = normalize(light.mPosition - v_out_.mMPos);
	vec3 center = (lightNormal + viewNormal) * 0.5;
	float diff = dot(fragNormal, lightNormal);
	float spec = pow(dot(fragNormal, center), material_.mShininess);

	vec3 ambient = light.mAmbient * texture(material_.mDiffuse0, v_out_.mUV).rgb;
	vec3 diffuse = light.mDiffuse * texture(material_.mDiffuse0, v_out_.mUV).rgb * diff;
	vec3 specular = light.mSpecular * texture(material_.mSpecular0, v_out_.mUV).rgb * spec;

	float distance = length(light.mPosition - v_out_.mMPos);
	float weight = 1 / (light.mK0 + light.mK1 * distance + light.mK2 * distance * distance);

	return (ambient + diffuse + specular) * weight;
}

vec3 CalculateSpot(LightSpot_ light, vec3 fragNormal, vec3 viewNormal)
{
	vec3 lightNormal = normalize(light.mPosition - v_out_.mMPos);
	float fragCone = dot(lightNormal, -light.mNormal);
	float cutWeight = clamp((fragCone - light.mOutCone) / (light.mInCone - light.mOutCone), 0, 1);
	if (cutWeight == 0) { return vec3(0, 0, 0); }

	vec3 center = (lightNormal + viewNormal) * 0.5;
	float diff = dot(fragNormal, lightNormal);
	float spec = pow(dot(fragNormal, center), material_.mShininess);

	vec3 ambient = light.mAmbient * texture(material_.mDiffuse0, v_out_.mUV).rgb;
	vec3 diffuse = light.mDiffuse * texture(material_.mDiffuse0, v_out_.mUV).rgb * diff;
	vec3 specular = light.mSpecular * texture(material_.mSpecular0, v_out_.mUV).rgb * spec;

	float distance = length(light.mPosition - v_out_.mMPos);
	float weight = 1 / (light.mK0 + light.mK1 * distance + light.mK2 * distance * distance);

	return (ambient + diffuse + specular) * weight * cutWeight;
}

void main()
{
	vec3 outColor = vec3(0, 0, 0);
	vec3 viewNormal = normalize(camera_pos_ - v_out_.mMPos);
	for (int i = 0; i != light_.mDirectNum; ++i)
	{
		outColor += CalculateDirect(light_.mDirects[i], v_out_.mNormal, viewNormal);
	}

	for (int i = 0; i != light_.mPointNum; ++i)
	{
		outColor += CalculatePoint(light_.mPoints[i], v_out_.mNormal, viewNormal);
	}

	for (int i = 0; i != light_.mSpotNum; ++i)
	{
		outColor += CalculateSpot(light_.mSpots[i], v_out_.mNormal, viewNormal);
	}

	const float offset = 1.0f / 300.0f;
	vec2 offsets[] = {
		vec2(-offset, offset),		vec2(0.0f, offset),		vec2(offset, offset), 
		vec2(-offset, 0.0f),		vec2(0.0f, 0.0f),		vec2(offset, 0.0f), 
		vec2(-offset, -offset),		vec2(0.0f, -offset),	vec2(offset, -offset)
	};

	//	ģ��
	//	float kerenl[] = {
	//		1.0f / 9.5f, 1.0f / 9.5f, 1.0f / 9.5f,
	//		1.0f / 9.5f, 1.5f / 9.5f, 1.0f / 9.5f,
	//		1.0f / 9.5f, 10.0f / 9.5f, 1.0f / 9.5f
	//	};

	//	��
	//	float kerenl[] = {
	//		-1, -1, -1,
	//		-1, 9, -1,
	//		-1, -1, -1
	//	};

	//	��Ե���
	float kerenl[] = {
		1, 1, 1,
		1, -8, 1,
		1, 1, 1
	};


	vec3 sampleTex[9];
	for (int i = 0; i != 9; ++i)
	{
		sampleTex[i] = texture(material_.mDiffuse0, v_out_.mUV + offsets[i]).rgb;
	}

	vec3 color = vec3(0.0);
	for (int i = 0; i != 9; ++i)
	{
		color += sampleTex[i] * kerenl[i];
	}
	color_ = vec4(color, 1);
}