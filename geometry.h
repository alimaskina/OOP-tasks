#include <math.h>
#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <vector>

namespace geom {
const double PI = 3.14159265358979323846;
const double PI_DEG = 180;
}  // namespace geom

struct Point;
class Line;
class Shape;
class Ellipse;
class Circle;
class Polygon;
class Rectangle;
class Triangle;

struct Point {
  double x;
  double y;

  Point();
  Point(double x, double y);

  Point& operator+=(const Point& other);
  Point& operator-=(const Point& other);
  Point& operator*=(double koef);
  Point& operator/=(double koef);

  double len() const;
  bool inSeg(const Point& a, const Point& b) const;
  void rotate(const Point& center, double angle);
  void reflect(const Point& center);
  Point projection(const Line& axis);
  void reflect(const Line& axis);
  void scale(const Point& center, double koef);
};

class Line {
 private:
  Point dir;
  Point shift;

 public:
  Line(const Point& point1, const Point& point2);
  Line(double k, double shift);
  Line(const Point& point, double k);

  const Point& getDir() const;
  const Point& getShift() const;
};

class Shape {
 public:
  virtual double perimeter() const = 0;
  virtual double area() const = 0;
  virtual bool operator==(const Shape& other) const = 0;
  virtual bool isCongruentTo(const Shape& other) const = 0;
  virtual bool isSimilarTo(const Shape& other) const = 0;
  virtual bool containsPoint(const Point& point) const = 0;

  virtual void scale(const Point& center, double koef) = 0;
  virtual void rotate(const Point& center, double angle) = 0;
  virtual void reflect(const Point& center) = 0;
  virtual void reflect(const Line& axis) = 0;

  virtual ~Shape(){};
};

class Ellipse : public Shape {
 protected:
  std::pair<Point, Point> focus;
  double dist;

  double focus_dist;
  double a;
  double b;

  void calc();

 public:
  Ellipse();
  Ellipse(Point f1, Point f2, double dist);

  double perimeter() const override;
  double area() const override;
  bool isCongruentTo(const Shape& other) const override;
  bool isSimilarTo(const Shape& other) const override;
  bool containsPoint(const Point& point) const override;
  bool operator==(const Shape& other) const override;

  std::pair<Point, Point> focuses();
  std::pair<Line, Line> directrices();
  double eccentricity();
  Point center();

  void scale(const Point& center, double koef) override;
  void rotate(const Point& center, double angle) override;
  void reflect(const Point& center) override;
  void reflect(const Line& axis) override;
  bool operator==(const Ellipse& other) const;
};

class Circle : public Ellipse {
 public:
  Circle(const Point& center, double radius);

  double radius();
};

class Polygon : public Shape {
 protected:
  std::vector<Point> vertices;
  int n;

 private:
  bool orderEqual(const Polygon& other, int start) const;
  bool orderRevEqual(const Polygon& other, int start) const;

  bool orderIsSimilar(const Polygon& other, std::vector<double>& angles,
                      std::vector<double>& other_angles, int start) const;

  bool orderRevIsSimilar(const Polygon& other, std::vector<double>& angles,
                         std::vector<double>& other_angles, int start) const;

 public:
  Polygon();
  Polygon(std::vector<Point> vertices);
  Polygon(std::initializer_list<Point> l);
  template <typename... Args>
  Polygon(Args... args) {
    vertices = {args...};
    n = vertices.size();
  }

  int verticesCount();
  std::vector<Point> getVertices();

  double perimeter() const override;
  double area() const override;
  bool isCongruentTo(const Shape& other) const override;
  bool isSimilarTo(const Shape& other) const override;
  bool containsPoint(const Point& point) const override;
  bool operator==(const Shape& other) const override;

  bool isCongruentToPolygon(const Polygon& other) const;
  bool isSimilarToPolygon(const Polygon& other) const;
  bool isConvex();

  void scale(const Point& center, double koef) override;
  void rotate(const Point& center, double angle) override;
  void reflect(const Point& center) override;
  void reflect(const Line& axis) override;

  Polygon& operator+=(const Point& point);
  Polygon& operator-=(const Point& point);
  bool operator==(const Polygon& other) const;
};

