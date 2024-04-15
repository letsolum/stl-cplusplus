#include <cmath>
#include <vector>

class Point {
  private:
    static const double eps;

  public:
    double x, y;
    static double getEps();
    Point();
    Point(double x, double y);
    Point& operator+=(const Point& second);
    Point& operator-=(const Point& second);
    Point& operator*=(double k);
    Point& operator/=(double k);
};

bool operator==(const Point& first, const Point& second);
bool operator!=(const Point& first, const Point& second);
bool operator<(const Point& first, const Point& second);
Point operator+(Point first, const Point& second);
Point operator-(Point first, const Point& second);
Point operator*(Point first, double k);
Point operator*(double k, Point first);
Point operator/(Point first, double k);
Point operator/(double k, Point first);
double operator^(const Point& first, const Point& second);
double operator*(const Point& first, const Point& second);
double distance(Point first, const Point& second);

class Line {
  private:
    Point fir, sec;

  public:
    Line();
    Line(Point first, Point second);
    Line(double k, double b);
    Line(Point first, double k);
    const Point& getFirst() const;
    const Point& getSecond() const;
};

bool operator==(const Line& first, const Line& second);
bool operator!=(const Line& first, const Line& second);

class Shape {
  public:
    virtual double perimeter() = 0;
    virtual double area() = 0;
    virtual bool operator!=(const Shape& another) const = 0;
    virtual bool isCongruentTo(const Shape& another) const = 0;
    virtual bool isSimilarTo(const Shape& another) const = 0;
    virtual bool containsPoint(const Point& point) const = 0;

    virtual void rotate(const Point& center, double angle) = 0;
    virtual void reflect(const Point& center) = 0;
    virtual void reflect(const Line& axis) = 0;
    virtual void scale(const Point& center, double coefficient) = 0;
    virtual ~Shape() = default;
};

bool operator==(const Shape& first, const Shape& second);

class Polygon : public Shape {
  protected:
    std::vector<Point> vertices;

  public:
    Polygon() = default;
    Polygon(const std::vector<Point>& raw);
    template <typename... Args>
    Polygon(Args... args) {
        (vertices.push_back(args), ...);
    }
    void normalize();
    int verticesCount();
    std::vector<double> getDiagonals() const;
    std::vector<Point> getVertices() const;
    bool isConvex();
    double perimeter() override;
    double area() override;
    bool operator!=(const Shape& another) const override;
    bool isCongruentTo(const Shape& another) const override;
    bool isSimilarTo(const Shape& another) const override;
    bool containsPoint(const Point& point) const override;
    void rotate(const Point& center, double angle) override;
    void reflect(const Point& center) override;
    void reflect(const Line& axis) override;
    void scale(const Point& center, double coefficient) override;
    friend bool checkCongruent(Polygon& save, std::vector<double>& diagonals,
                               const std::vector<Point>& vertices);
    friend bool operator==(const Polygon& first, const Polygon& second);
};

class Rectangle : public Polygon {
  public:
    Rectangle() = default;
    Rectangle(const Point& first, const Point& second, double k);
    Point center();
    std::pair<Line, Line> diagonals();
};

class Ellipse : public Shape {
  protected:
    Point foc1, foc2;
    double a, b;

  public:
    Ellipse() = default;
    Ellipse(const Point& f1, const Point& f2, double l);
    double perimeter() override;
    double area() override;
    bool operator!=(const Shape& another) const override;
    bool isCongruentTo(const Shape& another) const override;
    bool isSimilarTo(const Shape& another) const override;
    bool containsPoint(const Point& point) const override;
    void rotate(const Point& center, double angle) override;
    void reflect(const Point& center) override;
    void reflect(const Line& axis) override;
    void scale(const Point& center, double coefficient) override;
    std::pair<Point, Point> focuses();
    std::pair<Line, Line> directrices();
    double eccentricity();

    friend bool operator==(const Ellipse& first, const Ellipse& second);
};

class Circle : public Ellipse {
  public:
    Circle() = default;
    Circle(const Point& center, double radius);
    double radius();
    Point center();
};

class Square : public Rectangle {
  public:
    Square() = default;
    Square(const Point& first, const Point& second);
    Circle circumscribedCircle();
    Circle inscribedCircle();
};

class Triangle : public Polygon {
  public:
    Triangle() = default;
    Triangle(const Point& first, const Point& second, const Point& third);
    Circle circumscribedCircle();
    Circle inscribedCircle();
    Point centroid();
    Point orthocenter();
    Line EulerLine();
    Circle ninePointsCircle();
};
