Pass
	CullFace Front
	DepthTest
	DepthWrite
	RenderType Shadow
	DrawType Index

	Vertex
		#version 330 core

		layout(location = 0) in vec3 a_pos_;

		uniform mat4 matrix_mvp_;
		uniform mat4 matrix_m_;

		out V_OUT_ {
			vec3 mMPos;
		} v_out_;

		void main()
		{
			vec4 apos       = vec4(a_pos_, 1);
			v_out_.mMPos    = vec3(matrix_m_ * apos);
			gl_Position     = vec4(matrix_mvp_ * apos);
		}
	End Vertex

	Fragment
		#version 330 core

        layout (std140) uniform LightPoint_ {
			float mFar, mNear;
            float mK0, mK1, mK2;
            vec3 mAmbient;
            vec3 mDiffuse;
            vec3 mSpecular;
            vec3 mPosition;
        } light_point_;

		in V_OUT_{
			vec3 mMPos;
		} v_out_;

        uniform int light_type_;

        #define LIGHT_TYPE_DIRECT_ 0
        #define LIGHT_TYPE_POINT_ 1
        #define LIGHT_TYPE_SPOT_ 2

		void main()
		{
            switch (light_type_)
            {
            case LIGHT_TYPE_DIRECT_:
                {
                    gl_FragDepth = gl_FragCoord.z;
                }
                break;
            case LIGHT_TYPE_POINT_:
                {
					vec3 normal = v_out_.mMPos - light_point_.mPosition;
					gl_FragDepth = length(normal) / light_point_.mFar;
                }
                break;
            case LIGHT_TYPE_SPOT_:
                {
                    gl_FragDepth = gl_FragCoord.z;
                }
                break;
            }
		}
	End Fragment
End Pass

