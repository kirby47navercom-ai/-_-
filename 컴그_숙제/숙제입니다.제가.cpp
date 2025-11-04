#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것

#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "dtd.h" 
#define SIZE 800
// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void Keyboard(unsigned char key, int x, int y);
void Keyupboard(unsigned char key, int x, int y);
void SpecialKeyboard(int key, int x, int y); // 특수 키(화살표) 처리 함수 선언
void LoadOBJ(const char* filename);
void InitBuffer();
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
void TimerFunction(int value);
GLchar* filetobuf(const char* file);
// 전역 변수
GLint width = 1000, height = 1000;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;

random_device rd;
mt19937 gen(rd());
uniform_real_distribution<float> r_speed(0.1f, 0.2f);
uniform_int_distribution<int> r_size(50, 100);
uniform_int_distribution<int> r_m(0, 1);





typedef struct {
	float x, y, z;
} Vertex;
typedef struct {
	unsigned int v1, v2, v3;
} Face;
typedef struct {
	Vertex* vertices;
	size_t vertex_count;
	Face* faces;
	size_t face_count;
} Model;
class Shape {
public:
	vector<GLfloat> vertexData;
	vector<GLuint> indices;
	glm::vec3 t = { 0.0f,0.0f,0.0f };
	glm::vec3 s = { 1.0f,1.0f,1.0f };
	glm::vec3 r = { 0.0f,0.0f,0.0f };
	glm::vec3 colors;
	glm::mat4 modelMat = glm::mat4(1.0f);
	int shape_num;
	float angle = 0.0f;
	Shape(Model model, int i) {
		vertexData.clear();
		InitBuffer(model);
		ColorRandom(model);
		shape_num = i;
	}

	void InitBuffer(Model model) {
		vertexData.clear();
		// 모든 face의 꼭짓점 좌표를 중복 포함해서 vertexData에 추가
		for (size_t i = 0; i < model.face_count; ++i) {
			Face f = model.faces[i];
			Vertex v1 = model.vertices[f.v1];
			Vertex v2 = model.vertices[f.v2];
			Vertex v3 = model.vertices[f.v3];
			// 첫 번째 꼭짓점
			vertexData.push_back(v1.x);
			vertexData.push_back(v1.y);
			vertexData.push_back(v1.z);
			// 두 번째 꼭짓점
			vertexData.push_back(v2.x);
			vertexData.push_back(v2.y);
			vertexData.push_back(v2.z);
			// 세 번째 꼭짓점
			vertexData.push_back(v3.x);
			vertexData.push_back(v3.y);
			vertexData.push_back(v3.z);
		}
		// 인덱스는 필요 없으므로 비워둡니다
		indices.clear();
	}
	void Update(Model model) {
		//vertexData.clear();
		//for(size_t i = 0; i < model.vertex_count; ++i) {
		//	Vertex v = model.vertices[i];
		//	// OBJ 좌표를 OpenGL에 맞게 사용 (스케일링 제거, 0~1 범위 그대로)
		//	float x = v.x;
		//	float y = v.y;
		//	float z = v.z;
		//	vertexData.push_back(x);
		//	vertexData.push_back(y);
		//	vertexData.push_back(z);
		//}
		//indices.clear();
		//for(size_t i = 0; i < model.face_count; ++i) {
		//	indices.push_back(model.faces[i].v1);
		//	indices.push_back(model.faces[i].v2);
		//	indices.push_back(model.faces[i].v3);
		//}
	}
	void ColorRandom(Model model) {
		colors = { 0.0f,0.0f,0.0f };

		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		colors = glm::vec3(r, g, b);

	}
	glm::vec3 Center() {
		glm::vec3 center(0.0f);
		for (int i = 0; i < vertexData.size() / 3; ++i) {
			center.x += vertexData[i * 3 + 0];
			center.y += vertexData[i * 3 + 1];
			center.z += vertexData[i * 3 + 2];
		}
		center /= static_cast<float>(vertexData.size() / 3);
		return center;
	}


};
vector<Shape>shape;

