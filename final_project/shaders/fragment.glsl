#version 330

const float LINEAR_TERM = 0.0005;
const float QUADRATIC_TERM = 0.00009;
const float MIN_BRIGHTNESS = 0.16;

uniform sampler2D texture0;
uniform vec2 lightPosition;
uniform vec2 lightDirection;
uniform float lightAngle;
uniform float lightRange;
uniform float isCharging;

const int MAX_MAP_LANTERNS = 8;

uniform int lanternCount;
uniform vec2 lanternPositions[MAX_MAP_LANTERNS];

in vec2 fragTexCoord;
in vec2 fragPosition;

out vec4 finalColor;

float attenuate(float distance, float range)
{
    float brightness = 1.0 - smoothstep(0.0, range, distance);
    return max(brightness, MIN_BRIGHTNESS);
}

void main()
{
    vec4 texColor = texture(texture0, fragTexCoord);

    float dist = distance(lightPosition, fragPosition);
    float brightness = attenuate(dist, lightRange);

    if (lightAngle < 360.0)
    {
        vec2 pixelDirection = normalize(fragPosition - lightPosition);
        vec2 facingDirection = normalize(lightDirection);

        float angleCos = dot(pixelDirection, facingDirection);
        float cutoff = cos(radians(lightAngle * 0.5));

        float cone = smoothstep(cutoff, cutoff + 0.08, angleCos);
        brightness *= cone;
    }

    float lanternBrightness = 0.0;

    for (int i = 0; i < MAX_MAP_LANTERNS; i++)
    {
        if (i >= lanternCount) break;

        float lanternDistance = distance(lanternPositions[i], fragPosition);
        float lanternLight = 1.0 - smoothstep(0.0, 190.0, lanternDistance);

        lanternBrightness = max(lanternBrightness, lanternLight * 0.55);
    }

    brightness = max(brightness, lanternBrightness);
    brightness = max(brightness, MIN_BRIGHTNESS);

    finalColor = vec4(texColor.rgb * brightness, texColor.a);
}