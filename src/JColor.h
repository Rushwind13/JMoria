#ifndef __JCOLOR_H__
#define __JCOLOR_H__
#include "JMDefs.h"
#ifdef __WIN32__
#include "sdl.h"
#else
#include "SDL2/SDL.h"
#endif //__WIN32__
#define COLOR_EXPAND(color) (color).m_vRG.x, (color).m_vRG.y, (color).m_vBA.x, (color).m_vBA.y
class JColor
{
public:
	JColor()
	{
		m_vRG.Init();
		m_vBA.Init();
	};

	JColor( Uint8 r, Uint8 g, Uint8 b, Uint8 a )
	{
		m_vRG.Init(r, g);
		m_vBA.Init(b, a);
	};

	void GetColor( Uint8 &r, Uint8 &g, Uint8 &b, Uint8 &a )
	{
		r = m_vRG.x;
		g = m_vRG.y;
		b = m_vBA.x;
		a = m_vBA.y;
	};

	Uint8 *GetColor(Uint8 *color)
	{
		color[0] = m_vRG.x;
		color[1] = m_vRG.y;
		color[2] = m_vBA.x;
		color[3] = m_vBA.y;

		return color;
	};

	void SetColor( JColor &in )
	{
		Uint8 r,g,b,a;
		in.GetColor(r,g,b,a);

		m_vRG.x = r;
		m_vRG.y = g;
		m_vBA.x = b;
		m_vBA.y = a;
	};

	void SetColor( Uint8 r, Uint8 g, Uint8 b, Uint8 a )
	{
		m_vRG.x = r;
		m_vRG.y = g;
		m_vBA.x = b;
		m_vBA.y = a;
	};

	void SetColor( char *szColor )
	{
		char *c;
		c = strtok( szColor, "," );
		m_vRG.x = atoi(c);
		c = strtok( NULL, "," );
		m_vRG.y = atoi(c);
		c = strtok( NULL, "," );
		m_vBA.x = atoi(c);
		c = strtok( NULL, "," );
		m_vBA.y = atoi(c);
	};

	JIVector m_vRG;
	JIVector m_vBA;
};
#endif // __JCOLOR_H__