class Block {
public:
	vector<GLfloat> vertexData;
	vector<GLuint> indices;
	glm::vec3 t = { 0.0f,0.0f,0.0f };
	glm::vec3 s = { 1.0f,1.0f,1.0f };
	glm::vec3 r = { 0.0f,0.0f,0.0f };
	glm::vec3 colors;
	glm::mat4 modelMat = glm::mat4(1.0f);
	int size = 0;
	float speed = 0.0f;
	float angle = 0.0f;
	Block(Model model, int size_,float speed_) {
		vertexData.clear();
		InitBuffer(model);
		ColorRandom(model);
		size = size_;
		speed = speed_;
	}
	void InitBuffer(Model model) {
		vertexData.clear();
		for (size_t i = 0; i < model.face_count; ++i) {
			Face f = model.faces[i];
			Vertex v1 = model.vertices[f.v1];
			Vertex v2 = model.vertices[f.v2];
			Vertex v3 = model.vertices[f.v3];
			// 첫 번째 꼭짓점
			vertexData.push_back(v1.x);
			vertexData.push_back(v1.y);
			vertexData.push_back(v1.z);
			// 두 번째 꼭짓점
			vertexData.push_back(v2.x);
			vertexData.push_back(v2.y);
			vertexData.push_back(v2.z);
			// 세 번째 꼭짓점
			vertexData.push_back(v3.x);
			vertexData.push_back(v3.y);
			vertexData.push_back(v3.z);
		}
		indices.clear();
	}
	void Update(Model model) {
		//vertexData.clear();
		//for(size_t i = 0; i < model.vertex_count; ++i) {
		//	Vertex v = model.vertices[i];
		//	// OBJ 좌표를 OpenGL에 맞게 사용 (스케일링 제거, 0~1 범위 그대로)
		//	float x = v.x;
		//	float y = v.y;
		//	float z = v.z;
		//	vertexData.push_back(x);
		//	vertexData.push_back(y);
		//	vertexData.push_back(z);
		//}
		//indices.clear();
		//for(size_t i = 0; i < model.face_count; ++i) {
		//	indices.push_back(model.faces[i].v1);
		//	indices.push_back(model.faces[i].v2);
		//	indices.push_back(model.faces[i].v3);
		//}
	}
	void ColorRandom(Model model) {
		colors = { 0.0f,0.0f,0.0f };

		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		colors = glm::vec3(r, g, b);

	}
};
vector<Block>block;
void read_newline(char* str) {
	char* pos;
	if ((pos = strchr(str, '\n')) != NULL)
		*pos = '\0';
}