class Rectangle : public Polygon {
 public:
  Rectangle();
  Rectangle(const Point& a, const Point& b, double koef);

  Point center();
  std::pair<Line, Line> diagonals();

  double perimeter() const override;
  double area() const override;
};

class Square : public Rectangle {
 public:
  Square(const Point& a, const Point& b);

  Circle circumscribedCircle();
  Circle inscribedCircle();
};

class Triangle : public Polygon {
 public:
  Triangle(const Point& a, const Point& b, const Point& c);

  Circle circumscribedCircle();
  Circle inscribedCircle();
  Point centroid();
  Point orthocenter();
  Line EulerLine();
  Circle ninePointsCircle();
};

// POINT

// Объявление операторов Point
Point operator+(Point a, const Point& b);
Point operator-(Point a, const Point& b);
Point operator*(Point a, double koef);
Point operator/(Point a, double koef);
bool operator==(const Point& a, const Point& b);
bool operator!=(const Point& a, const Point& b);
double operator*(const Point& a, const Point& b);
double operator^(const Point& a, const Point& b);

double angle_deg(const Point& a, const Point& b);

// Функция для сравнения double
int sign(double a, double b) {
  double const EPS = 0.0000001;
  if (a - b > EPS) {
    return 1;
  }
  if (a - b < -EPS) {
    return -1;
  }
  return 0;
}

// Конструкторы Point
Point::Point() : x(0), y(0){};
Point::Point(double x, double y) : x(x), y(y){};

// Внутренние операторы Point
Point& Point::operator+=(const Point& other) {
  x += other.x;
  y += other.y;
  return *this;
}

Point& Point::operator-=(const Point& other) {
  x -= other.x;
  y -= other.y;
  return *this;
}

Point& Point::operator*=(double koef) {
  x *= koef;
  y *= koef;
  return *this;
}

Point& Point::operator/=(double koef) {
  x /= koef;
  y /= koef;
  return *this;
}

bool operator==(const Point& a, const Point& b) {
  return !sign(a.x, b.x) && !sign(a.y, b.y);
}

bool operator!=(const Point& a, const Point& b) { return !(a == b); }

// Методы Point
double Point::len() const { return std::sqrt((x * x + y * y)); }

bool Point::inSeg(const Point& a, const Point& b) const {
  return !sign((*this - a) ^ (*this - b), 0);
}

void Point::rotate(const Point& center, double angle) {
  angle = (angle * geom::PI) / geom::PI_DEG;
  double new_x, new_y;
  double dx = x - center.x, dy = y - center.y;
  new_x = center.x + dx * cos(angle) - dy * sin(angle);
  new_y = center.y + dx * sin(angle) + dy * cos(angle);
  x = new_x;
  y = new_y;
}

void Point::reflect(const Point& center) {
  Point delta = center - *this;
  delta += delta;
  *this += delta;
}

Point Point::projection(const Line& axis) {
  Point dir = axis.getDir(), shift = axis.getShift();
  return shift - dir * (((shift - *this) * dir) / (dir * dir)) - *this;
}

void Point::reflect(const Line& axis) { *this += projection(axis) * 2; }

void Point::scale(const Point& center, double koef) {
  Point delta = *this - center;
  *this = center + delta * koef;
}

// Внешние операторы Point
Point operator+(Point a, const Point& b) {
  a += b;
  return a;
}

Point operator-(Point a, const Point& b) {
  a -= b;
  return a;
}

Point operator*(Point a, double koef) {
  a *= koef;
  return a;
}

Point operator/(Point a, double koef) {
  a /= koef;
  return a;
}

double operator*(const Point& a, const Point& b) {
  return a.x * b.x + a.y * b.y;
}

double operator^(const Point& a, const Point& b) {
  return a.x * b.y - b.x * a.y;
}

double angle_deg(const Point& a, const Point& b) {
  return (std::atan2(a ^ b, a * b)) / geom::PI * geom::PI_DEG;
}

// LINE

// Конструкторы Line
Line::Line(const Point& point1, const Point& point2)
    : dir(point1 - point2), shift(point1){};
Line::Line(double k, double shift) : dir({1, k}), shift({0, shift}){};
Line::Line(const Point& point, double k) : dir({1, k}), shift(point){};

