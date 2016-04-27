#pragma once
#include "types.h"
#include "v2f.h"
#include <stdlib.h>
#include <vector>

// ported from osu-web

// gets coords at point t (0-1) on a line
v2f pt_on_line(const v2f& p1, const v2f& p2, f32 t);

// gets coords at point t (0-1) on a circular arc of length len of a circle
// that intersects p1, p2, p3, starting at p1.
v2f pt_on_circular_arc(
		const v2f& p1, const v2f& p2, const v2f& p3, f32 t, f32 len);

// checks if pt is inside a circle at center with radius r
bool pt_in_circle(const v2f& pt, const v2f& center, f32 r);

struct curve {
	static const size_t max_points = 0xFF;
	size_t num_points = 0;
	v2f points[max_points];

	void init(const v2f* pts, size_t npts);
	virtual v2f at(f32 t) const = 0;

	// approximates the length of the curve
	f32 len() const;

	// compute granular positions for the entire curve
	void compute(std::vector<v2f>* dst);
};

struct bezier : curve {
	// gets coords at point t (0-1) inside the curve
	v2f at(f32 t) const;
};

struct catmull : curve {
	v2f at(f32 t) const;
};
