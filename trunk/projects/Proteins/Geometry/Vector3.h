/*-----------------------------------------.---------------------------------.
| Filename: Vector3.h                      | Vector in a 3D-space            |
| Author  : Francis Maes                   |                                 |
| Started : 20/04/2010 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_GEOMETRY_VECTOR3_H_
# define LBCPP_PROTEIN_GEOMETRY_VECTOR3_H_

# include <lbcpp/Data/Variable.h>

extern "C"
{
  struct kdtree;
};

namespace lbcpp
{

namespace impl
{

class Vector3
{
public:
  Vector3(double x, double y, double z) : x(x), y(y), z(z) {}
  Vector3(double value) : x(value), y(value), z(value) {}
  Vector3() : x(DBL_MAX), y(DBL_MAX), z(DBL_MAX) {}

  bool exists() const
    {return x != DBL_MAX || y != DBL_MAX || z != DBL_MAX;}

  String toString() const
    {return String(x) + T(" ") + String(y) + T(" ") + String(z);}

  static Vector3 fromString(const String& str, MessageCallback& callback);

  double getX() const
    {return x;}

  void setX(double x)
    {this->x = x;}

  double getY() const
    {return y;}

  void setY(double y)
    {this->y = y;}

  double getZ() const
    {return z;}

  void setZ(double z)
    {this->z = z;}

  void set(double x, double y, double z)
    {this->x = x; this->y = y; this->z = z;}

  Vector3 operator +(const Vector3& otherVector) const
    {return Vector3(x + otherVector.x, y + otherVector.y, z + otherVector.z);}

  Vector3 operator -(const Vector3& otherVector) const
    {return Vector3(x - otherVector.x, y - otherVector.y, z - otherVector.z);}

  Vector3 operator *(const double value) const
    {return Vector3(x * value, y * value, z * value);}

  Vector3 operator /(const double value) const
  {
    jassert(value);
    double inverseValue = 1.0 / value;
    return (*this) * inverseValue;
  }

  Vector3& addWeighted(const Vector3& otherVector, double weight)
  {
    if (weight)
    {
      x += otherVector.x * weight;
      y += otherVector.y * weight;
      z += otherVector.z * weight;
    }
    return *this;    
  }

  Vector3& operator +=(const Vector3& otherVector)
  {
    x += otherVector.x;
    y += otherVector.y;
    z += otherVector.z;
    return *this;
  }

  Vector3& operator -=(const Vector3& otherVector)
  {
    x -= otherVector.x;
    y -= otherVector.y;
    z -= otherVector.z;
    return *this;
  }

  Vector3& operator *=(const double value)
    {x *= value; y *= value; z *= value; return *this;}

  Vector3& operator /=(const double value)
  {
    jassert(value);
    double inverseValue = 1.0 / value;
    return (*this) *= inverseValue;
  }

  double dotProduct(const Vector3& otherVector) const
    {return x * otherVector.x + y * otherVector.y + z * otherVector.z;}

  double angle(const Vector3& otherVector) const
  {
    double n1 = sumOfSquares();
    double n2 = otherVector.sumOfSquares();
    if (!n1 || !n2)
      return 0.0;
    return acos(dotProduct(otherVector) / sqrt(n1 * n2));
  }

  Vector3 crossProduct(const Vector3& otherVector) const
  {
    return Vector3(y * otherVector.z - otherVector.y * z,
                    z * otherVector.x - otherVector.z * x,
                    x * otherVector.y - otherVector.x * y);
  }

  double sumOfSquares() const
    {return x * x + y *y + z * z;}

  double l2norm() const
    {return sqrt(sumOfSquares());}

  Vector3 normalized() const
  {
    double s = sumOfSquares();
    if (s == 1)
      return *this;
    else
    {
      s = 1.0 / sqrt(s);
      return (*this) * s;
    }
  }

  bool operator ==(const Vector3& otherVector) const
    {return x == otherVector.x && y == otherVector.y && z == otherVector.z;}

  bool operator !=(const Vector3& otherVector) const
    {return x != otherVector.x || y != otherVector.y || z != otherVector.z;}

  double x, y, z;
};

}; /* namespace impl */

class Vector3 : public Object
{
public:
  Vector3(const impl::Vector3& value) : value(value) {}
  Vector3() {}

  impl::Vector3 getValue() const
    {return value;}

  impl::Vector3& getValue()
    {return value;}

  double getX() const
    {return value.x;}

  void setX(double x)
    {value.x = x;}

  double getY() const
    {return value.y;}

  void setY(double y)
    {value.y = y;}

  double getZ() const
    {return value.z;}

  void setZ(double z)
    {value.z = z;}

  // Object
  virtual String toString() const
    {return value.toString();}

  virtual bool loadFromString(const String& str, MessageCallback& callback)
    {value = impl::Vector3::fromString(str, callback); return true;}

private:
  friend class Vector3Class;
  impl::Vector3 value;
};

typedef ReferenceCountedObjectPtr<Vector3> Vector3Ptr;

extern ClassPtr vector3Class();

class Vector3KDTree : public Object
{
public:
  Vector3KDTree();
  virtual ~Vector3KDTree();

  void insert(size_t index, const impl::Vector3& position);
  void findPointsInSphere(const impl::Vector3& center, double radius, std::vector<size_t>& results);

private:
  struct kdtree* tree;
};
typedef ReferenceCountedObjectPtr<Vector3KDTree> Vector3KDTreePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_GEOMETRY_VECTOR3_H_