// Геттеры Line
const Point& Line::getDir() const { return dir; }
const Point& Line::getShift() const { return shift; }

// Внешние поераторы Line
bool operator==(const Line& line1, const Line& line2) {
  if (sign(line1.getDir() ^ line2.getDir(), 0)) {
    return false;
  }
  return !sign((line1.getShift() - line2.getShift()) ^ line1.getDir(), 0);
}

bool operator!=(const Line& line1, const Line& line2) {
  return !(line1 == line2);
}

// ELLIPS

// конструктор Ellips
Ellipse::Ellipse(Point f1, Point f2, double dist)
    : focus({f1, f2}), dist(dist) {
  calc();
}
Ellipse::Ellipse()
    : focus({{0, 0}, {0, 0}}), dist(0), focus_dist(0), a(0), b(0){};

// Наследуемые методы Ellips
double Ellipse::perimeter() const {
  return geom::PI * (3 * (a + b) - std::sqrt((3 * a + b) * (a + 3 * b)));
}

double Ellipse::area() const { return geom::PI * a * b; };

bool Ellipse::isCongruentTo(const Shape& other) const {
  if (typeid(other) == typeid(Ellipse) || typeid(other) == typeid(Circle)) {
    const Ellipse& temp = dynamic_cast<const Ellipse&>(other);
    return isSimilarTo(temp) && !sign(area(), other.area());
  }
  return false;
}

bool Ellipse::isSimilarTo(const Shape& other) const {
  if (typeid(other) == typeid(Ellipse) || typeid(other) == typeid(Circle)) {
    const Ellipse& temp = dynamic_cast<const Ellipse&>(other);
    double k_focus = temp.focus_dist / focus_dist;
    double k_dist = temp.dist / dist;
    return !sign(k_focus, k_dist);
  }
  return false;
}

bool Ellipse::containsPoint(const Point& point) const {
  double d_point = (point - focus.first).len() + (point - focus.second).len();
  return sign(d_point, dist) <= 0;
}

bool Ellipse::operator==(const Shape& other) const {
  if (typeid(other) == typeid(Ellipse) || typeid(other) == typeid(Circle)) {
    const Ellipse& temp = dynamic_cast<const Ellipse&>(other);
    return (dist == temp.dist) && ((focus.first == temp.focus.first &&
                                    focus.second == temp.focus.second) ||
                                   (focus.first == temp.focus.second &&
                                    focus.second == temp.focus.first));
  }
  return false;
}

// Свои методы Ellips
void Ellipse::calc() {
  focus_dist = (focus.first - focus.second).len() / 2;
  a = dist / 2;
  b = std::sqrt(a * a - focus_dist * focus_dist);
}

std::pair<Point, Point> Ellipse::focuses() { return focus; }

std::pair<Line, Line> Ellipse::directrices() {
  Point dir = focus.first - focus.second, cent = center();
  dir /= dir.len();
  double e = eccentricity();
  Point x1 = cent + dir * (a / e);
  Point x2 = cent - dir * (a / e);
  dir.rotate(focus.first, geom::PI_DEG / 2);

  Line d1(x1, x1 + dir);
  Line d2(x2, x2 + dir);
  return {d1, d2};
}

double Ellipse::eccentricity() { return focus_dist / a; };

Point Ellipse::center() { return (focus.first + focus.first) / 2; }

void Ellipse::scale(const Point& center, double koef) {
  focus.first.scale(center, koef);
  focus.second.scale(center, koef);
  dist *= koef;
  calc();
}

void Ellipse::rotate(const Point& center, double angle) {
  focus.first.rotate(center, angle);
  focus.second.rotate(center, angle);
}

void Ellipse::reflect(const Point& center) {
  focus.first.reflect(center);
  focus.second.reflect(center);
}

void Ellipse::reflect(const Line& axis) {
  focus.first.reflect(axis);
  focus.second.reflect(axis);
}

bool Ellipse::operator==(const Ellipse& other) const {
  return (dist == other.dist) && ((focus.first == other.focus.first &&
                                   focus.second == other.focus.second) ||
                                  (focus.first == other.focus.second &&
                                   focus.second == other.focus.first));
}

// CIRCLE

