#version 330 compatibility

uniform float uTime;
uniform float uVelocity;

out vec3 vN;
out vec3 vL;
out vec3 vE;
out vec4 vPosition;

const vec3 LIGHTPOSITION = vec3(0., 5., 0.);

void
main()
{
	vec4 ECposition = gl_ModelViewMatrix * gl_Vertex;
	vec3 newNormal = vec3(0., 1., 0.);
	vec4 newVec = gl_Vertex;
	//newNormal.x = 0.2 * newVec.y;
	//newNormal.z = 0.4 * newVec.y;
	vN = normalize(gl_NormalMatrix * newNormal);
	vL = LIGHTPOSITION - ECposition.xyz;
	vE = vec3(0., 0., 0.) - ECposition.xyz;

	newVec.y = 0.4*sin( (360 / 360 * newVec.x) - (uVelocity * uTime)) + 1.0*cos( (360 / 100 * newVec.x * newVec.z) - (10.0*uVelocity * uTime)) * 0.4*cos( (360 / 360 * newVec.z) - (uVelocity * uTime));
	gl_Position = gl_ModelViewProjectionMatrix * newVec;
	vPosition = newVec;
}