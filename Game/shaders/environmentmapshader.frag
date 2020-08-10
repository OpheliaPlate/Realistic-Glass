#version 450

layout(binding = 1) uniform LightingBufferObject {
    vec4 viewPosition;
    vec4 lightPosition;
    vec4 lightColor;
    int glassAlgo;
} lbo;

layout (binding = 2) uniform samplerCube shadowCubeMap;

layout(binding = 3) uniform sampler2D texSampler1;
layout(binding = 4) uniform sampler2D texSampler2;
layout(binding = 5) uniform sampler2D texSampler3;
layout(binding = 6) uniform sampler2D texSampler4;
layout(binding = 7) uniform sampler2D texSampler5;
layout(binding = 8) uniform sampler2D texSampler6;

layout(binding = 9) uniform samplerCube environmentMap1;
layout(binding = 10) uniform samplerCube environmentMap2;
layout(binding = 11) uniform samplerCube environmentMap3;
layout(binding = 12) uniform samplerCube environmentMap4;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) flat in vec3 fragInstancePos;
layout(location = 5) flat in int fragTexIndex;
layout(location = 6) flat in int fragSelected;
layout(location = 7) flat in float fragFresnel;
layout(location = 8) flat in int fragInstanceIndex;
layout(location = 9) flat in vec3 fragInstanceColor;
layout(location = 10) flat in int fragInstanceReferencePointIndex;
layout(location = 11) in vec3 fragInstanceReferencePoint;
layout(location = 12) flat in float fragInstanceRefraction;
layout(location = 13) flat in vec3 referencePoint;

layout(location = 0) out vec4 outColor;

#define EPSILON 0.001
#define SHADOW_OPACITY 0.3

vec3 hitParallax(in vec3 x, in vec3 R, in samplerCube distmap) {
    float environmentMapDist = texture(distmap, R).a;
    float dp = environmentMapDist - dot(x, R);
    vec3 p = x + R * dp;
    return p;
}

