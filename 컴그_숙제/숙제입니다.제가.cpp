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
#define ROBOT 7
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
void MouseWheel(int wheel, int direction, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void Keyupboard(unsigned char key, int x, int y);
void SpecialKeyboard(int key, int x, int y); // 특수 키(화살표) 처리 함수 선언
void LoadOBJ(const char* filename);
void InitBuffer();
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
void TimerFunction(int value);
void GenerateMaze();
void Initrobot();
void updaterobot();
void PassiveMotion(int x, int y);
GLchar* filetobuf(const char* file);
void SpecialUpKeyboard(int key, int x, int y);

// 전역 변수
GLint width = 1000, height = 1000;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;

random_device rd;
mt19937 gen(rd());
uniform_real_distribution<float> r_speed(0.1f, 0.2f);
uniform_int_distribution<int> r_size(50, 100);
uniform_int_distribution<int> r_m(0, 1);
uniform_real_distribution<float> r_color(0.0f, 1.0f);


/*
现在立刻动身, 谁也不受伤不然你们都死定了, 侦探游戏也玩完了回到现实
中不要忘记传达说Stellaron猎人送你们最后一程的事实，这是扫荡开始行
动员执行目标固定，立即通过处决协议，打破焦土作战执行，等了很久。我
们以前见过吧 我是史泰拉龙猎人山姆虽然很早就出现在你面前想告诉大家
事实，但是比预想的要多很多，尝试了十一次，但是都以失败告终。在这期
间，我不知不觉地与这个世界有着千丝万缕的联系，无法摆脱剧本的束缚。
正如埃利奥所说，我们在这梦想的土地上会收获难忘的收获。对于智娜来
说，没有像他和卡夫卡一样洞察人心的洞察力和像布莱德一样出色的特技。
我擅长的东西大多适用于不必可怜的反派, 所以我能用的手段也只有一个是
为了给你看我地全部像萤火虫一样做好死亡地准备跳入火海中生活希望在清
醒地现实中再次相见
*/


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
	vector<glm::vec3> normals;
} Model;
class Shape {
public:
	vector<GLfloat> vertexData;
	vector<GLfloat> normalData;
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
		for (size_t i = 0; i < model.normals.size(); ++i) {
			normalData.push_back(model.normals[i].x);
			normalData.push_back(model.normals[i].y);
			normalData.push_back(model.normals[i].z);
		}
		// 인덱스는 필요 없으므로 비워둡니다
		indices.clear();
	}
	void ColorRandom(Model model) {
		colors = { 0.0f,0.0f,0.0f };

		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		colors = glm::vec3(r, g, b);

	}

};
vector<Shape>shape;

class Block {
public:
	vector<GLfloat> vertexData;
	vector<GLfloat> normalData;
	vector<GLuint> indices;
	glm::vec3 t = { 0.0f,0.0f,0.0f };
	glm::vec3 s = { 1.0f,1.0f,1.0f };
	glm::vec3 r = { 0.0f,0.0f,0.0f };
	glm::vec3 colors;
	glm::mat4 modelMat = glm::mat4(1.0f);
	string name;
	bool line = false;
	int size = 0;
	float speed = 0.0f;
	float angle = 0.0f;
	bool start = false;
	bool end = false;
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
		for (size_t i = 0; i < model.normals.size(); ++i) {
			normalData.push_back(model.normals[i].x);
			normalData.push_back(model.normals[i].y);
			normalData.push_back(model.normals[i].z);
		}
		indices.clear();
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
vector<Shape>robot;

class RobotBounds {
public:
	glm::vec3 center = { 0.0f, 0.0f, 0.0f };
	glm::vec3 halfExtents = { 0.0f, 0.0f, 0.0f };
};
RobotBounds robotbb;

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
	std::vector<glm::vec3> temp_normals;
	std::vector<Face> temp_faces;

	char line[365];
	while (fgets(line, sizeof(line), file)) {
		read_newline(line);

		// v 라인 처리 (정점)
		if (line[0] == 'v' && line[1] == ' ') {
			Vertex v;
			if (sscanf_s(line + 2, "%f %f %f", &v.x, &v.y, &v.z) == 3) {
				temp_vertices.push_back(v);
			}
		}
		// vn 라인 처리 (노멀 벡터)
		else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ') {
			glm::vec3 normal;
			if (sscanf_s(line + 3, "%f %f %f", &normal.x, &normal.y, &normal.z) == 3) {
				temp_normals.push_back(normal);
			}
		}
		// f 라인 처리 - v//vn 형식 (f 1//2 7//2 5//2)
		else if (line[0] == 'f' && line[1] == ' ') {
			char face_str[365];
			if (strlen(line + 2) >= sizeof(face_str)) {
				continue;
			}
			strcpy_s(face_str, sizeof(face_str), line + 2);

			// strtok를 사용하여 공백으로 분리된 각 정점 데이터를 가져옵니다.
			char* token = strtok(face_str, " ");
			std::vector<unsigned int> face_indices; // 정점 인덱스
			std::vector<unsigned int> normal_indices; // 노멀 인덱스


			while (token != NULL) {
				unsigned int v_idx = 0, vn_idx = 0;


				if (strchr(token, '/') != NULL) {
					if (sscanf_s(token, "%u//%u", &v_idx, &vn_idx) == 2) {
						face_indices.push_back(v_idx - 1);
						normal_indices.push_back(vn_idx - 1);
					}
	
					else if (sscanf_s(token, "%u/%*u/%u", &v_idx, &vn_idx) == 2) {
						face_indices.push_back(v_idx - 1);
						normal_indices.push_back(vn_idx - 1);
					}
		
					else if (sscanf_s(token, "%u", &v_idx) == 1) {
						face_indices.push_back(v_idx - 1);
						normal_indices.push_back(0); // 기본 노멀
					}
				}
				else {
		
					if (sscanf_s(token, "%u", &v_idx) == 1) {
						face_indices.push_back(v_idx - 1);
						normal_indices.push_back(0); // 기본 노멀
					}
				}
				token = strtok(NULL, " ");
			}

	
			if (face_indices.size() >= 3) {
		
				Face f;
				f.v1 = face_indices[0];
				f.v2 = face_indices[1];
				f.v3 = face_indices[2];
				temp_faces.push_back(f);

				if (normal_indices.size() >= 3) {
					model.normals.push_back(temp_normals[normal_indices[0]]);
					model.normals.push_back(temp_normals[normal_indices[1]]);
					model.normals.push_back(temp_normals[normal_indices[2]]);
				}
			}
		}
	}
	fclose(file);

	model.vertex_count = temp_vertices.size();
	model.face_count = temp_faces.size();

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

