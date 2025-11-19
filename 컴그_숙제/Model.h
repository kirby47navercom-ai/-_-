#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp> // 쿼터니언 필수


// stb_image.h는 main.cpp에서 이미 include 했으므로 여기선 include만 합니다.
#include "stb_image.h"

using namespace std;

// 한 정점이 영향을 받는 최대 뼈 개수 (게임 표준 = 4)
#define MAX_BONE_INFLUENCE 4

// -----------------------------------------------------------
// 1. 헬퍼 클래스: Assimp 행렬을 GLM 행렬로 변환
// -----------------------------------------------------------
class AssimpGLMHelpers {
public:
	static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from) {
		glm::mat4 to;
		// Assimp는 Row-major, GLM은 Column-major이지만 
		// Assimp 행렬 구조상 이렇게 대입하면 맞습니다.
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
	}

	static inline glm::vec3 GetGLMVec(const aiVector3D& vec) {
		return glm::vec3(vec.x, vec.y, vec.z);
	}

	static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation) {
		return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
	}
};

// -----------------------------------------------------------
// 2. 구조체 정의
// -----------------------------------------------------------

struct ModelVertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;

	// 뼈 정보 (스키닝용)
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
	GLuint id;
	string type;
	string path;
};

struct BoneInfo {
	int id;
	glm::mat4 offset; // 뼈의 기본 위치 (Bind Pose)
	glm::mat4 finalTransformation;
};

// -----------------------------------------------------------
// 3. Mesh 클래스
// -----------------------------------------------------------
class Mesh {
public:
	vector<ModelVertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	unsigned int VAO;

	Mesh(vector<ModelVertex> vertices, vector<unsigned int> indices, vector<Texture> textures) {
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
		setupMesh();
	}

	void Draw(GLuint shaderID) {
		// 텍스처 바인딩 (무조건 0번 슬롯)
		if (!textures.empty()) {
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(glGetUniformLocation(shaderID, "texture_diffuse1"), 0);
			glBindTexture(GL_TEXTURE_2D, textures[0].id);
		}

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
	}

private:
	unsigned int VBO, EBO;

	void setupMesh() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ModelVertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// 1. Position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)0);
		// 2. Normal
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof(ModelVertex, Normal));
		// 3. TexCoords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof(ModelVertex, TexCoords));
		// 4. Bone IDs (정수형)
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 4, GL_INT, sizeof(ModelVertex), (void*)offsetof(ModelVertex, m_BoneIDs));
		// 5. Weights
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof(ModelVertex, m_Weights));

		glBindVertexArray(0);
	}
};

// -----------------------------------------------------------
// 4. NewModel 클래스 (애니메이션 로직 포함)
// -----------------------------------------------------------
class NewModel {
public:
	vector<Texture> textures_loaded;
	vector<Mesh> meshes;
	string directory;
	bool gammaCorrection;

	// 모델 Transform
	glm::vec3 pos = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	float angle = 0.0f;

	// 애니메이션 데이터
	map<string, BoneInfo> m_BoneInfoMap;
	int m_BoneCounter = 0;
	Assimp::Importer m_Importer;
	const aiScene* m_Scene;
	glm::mat4 m_GlobalInverseTransform;

	int currentTextureNum = 1;

	NewModel(string const& path, bool gamma = false) : gammaCorrection(gamma) {
		loadModel(path);
	}

	// --- Draw 함수: 시간(time)을 받아 움직임을 계산 ---
	void Draw(GLuint shaderID, float currentTime) {
		// 1. 모델 행렬 전송
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, pos);
		model = glm::scale(model, scale);
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Mixamo 눕는 문제 해결

		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix3fv(glGetUniformLocation(shaderID, "modelNormal"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(model)))));

		// 2. 뼈 애니메이션 계산
		vector<glm::mat4> transforms;
		BoneTransform(currentTime, transforms);

		// 3. 계산된 뼈 행렬들을 셰이더로 전송 (최대 100개)
		for (unsigned int i = 0; i < transforms.size(); ++i) {
			string name = "finalBonesMatrices[" + to_string(i) + "]";
			glUniformMatrix4fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, glm::value_ptr(transforms[i]));
		}

		// 4. 메쉬 그리기
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shaderID);
	}

