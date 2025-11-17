#pragma once

#include <gl/glew.h>
#include <glm/glm.hpp>
#include <map>
#include <string> 

class Image
{
public:
    GLuint textureID;
    glm::vec2 position = glm::vec2(0.0f);
    glm::vec2 size = glm::vec2(100.0f, 100.0f);
    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 uvRect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

private:
    GLuint VAO = 0;
    GLuint VBO = 0;

public:
    Image(GLuint texID, const glm::vec2& pos, const glm::vec2& s);
    ~Image();

    void InitRenderData();
    void Draw(GLuint shaderProgramID, const glm::mat4& projection);

private:
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
};