void main()
{
    float dist = distance(fragPosition, vec3(lbo.lightPosition));
    
    vec3 fixedLight;
    {
        // ambient
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * vec3(lbo.lightColor);
          
        // diffuse
        vec3 norm = normalize(fragNormal);
        vec3 lightDir = normalize(vec3(lbo.lightPosition) - fragPosition);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(lbo.lightColor);
        
        // specular
        float specularStrength = 0.5;
        vec3 viewDir = normalize(vec3(lbo.viewPosition) - fragPosition);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * vec3(lbo.lightColor);
        fixedLight = (ambient + diffuse + specular);
    }
    
    vec3 lightVec = fragPosition - vec3(lbo.lightPosition);
    float sampledDist = texture(shadowCubeMap, lightVec).r;
    float shadow = (length(lightVec) <= sampledDist + EPSILON) ? 1.0 : SHADOW_OPACITY;
    
    vec3 x = fragPosition - fragInstancePos;

    vec3 reflection;
    vec3 refraction;
    if (fragTexIndex > 5) {
        vec3 Fp = vec3(fragFresnel);
        float n = 1 / fragInstanceRefraction;

        vec3 N = fragNormal;
        //vec3 V1 = fragInstanceReferencePoint - vec3(lbo.viewPosition);
        vec3 V = fragPosition - vec3(-referencePoint.x, -referencePoint.y, -referencePoint.z); //vec3(lbo.viewPosition);
        vec3 VN = normalize(V);
        vec3 NN = normalize(N);
        vec3 R1 = reflect(VN, NN); // Reflection direction
        vec3 R2 = refract(VN, NN, n); // Reflection direction
        
        // New linear hit
        vec3 r1;
        vec3 r2;
        vec3 F = Fp + pow(1-dot(NN, -VN), 5) * (1-Fp);
        switch(fragInstanceReferencePointIndex) {
            case 1:
                //if (lbo.glassAlgo == 0) {
                    // Old paralax hit
                    r1 = hitParallax(x, R1, environmentMap1);
                    r2 = hitParallax(x, R2, environmentMap1);
                //} else {
                    //r1 = hit(x, R1, environmentMap1);
                    //r2 = hit(x, R2, environmentMap1);
                //}
                
                reflection = F * texture(environmentMap1, r1).rgb;
                refraction = (1-F) * texture(environmentMap1, r2).rgb;
                break;
            case 2:
                //if (lbo.glassAlgo == 0) {
                    // Old paralax hit
                    r1 = hitParallax(x, R1, environmentMap2);
                    r2 = hitParallax(x, R2, environmentMap2);
                //} else {
                    //r1 = hit(x, R1, environmentMap2);
                    //r2 = hit(x, R2, environmentMap2);
                //}
                
                reflection = F * texture(environmentMap2, r1).rgb;
                refraction = (1-F) * texture(environmentMap2, r2).rgb;
                break;
            case 3:
                //if (lbo.glassAlgo == 0) {
                    // Old paralax hit
                    r1 = hitParallax(x, R1, environmentMap3);
                    r2 = hitParallax(x, R2, environmentMap3);
                //} else {
                    //r1 = hit(x, R1, environmentMap3);
                    //r2 = hit(x, R2, environmentMap3);
                //}
                
                reflection = F * texture(environmentMap3, r1).rgb;
                refraction = (1-F) * texture(environmentMap3, r2).rgb;
                break;
            case 4:
                //if (lbo.glassAlgo == 0) {
                    // Old paralax hit
                    r1 = hitParallax(x, R1, environmentMap4);
                    r2 = hitParallax(x, R2, environmentMap4);
                //} else {
                    //r1 = hit(x, R1, environmentMap4);
                    //r2 = hit(x, R2, environmentMap4);
                //}
                
                reflection = F * texture(environmentMap4, r1).rgb;
                refraction = (1-F) * texture(environmentMap4, r2).rgb;
                break;
        }
        outColor = vec4(fragInstanceColor * (reflection + refraction), 1.0);
        return;
    }
    
    if (fragTexIndex == 0) {
        outColor = vec4(fixedLight * texture(texSampler1, fragTexCoord).rgb * shadow, dist);
    } else if (fragTexIndex == 1) {
        outColor = vec4(fixedLight * texture(texSampler2, fragTexCoord).rgb * shadow, dist);
    } else if (fragTexIndex == 2) {
        outColor = vec4(fixedLight * texture(texSampler3, fragTexCoord).rgb * shadow, dist);
    } else if (fragTexIndex == 3) {
        int selected = fragSelected;
        while (selected > 0) {
            selected -= 1;
            int ySelected = (selected % 100) / 10;
            int xSelected = selected % 10;
            int x = int(((fragPosition.x + 0.15) * 100.0)) / 3;
            int y = int(((fragPosition.y + 0.15) * 100.0)) / 3;
            if (x == xSelected && y == ySelected) {
                outColor = vec4(0.8f * vec3(0.7f, 0.1f, 0.8f) + 0.2f * fixedLight * texture(texSampler5, fragTexCoord).rgb * shadow, dist);
                return;
            }
            
            selected /= 100;
        }
        
        outColor = vec4(0.5 * fixedLight * texture(texSampler4, fragTexCoord).rgb * shadow, dist);
    } else if (fragTexIndex == 4) {
        if (fragSelected != 0) {
            outColor = vec4(0.8f * vec3(1.0f, 0.2f, 0.1f) + 0.2f * fixedLight * texture(texSampler5, fragTexCoord).rgb * shadow, dist);
        } else {
            outColor = vec4(fixedLight * texture(texSampler5, fragTexCoord).rgb * shadow, dist);
        }
    } else {
        if (fragSelected != 0) {
            outColor = vec4(0.8f * vec3(0.3f, 0.1f, 1.0f) + 0.2f * fixedLight * texture(texSampler6, fragTexCoord).rgb * shadow, dist);
        } else {
            outColor = vec4(fixedLight * texture(texSampler6, fragTexCoord).rgb * shadow, dist);
        }
    }
}