private:
	void loadModel(string const& path) {
		// 애니메이션을 위해 LimitBoneWeights 필수
		currentTextureNum = 1;
		m_Scene = m_Importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals  | aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights);

		if (!m_Scene || m_Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_Scene->mRootNode) {
			cout << "ERROR::ASSIMP:: " << m_Importer.GetErrorString() << endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));
		if (directory == "") directory = path.substr(0, path.find_last_of('\\'));

		m_GlobalInverseTransform = AssimpGLMHelpers::ConvertMatrixToGLMFormat(m_Scene->mRootNode->mTransformation);
		m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);

		RemoveRootMotion();

		processNode(m_Scene->mRootNode, m_Scene);
	}

	void processNode(aiNode* node, const aiScene* scene) {
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
		vector<ModelVertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			ModelVertex vertex;
			// 뼈 데이터 초기화 (-1로 설정)
			SetVertexBoneDataToDefault(vertex);

			vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
			if (mesh->HasNormals())
				vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

			if (mesh->mTextureCoords[0]) {
				vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
				vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		// 뼈 가중치 추출
		ExtractBoneWeightForVertices(vertices, mesh, scene);

		return Mesh(vertices, indices, textures);
	}

	// --- 뼈/애니메이션 헬퍼 함수들 ---

	void SetVertexBoneDataToDefault(ModelVertex& vertex) {
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			vertex.m_BoneIDs[i] = -1;
			vertex.m_Weights[i] = 0.0f;
		}
	}

	void SetVertexBoneData(ModelVertex& vertex, int boneID, float weight) {
		for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
			// 빈 슬롯(-1)을 찾아서 채움
			if (vertex.m_BoneIDs[i] < 0) {
				vertex.m_Weights[i] = weight;
				vertex.m_BoneIDs[i] = boneID;
				break;
			}
		}
	}

	void ExtractBoneWeightForVertices(vector<ModelVertex>& vertices, aiMesh* mesh, const aiScene* scene) {
		for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			int boneID = -1;
			string boneName = mesh->mBones[boneIndex]->mName.C_Str();

			if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) {
				BoneInfo newBoneInfo;
				newBoneInfo.id = m_BoneCounter;
				newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
				m_BoneInfoMap[boneName] = newBoneInfo;
				boneID = m_BoneCounter;
				m_BoneCounter++;
			}
			else {
				boneID = m_BoneInfoMap[boneName].id;
			}

			auto weights = mesh->mBones[boneIndex]->mWeights;
			int numWeights = mesh->mBones[boneIndex]->mNumWeights;

			for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
				int vertexId = weights[weightIndex].mVertexId;
				float weight = weights[weightIndex].mWeight;
				SetVertexBoneData(vertices[vertexId], boneID, weight);
			}
		}
	}

	// --- 키프레임 보간 함수들 ---

	// 위치 보간
	glm::mat4 CalcInterpolatedPosition(float AnimationTime, const aiNodeAnim* pNodeAnim) {
		if (pNodeAnim->mNumPositionKeys == 1)
			return glm::translate(glm::mat4(1.0f), AssimpGLMHelpers::GetGLMVec(pNodeAnim->mPositionKeys[0].mValue));

		unsigned int index = 0;
		for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
				index = i;
				break;
			}
		}
		unsigned int nextIndex = (index + 1);
		float deltaTime = (float)(pNodeAnim->mPositionKeys[nextIndex].mTime - pNodeAnim->mPositionKeys[index].mTime);
		float factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[index].mTime) / deltaTime;
		factor = glm::clamp(factor, 0.0f, 1.0f);

		const aiVector3D& start = pNodeAnim->mPositionKeys[index].mValue;
		const aiVector3D& end = pNodeAnim->mPositionKeys[nextIndex].mValue;
		aiVector3D delta = start + factor * (end - start);
		return glm::translate(glm::mat4(1.0f), AssimpGLMHelpers::GetGLMVec(delta));
	}

	// 회전 보간 (쿼터니언 SLERP)
	glm::mat4 CalcInterpolatedRotation(float AnimationTime, const aiNodeAnim* pNodeAnim) {
		if (pNodeAnim->mNumRotationKeys == 1) {
			aiQuaternion r = pNodeAnim->mRotationKeys[0].mValue;
			return glm::toMat4(glm::quat(r.w, r.x, r.y, r.z));
		}

		unsigned int index = 0;
		for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
				index = i;
				break;
			}
		}
		unsigned int nextIndex = (index + 1);
		float deltaTime = (float)(pNodeAnim->mRotationKeys[nextIndex].mTime - pNodeAnim->mRotationKeys[index].mTime);
		float factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[index].mTime) / deltaTime;
		factor = glm::clamp(factor, 0.0f, 1.0f);

		const aiQuaternion& start = pNodeAnim->mRotationKeys[index].mValue;
		const aiQuaternion& end = pNodeAnim->mRotationKeys[nextIndex].mValue;
		aiQuaternion out;
		aiQuaternion::Interpolate(out, start, end, factor);
		out = out.Normalize();
		return glm::toMat4(glm::quat(out.w, out.x, out.y, out.z));
	}

	// 크기 보간
	glm::mat4 CalcInterpolatedScaling(float AnimationTime, const aiNodeAnim* pNodeAnim) {
		if (pNodeAnim->mNumScalingKeys == 1)
			return glm::scale(glm::mat4(1.0f), AssimpGLMHelpers::GetGLMVec(pNodeAnim->mScalingKeys[0].mValue));

		unsigned int index = 0;
		for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
				index = i;
				break;
			}
		}
		unsigned int nextIndex = (index + 1);
		float deltaTime = (float)(pNodeAnim->mScalingKeys[nextIndex].mTime - pNodeAnim->mScalingKeys[index].mTime);
		float factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[index].mTime) / deltaTime;
		factor = glm::clamp(factor, 0.0f, 1.0f);

		const aiVector3D& start = pNodeAnim->mScalingKeys[index].mValue;
		const aiVector3D& end = pNodeAnim->mScalingKeys[nextIndex].mValue;
		aiVector3D delta = start + factor * (end - start);
		return glm::scale(glm::mat4(1.0f), AssimpGLMHelpers::GetGLMVec(delta));
	}
	void RemoveRootMotion() {
		if (!m_Scene || m_Scene->mNumAnimations == 0) return;

		aiAnimation* pAnimation = m_Scene->mAnimations[0];

		//cout << "------- Root Motion Cleaning Start -------" << endl;

		for (unsigned int i = 0; i < pAnimation->mNumChannels; i++) {
			aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];
			string nodeName(pNodeAnim->mNodeName.data);

			// 소문자로 변환
			string lowerName = nodeName;
			std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

			// 🚨 [조건 추가] 
			// 1. 이름에 hips, root, pelvis, armature, mixamo가 들어가거나
			// 2. i == 0 (애니메이션 리스트의 첫 번째 놈은 무조건 대장 뼈일 확률이 99%)
			if (i == 0 ||
				lowerName.find("hips") != string::npos ||
				lowerName.find("root") != string::npos ||
				lowerName.find("pelvis") != string::npos ||
				lowerName.find("armature") != string::npos || // Armature 추가
				lowerName.find("mixamo") != string::npos)     // Mixamo 접두어 추가
			{
				// 위치 키프레임(0초~끝까지) 순회하며 이동 삭제
				for (unsigned int k = 0; k < pNodeAnim->mNumPositionKeys; k++) {
					aiVector3D& pos = pNodeAnim->mPositionKeys[k].mValue;

					// X(좌우), Z(앞뒤) 이동 삭제 -> 0.0
					pos.x = 0.0f;
					pos.z = 0.0f;

					// Walk 애니메이션은 위아래(Y)로도 많이 튀면 보기 싫으니
					// Y축 이동도 너무 크면 줄여버립니다 (선택 사항)
					// pos.y = 0.0f; // <-- 만약 캐릭터가 공중에 뜬다면 이 주석을 푸세요
				}
				//cout << "[FIXED] Root Motion Removed for: " << nodeName << endl;
			}
		}
		//cout << "------------------------------------------" << endl;
	}

	// 노드 계층 구조 순회 및 행렬 계산
	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& parentTransform) {
		string nodeName(pNode->mName.data);
		const aiAnimation* pAnimation = m_Scene->mAnimations[0];

		glm::mat4 nodeTransformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(pNode->mTransformation);

		const aiNodeAnim* pNodeAnim = nullptr;
		// 애니메이션 채널에서 해당 노드 찾기
		for (unsigned int i = 0; i < pAnimation->mNumChannels; i++) {
			if (string(pAnimation->mChannels[i]->mNodeName.data) == nodeName) {
				pNodeAnim = pAnimation->mChannels[i];
				break;
			}
		}

		if (pNodeAnim) {
			glm::mat4 Scaling = CalcInterpolatedScaling(AnimationTime, pNodeAnim);
			glm::mat4 Rotation = CalcInterpolatedRotation(AnimationTime, pNodeAnim);
			glm::mat4 Translation = CalcInterpolatedPosition(AnimationTime, pNodeAnim);

			// ---------------------------------------------------------
			// 🚨 [강력 수정] 루트 모션(잔상) 제거 로직
			// ---------------------------------------------------------

			// 1. 뼈 이름을 소문자로 변환하여 대소문자 문제 해결
			string lowerName = nodeName;
			std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
				[](unsigned char c) { return std::tolower(c); });

			// 2. 엉덩이/골반/루트 뼈인지 광범위하게 검사
			if (lowerName.find("hips") != string::npos ||
				lowerName.find("root") != string::npos ||
				lowerName.find("pelvis") != string::npos)
			{
				// 위치값 가져오기
				glm::vec3 pos = glm::vec3(Translation[3]);

				// 3. X(좌우), Z(앞뒤) 이동을 0.0f로 죽여버림 (Y축 위아래만 허용)
				Translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, pos.y, 0.0f));

				// (디버깅용) 콘솔에 이 메시지가 뜨는지 꼭 확인하세요!
				// cout << "[System] Root Motion FIXED for bone: " << nodeName << endl;
			}
			// ---------------------------------------------------------

			nodeTransformation = Translation * Rotation * Scaling;
		}

		glm::mat4 GlobalTransformation = parentTransform * nodeTransformation;

		if (m_BoneInfoMap.find(nodeName) != m_BoneInfoMap.end()) {
			int BoneIndex = m_BoneInfoMap[nodeName].id;
			m_BoneInfoMap[nodeName].finalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfoMap[nodeName].offset;
		}

		for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
			ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
		}
	}

	// 텍스처 로딩 함수 (기존 성공한 로직 그대로 사용)
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
		vector<Texture> textures;

		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++) {
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip) {
				Texture texture;
				string filename = to_string(currentTextureNum) + ".png";
				string fullPath = directory + '/' + filename;
				texture.id = TextureFromFile(fullPath.c_str());

				if (texture.id == 0) texture.id = TextureFromFile(filename.c_str());

				if (texture.id != 0) {
					texture.type = typeName;
					texture.path = str.C_Str();
					textures.push_back(texture);
					textures_loaded.push_back(texture);
					//cout << "[Success] Loaded: " << filename << endl;
				}
			}
		}
		if (currentTextureNum < 6) currentTextureNum++;
		return textures;
	}

	unsigned int TextureFromFile(const char* path) {
		string filename = string(path);
		unsigned int textureID;
		glGenTextures(1, &textureID);
		int width, height, nrComponents;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data) {
			GLenum format = (nrComponents == 1) ? GL_RED : (nrComponents == 3) ? GL_RGB : GL_RGBA;
			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			stbi_image_free(data);
			return textureID;
		}
		else {
			stbi_image_free(data);
			return 0;
		}
	}

