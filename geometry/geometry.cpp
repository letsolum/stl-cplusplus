#include "geometry.h"
#include <algorithm>

const double Point::eps = 1e-6;

Point::Point() : x(0.0), y(0.0) {}

Point::Point(double x, double y) : x(x), y(y) {}

double Point::getEps() {
    return eps;
}

bool equals(double a, double b, double eps) {
    return a + eps > b && a - eps < b;
}

bool operator==(const Point& first, const Point& second) {
    return equals(first.x, second.x, first.getEps()) &&
           equals(first.y, second.y, first.getEps());
}

bool operator!=(const Point& first, const Point& second) {
    return !(first == second);
}

bool operator<(const Point& first, const Point& second) {
    return (first.x < second.x ||
            (equals(first.x, second.x, first.getEps()) && first.y < second.y));
}

bool comp(const Point& a,
          const Point& b) {  //comparator for sorting by polar angle
    Point c(1e5, 1e5);
    double alpha = atan2(a ^ c, a * c);
    double betta = atan2(b ^ c, b * c);
    return alpha < betta;
}

Point& Point::operator+=(const Point& second) {
    x += second.x;
    y += second.y;
    return *this;
}

Point& Point::operator-=(const Point& second) {
    x -= second.x;
    y -= second.y;
    return *this;
}

Point operator+(Point first, const Point& second) {
    return first += second;
}

Point operator-(Point first, const Point& second) {
    return first -= second;
}

Point& Point::operator*=(double k) {
    x *= k;
    y *= k;
    return *this;
}

Point& Point::operator/=(double k) {
    k = 1.0 / k;
    x *= k;
    y *= k;
    return *this;
}

Point operator*(Point first, double k) {
    return first *= k;
}

Point operator*(double k, Point first) {
    return first * k;
}

Point operator/(Point first, double k) {
    return first /= k;
}

Point operator/(double k, Point first) {
    return first / k;
}

double operator^(const Point& first,
                 const Point& second) {  //vector multiplication
    return first.x * second.y - first.y * second.x;
}

double operator*(const Point& first,
                 const Point& second) {  //scalar multiplication
    return first.x * second.x + first.y * second.y;
}

double distance(Point first, const Point& second) {
    first -= second;
    return sqrt(first.x * first.x + first.y * first.y);
}

Line::Line() : fir(0.0, 0.0), sec(0.0, 0.0) {}

Line::Line(Point first, Point second) {
    if (equals(first.x, second.x, first.getEps())) {
        fir = Point(first.x, 0.0);
        sec = Point(first.x, 1.0);
        return;
    }
    double k = (second.y - first.y) / (second.x - first.x);
    double b = first.y - first.x * k;
    fir = Point(0.0, b);
    sec = Point(1.0, k + b);
}

Line::Line(double k, double b) : fir(0.0, b), sec(1.0, k + b) {}

Line::Line(Point first, double k)
    : fir(0.0, first.y - k * first.x), sec(1.0, first.y - k * first.x + k) {}

const Point& Line::getFirst() const {
    return fir;
}

const Point& Line::getSecond() const {
    return sec;
}

bool operator==(const Line& first, const Line& second) {
    return first.getFirst() == second.getFirst() &&
           first.getSecond() == second.getSecond();
}

bool operator!=(const Line& first, const Line& second) {
    return !(first == second);
}

int sign(double x) {
    if (equals(x, 0.0, 1e-6)) {
        return 0;
    }
    return static_cast<int>(x > 0) * 2 - 1;
}

Point rotation(const Point& first, const Point& second, double angle) {
    angle *= asin(1.0) / 90.0;
    Point answer = second - first;
    return {cos(angle) * answer.x - sin(angle) * answer.y + first.x,
            sin(angle) * answer.x + cos(angle) * answer.y + first.y};
}

Polygon::Polygon(const std::vector<Point>& raw) : vertices(raw) {}

int Polygon::verticesCount() {
    return vertices.size();
}

std::vector<Point> Polygon::getVertices() const {
    return vertices;
}

