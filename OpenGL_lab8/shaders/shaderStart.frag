#version 410 core

// Input from vertex shader
in vec3 fNormal;          // Normal vector in eye coordinates
in vec4 fPosEye;          // Fragment position in eye coordinates
in vec2 fragTexCoords;    // Texture coordinates
in vec4 fragPosLightSpace; // Fragment position in light space (added for shadows)

// Output color
out vec4 fColor;

// Uniforms
uniform vec3 lightPosEye;  // Light position in eye coordinates
uniform vec3 lightColor;   // Light color
uniform vec3 viewPosEye;   // Camera position in eye coordinates

// Textures
uniform sampler2D diffuseTexture;  // Diffuse map
uniform sampler2D specularTexture; // Specular map
uniform sampler2D shadowMap;       // Shadow map (added)

// Lighting parameters
const float ambientStrength = 0.8f;
const float specularStrength = 1.5f;
const float shininess = 64.0f;

// Attenuation parameters
const float constant = 1.0f; 
const float linear = 0.0045f; 
const float quadratic = 0.0075f;

// Light components
vec3 ambient;
vec3 diffuse;
vec3 specular;

// Shadow bias to reduce shadow acne
const float shadowBias = 0.005f;

// Additional light sources
in vec3 lightPosEye1; 
uniform vec3 pointLight1;
uniform vec3 pointLightColor1;

// Function to compute light components (added)
void computeLightComponents() {
    // Transform normal
    vec3 normalEye = normalize(fNormal);

    // Compute light direction
    vec3 lightDirN = normalize(lightPosEye - fPosEye.xyz);

    // Compute view direction
    vec3 viewDirN = normalize(viewPosEye - fPosEye.xyz);

    // Compute ambient lighting
    ambient = ambientStrength * lightColor * texture(diffuseTexture, fragTexCoords).rgb;

    // Compute diffuse lighting
    float diff = max(dot(normalEye, lightDirN), 0.0);
    diffuse = diff * lightColor * texture(diffuseTexture, fragTexCoords).rgb;

    // Compute specular lighting
    vec3 halfVector = normalize(lightDirN + viewDirN);
    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
    specular = specCoeff * lightColor * texture(specularTexture, fragTexCoords).rgb;
}

// Function to compute the shadow factor
float computeShadow(vec4 fragPosLightSpace) {
    // Transform fragment position to normalized device coordinates
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Check if fragment is outside the shadow map
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    // Retrieve the closest depth from the shadow map
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // Current depth from light's perspective
    float currentDepth = projCoords.z;

    // Compare depths and determine if the fragment is in shadow
    return currentDepth > closestDepth + shadowBias ? 1.0 : 0.0;
}

// Function to compute point light
void computePointLight(vec3 lPE, vec3 pointColor) {
    // Transform normal
    vec3 normalEye = normalize(fNormal);

    // Compute light direction
    vec3 lightDirN = normalize(lPE - fPosEye.xyz);

    // Compute view direction
    vec3 viewDirN = normalize(-fPosEye.xyz);

    vec3 halfVector = normalize(lightDirN + viewDirN);
    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);

    // Compute distance to light
    float dist = length(lPE - fPosEye.xyz);

    // Compute attenuation
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

    // Calculate light components
    vec3 ambient1 = att * ambientStrength * pointColor * texture(diffuseTexture, fragTexCoords).rgb;
    vec3 diffuse1 = att * max(dot(normalEye, lightDirN), 0.0f) * pointColor * texture(diffuseTexture, fragTexCoords).rgb;
    vec3 specular1 = att * specularStrength * specCoeff * pointColor * texture(specularTexture, fragTexCoords).rgb;

    ambient += ambient1;
    diffuse += diffuse1;
    specular += specular1;
}

// Function to compute fog factor
float computeFog() {
    float fogDensity = 0.02f;
    float fragmentDistance = length(fPosEye.xyz);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2.0));
    return clamp(fogFactor, 0.0f, 1.0f);
}

void main() {
    // Compute light components
    computeLightComponents();
	
    // Compute point light
    computePointLight(lightPosEye1, pointLightColor1);

    // Compute shadow factor
    float shadow = computeShadow(fragPosLightSpace);

    // Combine light components with shadow modulation
    vec3 color = ambient + (1.0 - shadow) * (diffuse + specular);
    color = min(color, vec3(1.0f));

    // Compute fog factor
    float fogFactor = computeFog();

    // Fog color
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

    // Final color with fog applied
    vec4 finalColor = mix(fogColor, vec4(color, 1.0f), fogFactor);

    // Output final color
    fColor = finalColor;
}
