
#include "Color.h"

Color::Color(void)
:red(1.),green(1.),blue(1.0),alpha(1.0)
{
}

Color::Color(GLubyte red,GLubyte green,GLubyte blue)
:red(red) , green(green) ,blue(blue)
{
}

Color::Color(GLubyte red,GLubyte green,GLubyte blue,GLubyte alpha)
:red(red/255.f) , green(green/255.f) ,blue(blue/255.f),alpha(alpha/255.f)
{
}

Color::Color(GLfloat red,GLfloat green,GLfloat blue)
:red(red) , green(green) ,blue(blue)
{
}

Color::Color(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha)
:red(red) , green(green) ,blue(blue) ,alpha(alpha)
{
}

Color::Color(const Color& other)
:red(other.red) , green(other.green) ,blue(other.blue) ,alpha(other.alpha)
{
}

Color::setColor(GLubyte red,GLubyte green,GLubyte blue)
{
	this.red = red/255.f;
	this.green = green/255.f;
	this.blue = blue/255.f;
}

Color::setColor(GLubyte red,GLubyte green,GLubyte blue,GLubyte alpha)
{
	this.red = red/255.f;
	this.green = green/255.f;
	this.blue = blue/255.f;
	this.alpha = alpha/255.f;
}

Color::setColor(GLfloat red,GLfloat green,GLfloat blue)
{

}