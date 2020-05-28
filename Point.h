#pragma once

#include <iostream>
#include <cmath>
#include <numeric>
#include <limits>

template <typename T>
struct Point {
    T x;
    T y;
};


template <typename T>
class PVector {
public:
    explicit PVector(double a, double b);
    explicit PVector(Point<T> a, Point<T> b);
    bool operator == (PVector rhs);
    PVector operator - ();
    double length() const;
    T x;
    T y;
};


template <typename T>
Point<T> operator + (Point<T> lhs, Point<T> rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

template <typename T>
Point<T> operator - (Point<T> lhs, Point<T> rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

template <typename T>
Point<T> operator / (Point<T> lhs, double a) {
    return { lhs.x / a, lhs.y / a};
}

template <typename T>
Point<T> operator * (Point<T> lhs, double a) {
    return {lhs.x * a, lhs.y * a};
}

template <typename T>
bool operator < (Point<T> lhs, Point<T> rhs) {
    return (lhs.x * lhs.x + lhs.y * lhs.y) < (lhs.x * lhs.x + lhs.y * lhs.y);
}

template <typename T>
double operator * (PVector<T> lhs, PVector<T> rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

template <typename T>
bool is_parallel(const PVector<T>& lhs, const PVector<T>& rhs) {
    return (lhs.x * rhs.y - lhs.y * rhs.y) == 0;
}

template <typename T>
bool PVector<T>::operator == (PVector<T> rhs) {
    return
            std::abs(x - rhs.x) < std::numeric_limits<double>::epsilon() * 100
            && std::abs(y - rhs.y) < std::numeric_limits<double>::epsilon() * 100;
}

template <typename T>
double PVector<T>::length() const {
    return sqrt(x*x + y*y);
}

template <typename T>
PVector<T>::PVector(double a, double b)
        : x(a), y(b) {

}

template <typename T>
PVector<T>::PVector(Point<T> a, Point<T> b)
        : x(b.x - a.x), y(b.y - a.y){

}

template <typename T>
PVector<T> PVector<T>::operator - () {
    return PVector(-x, -y);
}

template <typename T>
bool is_perpendecular(const PVector<T>& lhs, const PVector<T>& rhs) {
    return (lhs * rhs) == 0;
}

template <typename T>
std::ostream& operator << (std::ostream& str, const Point<T>& p) {
    return str << p.x << " " << p.y;
}

template <typename T>
std::istream& operator >> (std::istream& str, Point<T>& p) {
    return str >> p.x >> p.y;
}