std::vector<double> Polygon::getDiagonals() const {
    std::vector<double> diagonals;
    for (size_t i = 0; i < vertices.size(); ++i) {
        for (size_t j = i + 1; j < vertices.size(); ++j) {
            diagonals.push_back(distance(vertices[i], vertices[j]));
        }
    }
    std::sort(diagonals.begin(), diagonals.end());
    return diagonals;
}

bool Polygon::isConvex() {
    bool flag = true;
    for (size_t i = 1; i < vertices.size(); ++i) {
        double a = vertices[(i + 1) % vertices.size()].y - vertices[i].y;
        double b = vertices[i].x - vertices[(i + 1) % vertices.size()].x;
        double c = vertices[i].y * vertices[(i + 1) % vertices.size()].x -
                   vertices[i].x * vertices[(i + 1) % vertices.size()].y;
        if ((a * vertices[(i - 1)].x + b * vertices[i - 1].y + c) *
                (a * vertices[(i + 2) % vertices.size()].x +
                 b * vertices[(i + 2) % vertices.size()].y + c) <
            0) {
            flag = false;
        }
    }
    return flag;
}

double Polygon::perimeter() {
    double answer = 0.0;
    for (size_t i = 0; i < vertices.size(); ++i) {
        answer += distance(vertices[i], vertices[(i + 1) % vertices.size()]);
    }
    return answer;
}

double Polygon::area() {
    double answer = 0.0;
    for (size_t i = 1; i < vertices.size() - 1; ++i) {
        answer += (vertices[i] - vertices[0]) ^ (vertices[i + 1] - vertices[0]);
    }
    if (answer < 0.0) {
        answer = -answer;
    }
    return answer / 2.0;
}

bool operator==(const Shape& first, const Shape& second) {
    const Polygon* p1 = dynamic_cast<const Polygon*>(&first);
    const Polygon* p2 = dynamic_cast<const Polygon*>(&second);
    const Ellipse* e1 = dynamic_cast<const Ellipse*>(&first);
    const Ellipse* e2 = dynamic_cast<const Ellipse*>(&second);
    return (p1 != nullptr && p2 != nullptr && *p1 == *p2) ||
           (e1 != nullptr && e2 != nullptr && *e1 == *e2);
}

bool checkCongruent(Polygon& save, std::vector<double>& diagonals,
                    const std::vector<Point>& vertices) {
    for (size_t i = 0; i < vertices.size(); ++i) {
        auto diag2 = save.getDiagonals();
        bool flag = true;
        for (size_t j = 0; j < diagonals.size(); ++j) {
            if (!equals(diagonals[j], diag2[j], vertices[0].getEps())) {
                flag = false;
            }
        }
        if (flag) {
            return true;
        }
        std::rotate(save.vertices.begin(), save.vertices.begin() + 1,
                    save.vertices.end());
    }
    return false;
}

bool Polygon::isCongruentTo(const Shape& another) const {
    const Polygon* second = dynamic_cast<const Polygon*>(&another);
    if (second == nullptr || second->vertices.size() != vertices.size()) {
        return false;
    }
    Polygon save = Polygon(second->getVertices());
    std::vector<double> diagonals = getDiagonals();
    if (checkCongruent(save, diagonals, vertices)) {
        return true;
    }
    reverse(save.vertices.begin(), save.vertices.end());
    return checkCongruent(save, diagonals, vertices);
}

bool Polygon::isSimilarTo(const Shape& another) const {
    const Polygon* second = dynamic_cast<const Polygon*>(&another);
    if (second == nullptr || second->vertices.size() != vertices.size()) {
        return false;
    }
    Polygon save1 = *this, save2 = *second;
    save1.normalize();
    save2.normalize();
    for (size_t shift = 0; shift < save1.vertices.size(); ++shift) {
        double cur_k = 0.0;
        for (size_t i = 0; i < save1.vertices.size(); ++i) {
            size_t ind = (shift + i) % save1.vertices.size();
            size_t ind2 = (shift + i + 1) % save1.vertices.size();
            double k =
                distance(save2.vertices[i],
                         save2.vertices[(i + 1) % save1.vertices.size()]) /
                distance(save1.vertices[ind], save1.vertices[ind2]);
            if (equals(cur_k, 0.0, save1.vertices[0].getEps())) {
                cur_k = k;
            } else if (!equals(cur_k, k, save1.vertices[0].getEps())) {
                break;
            }
            if (i + 1 == save1.vertices.size()) {
                return true;
            }
        }
    }
    return false;
}

