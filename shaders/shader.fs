#version 460 core

layout (location = 0) in vec3 aTexCoord;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aFragPos;
layout (location = 3) in vec3 aColor;

out vec4 fragColor;

uniform bool uDrawBox;
uniform bool uDrawIso;
uniform bool uCPU;
uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform sampler3D volumeTex;
uniform sampler2D sliceTex;

vec3 jet(float t) {
    t = clamp(t, 0.0, 1.0);

    float r = clamp(1.5 - abs(4.0*t - 3.0), 0.0, 1.0);
    float g = clamp(1.5 - abs(4.0*t - 2.0), 0.0, 1.0);
    float b = clamp(1.5 - abs(4.0*t - 1.0), 0.0, 1.0);

    return vec3(r, g, b);
}

vec4 applyPhongShading() {
    vec3 lightDir = normalize(uLightPos - aFragPos);
    vec3 viewDir = normalize(uViewPos - aFragPos);
    vec3 norm = normalize(aNormal);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 materialColor = uCPU ? 1 - aColor: aColor;

    float kd = 0.6;
    float ks = 0.3;
    float ka = 0.1;

    vec3 specular = vec3(0.0);

    // This is to prevent any ghost highlight on back face
    if (dot(norm, lightDir) > 0.0) {
        specular = ks * pow(max(dot(reflectDir, viewDir), 0.0), 3) * lightColor;
    }

    vec3 diffuse = kd * max(dot(norm, lightDir), 0.0) * materialColor * lightColor;
    vec3 ambient = ka * 1.0 * lightColor * materialColor;

    return vec4(diffuse + specular + ambient, 1.0);
}

void main()
{
    if (uDrawBox) {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    if (uDrawIso) {
        fragColor = applyPhongShading();
        return;
    }

    if (uCPU) {
        fragColor = vec4(1.0 - jet(texture(sliceTex, aTexCoord.xy).r), 1.0);
    }
    else {
        fragColor = vec4(jet(texture(volumeTex, aTexCoord).r), 1.0);
    }
}
