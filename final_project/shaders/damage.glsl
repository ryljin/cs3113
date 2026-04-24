#version 330

uniform float damageAmount;

out vec4 finalColor;

void main()
{
    finalColor = vec4(1.0, 0.0, 0.0, damageAmount);
}