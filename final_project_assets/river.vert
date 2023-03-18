#version 330 compatibility

out vec2 vST;
out vec3 vN;
out vec3 vL;
out vec3 vE;

// The sun
const vec3 LIGHTPOSITION = vec3(30., 500., -30.);

void
main()
{
	// Coordinate from texture 0. OUTDATED: Apparently should be replaced with vertex attribute
	vST = gl_MultiTexCoord0.st;
	
	// Vertex in eye coordinates
	vec4 ECposition = gl_ModelViewMatrix * gl_Vertex;
	vN = normalize(gl_NormalMatrix * gl_Normal);
	// Vector from vertex to sun
	vL = LIGHTPOSITION - ECposition.xyz;
	// Vector from vertex to eye position (origin)
	vE = vec3(0., 0., 0.) - ECposition.xyz;

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}