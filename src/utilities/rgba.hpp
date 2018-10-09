#pragma once

#include <ostream>
#include <algorithm>
#include "num.hpp"

class rgb {
public:
	unsigned char r;
	unsigned char g;
	unsigned char b;

	constexpr rgb() : r(0), g(0), b(0) {}
	constexpr rgb(unsigned char const vr, unsigned char const vg, unsigned char const vb) : r(vr), g(vg), b(vb) {}

	template <class T>
	constexpr rgb(T val) : r(val), g(val), b(val) {}

	rgb& operator= (rgb other) {
		r = other.r;
		g = other.g;
		b = other.b;
		return *this;
	}

	template <class T>
	rgb& operator=(T other) {
		*this = rgb(other);
		return *this;
	}

    rgb& operator+= (rgb other) {
		r = (unsigned char) num::clamp((int) r + other.r , 0 , 255);
		g = (unsigned char) num::clamp((int) g + other.g , 0 , 255);
		b = (unsigned char) num::clamp((int) b + other.b , 0 , 255);
		return *this;
	}

	rgb& operator-= (rgb other) {
        r = (unsigned char) num::clamp((int) r - other.r , 0 , 255);
		g = (unsigned char) num::clamp((int) g - other.g , 0 , 255);
		b = (unsigned char) num::clamp((int) b - other.b , 0 , 255);
		return *this;
	}

	rgb& operator*= (rgb other) {
        r = (unsigned char) num::clamp((int) r * other.r , 0 , 255);
		g = (unsigned char) num::clamp((int) g * other.g , 0 , 255);
		b = (unsigned char) num::clamp((int) b * other.b , 0 , 255);
		return *this;
	}

	rgb& operator/= (rgb other) {
        r = (unsigned char) num::clamp((int) r / other.r , 0 , 255);
		g = (unsigned char) num::clamp((int) g / other.g , 0 , 255);
		b = (unsigned char) num::clamp((int) b / other.b , 0 , 255);
		return *this;
	}

	friend rgb operator+ (rgb lhs, rgb const &rhs) {
        lhs += rhs;
        return lhs;
    }

	friend rgb operator- (rgb lhs, rgb const &rhs) {
        lhs -= rhs;
        return lhs;
    }

	friend rgb operator* (rgb lhs, rgb const &rhs) {
        lhs *= rhs;
        return lhs;
    }

	friend rgb operator/ (rgb lhs, rgb const &rhs) {
        lhs /= rhs;
        return lhs;
    }

	bool operator!= (rgb other) {
		return (r != other.r) || (g != other.g) || (b != other.b);
	}

	bool operator== (rgb other) {
		return (r == other.r) && (g == other.g) && (b == other.b);
	}

    rgb clamp(rgb const &lo, rgb const &hi) {
        return rgb(
            std::max(std::min(r, hi.r),lo.r),
            std::max(std::min(g, hi.g),lo.g),
            std::max(std::min(b, hi.b),lo.b)
        );
    }

    unsigned char *putFramebuffer(unsigned char *pixel) const {
        *(pixel++) = r;
        *(pixel++) = g;
        *(pixel++) = b;
        return pixel;
    }

	friend std::ostream & operator << (std::ostream &os, const rgb &v)
	{
		os << "[" << (int) v.r << "," << (int) v.g << "," << (int) v.b << "]";
		return os;
	}
};


class rgba {
public:
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;

	constexpr rgba() : r(0), g(0), b(0), a(0) {}
	constexpr rgba(rgb const v, unsigned char va) : r(v.r), g(v.g), b(v.b), a(va) {}
	constexpr rgba(unsigned char const vr, unsigned char const vg, unsigned char const vb, unsigned char const va) : r(vr), g(vg), b(vb), a(va) {}

	template <class T>
	constexpr rgba(T val) : r(val), g(val), b(val), a(val) {}

	rgba& operator= (rgba other) {
		r = other.r;
		g = other.g;
		b = other.b;
		a = other.a;
		return *this;
	}

