#include "Image.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

struct SpriteVertex {
    glm::vec2 position;
    glm::vec2 texCoord;
};

Image::Image(GLuint texID, const glm::vec2& pos, const glm::vec2& s)
    : textureID(texID), position(pos), size(s)
{
    InitRenderData();
}

Image::~Image()
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
}

void Image::InitRenderData()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SpriteVertex) * 6, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void*)(sizeof(glm::vec2)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Image::Draw(GLuint shaderProgramID, const glm::mat4& projection)
{
    glUseProgram(shaderProgramID);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform4fv(glGetUniformLocation(shaderProgramID, "spriteColor"), 1, &color[0]);

    float u1 = uvRect.x;
    float v1 = uvRect.y;
    float u2 = uvRect.x + uvRect.z;
    float v2 = uvRect.y + uvRect.w;

    SpriteVertex vertices[6] = {
        {{-0.5f, 0.5f},    {u1, v2}},
        {{ 0.5f, 0.5f},    {u2, v2}},
        {{ 0.5f, -0.5f},   {u2, v1}},

        {{ 0.5f, -0.5f},   {u2, v1}},
        {{-0.5f, -0.5f},   {u1, v1}},
        {{-0.5f, 0.5f},    {u1, v2}}
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(shaderProgramID, "imageTexture"), 0);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);
}