#ifndef COLOR32_HPP_
#define COLOR32_HPP_

#include <iostream>
#include <glm/glm.hpp>

class Color32
{
public:
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;

	const uint8_t Red() const
	{
		return (r);
	}

	const uint8_t Green() const
	{
		return (g);
	}

	const uint8_t Blue() const
	{
		return (b);
	}

	const uint8_t Alpha() const
	{
		return (a);
	}

	inline const float Red_f() const
	{
		return (r / 255.f);
	}

	inline const float Green_f() const
	{
		return (g / 255.f);
	}

	inline const float Blue_f() const
	{
		return (b / 255.f);
	}

	inline const float Alpha_f() const
	{
		return (a / 255.f);
	}

	Color32()
	{
		r = 0;
		g = 0;
		b = 0;
		a = 255;
	}

	Color32(uint8_t nr, uint8_t ng, uint8_t nb)
	{
		r = nr;
		g = ng;
		b = nb;
		a = 255;
	}
	
	Color32(uint8_t nr, uint8_t ng, uint8_t nb, uint8_t na)
	{
		r = nr;
		g = ng;
		b = nb;
		a = na;
	}

	Color32(const glm::vec3& rgb)
	{
		r = (uint8_t) (rgb.x * 255);
		g = (uint8_t) (rgb.y * 255);
		b = (uint8_t) (rgb.z * 255);
		a = (uint8_t) 255;
	}

	Color32(const glm::vec4& rgba)
	{
		r = (uint8_t) (rgba.x * 255);
		g = (uint8_t) (rgba.y * 255);
		b = (uint8_t) (rgba.z * 255);
		a = (uint8_t) (rgba.w * 255);
	}

	glm::vec3 toVector() const
	{
		return glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
	}

	std::string getStringHexa() const
	{
		std::string str = "";

		str += getHexaOnNbr((r / 16));
		str += getHexaOnNbr((r % 16));
		str += getHexaOnNbr((g / 16));
		str += getHexaOnNbr((g % 16));
		str += getHexaOnNbr((b / 16));
		str += getHexaOnNbr((b % 16));
		str += getHexaOnNbr((a / 16));
		str += getHexaOnNbr((a % 16));

		return (str);
	}

	std::wstring getWStringHexa() const
	{
		std::wstring str = L"";

		str += getWHexaOnNbr((r / 16));
		str += getWHexaOnNbr((r % 16));
		str += getWHexaOnNbr((g / 16));
		str += getWHexaOnNbr((g % 16));
		str += getWHexaOnNbr((b / 16));
		str += getWHexaOnNbr((b % 16));
		str += getWHexaOnNbr((a / 16));
		str += getWHexaOnNbr((a % 16));

		return (str);
	}

	static Color32 Zero()
	{
		return (Color32(0, 0, 0, 0));
	}

	bool operator<(const Color32& scolor) const
	{
		uint32_t fc = 0;
		uint32_t sc = 0;

		fc = (((((r << 8) + g) << 8) + b) << 8) + a;
		sc = (((((scolor.r << 8) + scolor.g) << 8) + scolor.b) << 8) + scolor.a;
		return (fc < sc);
	}

    Color32 operator!() const
    {
        return (Color32(255 - r, 255 - g, 255 - b, a));
    }

    float greyEquivalent()
    {
        return ((0.299f * (float)r / 255) + (0.587f * (float)g / 255) + (0.114f * (float)b / 255));
    }

private:
	char getHexaOnNbr(char nbr) const
	{
		if (nbr < 10)
			return (nbr + '0');
		else
			return (nbr + 'A' - 10);
	}

	wchar_t getWHexaOnNbr(char nbr) const
	{
		if (nbr < 10)
			return (nbr + L'0');
		else
			return (nbr + L'A' - 10);
	}
};

inline bool operator==(const Color32& first, const Color32& second)
{
	if (first.Red() == second.Red() && first.Green() == second.Green() && first.Blue() == second.Blue() && first.Alpha() == second.Alpha())
		return (true);
	return (false);
}

inline bool operator!=(const Color32& first, const Color32& second)
{
	return (!(first == second));
}

inline std::ostream& operator<<(std::ostream& out, const Color32& color)
{
	out << "[" << (int)color.r << ", " << (int)color.g << ", " << (int)color.b << ", " << (int)color.a << "]";
	return (out);
}

#endif // !Color32
