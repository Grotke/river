#version 330 compatibility
uniform float uKa, uKd, uKs; // coefficients of each type of lighting: ambient, diffuse, specular
uniform vec3 uColor;		// object color
uniform vec3 uSpecularColor; // Specular highlight color
uniform float uShininess;	// specular exponent aka shininess
uniform float uBlocks;	// Texture split up into uBlocks x uBlocks squares
uniform int uTerrainTextureWidth;
uniform int uTerrainTextureHeight;
uniform int uBlockSize; // Pixels per block ie TerrainTextureHeight / uBlocks
// Flags to toggle features
uniform bool uUseTransparancy;
uniform bool uUseEdgeTransparancy;
uniform bool uShowWater;
uniform bool uShinyWater;

// Textures and time
uniform sampler2D uTerrainTexUnit;
uniform sampler2D uRiverMapTexUnit;
uniform sampler2D uWaterBaseTexUnit;
uniform sampler2D uWaterNormalsTexUnit;
uniform sampler2D uFlowMapTexUnit;
uniform float uTime;

// From vertex shader
in vec2 vST;	// texture coords
in vec3 vN;		// normal vector
in vec3 vL;		// vector from point to sun
in vec3 vE;		// vector from point to eye

void
main()
{
	vec3 Normal;
	vec3 objectColor;
	vec3 riverMapValue = texture(uRiverMapTexUnit, vST).rgb;
	float shinyModifier = 1.0f;
	float specularModifier = 1.0f;
	// If water is here, set up the water
	// River map marks water as blue, anything else as white
	if(riverMapValue.b - riverMapValue.r > 0.05f){
		if(uShinyWater){
			shinyModifier = 10.0f;
			specularModifier = 5.0f;
			//shinyModifier = 100.0f;
			//specularModifier = 2.0f;
		}
		// Decide what block/tile we're in
		int blockCol = int(floor(vST.s * uBlocks));
		int blockRow = int(floor(vST.t * uBlocks));
		// Figure out which pixel starts the block
		int blockStartSPixel = blockCol * uBlockSize;
		int blockStartTPixel = blockRow * uBlockSize;
		
		// Figure out the actual pixel we're on now
		int actualTexelS = int(floor(uTerrainTextureWidth * vST.s));
		int actualTexelT = int(floor(uTerrainTextureHeight * vST.t));
		// Map the tile into S, T coordinates
		// Each tile has its own ST coordinates. We're basically computing the current pixel as a percentage of the tile to get ST coordinates from 0..1
		// UNUSED, think this was meant to be used with the flow map
		float blockT = float(actualTexelT - blockStartTPixel) / float(uBlockSize);
		float blockS = float(actualTexelS - blockStartSPixel) / float(uBlockSize);

		// Water transparency
		float alpha = 0.4;
		// Distance that we're considering "close to land"
		float offset = 0.002;
		// Not quite sure what normal multiplier does. Maybe make lighting slightly weaker for shallow water?
		float NormalMultiplier = 1.0;
		// Water speed
		float speed = 1.0;
		// Make the water close to land slightly faster and more transparent to mimic shallow water
		if(uUseEdgeTransparancy){
			vec2 newSTs[4];
			// Check above, below, left, and right of the current pixel to see if it's close to land
			// If so, apply the shallow water settings to the current pixel
			newSTs[0] = vec2(vST.s + offset, vST.t);
			newSTs[1] = vec2(vST.s - offset, vST.t);
			newSTs[2] = vec2(vST.s , vST.t + offset);
			newSTs[3] = vec2(vST.s, vST.t - offset);
			for(int i = 0; i < 4; i++){
				vec3 possibleLand = texture(uRiverMapTexUnit, newSTs[i]).rgb;
				// Found land, only need 1 section to be land so apply shallow water settings and break out of loop
				if(possibleLand.b - possibleLand.r < 0.05f){
					NormalMultiplier = 0.95;
					speed = 3.0;
					alpha = 0.2;
					break;
				}
			}
		}
		// Scroll the water by varying the T on time
		vec2 waterST = vec2(blockS, blockT + uTime * speed);
		Normal = normalize(texture(uWaterNormalsTexUnit, waterST).rgb) * NormalMultiplier;
		// Hide water if requested
		if(!uShowWater){
			alpha = 0.0;
			Normal = normalize(vN);
		} else if (!uUseTransparancy){
			// Turn off transparency of water
			alpha = 1.0;
		}
		// Blend the water with the terrain underneath so the riverbed is visible through the water
		objectColor = alpha*texture(uWaterBaseTexUnit, waterST).rgb + (1 - alpha)*texture(uTerrainTexUnit, vST).rgb;;
	} else {
		// No water here so just use terrain texture
		Normal = normalize(vN);
		objectColor = texture(uTerrainTexUnit, vST).rgb;
	}

	vec3 Light = normalize(vL);
	vec3 Eye = normalize(vE);
	
	vec3 ambient = uKa * uColor;
	
	// Is light perpendicular or behind this point? If so, no diffuse lighting
	float d = max(dot(Normal, Light), 0.);
	vec3 diffuse = uKd * d * uColor;
	
	float s = 0;
	// If point is receiving light
	if(dot(Normal, Light) > 0.)
	{
		// Compute specular lighting based on amount of light going directly into eye
		vec3 ref = normalize(reflect(-Light, Normal));
		s = pow(max(dot(Eye, ref), 0.), uShininess*shinyModifier);
	
	}
	
	vec3 specular = uKs * s * uSpecularColor * specularModifier;

	// Is this right?
	gl_FragColor = vec4((ambient + diffuse) * objectColor + specular, 1.0);
}