	template <class T>
	rgba& operator=(T other) {
		*this = rgba(other);
		return *this;
	}

    rgba& operator*= (double other) {
        r = (unsigned char) num::clamp((double) r * other , 0.0 , 255.0);
        g = (unsigned char) num::clamp((double) g * other , 0.0 , 255.0);
        b = (unsigned char) num::clamp((double) b * other , 0.0 , 255.0);
        a = (unsigned char) num::clamp((double) a * other , 0.0 , 255.0);
        return *this;
    }

    rgba& operator/= (double other) {
        r = (unsigned char) num::clamp((double) r / other , 0.0 , 255.0);
        g = (unsigned char) num::clamp((double) g / other , 0.0 , 255.0);
        b = (unsigned char) num::clamp((double) b / other , 0.0 , 255.0);
        a = (unsigned char) num::clamp((double) a / other , 0.0 , 255.0);
        return *this;
    }

	rgba& operator+= (rgba other) {
		r = (unsigned char) num::clamp((int) r + other.r , 0 , 255);
		g = (unsigned char) num::clamp((int) g + other.g , 0 , 255);
		b = (unsigned char) num::clamp((int) b + other.b , 0 , 255);
		a = (unsigned char) num::clamp((int) a + other.a , 0 , 255);
		return *this;
	}

	rgba& operator-= (rgba other) {
        r = (unsigned char) num::clamp((int) r - other.r , 0 , 255);
		g = (unsigned char) num::clamp((int) g - other.g , 0 , 255);
		b = (unsigned char) num::clamp((int) b - other.b , 0 , 255);
		a = (unsigned char) num::clamp((int) a - other.a , 0 , 255);
		return *this;
	}

	rgba& operator*= (rgba other) {
        r = (unsigned char) num::clamp((int) r * other.r , 0 , 255);
		g = (unsigned char) num::clamp((int) g * other.g , 0 , 255);
		b = (unsigned char) num::clamp((int) b * other.b , 0 , 255);
		a = (unsigned char) num::clamp((int) a * other.a , 0 , 255);
		return *this;
	}

	rgba& operator/= (rgba other) {
        r = (unsigned char) num::clamp((int) r / other.r , 0 , 255);
		g = (unsigned char) num::clamp((int) g / other.g , 0 , 255);
		b = (unsigned char) num::clamp((int) b / other.b , 0 , 255);
		a = (unsigned char) num::clamp((int) a / other.a , 0 , 255);
		return *this;
	}

	rgba clamp(rgba const &lo, rgba const &hi) {
		return rgba(
			std::max(std::min(r, hi.r),lo.r),
			std::max(std::min(g, hi.g),lo.g),
			std::max(std::min(b, hi.b),lo.b),
			std::max(std::min(a, hi.a),lo.a)
		);
	}

	friend rgba operator+ (rgba lhs, rgba const &rhs) { lhs += rhs; return lhs; }
	friend rgba operator- (rgba lhs, rgba const &rhs) { lhs -= rhs; return lhs; }
	friend rgba operator* (rgba lhs, rgba const &rhs) { lhs *= rhs; return lhs; }
	friend rgba operator/ (rgba lhs, rgba const &rhs) { lhs /= rhs; return lhs; }

	bool operator!= (rgba other) {
		return (r != other.r) || (g != other.g) || (b != other.b) || (a != other.a);
	}

	bool operator== (rgba other) {
		return (r == other.r) && (g == other.g) && (b == other.b) && (a == other.a);
	}

	rgb roRgb() {
		return rgb(r,g,b);
	}

    unsigned char *putFramebuffer(unsigned char *pixel) const {
        *(pixel++) = r;
        *(pixel++) = g;
        *(pixel++) = b;
        *(pixel++) = a;
        return pixel;
    }

	friend std::ostream & operator << (std::ostream &os, const rgba &v)
	{
		os << "[" << (int) v.r << "," << (int) v.g << "," << (int) v.b << "," << (int) v.a << "]";
		return os;
	}
};