Model read_obj_file(const char* filename) {
	FILE* file;
	Model model{};

	// 파일 열기 (fopen_s를 사용하므로 Visual Studio 환경에 적합)
	fopen_s(&file, filename, "r");
	if (!file) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}

	// 임시 벡터를 사용하여 동적으로 정점 및 면 데이터를 저장합니다.
	std::vector<Vertex> temp_vertices;
	std::vector<Face> temp_faces;

	char line[365];
	while (fgets(line, sizeof(line), file)) {
		read_newline(line);

		// v 라인 처리
		if (line[0] == 'v' && line[1] == ' ') {
			Vertex v;
			if (sscanf_s(line + 2, "%f %f %f", &v.x, &v.y, &v.z) == 3) {
				temp_vertices.push_back(v);
			}
		}
		// f 라인 처리 - N각형을 삼각 분할(Triangulation)합니다.
		else if (line[0] == 'f' && line[1] == ' ') {
			char face_str[365];
			// 원본 라인을 훼손하지 않기 위해 복사 (strtok 사용을 위함)
			// '제목 없음.obj' 파일이 'f v1 v2 v3 v4...' 형식이라고 가정합니다.
			if (strlen(line + 2) >= sizeof(face_str)) {
				// 버퍼 오버플로우 방지 (필요 시 더 큰 버퍼 사용)
				continue;
			}
			strcpy_s(face_str, sizeof(face_str), line + 2);

			// strtok를 사용하여 공백으로 분리된 각 정점 데이터를 가져옵니다.
			char* token = strtok(face_str, " ");
			std::vector<unsigned int> face_indices; // 면을 이루는 정점 인덱스 저장

			// 토큰(정점 인덱스)을 읽습니다.
			while (token != NULL) {
				unsigned int v_idx;

				// v/vt/vn 형태일 경우, 첫 '/' 이전의 v만 파싱
				if (strchr(token, '/') != NULL) {
					// v/vt/vn 형태일 경우, 첫 '/' 이전의 v만 파싱
					if (sscanf_s(token, "%u", &v_idx) == 1) {
						face_indices.push_back(v_idx - 1); // 1-based index를 0-based index로 변환
					}
				}
				else {
					// v 형태일 경우, v만 파싱
					if (sscanf_s(token, "%u", &v_idx) == 1) {
						face_indices.push_back(v_idx - 1); // 1-based index를 0-based index로 변환
					}
				}
				token = strtok(NULL, " ");
			}

			// N각형(N >= 3) 면을 삼각 분할(Fan Triangulation)하여 저장합니다.
			if (face_indices.size() >= 3) {
				// 0번 인덱스를 중심으로 삼각형을 만듭니다. (0, 1, 2), (0, 2, 3), (0, 3, 4)...
				for (size_t i = 1; i < face_indices.size() - 1; ++i) {
					Face f;
					f.v1 = face_indices[0]; // 중심점
					f.v2 = face_indices[i];
					f.v3 = face_indices[i + 1];
					temp_faces.push_back(f);
				}
			}
		}
	}
	fclose(file);

	// 최종적으로 모델 구조체에 동적 할당 및 복사합니다.
	model.vertex_count = temp_vertices.size();
	model.face_count = temp_faces.size();

	// 기존 코드가 malloc/free를 사용하므로 이 방식을 유지합니다.
	model.vertices = (Vertex*)malloc(model.vertex_count * sizeof(Vertex));
	model.faces = (Face*)malloc(model.face_count * sizeof(Face));

	if (model.vertex_count > 0) {
		std::copy(temp_vertices.begin(), temp_vertices.end(), model.vertices);
	}
	if (model.face_count > 0) {
		std::copy(temp_faces.begin(), temp_faces.end(), model.faces);
	}

	return model;
}

vector<Model> model;
int facenum = -1;
int modelType = 0; // 0: Cube, 1: Cone
bool allFaceDraw = true; // true: 전체 면 그리기, false: 한 면씩 그리기

//숙제에서 쓸 변수들 코드 네임 황혼 ghkdghs
//카메라
glm::vec3 cameraPos = glm::vec3(0.0f, 20.0f, 40.0f);
glm::vec3 modelPos = glm::vec3(0.0f, 0.0f, 0.0f);  // 도형 위치 (X, Y, Z 이동량)
glm::vec3 camera_move = glm::vec3(0.0f, 0.0f, 0.0f); 


int shape_check = 0;

//카메라 무브
int ws_ = 0;
int ad_ = 0;
int pn_ = 0;

//카메라 각도
float camera_x_angle = 0.0f;
float camera_y_angle = 0.0f;

//카메라 x고정
bool camera_x_lock = false;


//마우스
bool left_mouse = false;
bool right_mouse = false;
int right_mousex, right_mousey;
int left_mousex, left_mousey;
float sense = 0.1f;

//할거 8 7 5 6을 이용하여 카메라가 돌려져도 이동시키기 각도를 가져와서 ㄱㄱ


//가로 세로 받는 변수
int block_width = 0;
int block_height = 0;
int block_start = 0;

//게임 시작 변수
bool start = false;




//명령어 라스트 커멘드
bool op = false;
bool m = false;
bool v = false;



GLfloat tranformx(int x) {
	return ((float)x / (width / 2)) - 1.0f;
}
GLfloat tranformy(int y) {
	return ((height - (float)y) / (height / 2)) - 1.0f;
}


