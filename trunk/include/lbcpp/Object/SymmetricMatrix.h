/*-----------------------------------------.---------------------------------.
| Filename: SymmetricMatrix.h              | Symmetric Matrix of variables   |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_SYMMETRIC_MATRIX_H_
# define LBCPP_OBJECT_SYMMETRIC_MATRIX_H_

# include "Vector.h"

namespace lbcpp
{

// matrix = container of rows, row = container of Variables
class SymmetricMatrix : public Container 
{
public:
  SymmetricMatrix(TypePtr contentType, size_t dimension = 0);
  SymmetricMatrix() {}

  /*
  ** SymmetricMatrix
  */
  size_t getDimension() const
    {return dimension;}

  void setElement(size_t i, size_t j, const Variable& value)
    {values.setVariable(getIndex(i, j), value);}

  Variable getElement(size_t i, size_t j) const
    {return values.getVariable(getIndex(i, j));}

  /*
  ** Container
  */
  TypePtr getMatrixElementsType() const
    {jassert(thisClass); return thisClass->getTemplateArgument(0);}

  virtual TypePtr getElementsType() const;

  virtual size_t getNumVariables() const
    {return dimension;}
  
  virtual Variable getVariable(size_t index) const;
  virtual void setVariable(size_t index, const Variable& value);

  /*
  ** Object
  */
  virtual String toString() const;
  virtual String getShortSummary() const;
  virtual void saveToXml(XmlElement* xml) const;
  virtual bool loadFromXml(XmlElement* xml, ErrorHandler& callback);

private:
  Vector values;
  size_t dimension;

  size_t getIndex(size_t i, size_t j) const;
};

typedef ReferenceCountedObjectPtr<SymmetricMatrix> SymmetricMatrixPtr;

extern ClassPtr symmetricMatrixClass(TypePtr elementsType);

class SymmetricMatrixRow : public Container
{
public:
  SymmetricMatrixRow(SymmetricMatrixPtr matrix, size_t rowNumber)
    : Container(matrix->getElementsType()), matrix(matrix), rowNumber(rowNumber) {}
  SymmetricMatrixRow() {}

  virtual TypePtr getElementsType() const
    {jassert(thisClass); return thisClass->getTemplateArgument(0);}

  virtual size_t getNumVariables() const
    {return matrix->getDimension();}
  
  virtual Variable getVariable(size_t index) const
    {return matrix->getElement(rowNumber, index);}

  virtual void setVariable(size_t index, const Variable& value)
    {matrix->setElement(rowNumber, index, value);}

protected:
  SymmetricMatrixPtr matrix;
  size_t rowNumber;
};

extern ClassPtr symmetricMatrixRowClass(TypePtr elementsType);


}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_SYMMETRIC_MATRIX_H_
