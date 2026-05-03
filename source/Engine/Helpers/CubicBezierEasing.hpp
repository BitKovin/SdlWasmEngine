#include <cmath>
#include <stdexcept>

// ─────────────────────────────────────────────
//  CubicBezierEasing
//
//  Matches CSS cubic-bezier(x1, y1, x2, y2).
//  Control points P0=(0,0) and P3=(1,1) are fixed.
//  x1/x2 must be in [0,1]; y1/y2 are unclamped
//  (allowing overshoot, e.g. bounce effects).
//
//  Usage:
//    CubicBezierEasing ease(0.72, -0.01, 0.47, 1.0);
//    double y = ease(0.5);   // progress in [0,1]
// ─────────────────────────────────────────────
class CubicBezierEasing {
public:
    CubicBezierEasing(double x1, double y1, double x2, double y2)
        : x1_(x1), y1_(y1), x2_(x2), y2_(y2)
    {
        if (x1 < 0.0 || x1 > 1.0 || x2 < 0.0 || x2 > 1.0)
            throw std::invalid_argument("x1 and x2 must be in [0, 1]");
    }

    // Evaluate easing at progress t ∈ [0, 1]
    double operator()(double t) const {
        if (t <= 0.0) return 0.0;
        if (t >= 1.0) return 1.0;

        return sampleY(solveT(t));
    }

private:
    double x1_, y1_, x2_, y2_;

    static constexpr double NEWTON_ITERATIONS = 8;
    static constexpr double NEWTON_MIN_SLOPE = 0.001;
    static constexpr double SUBDIVISION_PRECISION = 1e-7;
    static constexpr int    SUBDIVISION_MAX_ITER = 10;

    // ── Bernstein polynomial helpers ──────────────
    static double A(double a1, double a2) { return 1.0 - 3.0 * a2 + 3.0 * a1; }
    static double B(double a1, double a2) { return 3.0 * a2 - 6.0 * a1; }
    static double C(double a1) { return 3.0 * a1; }

    // Cubic bezier coordinate for parameter u
    double sampleX(double u) const {
        return ((A(x1_, x2_) * u + B(x1_, x2_)) * u + C(x1_)) * u;
    }
    double sampleY(double u) const {
        return ((A(y1_, y2_) * u + B(y1_, y2_)) * u + C(y1_)) * u;
    }

    // Derivative of x w.r.t. u (for Newton's method)
    double sampleDerivX(double u) const {
        return 3.0 * A(x1_, x2_) * u * u + 2.0 * B(x1_, x2_) * u + C(x1_);
    }

    // ── Solver: given x-progress, find bezier parameter u ──

    // Newton–Raphson (fast, used when slope is healthy)
    double newtonRaphson(double x, double u) const {
        for (int i = 0; i < NEWTON_ITERATIONS; ++i) {
            double slope = sampleDerivX(u);
            if (std::abs(slope) < 1e-12) break;
            u -= (sampleX(u) - x) / slope;
        }
        return u;
    }

    // Binary subdivision (robust fallback)
    double binarySubdivide(double x, double lo, double hi) const {
        double mid = 0.0, xMid = 0.0;
        for (int i = 0; i < SUBDIVISION_MAX_ITER; ++i) {
            mid = lo + (hi - lo) / 2.0;
            xMid = sampleX(mid) - x;
            if (std::abs(xMid) < SUBDIVISION_PRECISION) break;
            (xMid > 0.0 ? hi : lo) = mid;
        }
        return mid;
    }

    // Pick Newton or subdivision depending on local slope
    double solveT(double x) const {
        double u = x; // initial guess
        double slope = sampleDerivX(u);

        if (slope >= NEWTON_MIN_SLOPE)
            return newtonRaphson(x, u);

        // Slope too flat → subdivision is safer
        double lo = 0.0, hi = 1.0;
        return binarySubdivide(x, lo, hi);
    }
};