bool Polygon::containsPoint(const Point& point) const {
    int sign = 0;
    if (((vertices[0] - vertices[1]) ^ (point - vertices[1])) > 0) {
        sign = 1;
    } else if (equals((vertices[0] - vertices[1]) ^ (point - vertices[1]), 0.0,
                      vertices[0].getEps())) {
        sign = 0;
    } else {
        sign = -1;
    }
    for (size_t i = 0; i < vertices.size(); ++i) {
        double cur = ((vertices[i] - vertices[(i + 1) % vertices.size()]) ^
                      (point - vertices[(i + 1) % vertices.size()]));
        if (sign * cur < 0) {
            return false;
        }
    }
    if (((vertices[1] - vertices[2]) ^ (point - vertices[2])) > 0) {
        sign = 1;
    } else if (equals((vertices[1] - vertices[2]) ^ (point - vertices[2]), 0.0,
                      vertices[1].getEps())) {
        sign = 0;
    } else {
        sign = -1;
    }
    for (size_t i = 0; i < vertices.size(); ++i) {
        double cur = ((vertices[i] - vertices[(i + 1) % vertices.size()]) ^
                      (point - vertices[(i + 1) % vertices.size()]));
        if (sign * cur < 0) {
            return false;
        }
    }
    return true;
}

void Polygon::normalize() {
    std::sort(vertices.begin(), vertices.end(), comp);
}

void Polygon::rotate(const Point& center, double angle) {
    for (size_t i = 0; i < vertices.size(); ++i) {
        vertices[i] = rotation(center, vertices[i], angle);
    }
}

void Polygon::reflect(const Point& center) {
    for (size_t i = 0; i < vertices.size(); ++i) {
        vertices[i] = center + (center - vertices[i]);
    }
}

void Polygon::reflect(const Line& axis) {
    if (axis.getFirst().x == axis.getSecond().x) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            vertices[i].x =
                axis.getFirst().x + (axis.getFirst().x - vertices[i].x);
        }
    } else {
        double b = axis.getFirst().y;
        double k = axis.getSecond().y - b;
        double alpha = atan(k);
        if (alpha < 0) {
            alpha *= -1.0;
        }
        for (size_t i = 0; i < vertices.size(); ++i) {
            Line second(vertices[i], k);
            double shift = b - second.getFirst().y;
            double perpendicular = shift * cos(alpha);
            vertices[i].y += 2 * perpendicular * cos(alpha);
            vertices[i].x += 2 * perpendicular * sin(alpha) * sign(k) * (-1.0);
        }
    }
}

void Polygon::scale(const Point& center, double coefficient) {
    for (size_t i = 0; i < vertices.size(); ++i) {
        vertices[i] = center + fabs(coefficient) *
                                   (coefficient / fabs(coefficient)) *
                                   (vertices[i] - center);
    }
}

Rectangle::Rectangle(const Point& first, const Point& second, double k) {
    Point center = (first + second) * 0.5;
    if (k > 1) {
        k = 1.0 / k;
    }
    double alpha =
        ((first.x < second.x) || (equals(first.x, second.x, first.getEps()) &&
                                  first.y < second.y)
             ? -1
             : 1) *
        90.0 / asin(1.0) * asin(k);
    Polygon d1(first, second);
    d1.rotate(center, alpha);
    vertices = {d1.getVertices()[0], second, d1.getVertices()[1], first};
}

std::pair<Line, Line> Rectangle::diagonals() {
    Line first(vertices[0], vertices[2]), second(vertices[1], vertices[3]);
    return std::make_pair(first, second);
}

