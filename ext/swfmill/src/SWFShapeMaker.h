#ifndef SWF_SHAPEMAKER_H
#define SWF_SHAPEMAKER_H

#include "Geom.h"
#include <cstdio>
#include <libxml/tree.h>
#include <cmath>

namespace SWF {

class ShapeItem;
template <class T> class List;

class Bezier {
	public:
		Point p0, p1, p2, p3;
		Bezier() { ; }
		Bezier( const Point& _p0, const Point& _p1, const Point& _p2, const Point& _p3 )
			: p0(_p0), p1(_p1), p2(_p2), p3(_p3) { ; }
		Bezier &set( const Point& _p0, const Point& _p1, const Point& _p2, const Point& _p3 ) {
			p0 = _p0; p1 = _p1; p2 = _p2; p3 = _p3;
			return *this;
		}
};

class ShapeMaker {
	public:
		ShapeMaker( List<ShapeItem>* edges, double fx = 1, double fy = 1, double ofsx = 0, double ofsy = 0 );
	
		void setStyle( int _fillStyle0=-1, int _fillStyle1=-1, int _lineStyle=-1 ) {
			fillStyle0 = _fillStyle0;
			fillStyle1 = _fillStyle1;
			lineStyle = _lineStyle;
		}
	
		void setup( double x = 0, double y = 0 );
		void setupR( double x = 0, double y = 0 );

		void lineTo( double x, double y );
		void lineToR( double x, double y );
		
		void curveTo( double cx, double cy, double ax, double ay );
		void curveToR( double cx, double cy, double ax, double ay );

		void smoothCurveTo( double ax, double ay );
		void smoothCurveToR( double ax, double ay );

		void cubicTo( double x1, double y1, double x2, double y2, double ax, double ay );
		void cubicToR( double x1, double y1, double x2, double y2, double ax, double ay );

		void smoothCubicTo( double x2, double y2, double ax, double ay );
		void smoothCubicToR( double x2, double y2, double ax, double ay );

		void close(bool stroke = true);
		void finish();
	
		void rect( double x, double y, double width, double height, double rx = 0, double ry = 0 );
		void ellipse( double cx, double cy, double rx, double ry );

		void arcTo( double rx, double ry, double rotation, bool largeArcFlag, bool sweepFlag, double x, double y );
		void arcToR( double rx, double ry, double rotation, bool largeArcFlag, bool sweepFlag, double x, double y );
	
		void boundsWriteXML( xmlNodePtr node, double border = -1 );
	
		double getLastX() { return lastx; }
		double getLastY() { return lasty; }

		double getSmoothX() { return smoothx; }
		double getSmoothY() { return smoothy; }

		Rect getBounds() { return Rect(minx, miny, maxx, maxy); }
		
	protected:
		void cubicToRec( const Point& a, const Point& b, const Point& c, const Point& d, double k, int iteration=0 );

		void doSetup( double _x=0, double _y=0, bool hasMoveTo=true, int _fillStyle0=-1, int _fillStyle1=-1, int _lineStyle=-1);

		void ellipseSegment( double cx, double cy, double rx, double ry, double phi, double theta, double dTheta);
	
		void minmax( double x, double y ) {
			if( !have_first ) {
				have_first = true;
				minx = x; maxx=x; miny=y; maxy=y;
			} else {
				if( x < minx ) minx=x;
				if( y < miny ) miny=y;
				if( x > maxx ) maxx=x;
				if( y > maxy ) maxy=y;
			}
		}
	
		List<ShapeItem>* edges;
		double factorx, factory;
		double offsetx, offsety;
		double diffx, diffy;
		double lastx, lasty;
		double lastsetupx, lastsetupy;
		double minx, miny, maxx, maxy;
		double smoothx, smoothy;
		bool have_first;
	
		int fillStyle0, fillStyle1, lineStyle;

		// rounding error accumulation compensation
		double roundx, roundy;
		int roundX( double x ) { return round( x, &roundx ); }
		int roundY( double y ) { return round( y, &roundy ); }
		int round( double v, double *acc ) {
				int r = (int)v;
				*acc += v-(double)r;
				while( *acc >= .5 ) {
					*acc -= 1.0;
					r++;
				}
				while( *acc <= -.5 ) {
					*acc += 1.0;
					r--;
				}
				return r;
			}
		void roundReset() {
			roundx = roundy = 0;
			}
};

	
}
#endif