public:
	// 외부에서 호출하는 뼈 변형 함수
	void BoneTransform(float timeInSeconds, vector<glm::mat4>& Transforms) {
		glm::mat4 Identity = glm::mat4(1.0f);

		float TicksPerSecond = 25.0f;
		float Duration = 0.0f;

		if (m_Scene->mNumAnimations > 0) {
			TicksPerSecond = (float)(m_Scene->mAnimations[0]->mTicksPerSecond != 0 ? m_Scene->mAnimations[0]->mTicksPerSecond : 25.0f);
			Duration = (float)m_Scene->mAnimations[0]->mDuration;
		}
		else {
			// 애니메이션 없으면 리턴
			return;
		}

		float TimeInTicks = timeInSeconds * TicksPerSecond;
		float AnimationTime = fmod(TimeInTicks, Duration);

		ReadNodeHeirarchy(AnimationTime, m_Scene->mRootNode, Identity);

		Transforms.resize(100); // 셰이더 배열 크기(100)에 맞춤
		for (int i = 0; i < 100; i++) Transforms[i] = glm::mat4(1.0f); // 초기화

		for (auto const& [name, info] : m_BoneInfoMap) {
			if (info.id < 100) {
				Transforms[info.id] = info.finalTransformation; // 셰이더로 보낼 최종 행렬
			}
		}
	}
};

// BoneInfo 구조체 정의 (map 사용을 위해 위쪽 정의가 완전해야 함)
// 위 코드에 이미 struct BoneInfo { int id; glm::mat4 offset; glm::mat4 finalTransformation; }; 가 필요함.
// 위쪽 BoneInfo 구조체에 finalTransformation 멤버를 추가해야 합니다.