bool CheckCollision(const glm::vec3& robotPos, const glm::vec3& robotHalf, const Block& wall) {
	glm::vec3 wallCenter = wall.t;
	glm::vec3 wallHalf = wall.s * 0.5f;

	float distX = std::abs(robotPos.x - wallCenter.x);
	bool collisionX = distX < (robotHalf.x + wallHalf.x);

	float distY = std::abs(robotPos.y - wallCenter.y);
	bool collisionY = distY < (robotHalf.y + wallHalf.y);

	float distZ = std::abs(robotPos.z - wallCenter.z);
	bool collisionZ = distZ < (robotHalf.z + wallHalf.z);

	return collisionX && collisionY && collisionZ;
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

//은면제거 변수
bool silver = false;


//명령어 라스트 커멘드
bool op = false;
bool m = false;
bool v = false;

//미로 변수
bool maze = false;

//빛 색깔
glm::vec3 light_color(1.0f);

//객체 소환
bool recall = false;


//시점
int view = 0; // 0 전체 1 캐릭터 1인칭 2 캐릭터 2인칭 3 캐릭터 3인칭

//시작 위치
glm::vec3 start_pos = glm::vec3(0.0f, 0.0f, 0.0f);

//로봇 변수들
float speed = 0.01f;

//시점 전환 됐을 때 마우스로 다 보기 가능
bool view_change = false;


//마우스 고정 잠시 풀기
bool g = false;


//캐릭터가 위치한 발판은 검정색이 되게 ㄱㄱ


//키 눌렸냐 안눌렸냐
int key_ = 0;





GLfloat tranformx(int x) {
	return ((float)x / (width / 2)) - 1.0f;
}
GLfloat tranformy(int y) {
	return ((height - (float)y) / (height / 2)) - 1.0f;
}


//커비 zjql
void InitData() {
	view = 0;
	camera_x_angle = 0.0f;
	camera_y_angle = 0.0f;
	maze = false;
	start = false;
	recall = false;
	light_color = glm::vec3(1.0f);
	robot.clear();
	cameraPos = glm::vec3(0.0f, 20.0f, 40.0f);
	modelPos = glm::vec3(0.0f, 0.0f, 0.0f);  // 도형 위치 (X, Y, Z 이동량)
	camera_move = glm::vec3(0.0f, 0.0f, 0.0f);
	//block_width = 0;
	//block_height = 0;
	//나중에 지워주센
	block_width = 5;
	block_height = 5;
	while (block_width < 5 || block_height < 5 || block_width > 25 || block_height > 25) {
		cout << "가로 길이와 세로 길이를 입력해주세요." << endl;
		cout << "* 길이 제한 : 5 ~ 25 *" << endl;
		cout << "입력 : ";cin >> block_width >> block_height;

		if (cin.fail()) {
			cin.clear();
			cin.ignore(10000, '\n');
		}

	}
	block_start = block_width * block_height;
	system("chcp 65001");
	fstream f{ "commend.txt" };
	string s;
	while (getline(f, s))cout << s << endl;


	block.clear();
	for (float i = -(float)block_width / 2; i < (float)block_width / 2; i += 1.0f) { 
		for (float j = -(float)block_height / 2; j < (float)block_height / 2; j += 1.0f) { 
			block.push_back(Block(model[0], r_size(gen), r_speed(gen)));


			int x_idx = (int)(i + (float)block_width / 2);
			int z_idx = (int)(j + (float)block_height / 2);


			float f = (float)(x_idx + z_idx) / (float)(block_width + block_height);
			block.back().colors = { f ,1.0f - f,1.0f - f };

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
	vector<GLfloat> combinateData;
	for (size_t i = 0;i < block.size();++i) {
		for (size_t j = 0;j < block[i].vertexData.size() / 3;++j) {
			combinateData.push_back(block[i].vertexData[j * 3 + 0]);
			combinateData.push_back(block[i].vertexData[j * 3 + 1]);
			combinateData.push_back(block[i].vertexData[j * 3 + 2]);
			combinateData.push_back(block[i].normalData[j * 3 + 0]);
			combinateData.push_back(block[i].normalData[j * 3 + 1]);
			combinateData.push_back(block[i].normalData[j * 3 + 2]);

		}
	}
	if (!robot.empty()) {
		for (size_t i = 0;i < robot.size();++i) {
			for (size_t j = 0;j < robot[i].vertexData.size() / 3;++j) {
				combinateData.push_back(robot[i].vertexData[j * 3 + 0]);
				combinateData.push_back(robot[i].vertexData[j * 3 + 1]);
				combinateData.push_back(robot[i].vertexData[j * 3 + 2]);
				combinateData.push_back(robot[i].normalData[j * 3 + 0]);
				combinateData.push_back(robot[i].normalData[j * 3 + 1]);
				combinateData.push_back(robot[i].normalData[j * 3 + 2]);
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, combinateData.size() * sizeof(GLfloat), combinateData.data(), GL_DYNAMIC_DRAW);
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
	glutMouseWheelFunc(MouseWheel);
	glutSpecialFunc(SpecialKeyboard); // 특수 키(화살표) 함수 등록
	glutPassiveMotionFunc(PassiveMotion);
	glutSpecialUpFunc(SpecialUpKeyboard);

	glutMainLoop();
	return 0;
}




void DrawScene() {


	glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	if(silver)
		glEnable(GL_CULL_FACE);// 은면 제거 활성화
	else
		glDisable(GL_CULL_FACE);


	


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 깊이 버퍼 클리어 추가

	glUseProgram(shaderProgramID);
	glBindVertexArray(VAO);

	Update();



	// Uniform 매트릭스 매핑
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");
	GLint faceColorLoc = glGetUniformLocation(shaderProgramID, "faceColor");
	GLint modelNormalLocation = glGetUniformLocation(shaderProgramID, "modelNormal"); //--- modelNormal 값 전달: 모델 매트릭스의 역전치 행렬





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
			for (int j = 0; j < block[i].vertexData.size() / 9; ++j) {
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(block[i].modelMat));
				glUniformMatrix3fv(modelNormalLocation, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(block[i].modelMat)))));
				glUniform3f(faceColorLoc, block[i].colors[0], block[i].colors[1], block[i].colors[2]);
				glDrawArrays(GL_TRIANGLES, offset, 3);
				offset += 3;

			}
		}
		if (!robot.empty()) {
			for (int i = 0; i < robot.size(); ++i) {
				for (int j = 0; j < robot[i].vertexData.size() / 9; ++j) {
					glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(robot[i].modelMat));
					glUniformMatrix3fv(modelNormalLocation, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(robot[i].modelMat)))));
					glUniform3f(faceColorLoc, robot[i].colors[0], robot[i].colors[1], robot[i].colors[2]);
					glDrawArrays(GL_TRIANGLES, offset, 3);
					offset += 3;
				}
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
				glUniformMatrix3fv(modelNormalLocation, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(block[i].modelMat)))));
				glUniform3f(faceColorLoc, block[i].colors[0], block[i].colors[1], block[i].colors[2]);
				glDrawArrays(GL_TRIANGLES, offset, 3);
				offset += 3;
			}
				
		}
		if (!robot.empty()) {
			for (int i = 0; i < robot.size(); ++i) {
				for (int j = 0; j < robot[i].vertexData.size() / 9; ++j) {
					glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(robot[i].modelMat));
					glUniformMatrix3fv(modelNormalLocation, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(robot[i].modelMat)))));
					glUniform3f(faceColorLoc, robot[i].colors[0], robot[i].colors[1], robot[i].colors[2]);
					glDrawArrays(GL_TRIANGLES, offset, 3);
					offset += 3;
				}
			}
		}
	}



	GLint lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos"); //--- lightPos 값 전달: (0.0, 0.0, 5.0);
	GLint lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor"); //--- lightColor 값 전달: (1.0, 1.0, 1.0) 백색
	GLint viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos"); //--- viewPos 값 전달: 카메라 위치




	glm::vec3 lightPos = { 0.0f,200.0f,0.0f };
	//glm::mat4 lightModelMat = shape.back().modelMat;
	//lightPos = glm::vec3(lightModelMat * glm::vec4(lightPos, 1.0f));


	glUniform3f(lightPosLocation, 0, 20, 0);


	glUniform3f(lightColorLocation, light_color.x, light_color.y, light_color.z);

	glUniform3f(viewPosLocation, cameraPos.x, cameraPos.y, cameraPos.z);




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
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN&&(view ==0)) {
		left_mousex =x, left_mousey = y;
		left_mouse = true;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && (view == 0)) {
		left_mouse = false;
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && view == 0) {
		right_mousex = x, right_mousey = y;
		right_mouse = true;
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP && view == 0) {
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
void PassiveMotion(int x, int y) {
	if (view == 1 && !g) {
		int center_x = width / 2;
		int center_y = height / 2;
		int deltax = -x + left_mousex;
		int deltay = y - left_mousey;
		float custom_sense = sense * 2.0f;
		float old_y_angle = camera_y_angle;

		if (deltax != 0) {
			float angle = deltax * custom_sense;
			camera_x_angle += angle;

			if (camera_x_angle < -360.0f) camera_x_angle += 360.0f;
			if (camera_x_angle > 360.0f) camera_x_angle -= 360.0f;

			if (angle != 0.0f) {
				glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				camera_move = glm::vec3(rotation * glm::vec4(camera_move - cameraPos, 1.0f)) + cameraPos;
			}
		}

		if (deltay != 0 && !camera_x_lock) {
			float angle = -deltay * custom_sense;
			camera_y_angle += angle;

			if (camera_y_angle < -45.0f || camera_y_angle > 45.0f) {
				camera_y_angle = old_y_angle;
				angle = 0.0f;
			}

			if (angle != 0.0f) {

				glm::vec3 c = camera_move - cameraPos; 
				glm::vec3 v = glm::normalize(glm::cross(c, glm::vec3(0.0f, 1.0f, 0.0f)));
				glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), v);


				camera_move = glm::vec3(rotation * glm::vec4(camera_move - cameraPos, 1.0f)) + cameraPos;
			}
		}

		if (x != center_x || y != center_y) {
			glutWarpPointer(center_x, center_y);
			left_mousex = center_x;
			left_mousey = center_y;
		}
		else {
			left_mousex = x;
			left_mousey = y;
		}
	}
	if (view == 3&& !g) {

		int center_x = width / 2;
		int center_y = height / 2;

		int deltax = x - left_mousex;
		int deltay = y - left_mousey;

		float custom_sense = sense * 2.0f;
		float old_y_angle = camera_y_angle;

		if (deltax != 0) {
			float angle = deltax * custom_sense;

			camera_x_angle += angle;

			if (camera_x_angle < -360.0f) camera_x_angle += 360.0f;
			if (camera_x_angle > 360.0f) camera_x_angle -= 360.0f;

			if (angle != 0.0f) {
				glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				cameraPos = glm::vec3(rotation * glm::vec4(cameraPos - camera_move, 1.0f)) + camera_move;
			}
		}

		if (deltay != 0 && !camera_x_lock) {
			float angle = -deltay * custom_sense;

			camera_y_angle += angle;

			if (camera_y_angle < -60.0f || camera_y_angle > 30.0f) {
				camera_y_angle = old_y_angle;
				angle = 0.0f;
			}

			if (angle != 0.0f) {
				glm::vec3 c = cameraPos - camera_move;
				glm::vec3 v = glm::normalize(glm::cross(c, glm::vec3(0.0f, 1.0f, 0.0f)));
				glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), v);
				cameraPos = glm::vec3(rotation * glm::vec4(cameraPos - camera_move, 1.0f)) + camera_move;
			}
		}

		if (x != center_x || y != center_y) {
			glutWarpPointer(center_x, center_y);

			left_mousex = center_x;
			left_mousey = center_y;
		}
		else {
			left_mousex = x;
			left_mousey = y;
		}

		glutPostRedisplay();
	}

}
void MouseWheel(int wheel, int direction, int x, int y)
{
	float zoomSpeed = 1.0f;

	glm::vec3 forward = glm::normalize(camera_move - cameraPos);

	if (direction > 0) {
		cameraPos += forward * zoomSpeed;
	}
	else {
		cameraPos -= forward * zoomSpeed;
	}

	glutPostRedisplay();
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
void camera_wasd(char i) {
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
	if (!op&& view==0) {
		glm::vec3 v = { 0.0f,0.0f,-0.1f };
		glm::mat4 m(1.0f);
		m = glm::rotate(m, glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
		v = glm::vec3(m * glm::vec4(v, 1.0f));
		cameraPos += v;
		camera_move += v;
	}
	break;
	case 'Z':
	if (!op && view == 0) {
		glm::vec3 v = { 0.0f,0.0f,0.1f };
		glm::mat4 m(1.0f);
		m = glm::rotate(m, glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
		v = glm::vec3(m * glm::vec4(v, 1.0f));
		cameraPos += v;
		camera_move += v;
	}
	break;
	case 'm':
	m = true;
	break;
	case 'M':
	m = false;
	break;
	case 'y':
	if(view == 0)
	camera_y_rotate(true);
	break;
	case 'Y':
		if (view == 0)
	camera_y_rotate(false);
	break;
	case 'r':
	if (!maze) {
		maze = true;
		GenerateMaze();
	}
	break;
	case 'v':
	v = !v;
	if (!v)m = true;
	else m = false;
	break;
	case 's':
		if (!recall&& maze) {
			recall = true;
			Initrobot();
		}
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
		if (recall) {
			view = 1;
			camera_x_angle = robot[2].angle;
			cameraPos = robot[0].t;
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec3 forward_offset = glm::vec3(rotation * glm::vec4(0.0f, 0.0f, 0.1f, 1.0f));
			camera_move = cameraPos + forward_offset;

			
			
		}
	break;
	case '3':
		if (recall) {
			view = 3;
			camera_x_angle = robot[2].angle;
			camera_y_angle = 0.0f;
			camera_move = robot[2].t;
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec3 forward_offset = glm::vec3(rotation * glm::vec4(0.0f, 0.4f, -0.3f, 1.0f));
			cameraPos = camera_move + forward_offset;
			
		}
	break;
	case 'c':
	system("cls");
	InitData();
	break;
	case 'q':
	exit(0);
	break;



	//추가구현들 
	//카메라 묶기 햇죠ㅕ용
	case '[':
	{
		static bool b = false;
		static glm::vec3 s_light_color;

		b = !b;
		if (b) {
			s_light_color = light_color;
			light_color = glm::vec3(0.0f, 0.0f, 0.0f);
		}
		else light_color = s_light_color;

	}
	break;
	case ']':
	{
		light_color = glm::vec3(r_color(gen), r_color(gen), r_color(gen));
	}
	break;
	case '8':
		if (view == 0) {
			glm::vec3 v = { 0.0f,0.0f,-0.2 };
			glm::mat4 m(1.0f);
			m = glm::rotate(m, glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
			v = glm::vec3(m * glm::vec4(v, 1.0f));
			cameraPos += v;
			camera_move += v;
		}
		break;
	case '5':
		if (view == 0) {
			glm::vec3 v = { 0.0f,0.0f,0.2 };
			glm::mat4 m(1.0f);
			m = glm::rotate(m, glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
			v = glm::vec3(m * glm::vec4(v, 1.0f));
			cameraPos += v;
			camera_move += v;
		}
		break;
	case '4':
		if (view == 0) {
			glm::vec3 v = { -0.2f,0.0f,0.0 };
			glm::mat4 m(1.0f);
			m = glm::rotate(m, glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
			v = glm::vec3(m * glm::vec4(v, 1.0f));
			cameraPos += v;
			camera_move += v;
		}
		break;
	case '6':
		if (view == 0) {
			glm::vec3 v = { 0.2f,0.0f,0.0 };
			glm::mat4 m(1.0f);
			m = glm::rotate(m, glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
			v = glm::vec3(m * glm::vec4(v, 1.0f));
			cameraPos += v;
			camera_move += v;
		}
		break;
	case 't':
		camera_x_lock = true;
		break;
	case '9':
		silver = !silver;
		break;

	case '2':
		if (recall) {
			view = 2;
			camera_x_angle = robot[2].angle;
			camera_move=glm::vec3(0.0f);
			cameraPos = glm::vec3(-10.0f, max(float(block_width), float(block_height))+40.0f, -10.0f);

		}
		break;
	case '0':	
		view = 0;
		view_change = false;
		break;
	case 'g':
		g = true;
		break;

	}

	glutPostRedisplay();
}

// 특수 키(화살표) 처리 함수: 도형 회전
void SpecialKeyboard(int key, int x, int y)
{
	if (recall && view != 0) {
		glm::vec3 old_robot_t[ROBOT];
		for (int i = 0; i < robot.size(); ++i) {
			old_robot_t[i] = robot[i].t;
		}

		glm::vec3 robotHalf = robotbb.halfExtents;

		switch (key)
		{
		case GLUT_KEY_UP:
			for (int i = 0; i < robot.size(); ++i) {
				glm::vec3 v = { 0.0f,0.0f,view != 2 ? 0.1f : -0.1f };
				glm::mat4 m(1.0f);
				m = glm::rotate(m, glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
				v = glm::vec3(m * glm::vec4(v, 1.0f));
				robot[i].t += v;

				if (view != 2) {
					robot[i].angle = camera_x_angle;
				}
			}
			key_ = 1;
			break;
		case GLUT_KEY_DOWN:
			for (int i = 0; i < robot.size(); ++i) {
				glm::vec3 v = { 0.0f,0.0f,view != 2 ? -0.1f : 0.1f };
				glm::mat4 m(1.0f);
				m = glm::rotate(m, glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
				v = glm::vec3(m * glm::vec4(v, 1.0f));
				robot[i].t += v;

				if (view != 2) {
					robot[i].angle = camera_x_angle + 180.0f;
				}
			}
			key_ = 2;
			break;
		case GLUT_KEY_LEFT:
			for (int i = 0; i < robot.size(); ++i) {
				glm::vec3 v = { view != 2 ? 0.1f : -0.1f ,0.0f,0.0f };
				glm::mat4 m(1.0f);
				m = glm::rotate(m, glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
				v = glm::vec3(m * glm::vec4(v, 1.0f));
				robot[i].t += v;

				if (view != 2) {
					robot[i].angle = camera_x_angle + 90.0f;
				}
			}
			key_ = 3;
			break;
		case GLUT_KEY_RIGHT:
			for (int i = 0; i < robot.size(); ++i) {
				glm::vec3 v = { view != 2 ? -0.1f : 0.1f,0.0f,0.0f };
				glm::mat4 m(1.0f);
				m = glm::rotate(m, glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
				v = glm::vec3(m * glm::vec4(v, 1.0f));
				robot[i].t += v;

				if (view != 2) {
					robot[i].angle = camera_x_angle - 90.0f;
				}
			}
			key_ = 4;
			break;
		default:
			break;
		}

		glm::vec3 newRobotCenter = robot[2].t;
		bool collided = false;
		for (auto& wall : block) {
			if (CheckCollision(newRobotCenter, robotHalf, wall)) {
				if (wall.start) {
					cout << wall.name << endl;
				}
				else if (wall.end) {
					cout << wall.name << endl;
					recall = false;
					view = 0;
					robot.clear();
					cameraPos = glm::vec3(0.0f, 20.0f, 40.0f);
					camera_move = glm::vec3(0.0f, 0.0f, 0.0f);
					camera_x_angle = 0.0f;
					camera_y_angle = 0.0f;
					break;
				}
				else if (wall.line) {
					cout << wall.name << endl;
					wall.colors = { 0.0f,0.0f,0.0f };
				}
				else {
					cout << wall.name << endl;
					collided = true;
				}
			}
			else if (!CheckCollision(robot[2].t, robotHalf, wall) && wall.line) {
				wall.colors = { 0.8f,0.8f,0.8f };

			}
		}

		if (collided) {
			for (int i = 0; i < robot.size(); ++i) {
				robot[i].t = old_robot_t[i];
			}
		}

		glutPostRedisplay();
	}
}
void Keyupboard(unsigned char key, int x, int y) {
	switch (key) {
	case 't':
		camera_x_lock = false;;
		break;
	case 'g':
		g = false;
		break;
	}
}
void SpecialUpKeyboard(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT:
	case GLUT_KEY_DOWN:
		key_ = false;
		break;
	default:
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
		if (!block[i].line) {
			int a = (r_m(gen) == 0 ? 1 : -1);
			if (block[i].t.y < 3.0f)a = 1;
			block[i].s.y += block[i].speed * 0.5f * a;
			block[i].t.y += block[i].speed * 0.25f * a;
			block[i].colors = { (block[i].colors.x + 0.01f) >= 1.0f ? 0.0f : (block[i].colors.x + 0.01f)
					,(block[i].colors.y + 0.01f) >= 1.0f ? 0.0f : (block[i].colors.y + 0.01f)
					,(block[i].colors.z + 0.01f) >= 1.0f ? 0.0f : (block[i].colors.z + 0.01f) };
		}
	}
}
void D_animation() {
	for (int i = 0;i < block.size();++i) {
		if (block[i].t.y > 1.0f&& !block[i].line) {
			block[i].s.y -= block[i].speed * 0.1f;
			block[i].t.y -= block[i].speed * 0.05f;
		}

	}
}
void camera_move_update() {
	if (robot.empty()) return;
	glm::vec3 new_robot = robot[2].t;
	glm::vec3 old_robot = camera_move;
	if (view == 1) {
		glm::vec3 robot_translation = robot[0].t - cameraPos;
		cameraPos = robot[0].t;
		camera_move += robot_translation;
	}

	else if (view == 3) {
		glm::vec3 robot_translation = new_robot - old_robot;
		cameraPos += robot_translation;
		camera_move = new_robot;
	}
}
void TimerFunction(int value)
{
	camera_move -= glm::vec3(ad_ * 0.5f, ws_ * 0.5f, pn_ * 0.5f);
	
	for (int i = 0;i < block.size();++i) {
		glm::mat4 m = glm::mat4(1.0f);

		m = glm::translate(m, block[i].t);
		m = glm::rotate(m, glm::radians(block[i].angle), glm::vec3(0.0f, 1.0f, 0.0f));
		m = glm::scale(m, block[i].s);
		block[i].modelMat = m;
	}
	if (recall)
	updaterobot();
	

	if (recall)
	camera_move_update();





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




	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); //--- 노말 속성
	glEnableVertexAttribArray(1);


	glBindVertexArray(0);
}

void GenerateMaze() {
	// 1. 미로 상태를 저장할 2D 그리드 초기화 (1: 벽, 0: 통로)
	vector<vector<int>> mazeGrid(block_width, vector<int>(block_height, 1));
	// DFS 방문 여부를 체크할 2D 그리드
	vector<vector<bool>> visited(block_width, vector<bool>(block_height, false));
	// DFS를 위한 스택
	stack<pair<int, int>> s;

	// 2. 시작 위치를 (1, 1)로 강제 (테두리 안쪽)
	// (시작 위치는 1, 3, 5... 같은 홀수여야 2칸씩 이동하는 로직과 잘 맞습니다)
	int startX = block_width / 2;
	int startZ = block_height / 2;

	if (startX % 2 == 0) startX--; 
	if (startZ % 2 == 0) startZ--;

	
	if (startX < 1) startX = 1;
	if (startZ < 1) startZ = 1;

	block[startX * block_height + startZ].start = true;

	visited[startX][startZ] = true;
	mazeGrid[startX][startZ] = 0; // 통로
	s.push({ startX, startZ });

	// 3. DFS (Recursive Backtracker) 수행
	while (!s.empty()) {
		auto [cx, cz] = s.top();

		// 2칸 떨어진 이웃 탐색 (상, 하, 좌, 우)
		vector<pair<int, int>> neighbors;
		int dx[] = { 0, 0, 2, -2 }; // X 이동
		int dz[] = { 2, -2, 0, 0 }; // Z 이동

		for (int i = 0; i < 4; ++i) {
			int nx = cx + dx[i];
			int nz = cz + dz[i];

		
			// 그리드 테두리(0, max)를 제외한 안쪽(1 ~ max-2)에서만 탐색
			if (nx > 0 && nx < block_width - 1 && nz > 0 && nz < block_height - 1 && !visited[nx][nz]) {
				neighbors.push_back({ nx, nz });
			}
	
		}

		if (!neighbors.empty()) {
			// 이웃 중 하나를 무작위로 선택
			uniform_int_distribution<int> r_neighbor(0, neighbors.size() - 1);
			auto [nx, nz] = neighbors[r_neighbor(gen)];

			// 방문 처리 및 통로 생성
			visited[nx][nz] = true;
			mazeGrid[nx][nz] = 0;
			// 현재 셀과 이웃 셀 사이의 벽도 통로로 만듦
			mazeGrid[cx + (nx - cx) / 2][cz + (nz - cz) / 2] = 0;

			s.push({ nx, nz }); // 새 이웃을 스택에 추가
		}
		else {
			// 막다른 길이므로 스택에서 제거 (백트래킹)
			s.pop();
		}
	}

	// 4. 가장자리에 단 하나의 출구 생성 (수정된 로직)
	//    테두리 벽 안쪽(1, max-2)이 통로(0)인 지점을 찾아, 그 바깥쪽 테두리를 뚫습니다.
	uniform_int_distribution<int> r_edge(0, 3);
	uniform_int_distribution<int> r_pos_w(1, block_width - 2);  // x축 랜덤 (1 ~ w-2)
	uniform_int_distribution<int> r_pos_h(1, block_height - 2); // z축 랜덤 (1 ~ h-2)

	bool exit_created = false;
	while (!exit_created) {
		int edge = r_edge(gen); // 0: 위, 1: 아래, 2: 오른쪽, 3: 왼쪽

		if (edge == 0) { // 위쪽 (Z = block_height - 1)
			int x = r_pos_w(gen);
			if (mazeGrid[x][block_height - 2] == 0) { // 안쪽이 통로라면
				mazeGrid[x][block_height - 1] = 0;    // 테두리를 뚫음
				block[x * block_height + (block_height - 1)].end = true;
				exit_created = true;
			}
		}
		else if (edge == 1) { // 아래쪽 (Z = 0)
			int x = r_pos_w(gen);
			if (mazeGrid[x][1] == 0) { // 안쪽이 통로라면
				mazeGrid[x][0] = 0;    // 테두리를 뚫음
				block[x * block_height + 0].end = true;
				exit_created = true;
			}
		}
		else if (edge == 2) { // 오른쪽 (X = block_width - 1)
			int z = r_pos_h(gen);
			if (mazeGrid[block_width - 2][z] == 0) { // 안쪽이 통로라면
				mazeGrid[block_width - 1][z] = 0;    // 테두리를 뚫음
				block[(block_width - 1) * block_height + z].end = true;
				exit_created = true;
			}
		}
		else { // 왼쪽 (X = 0)
			int z = r_pos_h(gen);
			if (mazeGrid[1][z] == 0) { // 안쪽이 통로라면
				mazeGrid[0][z] = 0;    // 테두리를 뚫음
				block[0 * block_height + z].end = true;
				exit_created = true;
			}
		}
	}

	// 5. 생성된 mazeGrid를 block 벡터에 적용 (기존과 동일)
	for (int x = 0; x < block_width; ++x) {
		for (int z = 0; z < block_height; ++z) {
			int index = x * block_height + z;

			if (mazeGrid[x][z] == 1) { // 벽
				block[index].name = "wall";
			}
			else { // 통로
				block[index].s.y = 0.1f; // 통로 높이
				block[index].t.y = 0.0f; // 통로 위치

				if (block[index].start) {
					block[index].colors = { 1.0f, 0.0f, 0.0f }; // 시작 색상
					start_pos = block[index].t;
					cout << block[index].t.x << ", " << block[index].t.z << endl;
					block[index].name = "start";
				}
				else if (block[index].end) {
					block[index].colors = { 0.0f, 1.0f, 0.0f }; // 출구 색상
					cout << block[index].t.x << ", " << block[index].t.z << endl;
					block[index].name = "exit";
				}
				else {
					block[index].colors = { 0.8f, 0.8f, 0.8f }; // 통로 색상
					block[index].name = "line";
					block[index].line = true;
				}
			}
		}
	}
}

void Initrobot() {
	robot.clear();
	float y = 0.25;
	//머리
	robot.push_back(Shape(model[0], 1));
	robot.back().t = { 0.0f,-0.05f,0.0f };
	robot.back().t += start_pos;
	robot.back().t.y += y;
	robot.back().s = { 0.03f,0.03f,0.03f };
	//robot.back().s *= 5;



	//코
	robot.push_back(Shape(model[0], 1));
	robot.back().t = { 0.0f,-0.05f,0.02f };
	robot.back().t += start_pos;
	robot.back().t.y += y;
	robot.back().s = { 0.01f,0.02f,0.01f };
	//robot.back().s *= 5;


	//몸통
	robot.push_back(Shape(model[0], 1));
	robot.back().t = { 0.0f,-0.1f,0.0f };
	robot.back().t += start_pos;
	robot.back().t.y += y;
	robot.back().s = { 0.06f,0.06f,0.06f };
	//robot.back().s *= 5;


	//왼팔
	robot.push_back(Shape(model[0], 1));
	robot.back().t = { -0.04f,-0.1f,0.0f };
	robot.back().t += start_pos;
	robot.back().t.y += y;
	robot.back().s = { 0.015f,0.05f,0.015f };
	//robot.back().s *= 5;


	//오른팔
	robot.push_back(Shape(model[0], 1));
	robot.back().t = { 0.04f,-0.1f,0.0f };
	robot.back().t += start_pos;
	robot.back().t.y += y;
	robot.back().s = { 0.015f,0.05f,0.015f };
	//robot.back().s *= 5;


	//왼다리
	robot.push_back(Shape(model[0], 1));
	robot.back().t = { -0.015f,-0.17f,0.0f };
	robot.back().t += start_pos;
	robot.back().t.y += y;
	robot.back().s = { 0.02f,0.07f,0.02f };
	//robot.back().s *= 5;


	//오른다리
	robot.push_back(Shape(model[0], 1));
	robot.back().t = { 0.015f,-0.17f,0.0f };
	robot.back().t += start_pos;
	robot.back().t.y += y;
	robot.back().s = { 0.02f,0.07f,0.02f };
	//robot.back().s *= 5;

	robotbb.halfExtents = { 0.055f, 0.14f, 0.03f };
}
void updaterobot() {
	if (robot.empty()) return;
	for (int i = 0;i < robot.size();++i) {
		glm::mat4 m = glm::mat4(1.0f);



		m = glm::translate(m, robot[2].t);
		m = glm::rotate(m, glm::radians(robot[i].angle), glm::vec3(0.0f, 1.0f, 0.0f));
		m = glm::translate(m, -robot[2].t);

		m = glm::translate(m, robot[i].t);
		m = glm::scale(m, robot[i].s);
		robot[i].modelMat = m;
	}

	static float walkPhase = 0.0f;
	float maxLegAngle = glm::clamp(600.0f * speed, 10.0f, 90.0f);
	float maxArmAngle = glm::clamp(400.0f * speed, 5.0f, 60.0f);

	if (key_) {
		walkPhase += speed * 120.0f;
		if (walkPhase > 360.0f) walkPhase -= 360.0f;

		float legAngle = maxLegAngle * sin(glm::radians(walkPhase));
		float armAngle = maxArmAngle * sin(glm::radians(walkPhase + 180.0f));

		glm::mat4 rotParent = glm::rotate(glm::mat4(1.0f), glm::radians(robot[2].angle), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 baseAxis = glm::vec3(1.0f, 0.0f, 0.0f);
		float sign = (key_ == 2) ? -1.0f : 1.0f;
		float amplitude = (key_ == 3 || key_ == 4) ? 0.6f : 1.0f;
		legAngle *= amplitude * sign;
		armAngle *= amplitude * sign;

		glm::vec3 legAxis = glm::normalize(glm::vec3(rotParent * glm::vec4(baseAxis, 0.0f)));
		glm::vec3 armAxis = legAxis;

		{
			glm::mat4 mleg = glm::mat4(1.0f);
			mleg = glm::translate(mleg, robot[2].t);
			mleg *= rotParent;
			mleg = glm::translate(mleg, robot[5].t - robot[2].t);
			mleg = glm::rotate(mleg, glm::radians(legAngle), legAxis);
			mleg = glm::scale(mleg, robot[5].s);
			robot[5].modelMat = mleg;
		}

		{
			glm::mat4 mleg = glm::mat4(1.0f);
			mleg = glm::translate(mleg, robot[2].t);
			mleg *= rotParent;
			mleg = glm::translate(mleg, robot[6].t - robot[2].t);
			mleg = glm::rotate(mleg, glm::radians(-legAngle), legAxis);
			mleg = glm::scale(mleg, robot[6].s);
			robot[6].modelMat = mleg;
		}

		{
			glm::mat4 marm = glm::mat4(1.0f);
			marm = glm::translate(marm, robot[2].t);
			marm *= rotParent;
			marm = glm::translate(marm, robot[3].t - robot[2].t);
			marm = glm::rotate(marm, glm::radians(armAngle), armAxis);
			marm = glm::scale(marm, robot[3].s);
			robot[3].modelMat = marm;
		}

		{
			glm::mat4 marm = glm::mat4(1.0f);
			marm = glm::translate(marm, robot[2].t);
			marm *= rotParent;
			marm = glm::translate(marm, robot[4].t - robot[2].t);
			marm = glm::rotate(marm, glm::radians(-armAngle), armAxis);
			marm = glm::scale(marm, robot[4].s);
			robot[4].modelMat = marm;
		}
	}
}

//switch (key)
//{
//case GLUT_KEY_UP:
//	for (int i = 0; i < robot.size(); ++i) {
//		glm::vec3 v = { 0.0f,0.0f,view != 2 ? 0.1f : -0.1f };
//		glm::mat4 m(1.0f);
//		m = glm::rotate(m, glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
//		v = glm::vec3(m * glm::vec4(v, 1.0f));
//		robot[i].t += v;
//
//		if (view != 2) {
//			robot[i].angle = camera_x_angle;
//		}
//	}
//	key_ = 1;
//	break;
//case GLUT_KEY_DOWN:
//	for (int i = 0; i < robot.size(); ++i) {
//		glm::vec3 v = { 0.0f,0.0f,view != 2 ? -0.1f : 0.1f };
//		glm::mat4 m(1.0f);
//		m = glm::rotate(m, glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
//		v = glm::vec3(m * glm::vec4(v, 1.0f));
//		robot[i].t += v;
//
//		if (view != 2) {
//			robot[i].angle = camera_x_angle + 180.0f;
//		}
//	}
//	key_ = 2;
//	break;
//case GLUT_KEY_LEFT:
//	for (int i = 0; i < robot.size(); ++i) {
//		glm::vec3 v = { view != 2 ? 0.1f : -0.1f ,0.0f,0.0f };
//		glm::mat4 m(1.0f);
//		m = glm::rotate(m, glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
//		v = glm::vec3(m * glm::vec4(v, 1.0f));
//		robot[i].t += v;
//
//		if (view != 2) {
//			robot[i].angle = camera_x_angle + 90.0f;
//		}
//	}
//	key_ = 3;
//	break;
//case GLUT_KEY_RIGHT:
//	for (int i = 0; i < robot.size(); ++i) {
//		glm::vec3 v = { view != 2 ? -0.1f : 0.1f,0.0f,0.0f };
//		glm::mat4 m(1.0f);
//		m = glm::rotate(m, glm::radians(camera_x_angle), glm::vec3(0.0f, 1.0f, 0.0f));
//		v = glm::vec3(m * glm::vec4(v, 1.0f));
//		robot[i].t += v;
//
//		if (view != 2) {
//			robot[i].angle = camera_x_angle - 90.0f;
//		}
//	}
//	key_ = 4;
//	break;
//default:
//	break;
//}