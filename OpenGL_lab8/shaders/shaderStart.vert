#version 410 core

// Layout de la locațiile input
layout(location = 0) in vec3 vPosition;  // Poziția vertecelor
layout(location = 1) in vec3 vNormal;    // Normalele vertecelor
layout(location = 2) in vec2 vTexCoords; // Coordonatele texturii

// Outputs către fragment shader
out vec3 fNormal;             // Normalul în coordonate de ochi
out vec4 fPosEye;             // Poziția fragmentului în coordonate de ochi
out vec2 fragTexCoords;       // Coordonatele texturii
out vec4 fragPosLightSpace;   // Poziția fragmentului în spațiul luminii

uniform mat4 model;           // Matricea modelului (transformarea modelului în coordonate globale)
uniform mat4 view;            // Matricea camerei (transformarea în coordonate de ochi)
uniform mat4 projection;      // Matricea proiecției
uniform mat3 normalMatrix;    // Matricea normalelor pentru transformarea corectă a normalelor
uniform mat4 lightSpaceTrMatrix; // Matricea de transformare pentru lumina în coordonate de clip

out vec3 lightPosEye1;        // Poziția luminii în coordonate de ochi
uniform vec3 pointLight1;     // Poziția luminii punctiforme

void main() 
{
    // Se transmit coordonatele texturii către fragment shader
    fragTexCoords = vTexCoords;

    // Se calculează poziția fragmentului în coordonate de ochi
    fPosEye = view * model * vec4(vPosition, 1.0f);

    // Se transformă normalul în coordonate de ochi
    fNormal = normalize(normalMatrix * vNormal);

    // Se calculează poziția fragmentului în coordonatele luminii
    fragPosLightSpace = lightSpaceTrMatrix * model * vec4(vPosition, 1.0f);

    // Se calculează poziția luminii în coordonate de ochi
    lightPosEye1 = (view * model * vec4(pointLight1, 1.0f)).xyz;

    // Se calculează poziția finală în coordonate de clip
    gl_Position = projection * view * model * vec4(vPosition, 1.0f);
}
