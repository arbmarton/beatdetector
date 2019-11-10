#version 330 core

uniform bool isRed;

out vec4 FragColor;

void main()
{
    if (isRed)
    {
        FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
    }
} 