#include "IECoreRI/Spline.h"

surface splineTest(
	uniform float splPositions[] = {};
	uniform color splValues[] = {};
)
{
	Ci = ieSpline( splPositions, splValues, t );
	Oi = 1;
}