Ellipse::Ellipse(const Point& f1, const Point& f2, double l) {
    foc1 = f1;
    foc2 = f2;
    if (f2 < f1) {
        std::swap(foc1, foc2);
    }
    double c = distance(foc1, foc2) / 2.0;
    b = sqrt(l * l / 4.0 - c * c);
    a = sqrt(b * b + c * c);
}

double Ellipse::perimeter() {
    return M_PI * (3 * (a + b) - sqrt((3 * a + b) * (a + 3 * b)));
}

double Ellipse::area() {
    return asin(1.0) * 2.0 * a * b;
}

bool Ellipse::isCongruentTo(const Shape& another) const {
    const Ellipse* second = dynamic_cast<const Ellipse*>(&another);
    if (second == nullptr) {
        return false;
    }
    Point center1 = (foc1 + foc2) / 2;
    Point center2 = (second->foc1 + second->foc2) / 2;
    if (!equals(distance(center1, foc1), distance(center2, second->foc1),
                foc1.getEps())) {
        return false;
    }
    if (!equals(distance(center1, foc2), distance(center2, second->foc2),
                foc2.getEps())) {
        return false;
    }
    if (!equals(a, second->a, foc1.getEps()) ||
        !equals(b, second->b, foc1.getEps())) {
        return false;
    }
    return true;
}

bool Ellipse::isSimilarTo(const Shape& another) const {
    const Ellipse* second = dynamic_cast<const Ellipse*>(&another);
    if (second == nullptr) {
        return false;
    }
    Point center1 = (foc1 + foc2) / 2;
    Point center2 = (second->foc1 + second->foc2) / 2;
    double k = distance(center1, foc1) / distance(center2, second->foc1);
    if (!equals(a, second->a * k, foc1.getEps()) ||
        !equals(b, second->b * k, foc1.getEps())) {
        return false;
    }
    if (!equals(distance(center1, foc2), k * distance(center2, second->foc2),
                foc2.getEps())) {
        return false;
    }
    return true;
}

bool Ellipse::containsPoint(const Point& point) const {
    return distance(point, foc1) + distance(point, foc2) < 2 * a ||
           equals(distance(point, foc1) + distance(point, foc2), 2 * a,
                  foc1.getEps());
}

void Ellipse::rotate(const Point& center, double angle) {
    angle = angle * asin(1.0) / 90;
    foc1 = rotation(center, foc1, angle);
    foc2 = rotation(center, foc2, angle);
}

void Ellipse::reflect(const Point& center) {
    foc1 = center + (center - foc1);
    foc2 = center + (center - foc2);
}

void Ellipse::reflect(const Line& axis) {
    Polygon fake(foc1, foc2);
    fake.reflect(axis);
    foc1 = fake.getVertices()[0];
    foc2 = fake.getVertices()[1];
}

void Ellipse::scale(const Point& center, double coefficient) {
    a *= coefficient;
    b *= coefficient;
    foc1 = center + fabs(coefficient) * (coefficient / fabs(coefficient)) *
                        (foc1 - center);
    foc2 = center + fabs(coefficient) * (coefficient / fabs(coefficient)) *
                        (foc2 - center);
}

std::pair<Point, Point> Ellipse::focuses() {
    return std::make_pair(foc1, foc2);
}

std::pair<Line, Line> Ellipse::directrices() {
    Point delta = (foc1 - foc2) / distance(foc1, foc2);
    double k = delta.x / delta.y;
    Point first = (foc1 + foc2) / 2 + delta * a / eccentricity();
    Point second = (foc1 + foc2) / 2 - delta * a / eccentricity();
    return {Line(first, k), Line(second, k)};
}

double Ellipse::eccentricity() {
    return distance(foc1, foc2) / (2.0 * a);
}

Circle::Circle(const Point& center, double radius) {
    a = b = radius;
    foc1 = foc2 = center;
}

double Circle::radius() {
    return a;
}