Pass
	CullFace Back
	DepthTest
	DepthWrite
	RenderQueue Geometric
	RenderType Forward
	DrawType Index

	Vertex
		#version 330 core

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
		#version 330 core

		struct LightDirectParam_ {
			mat4 mMatrix;
			vec3 mNormal;
			vec3 mAmbient;
			vec3 mDiffuse;
			vec3 mSpecular;
			vec3 mPosition;
		};

        struct LightPointParam_ {
			float mFar, mNear;
            float mK0, mK1, mK2;
            vec3 mAmbient;
            vec3 mDiffuse;
            vec3 mSpecular;
            vec3 mPosition;
        };

		struct LightSpotParam_ {
			float mK0;
            float mK1;
            float mK2;
			float mInCone;
            float mOutCone;
			mat4 mMatrix;
			vec3 mNormal;
			vec3 mAmbient;
			vec3 mDiffuse;
			vec3 mSpecular;
			vec3 mPosition;
		};

		layout (std140) uniform LightDirect_ {
			LightDirectParam_ mParam[2];
		} light_direct_;

        layout (std140) uniform LightPoint_ {
            LightPointParam_ mParam[4];
        } light_point_;

		layout (std140) uniform LightSpot_ {
			LightSpotParam_ mParam[4];
		} light_spot_;

		uniform sampler2D shadow_map_direct_0_;
		uniform sampler2D shadow_map_direct_1_;

		uniform samplerCube shadow_map_point_0_;
		uniform samplerCube shadow_map_point_1_;
		uniform samplerCube shadow_map_point_2_;
		uniform samplerCube shadow_map_point_3_;
		
		uniform sampler2D shadow_map_spot_0_;
		uniform sampler2D shadow_map_spot_1_;
		uniform sampler2D shadow_map_spot_2_;
		uniform sampler2D shadow_map_spot_3_;

		uniform int light_count_direct_;
        uniform int light_count_point_;
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

        int CheckInView(vec4 vec)
        {
            int ret = 0;
            if		(vec.x < -vec.w) ret |= 1;
            else if (vec.x >  vec.w) ret |= 2;
            else if (vec.y < -vec.w) ret |= 4;
            else if (vec.y >  vec.w) ret |= 8;
            else if (vec.z < -vec.w) ret |= 16;
            else if (vec.z >  vec.w) ret |= 32;
            return ret;
        }

        float CalculateDirectShadow(const int i, const sampler2D shadowMap)
        {
            vec4 pos = light_direct_.mParam[i].mMatrix * vec4(v_out_.mMPos, 1);
            if (CheckInView(pos) != 0) { return 0; }
            pos.xyz = pos.xyz / pos.w * 0.5f + 0.5f;
			float zorder = pos.z;
            float shadow = 0.0f;
            vec2 texstep = 1.0f / textureSize(shadowMap, 0);
            float depth = texture(shadowMap, pos.xy + vec2(-texstep.x,  texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2( 0,          texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2( texstep.x,  texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2(-texstep.x,  0		 )).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2( 0,          0	     )).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2( texstep.x,  0	     )).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2(-texstep.x, -texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2( 0,		   -texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2( texstep.x, -texstep.y)).r; shadow += zorder < depth? 1: 0;
            return shadow / 9.0f;
        }

		float CalculateSpotShadow(const int i, const sampler2D shadowMap)
		{
			vec4 pos = light_spot_.mParam[i].mMatrix * vec4(v_out_.mMPos, 1);
            if (CheckInView(pos) != 0) { return 0; }
            pos.xyz = pos.xyz / pos.w * 0.5f + 0.5f;
			float zorder = pos.z;
            float shadow = 0.0f;
            vec2 texstep = 1.0f / textureSize(shadowMap, 0);
            float depth = texture(shadowMap, pos.xy + vec2(-texstep.x,  texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2( 0,          texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2( texstep.x,  texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2(-texstep.x,  0        )).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2( 0,          0        )).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2( texstep.x,  0        )).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2(-texstep.x, -texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2( 0,		   -texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadowMap, pos.xy + vec2( texstep.x, -texstep.y)).r; shadow += zorder < depth? 1: 0;
            return shadow / 9.0f;
		}

		float CalculatePointShadow(const int i, const samplerCube shadowMap)
		{
			vec3 normal 	= v_out_.mMPos - light_point_.mParam[i].mPosition;
			float zorder 	= texture(shadowMap, normal).r;
			return length(normal) > zorder * light_point_.mParam[i].mFar? 0: 1;
		}

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

		vec3 CalculateDirect(const int i, vec3 fragNormal, vec3 cameraNormal, vec2 uv)
		{
			float shadow = 0;
			switch (i)
			{
			case 0: shadow = CalculateDirectShadow(i, shadow_map_direct_0_); break;
			case 1: shadow = CalculateDirectShadow(i, shadow_map_direct_0_); break;
			}

			float diff = CalculateDiffuseScale(fragNormal, -light_direct_.mParam[i].mNormal, cameraNormal);
			float spec = CalculateSpecularScale(fragNormal, -light_direct_.mParam[i].mNormal, cameraNormal);

			vec3 ambient = light_direct_.mParam[i].mAmbient * texture(material_.mDiffuse0, uv).rgb;
			vec3 diffuse = light_direct_.mParam[i].mDiffuse * texture(material_.mDiffuse0, uv).rgb * diff;
			vec3 specular = light_direct_.mParam[i].mSpecular * texture(material_.mSpecular, uv).rgb * spec;
			return ambient + (diffuse + specular) * shadow;
		}

        vec3 CalculatePoint(const int i, vec3 fragNormal, vec3 cameraNormal, vec2 uv)
        {
			float shadow = 0;
			switch (i)
			{
			case 0: shadow = CalculatePointShadow(i, shadow_map_point_0_); break;
			case 1: shadow = CalculatePointShadow(i, shadow_map_point_1_); break;
			case 2: shadow = CalculatePointShadow(i, shadow_map_point_2_); break;
			case 3: shadow = CalculatePointShadow(i, shadow_map_point_3_); break;
			}

            vec3 fragToLight = normalize(light_point_.mParam[i].mPosition - v_out_.mMPos);

			float diff = CalculateDiffuseScale(fragNormal, fragToLight, cameraNormal);
			float spec = CalculateSpecularScale(fragNormal, fragToLight, cameraNormal);

			vec3 ambient = light_point_.mParam[i].mAmbient * texture(material_.mDiffuse0, uv).rgb;
			vec3 diffuse = light_point_.mParam[i].mDiffuse * texture(material_.mDiffuse0, uv).rgb * diff;
			vec3 specular = light_point_.mParam[i].mSpecular * texture(material_.mSpecular, uv).rgb * spec;

			//	距离衰减
			float distance = CalculateDistanceScale(v_out_.mMPos, 
													light_point_.mParam[i].mPosition, 
													light_point_.mParam[i].mK0, 
													light_point_.mParam[i].mK1,
													light_point_.mParam[i].mK2);

			return (ambient + (diffuse + specular) * shadow) * distance;
        }

		vec3 CalculateSpot(const int i, vec3 fragNormal, vec3 cameraNormal, vec2 uv)
		{
			float shadow = 0;
			switch (i)
			{
			case 0: shadow = CalculateSpotShadow(i, shadow_map_spot_0_); break;
			case 1: shadow = CalculateSpotShadow(i, shadow_map_spot_1_); break;
			case 2: shadow = CalculateSpotShadow(i, shadow_map_spot_2_); break;
			case 3: shadow = CalculateSpotShadow(i, shadow_map_spot_3_); break;
			}

			vec3 fragToLight = normalize(light_spot_.mParam[i].mPosition - v_out_.mMPos);

			float diff = CalculateDiffuseScale(fragNormal, fragToLight, cameraNormal);
			float spec = CalculateSpecularScale(fragNormal, fragToLight, cameraNormal);

			vec3 ambient = light_spot_.mParam[i].mAmbient * texture(material_.mDiffuse0, uv).rgb;
			vec3 diffuse = light_spot_.mParam[i].mDiffuse * texture(material_.mDiffuse0, uv).rgb * diff;
			vec3 specular = light_spot_.mParam[i].mSpecular * texture(material_.mSpecular, uv).rgb * spec;

			//	光锥衰减
			float weight = CalculateOutConeScale(light_spot_.mParam[i].mInCone, 
                                                 light_spot_.mParam[i].mOutCone, 
                                                 light_spot_.mParam[i].mNormal, -fragToLight);

			//	距离衰减
			float distance = CalculateDistanceScale(v_out_.mMPos, 
                                                    light_spot_.mParam[i].mPosition, 
                                                    light_spot_.mParam[i].mK0, 
                                                    light_spot_.mParam[i].mK1, 
                                                    light_spot_.mParam[i].mK2);

			return (ambient + (diffuse + specular) * shadow) * weight * distance;
		}

		void main()
		{
			vec3 cameraNormal = normalize(camera_pos_ - v_out_.mMPos);

			vec3 fragNormal = vec3(texture(material_.mNormal, v_out_.mUV));
				 fragNormal = v_out_.mTBN * normalize(fragNormal *2 - 1.0);

			vec3 outColor = vec3(0, 0, 0);
			for (int i = 0; i != light_count_direct_; ++i)
			{
				outColor += CalculateDirect(i, fragNormal, cameraNormal, v_out_.mUV);
			}

            for (int i = 0; i != light_count_point_; ++i)
            {
                outColor += CalculatePoint(i, fragNormal, cameraNormal, v_out_.mUV);
            }

			for (int i = 0; i != light_count_spot_; ++i)
			{
				outColor += CalculateSpot(i, fragNormal, cameraNormal, v_out_.mUV);
			}

			color_ = vec4(outColor, 1.0f);
		}
	End Fragment
End Pass