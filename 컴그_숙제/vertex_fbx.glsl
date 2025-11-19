#version 330 core

// 1. 속성(Attribute) 정의
layout (location = 0) in vec3 aPos;       // 위치
layout (location = 1) in vec3 aNormal;    // 노멀
layout (location = 2) in vec2 aTexCoords; // 텍스처 좌표
layout (location = 3) in ivec4 aBoneIDs;  // 뼈 번호 (최대 4개, 정수형)
layout (location = 4) in vec4 aWeights;   // 가중치 (최대 4개, 실수형)

// 2. 출력 변수 (프래그먼트 셰이더로 전달)
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

// 3. Uniform 변수
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat3 modelNormal;

// 뼈 애니메이션 행렬 배열 (최대 100개)
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
    // 4. 뼈 변환 행렬 계산 (Skinning)
    mat4 BoneTransform = mat4(0.0f);
    float totalWeight = 0.0f;

    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        // 뼈 ID가 -1이면 영향 없음
        if(aBoneIDs[i] == -1) 
            continue;

        // 뼈 ID가 배열 범위를 넘어가면 안전장치
        if(aBoneIDs[i] >= MAX_BONES) 
        {
            break;
        }

        // (해당 뼈의 행렬 * 가중치)를 누적
        BoneTransform += finalBonesMatrices[aBoneIDs[i]] * aWeights[i];
        totalWeight += aWeights[i];
    }

    // 만약 뼈 정보가 없거나 가중치 합이 0이면, 애니메이션 없이 기본 상태 유지
    if (totalWeight == 0.0f) {
        BoneTransform = mat4(1.0f);
    }

    // 5. 정점 위치 변형 (애니메이션 적용)
    // 원래 위치(aPos)에 뼈 변환(BoneTransform)을 먼저 곱하고 -> 그 뒤에 월드(model) 변환
    vec4 animatedPos = BoneTransform * vec4(aPos, 1.0f);
    FragPos = vec3(model * animatedPos);
    
    // 6. 노멀 벡터 변형 (회전만 적용)
    vec3 animatedNormal = mat3(BoneTransform) * aNormal;
    Normal = normalize(modelNormal * animatedNormal);
    
    // 7. 텍스처 좌표 전달
    TexCoords = aTexCoords;
    
    // 8. 최종 화면 좌표 계산
    gl_Position = proj * view * vec4(FragPos, 1.0);
}