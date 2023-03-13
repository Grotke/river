#version 330 compatibility
uniform float uKa, uKd, uKs;
uniform vec3 uColor;
uniform vec3 uSpecularColor;
uniform float uShininess;
uniform float uFragTime;
uniform float uVelocity;
in vec3 vN;
in vec3 vL;
in vec3 vE;
in vec4 vPosition;

void
main()
{
	vec3 color = uColor;

	if(	(0.3*sin( (360 / 360 * vPosition.x) - (uVelocity * uFragTime)) + 0.4*cos( (360 / 360 * vPosition.z) - (uVelocity * uFragTime))) > 0.5){
		color = vec3(0.5, 0.5, 0.5);
	}

	vec3 Normal = normalize(vN);
	vec3 Light = normalize(vL);
	vec3 Eye = normalize(vE);
	
	vec3 ambient = uKa * color;
	
	float d = max(dot(Normal, Light), 0.);
	vec3 diffuse = uKd * d * color;
	
	float s = 0;
	if(dot(Normal, Light) > 0.)
	{
		vec3 ref = normalize(reflect(-Light, Normal));
		s = pow(max(dot(Eye, ref), 0.), uShininess);
	
	}
	
	vec3 specular = uKs * s * uSpecularColor;
	gl_FragColor = vec4(ambient + diffuse + specular, 1.0);
}