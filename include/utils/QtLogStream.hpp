#include <QtWidgets/QColorDialog>
#include "utils/Color32.hpp"

#ifndef QTLOGSTREAM_HPP_
#define QTLOGSTREAM_HPP_

inline std::ostream& operator<<(std::ostream& out, const QColor& color)
{
	out << "[" << color.red() << ", " << color.green() << ", " << color.blue() << ", " << color.alpha() << "]";
	return (out);
}

inline std::ostream& operator<<(std::ostream& out, const QString& data)
{
	out << data.toStdString();
	return (out);
}

inline bool operator==(const Color32& scscolor, const QColor& qcolor)
{
	if (scscolor.r == qcolor.red() && scscolor.g == qcolor.green() && scscolor.b == qcolor.blue() && scscolor.a == qcolor.alpha())
	{
		return (true);
	}
	return (false);
}

inline bool operator==(const QColor& qcolor, const Color32& scscolor)
{
	return (scscolor == qcolor);
}

inline bool operator!=(const Color32& scscolor, const QColor& qcolor)
{
	if (scscolor.r == qcolor.red() && scscolor.g == qcolor.green() && scscolor.b == qcolor.blue() && scscolor.a == qcolor.alpha())
		return (false);
	return (true);
}

inline bool operator!=(const QColor& qcolor, const Color32& scscolor)
{
	return (scscolor != qcolor);
}

inline QColor operator!(const QColor& qcolor)
{
	int Colortt = (qcolor.red() + qcolor.green() + qcolor.blue()) / 3;
	QColor newColor = (Colortt >= 128) ? QColor(0, 0, 0, 255) : QColor(255, 255, 255, 255);
	return (newColor);
}

inline Color32 Color32FromQColor(const QColor& color)
{
	return (Color32(color.red(), color.green(), color.blue(), color.alpha()));
}

inline QColor QColorFromColor32(const Color32& color)
{
	return (QColor(color.Red(), color.Green(), color.Blue(), color.Alpha()));
}

#endif // !QTLOGSTREAM_HPP_
