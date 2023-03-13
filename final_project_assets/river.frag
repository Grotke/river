#version 330 compatibility
uniform float uKa, uKd, uKs; // coefficients of each type of lighting
uniform vec3 uColor;		// object color
uniform vec3 uSpecularColor; // light color
uniform float uShininess;	// specular exponent
uniform float uBlocks;	// Texture split up into uBlocks x uBlocks squares
uniform int uTerrainTextureWidth;
uniform int uTerrainTextureHeight;
uniform int uBlockSize;
uniform bool uUseTransparancy;
uniform bool uUseEdgeTransparancy;
uniform bool uShowWater;
uniform bool uShinyWater;

uniform sampler2D uTerrainTexUnit;
uniform sampler2D uRiverMapTexUnit;
uniform sampler2D uWaterBaseTexUnit;
uniform sampler2D uWaterNormalsTexUnit;
uniform sampler2D uFlowMapTexUnit;
uniform float uTime;

in vec2 vST;	// texture coords
in vec3 vN;		// normal vector
in vec3 vL;		// vector from point to light
in vec3 vE;		// vector from point to eye

void
main()
{
	vec3 Normal;
	vec3 objectColor;
	vec3 riverMapValue = texture(uRiverMapTexUnit, vST).rgb;
	float shinyModifier = 1.0f;
	float specularModifier = 1.0f;
	if(riverMapValue.b - riverMapValue.r > 0.05f){
		if(uShinyWater){
			shinyModifier = 10.0f;
			specularModifier = 5.0f;
			//shinyModifier = 100.0f;
			//specularModifier = 2.0f;
		}
		int blockCol = int(floor(vST.s * uBlocks));
		int blockRow = int(floor(vST.t * uBlocks));
		int blockStartSPixel = blockCol * uBlockSize;
		int blockStartTPixel = blockRow * uBlockSize;
		
		int actualTexelS = int(floor(uTerrainTextureWidth * vST.s));
		int actualTexelT = int(floor(uTerrainTextureHeight * vST.t));
		float blockS = float(actualTexelT - blockStartTPixel) / float(uBlockSize);
		float blockT = float(actualTexelS - blockStartSPixel) / float(uBlockSize);
		float alpha = 0.4;
		float offset = 0.002;
		float NormalMultiplier = 1.0;
		float speed = 1.0;
		if(uUseEdgeTransparancy){
			vec2 newSTs[4];
			newSTs[0] = vec2(vST.s + offset, vST.t);
			newSTs[1] = vec2(vST.s - offset, vST.t);
			newSTs[2] = vec2(vST.s , vST.t + offset);
			newSTs[3] = vec2(vST.s, vST.t - offset);
			for(int i = 0; i < 4; i++){
				vec3 possibleLand = texture(uRiverMapTexUnit, newSTs[i]).rgb;
				if(possibleLand.b - possibleLand.r < 0.05f){
					NormalMultiplier = 0.95;
					speed = 3.0;
					alpha = 0.2;
					break;
				}
			}
		}
		vec2 waterST = vec2(blockS, blockT + uTime * speed);
		Normal = normalize(texture(uWaterNormalsTexUnit, waterST).rgb) * NormalMultiplier;
		if(!uShowWater){
			alpha = 0.0;
			Normal = normalize(vN);
		} else if (!uUseTransparancy){
			alpha = 1.0;
		}
		objectColor = alpha*texture(uWaterBaseTexUnit, waterST).rgb + (1 - alpha)*texture(uTerrainTexUnit, vST).rgb;;
	} else {
		Normal = normalize(vN);
		objectColor = texture(uTerrainTexUnit, vST).rgb;
	}

	vec3 Light = normalize(vL);
	vec3 Eye = normalize(vE);
	
	vec3 ambient = uKa * uColor;
	
	float d = max(dot(Normal, Light), 0.);
	vec3 diffuse = uKd * d * uColor;
	
	float s = 0;
	if(dot(Normal, Light) > 0.)
	{
		vec3 ref = normalize(reflect(-Light, Normal));
		s = pow(max(dot(Eye, ref), 0.), uShininess*shinyModifier);
	
	}
	
	vec3 specular = uKs * s * uSpecularColor * specularModifier;


	gl_FragColor = vec4((ambient + diffuse) * objectColor + specular, 1.0);
}