//커비 zjql
void InitData() {
	block_width = 0;
	block_height = 0;
	//나중에 지워주센
	block_width = 25;
	block_height = 25;
	while (block_width < 5 || block_height < 5 || block_width > 25 || block_height > 25) {
		cout << "가로 길이와 세로 길이를 입력해주세요." << endl;
		cout << "* 길이 제한 : 5 ~ 25 *" << endl;
		cout << "입력 : ";cin >> block_width >> block_height;

	}
	block_start = block_width * block_height;
	system("chcp 65001");
	fstream f{ "commend.txt" };
	string s;
	while (getline(f, s))cout << s << endl;


	block.clear();
	for (float i = -(float)block_width / 2;i < (float)block_height/2;i+=1.0f) {
		for (float j = -(float)block_height / 2;j < (float)block_width/2;j += 1.0f) {
			block.push_back(Block(model[0], r_size(gen), r_speed(gen)));

			float f = (i + j) / (float)(block_width + block_height);
			block.back().colors = {f ,1.0f - f,1.0f - f };

			block.back().t = glm::vec3(i, 0.0f, j);
			block.back().s.y = 0.1f;

			glm::mat4 m = glm::mat4(1.0f);

			m = glm::translate(m, block.back().t);
			m = glm::scale(m, block.back().s);

			block.back().modelMat = m;


		}
	}




}
void Update() {
	vector<GLfloat> vertexData;
	vector<GLuint> indices;

	for (size_t i = 0; i < block.size(); ++i) {
		block[i].Update(model[0]);
		vertexData.insert(vertexData.end(), block[i].vertexData.begin(), block[i].vertexData.end());

		// 인덱스 오프셋 보정 필요 (여러 모델 합칠 경우)
		GLuint offset = i == 0 ? 0 : (GLuint)(vertexData.size() / 3 - block[i].vertexData.size() / 3);
		for (auto idx : block[i].indices) indices.push_back(idx + offset);
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);
}
int main(int argc, char** argv) {
	model.push_back(read_obj_file("cube.obj"));
	InitData();

	srand(static_cast<unsigned>(time(0))); // 랜덤 시드 초기화
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // GLUT_DEPTH 추가
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("tung tung tung tung tung tung tung tung tung sours");

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cerr << "GLEW 초기화 실패" << std::endl;
		return -1;
	}


	MakeVertexShaders();
	MakeFragmentShaders();
	shaderProgramID = MakeShaderProgram();
	if (shaderProgramID == 0) {
		std::cerr << "셰이더 프로그램 생성 실패" << std::endl;
		return -1;
	}
	

	InitBuffers();
	glutTimerFunc(10, TimerFunction, 1);
	glutDisplayFunc(DrawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(Keyupboard);
	glutSpecialFunc(SpecialKeyboard); // 특수 키(화살표) 함수 등록

	glutMainLoop();
	return 0;
}



void MakeVertexShaders() {
	GLchar* vertexSource = filetobuf("vertex.glsl");
	if (!vertexSource) {
		std::cerr << "ERROR: vertex.glsl." << std::endl;
		return;
	}

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader\n" << errorLog << std::endl;
	}
	free(vertexSource);
}

void MakeFragmentShaders() {
	GLchar* fragmentSource = filetobuf("fragment.glsl");
	if (!fragmentSource) {
		std::cerr << "ERROR: fragment.glsl." << std::endl;
		return;
	}

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader\n" << errorLog << std::endl;
	}
	free(fragmentSource);
}

GLuint MakeShaderProgram() {
	GLuint shaderID = glCreateProgram();
	glAttachShader(shaderID, vertexShader);
	glAttachShader(shaderID, fragmentShader);
	glLinkProgram(shaderID);

	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program\n" << errorLog << std::endl;
		return 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glUseProgram(shaderID);
	return shaderID;
}

void InitBuffers() {
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);



	glBindVertexArray(0);
}

