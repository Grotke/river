#version 330 compatibility

out vec2 vST;
out vec3 vN;
out vec3 vL;
out vec3 vE;

const vec3 LIGHTPOSITION = vec3(30., 500., -30.);

void
main()
{
	vST = gl_MultiTexCoord0.st;
	
	vec4 ECposition = gl_ModelViewMatrix * gl_Vertex;
	vN = normalize(gl_NormalMatrix * gl_Normal);
	vL = LIGHTPOSITION - ECposition.xyz;
	vE = vec3(0., 0., 0.) - ECposition.xyz;

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}