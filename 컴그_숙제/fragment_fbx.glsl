#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1; // 텍스처

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ambientLight;

void main() {
    // 1. 텍스처 색상 가져오기
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // 투명한 부분 버림
    if(texColor.a < 0.1)
        discard;

    // 2. 조명 계산 (Ambient + Diffuse)
    vec3 ambient = ambientLight * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 반사광(Specular)은 필요하면 추가 (지금은 생략)

    // 3. 최종 색상 = (조명) * 텍스처
    vec3 result = (ambient + diffuse) * texColor.rgb;
    FragColor = vec4(result, 1.0);
}