void DrawScene() {


	glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	//if(silver)
	//	glEnable(GL_CULL_FACE);// 은면 제거 활성화
	//else
	//	glDisable(GL_CULL_FACE);


	


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 깊이 버퍼 클리어 추가

	glUseProgram(shaderProgramID);
	glBindVertexArray(VAO);

	Update();



	// Uniform 매트릭스 매핑
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");
	GLint faceColorLoc = glGetUniformLocation(shaderProgramID, "faceColor");





	glViewport(0, 0, width, height);
	{
		// Camera (View) 및 Projection 매트릭스 설정
		glm::mat4 view = glm::lookAt(cameraPos, camera_move, glm::vec3(0.0f, 1.0f, 0.0f)); // 뷰 매트릭스
		glm::mat4 proj;

		if (op) {
			float aspect = (float)width / (float)height;
			float size = 5.0f;
			proj = glm::ortho(-size * aspect, size * aspect, -size, size, 0.1f, 100.0f);
		}
		else {
			proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
		}


		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

		GLuint offset = 0;
		for (int i = 0; i < block.size(); ++i) {
			for (int j = 0; j < block[i].vertexData.size() / 9; ++j) { // colors 개수 == face 개수
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(block[i].modelMat));
				glUniform3f(faceColorLoc, block[i].colors[0], block[i].colors[1], block[i].colors[2]);
				glDrawArrays(GL_TRIANGLES, offset, 3);
				offset += 3;
			}
		}

	}
	glViewport(width / 2 + width / 4, height / 2 + width / 4, width / 4, height / 4);
	{
		float maxrange = max(block_width, block_height) / 2.0f + 2.0f; 

	
		glm::mat4 view = glm::lookAt(
			glm::vec3(0.0f, 30.0f, 0.0f),    
			glm::vec3(0.0f, 0.0f, 0.0f),     
			glm::vec3(0.0f, 0.0f, -1.0f)     
		);

		// 직교 투영을 블록 범위에 맞게 조정
		glm::mat4 proj = glm::ortho(-maxrange, maxrange,-maxrange, maxrange,0.1f, 50.0f);

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

		GLuint offset = 0;
		for (int i = 0; i < block.size(); ++i) {
			for (int j = 0; j < block[i].vertexData.size() / 9; ++j) {
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(block[i].modelMat));
				glUniform3f(faceColorLoc, block[i].colors[0], block[i].colors[1], block[i].colors[2]);
				glDrawArrays(GL_TRIANGLES, offset, 3);
				offset += 3;
			}
		}
	}


	



	glBindVertexArray(0);
	glutSwapBuffers();
}


void Reshape(int w, int h) {
	glViewport(0, 0, w, h);
	width = w;
	height = h;
}

void Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		left_mousex =x, left_mousey = y;
		left_mouse = true;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		left_mouse = false;
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		right_mousex = x, right_mousey = y;
		right_mouse = true;
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		right_mouse = false;
	}

	glutPostRedisplay();

}
void Motion(int x, int y)
{
	if (left_mouse) {
		int deltax = x - left_mousex;
		int deltay = y - left_mousey;

		

		if (deltax != 0) {
			float angle = deltax * sense;
			camera_x_angle += angle;
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
			cameraPos = glm::vec3(rotation * glm::vec4(cameraPos, 1.0f));
		}

		if (deltay != 0&& !camera_x_lock) {
			float angle = -deltay * sense;
			camera_y_angle += angle;
			glm::vec3 c = cameraPos - camera_move;
			glm::vec3 v = glm::normalize(glm::cross(c, glm::vec3(0.0f, 1.0f, 0.0f)));
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), v);
			cameraPos = glm::vec3(rotation * glm::vec4(cameraPos, 1.0f));
		}

		left_mousex = x;
		left_mousey = y;



		glutPostRedisplay();
	}
	if (right_mouse) {
		int deltax = x - right_mousex;
		int deltay = y - right_mousey;

		if (deltax != 0) {
			float angle = deltax * 0.05f;
			glm::vec3 right = glm::normalize(glm::cross(camera_move - cameraPos, glm::vec3(0.0f, 1.0f, 0.0f)));
			camera_move += right * angle;
		}

		if (deltay != 0) {
			float moveAmount = -deltay * 0.05f;
			glm::vec3 forward = glm::normalize(camera_move - cameraPos);
			glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
			glm::vec3 up = glm::normalize(glm::cross(right, forward));
			camera_move += up * moveAmount;
		}


		right_mousex = x;
		right_mousey = y;
		glutPostRedisplay();
	}
}

