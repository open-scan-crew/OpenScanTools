vec3 hsl2rgb(vec3 hsl)
{
    float f2;

    if (hsl.z < 0.5)
        f2 = hsl.z * (1.0 + hsl.y);
    else
        f2 = hsl.y + hsl.z * (1.f - hsl.y);

    float f1 = 2.0 * hsl.z - f2;

    float H = 6.f * hsl.x;
    float r = H - floor(H);
    int q = int(floor(H)) % 6;
    float a = f1 + (f2 - f1) * r;
    float b = f1 + (f2 - f1) * (1.f - r);

    switch (q)
    {
    case -6:
    case 0:
        return vec3(f2, a, f1);
    case -5:
    case 1:
        return vec3(b, f2, f1);
    case -4:
    case 2:
        return vec3(f1, f2, a);
    case -3:
    case 3:
        return vec3(f1, b, f2);
    case -2:
    case 4:
        return vec3(a, f1, f2);
    case -1:
    case 5:
        return vec3(f2, f1, b);
    default:
        return vec3(0.f, 1.f, 0.f);
    }
}

float rgb2luminance(vec3 color)
{
    float fmin = min(min(color.r, color.g), color.b); //Min. value of RGB
    float fmax = max(max(color.r, color.g), color.b); //Max. value of RGB
    return (fmin + fmax) / 2.0;
}

vec3 rgb2hsl(vec3 color) 
{
    vec3 hsl; // init to 0 to avoid warnings ? (and reverse if + remove first part)

    float fmin = min(min(color.r, color.g), color.b); //Min. value of RGB
    float fmax = max(max(color.r, color.g), color.b); //Max. value of RGB
    float delta = fmax - fmin; //Delta RGB value

    hsl.z = (fmax + fmin) / 2.0; // Luminance

    if (delta == 0.0) //This is a gray, no chroma...
    {
        hsl.x = 0.0; // Hue
        hsl.y = 0.0; // Saturation
    } else //Chromatic data...
    {
        if (hsl.z < 0.5)
            hsl.y = delta / (fmax + fmin); // Saturation
        else
            hsl.y = delta / (2.0 - fmax - fmin); // Saturation

        if (color.r == fmax)
            hsl.x = (color.g - color.b) / (6.0 * delta); // Hue
        else if (color.g == fmax)
            hsl.x = (1.0 / 3.0) + (color.b - color.r) / (6.0 * delta); // Hue
        else if (color.b == fmax)
            hsl.x = (2.0 / 3.0) + (color.r - color.g) / (6.0 * delta); // Hue

        if (hsl.x < 0.0)
            hsl.x += 1.0; // Hue
        else if (hsl.x > 1.0)
            hsl.x -= 1.0; // Hue
    }

    return hsl;
 }

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