// конструктор Circle
Circle::Circle(const Point& center, double radius) {
  focus = {center, center};
  dist = 2 * radius;
  calc();
}

// Свои методы и операторы Polygon
double Circle::radius() { return dist / 2; }

// POLYGON

// конструкторы Polygon
Polygon::Polygon(){};

Polygon::Polygon(std::vector<Point> vertices)
    : vertices(vertices), n(vertices.size()){};

Polygon::Polygon(std::initializer_list<Point> l)
    : Polygon(std::vector<Point>(l)){};

// Свои методы и операторы Polygon
int Polygon::verticesCount() { return n; };
std::vector<Point> Polygon::getVertices() { return vertices; }

bool Polygon::orderEqual(const Polygon& other, int start) const {
  bool good = true;
  int cur = 0;
  for (int i = start; i < start + n; ++i) {
    if (vertices[cur] != other.vertices[i % n]) {
      good = false;
      break;
    }
    ++cur;
  }
  return good;
}

bool Polygon::orderRevEqual(const Polygon& other, int start) const {
  bool good = true;
  int cur = 0;
  for (int i = start + n - 1; i >= start; --i) {
    if (vertices[cur] != other.vertices[(i % n + n) % n]) {
      good = false;
      break;
    }
    ++cur;
  }
  return good;
}

bool Polygon::operator==(const Polygon& other) const {
  if (n != other.n) {
    return false;
  }
  for (int start = 0; start < n; ++start) {
    if (orderEqual(other, start) || orderRevEqual(other, start)) {
      return true;
    }
  }
  return false;
}

Polygon& Polygon::operator+=(const Point& point) {
  for (int i = 0; i < n; ++i) {
    vertices[i] += point;
  }
  return *this;
}

Polygon& Polygon::operator-=(const Point& point) {
  for (int i = 0; i < n; ++i) {
    vertices[i] -= point;
  }
  return *this;
}

void Polygon::scale(const Point& center, double koef) {
  for (int i = 0; i < n; ++i) {
    vertices[i].scale(center, koef);
  }
}

void Polygon::rotate(const Point& center, double angle) {
  for (int i = 0; i < n; ++i) {
    vertices[i].rotate(center, angle);
  }
}

void Polygon::reflect(const Point& center) {
  for (int i = 0; i < n; ++i) {
    vertices[i].reflect(center);
  }
}

void Polygon::reflect(const Line& axis) {
  for (int i = 0; i < n; ++i) {
    vertices[i].reflect(axis);
  }
}

bool Polygon::isConvex() {
  if (n <= 3) {
    return true;
  }
  int sgn = sign((vertices[1] - vertices[0]) ^ (vertices[2] - vertices[0]), 0);
  for (int start = 0; start < n; ++start) {
    Point& p0 = vertices[start];
    Point& p1 = vertices[(start + 1) % n];
    Point& p2 = vertices[(start + 2) % n];
    if (sign((p1 - p0) ^ (p2 - p0), 0) != sgn) {
      return false;
    }
  }
  return true;
}

bool Polygon::isCongruentToPolygon(const Polygon& other) const {
  if (n != other.n) {
    return false;
  }
  if (n == 1) {
    return true;
  }
  return isSimilarToPolygon(other) && !sign(other.area(), area());
}

bool Polygon::orderIsSimilar(const Polygon& other, std::vector<double>& angles,
                             std::vector<double>& other_angles,
                             int start) const {
  Point other_edge = other.vertices[1] - other.vertices[0];
  Point edge = vertices[(start + 1) % n] - vertices[start];
  double sample = other_edge.len() / edge.len();
  bool good = true;
  int cur = 0;
  for (int i = start; i < start + n; ++i) {
    if (sign(angles[i % n], other_angles[cur])) {
      good = false;
      break;
    }

    other_edge = other.vertices[(cur + 1) % n] - other.vertices[cur];
    edge = vertices[(i + 1) % n] - vertices[(i % n)];
    if (sign(other_edge.len() / edge.len(), sample)) {
      good = false;
      break;
    }

    cur++;
  }
  return good;
}

