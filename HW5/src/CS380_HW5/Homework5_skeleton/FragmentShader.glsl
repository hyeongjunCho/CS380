#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentNormal;
in vec2 UV;

// Ouput data
out vec3 color;

//uniform vec3 uLight;
uniform vec3 dColor;
uniform vec3 pColor;
uniform vec3 sColor;
uniform float dIntensity;
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

uniform sampler2D myTextureSampler;

void main(){
	//vec3 tolight = normalize(uLight - fragmentPosition);	
	vec3 toV = -normalize(vec3(fragmentPosition));
	//vec3 h = normalize(toV + tolight);
	vec3 normal = normalize(fragmentNormal);
	vec3 colord = vec3(0.0);
	vec3 colorp = vec3(0.0);
	vec3 colors = vec3(0.0);
	float speculard = 0.0;
	float specularp = 0.0;
	float speculars = 0.0;
	float diffused = 0.0;
	float diffusep = 0.0;
	float diffuses = 0.0;
	vec3 intensityd = vec3(0.0);
	vec3 intensityp = vec3(0.0);
	vec3 intensitys = vec3(0.0);


	//float specular = pow(max(0.0, dot(h, normal)), 64.0);
	//float diffuse = max(0.0, dot(normal, tolight));
	
	//vec3 intensity = vec3(1.0,1.0,0.0) * diffuse + vec3(0.3, 0.3, 0.3)*specular;

	//directional
	vec3 fragmentColor = vec3(1.0,1.0,0.0);
	if (dL == 1)
	{
		vec3 tolightd = normalize(-Direction);
		vec3 hd = normalize(toV + tolightd);

		speculard = pow(max(0.0, dot(hd, normal)), 64.0);//r dot v == n dot h
		diffused = max(0.0, dot(normal, tolightd));//l dot n

		intensityd = fragmentColor *diffused + vec3(0.6, 0.6, 0.6)*speculard;
		intensityd = intensityd * dIntensity;

		colord = pow(intensityd, vec3(1.0 / 2.2)) * dColor;
	}

	//point
	if (pL == 1)
	{
		vec3 tolightp = normalize(pLocation - fragmentPosition);
		vec3 hp = normalize(toV + tolightp);

		specularp = pow(max(0.0, dot(hp, normal)), 64.0);//r dot v == n dot h
		diffusep = max(0.0, dot(normal, tolightp));//l dot n

		intensityp = fragmentColor *diffusep + vec3(0.6, 0.6, 0.6)*specularp;
		intensityp = intensityp * pIntensity;
		float d = length(pLocation - fragmentPosition);
		float att = 1.0 / (Fallout.x + Fallout.y * d + Fallout.z * d * d);
		intensityp = intensityp * att;

		colorp = pow(intensityp, vec3(1.0 / 2.2)) * pColor;
	}
	//spotlight
	if (sL == 1)
	{
		vec3 tolights = normalize(sLocation - fragmentPosition);
		vec3 hs = normalize(toV + tolights);

		speculars = pow(max(0.0, dot(hs, normal)), 64.0);//r dot v == n dot h
		diffuses = max(0.0, dot(normal, tolights));//l dot n
		if (dot(tolights, normalize(-Axis)) > cos(Radius))
			intensitys = fragmentColor *diffuses + vec3(0.6, 0.6, 0.6)*speculars;
		else
			intensitys = vec3(0.0,0.0,0.0);
		intensitys = intensitys * sIntensity;
		float d = length(sLocation - fragmentPosition);
		float att = 1.0 / (Fallout.x + Fallout.y * d + Fallout.z * d * d);
		//float spotatt;
		intensitys = intensitys * att;

		colors = pow(intensitys, vec3(1.0 / 2.2)) * sColor;
	}
	//color = colord + colorp + colors;
	vec3 sum_intensity = (intensityd + intensityp + intensitys);
	float float_intensity = (sum_intensity.x + sum_intensity.y + sum_intensity.z)/3.0;
	float diffuse = min((diffuses + diffusep + diffused), 1.0);
	float specular = min((speculars + specularp + speculard), 1.0);

	vec3 Kd = vec3(1.0, 1.0, 0.0);
	//TODO: Change material color to texture color		
	Kd = texture(myTextureSampler, UV).rgb;

	vec3 intensity = (Kd * diffuse + vec3(0.3, 0.3, 0.3)*specular) * float_intensity;
	
	vec3 finalColor = intensity;
	color = pow(finalColor, vec3(1.0 / 2.2));// Apply gamma correction    
}