void camera_y_rotate(bool b) {
	glm::mat4 m = glm::mat4(1.0f);
	
	if (b) {
		m = glm::rotate(m, glm::radians(5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	else {
		m = glm::rotate(m, glm::radians(-5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	
	cameraPos = glm::vec3(m * glm::vec4(cameraPos, 1.0f));
}
void camera_wasd(char i){
	switch (i) {
	case 'w':
	{
		glm::vec3 forward = glm::normalize(camera_move - cameraPos);
		cameraPos += forward * 0.1f;
		camera_move += forward * 0.1f;
		break;
	}
	}
}
void Keyboard(unsigned char key, int x, int y)
{
	if(start)
	switch (key)
	{
	case 'o':
	op = true;
	break;
	case 'p':
	op = false;
	break;
	case 'z':
	if (!op) {
		cameraPos.z -= 0.1;
		camera_move.z -= 0.1;
	}
	break;
	case 'Z':
	if (!op) {
		cameraPos.z += 0.1;
		camera_move.z += 0.1;
	}
	break;
	case 'm':
	m = true;
	break;
	case 'M':
	m = false;
	break;
	case 'y':
	camera_y_rotate(true);
	break;
	case 'Y':
	camera_y_rotate(false);
	break;
	case 'r':

	break;
	case 'v':
	v = !v;
	if (!v)m = true;
	else m = false;
	break;
	case 's':

	break;
	case '+':
	for (int i = 0;i < block.size();++i) {
		block[i].speed += 0.01f;
	}
	break;
	case '-':
	for (int i = 0;i < block.size();++i) {
		if(block[i].speed> 0.01f)
		block[i].speed -= 0.01f;
	}
	break;
	case '1':

	break;
	case '3':

	break;
	case 'c':
	system("cls");
	InitData();
	start = false;
	break;
	case 'q':
	exit(0);
	break;



	//추가구현들
	case '8':
		cameraPos.z -= 0.2;
		camera_move.z -= 0.2;
		break;
	case '5':
		cameraPos.z += 0.2;
		camera_move.z += 0.2;
		break;
	case '4':
		cameraPos.x -= 0.2;
		camera_move.x -= 0.2;
		break;
	case '6':
		cameraPos.x += 0.2;
		camera_move.x += 0.2;
		break;
	case 't':
		camera_x_lock = true;
		break;

	}

	glutPostRedisplay();
}

// 특수 키(화살표) 처리 함수: 도형 회전
void SpecialKeyboard(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		for (int i = 0; i < shape.size(); ++i) {
			shape[i].t.z -= 0.1f;
		}
		break;
	case GLUT_KEY_DOWN:
		for (int i = 0; i < shape.size(); ++i) {
			shape[i].t.z += 0.1f;
		}
		break;
	case GLUT_KEY_LEFT:
		for (int i = 0; i < shape.size(); ++i) {
			shape[i].t.x -= 0.1f;
		}
		break;
	case GLUT_KEY_RIGHT:
		for (int i = 0; i < shape.size(); ++i) {
			shape[i].t.x += 0.1f;
		}
		break;
	default:
		break;
	}

	glutPostRedisplay();  // 재렌더링 요청
}
void Keyupboard(unsigned char key, int x, int y) {
	switch (key) {
	case 't':
		camera_x_lock = false;;
		break;
	}
}

void Start_Wait() {
	for (int i = 0;i < block.size();++i) {
		if (block[i].size > 0) {
			block[i].s.y += block[i].speed * 0.5f;
			block[i].t.y += block[i].speed * 0.25f;	
			--block[i].size;
			if (block[i].size == 0)--block_start;
		}
	}
	if (block_start == 0)start = true;
}
void U_D_animation() {
	for (int i = 0;i < block.size();++i) {
		int a = (r_m(gen) == 0 ? 1 : -1);
		if (block[i].t.y < 3.0f)a = 1;
		block[i].s.y += block[i].speed * 0.5f* a;
		block[i].t.y += block[i].speed * 0.25f* a;
		block[i].colors = { (block[i].colors.x + 0.01f) >= 1.0f ? 0.0f : (block[i].colors.x + 0.01f)
				,(block[i].colors.y + 0.01f) >= 1.0f ? 0.0f : (block[i].colors.y + 0.01f)
				,(block[i].colors.z + 0.01f) >= 1.0f ? 0.0f : (block[i].colors.z + 0.01f) };

	}
}
void D_animation() {
	for (int i = 0;i < block.size();++i) {
		if (block[i].t.y > 1.0f) {
			block[i].s.y -= block[i].speed * 0.1f;
			block[i].t.y -= block[i].speed * 0.05f;
		}

	}
}
void TimerFunction(int value)
{
	camera_move -= glm::vec3(ad_ * 0.5f, ws_ * 0.5f, pn_ * 0.5f);
	
	for (int i = 0;i < block.size();++i) {
		glm::mat4 m = glm::mat4(1.0f);

		m = glm::rotate(m, glm::radians(block[i].angle), glm::vec3(0.0f, 1.0f, 0.0f));
		m = glm::translate(m, block[i].t);
		m = glm::scale(m, block[i].s);
		block[i].modelMat = m;
	}


	ad_ = ws_ = pn_ = 0;




	if(!start)Start_Wait();
	else {
		if (v)D_animation();
		else if (m)U_D_animation();
	}
	glutTimerFunc(10, TimerFunction, 1);
	glutPostRedisplay();
}

GLchar ch[256]{};
GLchar* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading
	if (!fptr) // Return NULL on failure
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	length = ftell(fptr); // Find out how many bytes into the file we are
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer
	fclose(fptr); // Close the file
	buf[length] = 0; // Null terminator
	return buf; // Return the buffer
}
//버텍스 세이더 객체 만들기
void make_vertexShaders()
{
	GLchar* vertexSource;

	//버텍스 세이더 읽어 저장하고 컴파일 하기
	//filetobuf : 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
	vertexSource = filetobuf("vertex.glsl");
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader \n" << errorLog << std::endl;
		return;
	}
}

//프래그먼트 세이더 객체 만들기
void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//프래그먼트 세이더 읽어 저장하고 컴파일하기
	fragmentSource = filetobuf("fragment.glsl"); // 프래그세이더 읽어오기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader \n" << errorLog << std::endl;
		return;
	}
}

//세이더 프로그램 만들고 세이더 객체 링크하기
GLuint make_shaderProgram()
{
	GLuint shaderID;
	shaderID = glCreateProgram(); //세이더 프로그램 만들기
	glAttachShader(shaderID, vertexShader); //세이더 프로그램에 버텍스 세이더 붙이기
	glAttachShader(shaderID, fragmentShader); //세이더 프로그램에 프래그먼트 세이더 붙이기
	glLinkProgram(shaderID); //세이더 프로그램 링크하기
	glDeleteShader(vertexShader); //세이더 객체를 세이더 프로그램에 링크했음으로 세이더 객체 자체는 삭제 가능
	glDeleteShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);// 세이더가 잘 연결되었는지 체크하기
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program  \n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(shaderID); //만들어진 세이더 프로그램 사용하기
	//여러 개의 세이더프로그램 만들 수 있고,그 중 한개의 프로그램을 사용하려면
	//glUseProgram 함수를 호출하여 사용 할 특정 프로그램을 지정한다
	//사용하기 직전에 호출할 수 있다
	return shaderID;
}


