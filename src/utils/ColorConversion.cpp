#include "utils/ColorConversion.h"

namespace utils::color
{
    float hue2rgb(float f1, float f2, float hue) {
        if (hue < 0.f)
            hue += 1.f;
        else if (hue > 1.f)
            hue -= 1.f;
        float res;
        if ((6.f * hue) < 1.f)
            res = f1 + (f2 - f1) * 6.f * hue;
        else if ((2.f * hue) < 1.f)
            res = f2;
        else if ((3.f * hue) < 2.f)
            res = f1 + (f2 - f1) * ((2.f / 3.f) - hue) * 6.f;
        else
            res = f1;
        return res;
    }

    glm::vec3 hsl2rgb(glm::vec3 hsl) {
        glm::vec3 rgb;

        if (hsl.y == 0.f) {
            rgb = glm::vec3(hsl.z); // Luminance
        }
        else {
            float f2;

            if (hsl.z < 0.5f)
                f2 = hsl.z * (1.f + hsl.y);
            else
                f2 = hsl.z + hsl.y - hsl.y * hsl.z;

            float f1 = 2.f * hsl.z - f2;

            rgb.x = hue2rgb(f1, f2, hsl.x + (1.f / 3.f));
            rgb.y = hue2rgb(f1, f2, hsl.x);
            rgb.z = hue2rgb(f1, f2, hsl.x - (1.f / 3.f));
        }
        return rgb;
    }

    glm::vec3 rgb2hsl(glm::vec3 c)
    {
        using namespace glm;
        glm::vec3 hsl; // init to 0 to avoid warnings ? (and reverse if + remove first part)

        float fmin = min(min(c.x, c.y), c.z); //Min. value of RGB
        float fmax = max(max(c.x, c.y), c.z); //Max. value of RGB
        float delta = fmax - fmin; //Delta RGB value

        hsl.z = (fmax + fmin) / 2.f; // Luminance

        if (delta == 0.f) //This is a gray, no chroma...
        {
            hsl.x = 0.f; // Hue
            hsl.y = 0.f; // Saturation
        }
        else //Chromatic data...
        {
            if (hsl.z < 0.5f)
                hsl.y = delta / (fmax + fmin); // Saturation
            else
                hsl.y = delta / (2.f - fmax - fmin); // Saturation

            if (c.x == fmax)
                hsl.x = (c.y - c.z) / (6.f * delta); // Hue
            else if (c.y == fmax)
                hsl.x = (1.f / 3.f) + (c.z - c.x) / (6.f * delta); // Hue
            else if (c.z == fmax)
                hsl.x = (2.f / 3.f) + (c.x - c.y) / (6.f * delta); // Hue

            if (hsl.x < 0.f)
                hsl.x += 1.f; // Hue
            else if (hsl.x > 1.f)
                hsl.x -= 1.f; // Hue
        }

        return hsl;
    }

    glm::vec3 hsv2rgb(glm::vec3 c_hsv)
    {
        using namespace glm;
        vec3 K = vec3(1.f, 2.f / 3.f, 1.f / 3.f);
        vec3 p = abs(fract(vec3(c_hsv.x) + K) * 6.f - vec3(3.f));
        return c_hsv.z * mix(vec3(1.f), clamp(p - vec3(1.f), 0.f, 1.f), c_hsv.y);
    }

    glm::vec3 rgb2hsv(glm::vec3 c)
    {
        using namespace glm;
        vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        vec4 p = mix(vec4(c.z, c.y, K.w, K.z), vec4(c.y, c.z, K.x, K.y), step(c.z, c.y));
        vec4 q = mix(vec4(p.x, p.y, p.w, c.x), vec4(c.x, p.y, p.z, p.x), step(p.x, c.x));

        float d = q.x - min(q.w, q.y);
        float e = 1.e-10f;
        return vec3(abs(q.z + (q.w - q.y) / (6.f * d + e)), d / (q.x + e), q.x);
    }
}
