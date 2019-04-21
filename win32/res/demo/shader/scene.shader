Pass
	CullFace Back
	DepthTest
	DepthWrite
	RenderQueue Geometric
	RenderType Forward
	DrawType Index

	Vertex
		#version 440 core

		layout(location = 0) in vec3 a_pos_;
		layout(location = 1) in vec2 a_uv_;
		layout(location = 2) in vec3 a_n_;
		layout(location = 3) in vec3 a_t_;
		layout(location = 4) in vec3 a_b_;

		uniform vec3 camera_pos_;
		uniform vec3 camera_eye_;
		uniform mat4 matrix_mvp_;
		uniform mat4 matrix_mv_;
		uniform mat4 matrix_m_;
		uniform mat4 matrix_p_;
		uniform mat4 matrix_v_;
		uniform mat3 matrix_n_;

		out V_OUT_ {
			vec3 mNormal;
			vec4 mMVPPos;
			vec3 mMVPos;
			vec3 mMPos;
			mat3 mTBNR;
			mat3 mTBN;
			vec2 mUV;
		} v_out_;

		void main()
		{
			vec4 apos = vec4(a_pos_, 1);
			v_out_.mMPos = vec3(matrix_m_ * apos);
			v_out_.mMVPos = vec3(matrix_mv_ * apos);
			v_out_.mNormal = vec3(matrix_n_ * a_n_);
			v_out_.mMVPPos = vec4(matrix_mvp_ * apos);

			vec3 T = normalize(vec3(matrix_m_ * vec4(a_t_, 0.0f)));
			vec3 B = normalize(vec3(matrix_m_ * vec4(a_b_, 0.0f)));
			vec3 N = normalize(vec3(matrix_m_ * vec4(a_n_, 0.0f)));
			v_out_.mTBN = mat3(T, B, N);
			v_out_.mTBNR = transpose(v_out_.mTBN);

			v_out_.mUV = a_uv_;

			gl_Position = v_out_.mMVPPos;
		}
	End Vertex

	Fragment
		#version 410 core

		struct LightDirectParam_ {
			int mSMP;			//		0
			mat4 mMatrix;		//		16
								//		16
								//		16
								//		16
			vec3 mNormal;		//		80
			vec3 mAmbient;		//		96
			vec3 mDiffuse;		//		112
			vec3 mSpecular;		//		128
			vec3 mPosition;		//		144
		};

		struct LightSpotParam_ {
			int mSMP;			//		0
			float mK0;			//		4
			float mK1;			//		8
			float mK2;			//		12
			float mInCone;		//		16
			float mOutCone;		//		20
			vec3 mNormal;		//		32
			vec3 mAmbient;		//		48
			vec3 mDiffuse;		//		64
			vec3 mSpecular;		//		80
			vec3 mPosition;		//		96
		};

		layout (std140) uniform LightDirect_ {
			LightDirectParam_ mParam[2];
		} light_direct_;

		layout (std140) uniform LightSpot_ {
			LightSpotParam_ mParam[4];
		} light_spot_;

		uniform int light_count_direct_;
		uniform int light_count_spot_;
		uniform vec3 camera_pos_;
		uniform vec3 camera_eye_;

		uniform struct Material_ {
			float mShininess;
			sampler2D mNormal;
			sampler2D mSpecular;
			sampler2D mDiffuse0;
			sampler2D mDiffuse1;
			sampler2D mDiffuse2;
			sampler2D mDiffuse3;
		} material_;

		in V_OUT_{
			vec3 mNormal;
			vec4 mMVPPos;
			vec3 mMVPos;
			vec3 mMPos;
			mat3 mTBNR;
			mat3 mTBN;
			vec2 mUV;
		} v_out_;

		out vec4 color_;

		//	计算漫反射缩放因子
		float CalculateDiffuseScale(vec3 fragNormal, vec3 lightNormal, vec3 cameraNormal)
		{
			return max(dot(fragNormal, lightNormal), 0);
		}

		//	计算镜面反射缩放因子
		float CalculateSpecularScale(vec3 fragNormal, vec3 lightNormal, vec3 cameraNormal)
		{
			vec3 h = (lightNormal + cameraNormal) * 0.5f;
			return pow(max(dot(fragNormal, h), 0), material_.mShininess);
		}

		//	计算距离衰减缩放因子
		float CalculateDistanceScale(vec3 fragPosition, vec3 lightPosition, float k0, float k1, float k2)
		{
			float len = length(fragPosition - lightPosition);
			return 1.0f / (k0 + k1 * len + k2 * len * len);
		}

		//	计算外锥衰减缩放因子
		float CalculateOutConeScale(float inCone, float outCone, vec3 lightNormal, vec3 lightToFrag)
		{
			float cone = dot(lightNormal, lightToFrag);
			return clamp((cone - outCone) / (inCone - outCone), 0, 1);
		}

		vec3 CalculateDirect(const LightDirectParam_ lightParam, vec3 fragNormal, vec3 cameraNormal, vec2 parallaxUV)
		{
			float diff = CalculateDiffuseScale(fragNormal, -lightParam.mNormal, cameraNormal);
			float spec = CalculateSpecularScale(fragNormal, -lightParam.mNormal, cameraNormal);

			vec3 ambient = lightParam.mAmbient * texture(material_.mDiffuse0, parallaxUV).rgb;
			vec3 diffuse = lightParam.mDiffuse * texture(material_.mDiffuse0, parallaxUV).rgb * diff;
			vec3 specular = lightParam.mSpecular * texture(material_.mSpecular, parallaxUV).rgb * spec;
			return ambient + diffuse + specular;
		}

		vec3 CalculateSpot(const LightSpotParam_ lightParam, vec3 fragNormal, vec3 cameraNormal, vec2 parallaxUV)
		{
			vec3 fragToLight = normalize(lightParam.mPosition - v_out_.mMPos);

			float diff = CalculateDiffuseScale(fragNormal, fragToLight, cameraNormal);
			float spec = CalculateSpecularScale(fragNormal, fragToLight, cameraNormal);

			vec3 ambient = lightParam.mAmbient * texture(material_.mDiffuse0, parallaxUV).rgb;
			vec3 diffuse = lightParam.mDiffuse * texture(material_.mDiffuse0, parallaxUV).rgb * diff;
			vec3 specular = lightParam.mSpecular * texture(material_.mSpecular, parallaxUV).rgb * spec;

			//	光锥衰减
			float weight = CalculateOutConeScale(lightParam.mInCone, lightParam.mOutCone, lightParam.mNormal, -fragToLight);

			//	距离衰减
			float distance = CalculateDistanceScale(v_out_.mMPos, lightParam.mPosition, lightParam.mK0, lightParam.mK1, lightParam.mK2);

			return (ambient + diffuse + specular) * distance * weight;
		}

		void main()
		{
			vec3 cameraNormal = normalize(camera_pos_ - v_out_.mMPos);

			vec3 fragNormal = vec3(texture(material_.mNormal, v_out_.mUV));
				 fragNormal = v_out_.mTBN * normalize(fragNormal *2 - 1.0);

			vec3 outColor = vec3(0, 0, 0);
			for (int i = 0; i != light_count_direct_; ++i)
			{
				outColor += CalculateDirect(light_direct_.mParam[i] , fragNormal, cameraNormal, v_out_.mUV);
			}

			for (int i = 0; i != light_count_spot_; ++i)
			{
				outColor += CalculateSpot(light_spot_.mParam[i] , fragNormal, cameraNormal, v_out_.mUV);
			}

			color_ = vec4(outColor, 1.0f);
		}
	End Fragment
End Pass