bool Polygon::orderRevIsSimilar(const Polygon& other,
                                std::vector<double>& angles,
                                std::vector<double>& other_angles,
                                int start) const {
  Point other_edge = other.vertices[1] - other.vertices[0];
  Point edge = vertices[(start + 1) % n] - vertices[start];
  double sample = other_edge.len() / edge.len();
  bool good = true;
  int cur = 0;
  for (int i = start; i > start - n; --i) {
    if (sign(angles[(i + n - 1) % n], other_angles[cur])) {
      good = false;
      break;
    }

    other_edge = other.vertices[(cur + 1) % n] - other.vertices[cur];
    edge = vertices[(i + 1 + n) % n] - vertices[((i + n) % n)];
    if (sign(other_edge.len() / edge.len(), sample)) {
      good = false;
      break;
    }
    cur++;
  }
  return good;
}

bool Polygon::isSimilarToPolygon(const Polygon& other) const {
  if (n != other.n) {
    return false;
  }
  if (n == 1) {
    return true;
  }
  std::vector<double> angles, other_angles;
  for (int i = 0; i < n; ++i) {
    Point p1 = vertices[i];
    Point p2 = vertices[(i + 1) % n];
    Point p3 = vertices[(i + 2) % n];
    angles.push_back(std::abs(angle_deg(p2 - p1, p3 - p2)));
  }
  for (int i = 0; i < n; ++i) {
    Point p1 = other.vertices[i];
    Point p2 = other.vertices[(i + 1) % n];
    Point p3 = other.vertices[(i + 2) % n];
    other_angles.push_back(std::abs(angle_deg(p2 - p1, p3 - p2)));
  }

  for (int start = 0; start < n; ++start) {
    if (orderIsSimilar(other, angles, other_angles, start) ||
        orderRevIsSimilar(other, angles, other_angles, start)) {
      return true;
    }
  }
  return false;
}

// Наследуемые методы Polygon

double Polygon::perimeter() const {
  double ans = 0;
  for (int i = 0; i < n; ++i) {
    int j = (i + 1) % n;
    Point delta = vertices[i] - vertices[j];
    ans += delta.len();
  }
  return ans;
};

double Polygon::area() const {
  double ans = 0;
  for (int i = 0; i < n; ++i) {
    Point from = vertices[i], to = vertices[(i + 1) % n];
    ans += (to.x - from.x) * (to.y + from.y);
  }
  return std::abs(ans / 2);
};

bool Polygon::containsPoint(const Point& point) const {
  int cross = 0;
  for (int i = 0; i < n; ++i) {
    Point a = vertices[i];
    Point b = vertices[(i + 1) % n];
    if (point.inSeg(a, b)) {
      return true;
    }
    if (a.y < b.y) {
      std::swap(a, b);
    }
    if (b.y > point.y || a.y <= point.y) {
      continue;
    }
    if (sign((a - point) ^ (b - point), 0) == -1) {
      cross++;
    }
  }
  return cross % 2;
};

bool Polygon::isCongruentTo(const Shape& other) const {
  const std::type_info& ti_other = typeid(other);
  if (ti_other == typeid(Polygon) || ti_other == typeid(Rectangle) ||
      ti_other == typeid(Square) || ti_other == typeid(Triangle)) {
    const Polygon& temp = dynamic_cast<const Polygon&>(other);
    return isCongruentToPolygon(temp);
  }
  return false;
};

bool Polygon::operator==(const Shape& other) const {
  const std::type_info& ti_other = typeid(other);
  if (ti_other == typeid(Polygon) || ti_other == typeid(Rectangle) ||
      ti_other == typeid(Square) || ti_other == typeid(Triangle)) {
    const Polygon& temp = dynamic_cast<const Polygon&>(other);
    return operator==(temp);
  }
  return false;
};

bool Polygon::isSimilarTo(const Shape& other) const {
  const std::type_info& ti_other = typeid(other);
  if (ti_other == typeid(Polygon) || ti_other == typeid(Rectangle) ||
      ti_other == typeid(Square) || ti_other == typeid(Triangle)) {
    const Polygon& temp = dynamic_cast<const Polygon&>(other);
    return isSimilarToPolygon(temp);
  }
  return false;
}

// RECTANGLE

