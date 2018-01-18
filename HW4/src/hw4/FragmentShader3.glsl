#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentColor;
in vec3 fragmentNormal;

// Ouput data
out vec3 color;

//uniform vec3 uLight;
uniform vec3 Color;
uniform vec3 pColor;
uniform vec3 sColor;
uniform float Intensity;
uniform float pIntensity;
uniform float sIntensity;
uniform vec3 Direction;
uniform vec3 Axis;
uniform vec3 pLocation;
uniform vec3 sLocation;
uniform float Radius;
uniform vec3 Fallout;
uniform int dL;
uniform int pL;
uniform int sL;

void main(){
	
	//color = vec3(1.0, 1.0, 1.0);
	//TODO: Assign fragmentColor as a final fragment color
	//color = fragmentColor;
	//TODO:Assign fragmentNormal as a final fragment color
	//vec3 normal = normalize(fragmentNormal);
	//color = normal;
	//TODO: Phong reflection model	

	vec3 toV = -normalize(vec3(fragmentPosition));
	vec3 normal = normalize(fragmentNormal);
	vec3 colord = vec3(0.0);
	vec3 colorp = vec3(0.0);
	vec3 colors = vec3(0.0);
	vec3 intensityd = vec3(0.0);
	vec3 intensityp = vec3(0.0);
	vec3 intensitys = vec3(0.0);
	vec3 intensity = vec3(0.0);

	//directional
	if (dL == 1)
	{
		vec3 tolightd = normalize(-Direction);
		vec3 hd = normalize(toV + tolightd);

		float speculard = pow(max(0.0, dot(hd, normal)), 64.0);//r dot v == n dot h
		float diffused = max(0.0, dot(normal, tolightd));//l dot n

		intensityd = fragmentColor *diffused + vec3(0.6, 0.6, 0.6)*speculard;
		intensityd = intensityd * Intensity;

	}

	//point
	if (pL == 1)
	{
		vec3 tolightp = normalize(pLocation - fragmentPosition);
		vec3 hp = normalize(toV + tolightp);

		float specularp = pow(max(0.0, dot(hp, normal)), 64.0);//r dot v == n dot h
		float diffusep = max(0.0, dot(normal, tolightp));//l dot n

		intensityp = fragmentColor *diffusep + vec3(0.6, 0.6, 0.6)*specularp;
		intensityp = intensityp * pIntensity;
		float d = length(pLocation - fragmentPosition);
		float att = 1.0 / (Fallout.x + Fallout.y * d + Fallout.z * d * d);
		intensityp = intensityp * att;
	}
	//spotlight
	if (sL == 1)
	{
		vec3 tolights = normalize(sLocation - fragmentPosition);
		vec3 hs = normalize(toV + tolights);

		float speculars = pow(max(0.0, dot(hs, normal)), 64.0);//r dot v == n dot h
		float diffuses = max(0.0, dot(normal, tolights));//l dot n
		if (dot(tolights, normalize(-Axis)) > cos(Radius))
			intensitys = fragmentColor *diffuses + vec3(0.6, 0.6, 0.6)*speculars;
		else
			intensitys = vec3(0.0,0.0,0.0);
		intensitys = intensitys * sIntensity;
		float d = length(sLocation - fragmentPosition);
		float att = 1.0 / (Fallout.x + Fallout.y * d + Fallout.z * d * d);
		//float spotatt;
		intensitys = intensitys * att;
	}
	intensity = min((intensityd + intensityp + intensitys),1.0);

	
	if (intensity.z > 0.95)
		color = vec3(0.35, 0.6, 1.0);
	else if (intensity.z > 0.5)
		color = vec3(0.21, 0.36, 0.6);
	else if (intensity.z > 0.25)
		color = vec3(0.14, 0.24, 0.4);
	else
		color = vec3(0.07, 0.12, 0.2);
}
