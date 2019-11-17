#include "SpectrogramBar.h"

#include "Shader.h"
#include "Utilities.h"

Shader* SpectrogramBar::shader = nullptr;

SpectrogramBar::SpectrogramBar(const float xPos, const float yPos, const float widthArg, const float heightArg)
    : xCoordBottomLeft(xPos)
    , yCoordBottomLeft(yPos)
    , width(widthArg)
    , height(heightArg)
{
    if (!shader)
    {
        shader = new Shader(getShaderPath("bar.vs"), getShaderPath("bar.fs"));
    }

    const float xNDC = xCoordBottomLeft * 2 - 1;
    const float yNDC = yCoordBottomLeft * 2 - 1;
    const float widthNDC = width * 2;
    const float heightNDC = height * 2;

    vertices = { xNDC + widthNDC,
                 yNDC + heightNDC,
                 0.0f,  // top right
                 xNDC + widthNDC,
                 yNDC,
                 0.0f,  // bottom right
                 xNDC,
                 yNDC,
                 0.0f,  // bottom left
                 xNDC,
                 yNDC + heightNDC,
                 0.0f };  // top left


    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(float), &indices[0], GL_STATIC_DRAW);  // TODO: turn the 6 into something dynamic

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
}

SpectrogramBar::SpectrogramBar(SpectrogramBar&& rhs) noexcept
    : xCoordBottomLeft(rhs.xCoordBottomLeft)
    , yCoordBottomLeft(rhs.yCoordBottomLeft)
    , width(rhs.width)
    , height(rhs.height)
    , VAO(rhs.VAO)
    , VBO(rhs.VBO)
    , EBO(rhs.EBO)
    , vertices(rhs.vertices)
{
    rhs.VAO = 0;
    rhs.VBO = 0;
    rhs.EBO = 0;
}

SpectrogramBar& SpectrogramBar::operator=(SpectrogramBar&& rhs) noexcept
{
    xCoordBottomLeft = rhs.xCoordBottomLeft;
    yCoordBottomLeft = rhs.yCoordBottomLeft;
    width = rhs.width;
    height = rhs.height;

    VAO = rhs.VAO;
    VBO = rhs.VBO;
    EBO = rhs.EBO;

    rhs.VAO = 0;
    rhs.VBO = 0;
    rhs.EBO = 0;

    vertices = rhs.vertices;

    return *this;
}

SpectrogramBar::~SpectrogramBar()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void SpectrogramBar::draw() const
{
    glUseProgram(shader->getID());

    shader->setBool("isRed", height > 0.5f);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, GLsizei(vertices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}