// конструкторы Rectangle
Rectangle::Rectangle(){};
Rectangle::Rectangle(const Point& a, const Point& b, double koef) {
  if (koef < 1) {
    koef = 1 / koef;
  }
  Point dir = b - a, center(0, 0);
  dir *= std::sqrt(dir.len() * dir.len() / (1 + koef * koef));
  dir /= (b - a).len();
  double angle = atan(koef) / geom::PI * geom ::PI_DEG;
  dir.rotate(center, angle);
  Point c = a + dir;
  Point d = b - dir;
  vertices = {a, c, b, d};
  n = 4;
}

// свои методы Rectangle
Point Rectangle::center() { return (vertices[0] + vertices[2]) / 2; }

std::pair<Line, Line> Rectangle::diagonals() {
  Line d1(vertices[0], vertices[2]);
  Line d2(vertices[1], vertices[3]);
  return {d1, d2};
}

// Наследуемые методы Rectangle
double Rectangle::perimeter() const {
  double a = (vertices[0] - vertices[1]).len();
  double b = (vertices[2] - vertices[1]).len();
  return 2 * (a + b);
}

double Rectangle::area() const {
  double a = (vertices[0] - vertices[1]).len();
  double b = (vertices[2] - vertices[1]).len();
  return a * b;
}

// SQUARE

// конструкторы Square
Square::Square(const Point& a, const Point& b) {
  Point dir = b - a, center(0, 0);
  dir *= std::sqrt(dir.len() * dir.len() / 2);
  dir /= (b - a).len();
  dir.rotate(center, 45);
  Point c = a + dir;
  Point d = b - dir;
  vertices = {a, c, b, d};
  n = 4;
}

// свои методы Square
Circle Square::circumscribedCircle() {
  double r = (vertices[2] - vertices[0]).len() / 2;
  Circle ans(center(), r);
  return ans;
}

Circle Square::inscribedCircle() {
  double r = (vertices[1] - vertices[0]).len() / 2;
  Circle ans(center(), r);
  return ans;
}

// TRIANGLE

// конструкторы Triangle
Triangle::Triangle(const Point& a, const Point& b, const Point& c) {
  vertices = {a, b, c};
  n = 3;
}

// свои методы Triangle
Circle Triangle::circumscribedCircle() {
  double s = area();
  double a = (vertices[2] - vertices[1]).len();
  double b = (vertices[2] - vertices[0]).len();
  double c = (vertices[1] - vertices[0]).len();
  double r = a * b * c / (4 * s);
  double alpha = (a * a / (8 * s * s)) *
                 ((vertices[0] - vertices[1]) * (vertices[0] - vertices[2]));
  double beta = (b * b / (8 * s * s)) *
                ((vertices[1] - vertices[0]) * (vertices[1] - vertices[2]));
  double gamma = (c * c / (8 * s * s)) *
                 ((vertices[2] - vertices[0]) * (vertices[2] - vertices[1]));
  Circle ans(vertices[0] * alpha + vertices[1] * beta + vertices[2] * gamma, r);
  return ans;
}

Circle Triangle::inscribedCircle() {
  double a = (vertices[2] - vertices[1]).len();
  double b = (vertices[2] - vertices[0]).len();
  double c = (vertices[1] - vertices[0]).len();
  Point center = vertices[0] * a + vertices[1] * b + vertices[2] * c;
  center /= (a + b + c);
  return Circle(center, 2 * area() / perimeter());
}

Point Triangle::centroid() {
  double x = vertices[0].x + vertices[1].x + vertices[2].x;
  double y = vertices[0].y + vertices[1].y + vertices[2].y;
  return {x / 3, y / 3};
}

Point Triangle::orthocenter() {
  Point m = (vertices[1] + vertices[2]) / 2;
  Circle circle = circumscribedCircle();
  Point center = circle.center();
  Point shift = m - center;
  return vertices[0] + shift * 2;
}

Line Triangle::EulerLine() {
  Line euler(centroid(), orthocenter());
  return euler;
};

Circle Triangle::ninePointsCircle() {
  Point a = vertices[0] + vertices[1];
  Point b = vertices[0] + vertices[2];
  Point c = vertices[1] + vertices[2];
  Triangle temp(a / 2, b / 2, c / 2);
  return temp.circumscribedCircle();
}