Point Circle::center() {
    return foc1;
}

Square::Square(const Point& first, const Point& second) {
    Polygon fake(first, second);
    fake.rotate((first + second) / 2, 90);
    vertices = {first, fake.getVertices()[0], second, fake.getVertices()[1]};
}

Circle Square::circumscribedCircle() {
    return Circle((vertices[0] + vertices[2]) / 2,
                  distance((vertices[0] + vertices[2]) / 2, vertices[0]));
}

Circle Square::inscribedCircle() {
    return Circle((vertices[0] + vertices[2]) / 2,
                  distance(vertices[0], vertices[1]));
}

Triangle::Triangle(const Point& first, const Point& second,
                   const Point& third) {
    vertices = {first, second, third};
}

Circle Triangle::circumscribedCircle() {
    Point O = vertices[0], B = vertices[1], C = vertices[2];
    B -= O;
    C -= O;
    double f1 = distance(B, {0.0, 0.0}) * distance(B, {0.0, 0.0});
    double f2 = distance(C, {0.0, 0.0}) * distance(C, {0.0, 0.0});
    double f3 = B.x * C.y - B.y * C.x;
    Point center((C.y * f1 - B.y * f2), (B.x * f2 - C.x * f1));
    center = center / (2 * f3) + O;
    B += O;
    C += O;
    return Circle(center, distance(O, B) * distance(B, C) * distance(C, O) /
                              (4 * area()));
}

Circle Triangle::inscribedCircle() {
    double x1 = vertices[0].x, y1 = vertices[0].y;
    double x2 = vertices[1].x, y2 = vertices[1].y;
    double x3 = vertices[2].x, y3 = vertices[2].y;
    double a = distance(vertices[0], vertices[1]);
    double b = distance(vertices[1], vertices[2]);
    double c = distance(vertices[2], vertices[0]);

    double x0 = a * x3 + b * x1 + c * x2;
    double y0 = a * y3 + b * y1 + c * y2;
    Point center = Point(x0, y0) / perimeter();
    return Circle(center, 2.0 * area() / perimeter());
}

Point Triangle::centroid() {
    return (vertices[0] + vertices[1] + vertices[2]) / 3.0;
}

Point Triangle::orthocenter() {
    Point a = vertices[0], b = vertices[1], c = vertices[2];
    double a1 = c.x - b.x, b1 = c.y - b.y, a2 = c.x - a.x, b2 = c.y - a.y;
    double c1 = a * (c - b);
    double c2 = b * (c - a);
    double matr = a1 * b2 - a2 * b1;
    return Point((c1 * b2 - c2 * b1) / matr, (a1 * c2 - a2 * c1) / matr);
}

Line Triangle::EulerLine() {
    return Line(centroid(), orthocenter());
}

Circle Triangle::ninePointsCircle() {
    Point a = (vertices[0] + vertices[1]) / 2;
    Point b = (vertices[1] + vertices[2]) / 2;
    Point c = (vertices[2] + vertices[0]) / 2;
    return Triangle(a, b, c).circumscribedCircle();
}

bool operator==(const Ellipse& first, const Ellipse& second) {
    Point center1 = (first.foc1 + first.foc2) / 2;
    Point center2 = (second.foc1 + second.foc2) / 2;
    return first.foc1 - center1 == second.foc1 - center2 &&
           first.foc2 - center1 == second.foc2 - center2 &&
           equals(first.a, second.a, first.foc1.getEps()) &&
           equals(first.b, second.b, first.foc1.getEps());
}

bool operator==(const Polygon& first, const Polygon& second) {
    Polygon cp(second.vertices);
    cp.normalize();
    Polygon save(first.vertices);
    save.normalize();
    for (size_t i = 0; i < first.vertices.size(); ++i) {
        if (save.vertices[i] != cp.vertices[i]) {
            return false;
        }
    }
    return true;
}

bool Polygon::operator!=(const Shape& another) const {
    return !(*this == another);
}

bool Ellipse::operator!=(const Shape& another) const {
    return !(*this == another);
}
