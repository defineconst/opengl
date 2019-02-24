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

	sampler2D mParallax0;
	float mShininess;
} material_;

in V_OUT_ {
	vec3 mNormal;
    vec4 mMVPPos;
    vec3 mMVPos;
    vec3 mMPos;
	mat3 mTBNR;
	mat3 mTBN;
    vec2 mUV;
} v_out_;

out vec4 color_;

//	������������������
float CalculateDiffuseScale(vec3 fragNormal, vec3 lightNormal, vec3 cameraNormal)
{
	return max(dot(fragNormal, lightNormal), 0);
}

//	���㾵�淴����������
float CalculateSpecularScale(vec3 fragNormal, vec3 lightNormal, vec3 cameraNormal)
{
	vec3 h = (lightNormal + cameraNormal) * 0.5f;
	return pow(max(dot(fragNormal, h), 0), material_.mShininess);
}

//	�������˥����������
float CalculateDistanceScale(vec3 fragPosition, vec3 lightPosition, float k0, float k1, float k2)
{
	float len =	length(fragPosition - lightPosition);
	return 1.0f / (k0 + k1 * len + k2 * len * len);
}

//	������׶˥����������
float CalculateOutConeScale(float inCone, float outCone, vec3 lightNormal, vec3 lightToFrag)
{
	float cone = dot(lightNormal, lightToFrag);
	return clamp((cone - outCone) / (inCone - outCone), 0, 1);
}

vec2 CalculateParallaxUV(vec3 cameraNormal)
{
	float h = texture(material_.mParallax0, v_out_.mUV).r;
	vec2 offset = cameraNormal.xy / cameraNormal.z * h;
	return v_out_.mUV - offset * 0.0f;
}

vec3 CalculateDirect(LightDirect_ light, vec3 fragNormal, vec3 cameraNormal, vec2 parallaxUV)
{
	float diff = CalculateDiffuseScale(fragNormal, -light.mNormal, cameraNormal);
	float spec = CalculateSpecularScale(fragNormal, -light.mNormal, cameraNormal);

	vec3 ambient = light.mAmbient * texture(material_.mDiffuse0, parallaxUV).rgb;
	vec3 diffuse = light.mDiffuse * texture(material_.mDiffuse0, parallaxUV).rgb * diff;
	vec3 specular = light.mSpecular * texture(material_.mDiffuse0, parallaxUV).rgb * spec;
	return ambient + diffuse + specular;
}

vec3 CalculatePoint(LightPoint_ light, vec3 fragNormal, vec3 cameraNormal, vec2 parallaxUV)
{
	vec3 lightNormal = normalize(light.mPosition - v_out_.mMPos);
	float diff = CalculateDiffuseScale(fragNormal, lightNormal, cameraNormal);
	float spec = CalculateSpecularScale(fragNormal, lightNormal, cameraNormal);

	vec3 ambient = light.mAmbient * texture(material_.mDiffuse0, parallaxUV).rgb;
	vec3 diffuse = light.mDiffuse * texture(material_.mDiffuse0, parallaxUV).rgb * diff;
	vec3 specular = light.mSpecular * texture(material_.mSpecular0, parallaxUV).rgb * spec;

	float distance = CalculateDistanceScale(v_out_.mMPos, light.mPosition, light.mK0, light.mK1, light.mK2);

	return (ambient + diffuse + specular) * distance;
}

vec3 CalculateSpot(LightSpot_ light, vec3 fragNormal, vec3 cameraNormal, vec2 parallaxUV)
{
	vec3 lightNormal = normalize(light.mPosition - v_out_.mMPos);

	float diff = CalculateDiffuseScale(fragNormal, lightNormal, cameraNormal);
	float spec = CalculateSpecularScale(fragNormal, lightNormal, cameraNormal);
	
	vec3 ambient = light.mAmbient * texture(material_.mDiffuse0, parallaxUV).rgb;
	vec3 diffuse = light.mDiffuse * texture(material_.mDiffuse0, parallaxUV).rgb * diff;
	vec3 specular = light.mSpecular * texture(material_.mSpecular0, parallaxUV).rgb * spec;

	//	��׶˥��
	float weight = CalculateOutConeScale(light.mInCone, light.mOutCone, light.mNormal, -lightNormal);

	//	����˥��
	float distance = CalculateDistanceScale(v_out_.mMPos, light.mPosition, light.mK0, light.mK1, light.mK2);

	return (ambient + diffuse + specular) * distance * weight;
}

void main()
{
	vec3 cameraNormal = normalize(camera_pos_ - v_out_.mMPos);
	//	ͨ���Ӳ���ͼƫ��UV
	vec3 tbnCameraNormal = vec3(0, 0, 0);
	tbnCameraNormal.x =  cameraNormal.x;
	tbnCameraNormal.y = -cameraNormal.y;
	tbnCameraNormal.z =  cameraNormal.z;
	tbnCameraNormal = v_out_.mTBNR * tbnCameraNormal;
	vec2 parallaxUV = CalculateParallaxUV(tbnCameraNormal);
	if (	parallaxUV.x < 0 || parallaxUV.x > 1 
		||	parallaxUV.y < 0 || parallaxUV.y > 1)
	{
		discard;
	}
	vec3 fragNormal =  vec3(texture(material_.mNormal0, parallaxUV));
		 fragNormal =  v_out_.mTBN * normalize(fragNormal * 2 - 1.0);
//	fragNormal = normalize(v_out_.mNormal);

	vec3 outColor = vec3(0, 0, 0);
	for (int i = 0; i != light_.mDirectNum; ++i)
	{
		outColor += CalculateDirect(light_.mDirects[i], fragNormal, cameraNormal, parallaxUV);
	}

	for (int i = 0; i != light_.mPointNum; ++i)
	{
		outColor += CalculatePoint(light_.mPoints[i], fragNormal, cameraNormal, parallaxUV);
	}

	for (int i = 0; i != light_.mSpotNum; ++i)
	{
		outColor += CalculateSpot(light_.mSpots[i], fragNormal, cameraNormal, parallaxUV);
	}

	color_ = vec4(outColor, 1.0f);
}