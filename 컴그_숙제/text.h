#pragma once
#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <ft2build.h>
#include FT_FREETYPE_H
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>
#include "dtd.h" 
#include <gl/glu.h>

class Text
{
private:
    FT_Library ft_library; 
    FT_Face ft_face;       
public:
    struct Character {
        GLuint TextureID;
        glm::ivec2 Size;
        glm::ivec2 Bearing;
        GLuint Advance;
    };

    std::map<char, Character> Characters;
    GLuint VAO, VBO;
    GLuint shader;
    glm::mat4 projection;

public:
    Text();

    void Init(const char* fontPath, GLuint shaderProgram, glm::mat4 proj);
    void Draw(const std::string& text, float x, float y, float scale, glm::vec3 color);
private:
    void LoadGlyph(FT_ULong char_code);
};
GLuint LoadShader(const char* vertexPath, const char* fragmentPath);