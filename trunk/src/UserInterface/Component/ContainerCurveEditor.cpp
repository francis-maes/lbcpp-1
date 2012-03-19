/*-----------------------------------------.---------------------------------.
| Filename: ContainerCurveEditor.cpp       | Container Curve Editor          |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 20:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ContainerCurveEditor.h"
#include <lbcpp/Data/RandomGenerator.h>
#include <lbcpp/Function/Evaluator.h>
using namespace lbcpp;

using juce::ComboBox;
using juce::Drawable;
using juce::AffineTransform;
using juce::Rectangle;
using juce::RectanglePlacement;
using juce::Justification;

class ContainerCurveDrawable : public Drawable
{
public:
  ContainerCurveDrawable(const ContainerPtr& table, const ContainerCurveEditorConfigurationPtr& configuration)
    : table(table), configuration(configuration)
  {
    selectedCurves = configuration->getSelectedCurves();
    table->makeOrder(configuration->getKeyVariableIndex(), true, order);
    computeHorizontalBounds();
    computeVerticalBounds();
    if (!isNumberValid(boundsX) || !isNumberValid(boundsY) || !isNumberValid(boundsWidth) || !isNumberValid(boundsHeight))
    {
      boundsX = -1000.0;
      boundsWidth = 2000.0;
      boundsY = -1.0;
      boundsHeight = 2.0;
    }
  }

  enum
  {
    pointCrossSize = 6,
    frameMarkerSize1 = 8,
    frameMarkerSize2 = 5,
    bottomValuesSize = 20,
  };

  virtual Drawable* createCopy() const
    {return new ContainerCurveDrawable(table, configuration);}

  virtual bool hitTest(float x, float y) const
    {return true;}

  virtual void getBounds(float& x, float& y, float& width, float& height) const
  {
    x = (float)boundsX;
    y = (float)(boundsY + boundsHeight);
    width = (float)boundsWidth;
    height = (float)-boundsHeight;
  }

  virtual void draw(Graphics& g, const AffineTransform& transform = AffineTransform::identity) const
  {
    static const double epsilon = 1e-9;
    if (fabs(boundsWidth) < epsilon || fabs(boundsHeight) < epsilon)
      return;

    drawZeroAxis(g, transform, configuration->getYAxis(), false);
    drawZeroAxis(g, transform, configuration->getXAxis(), true);
    drawFrame(g, transform);

    if (table->getNumElements() == 0)
      return;

    for (size_t i = 0; i < selectedCurves.size(); ++i)
    {
      drawCurveLine(g, transform, selectedCurves[i], configuration->getCurve(selectedCurves[i]));
      drawCurvePoint(g, transform, selectedCurves[i], configuration->getCurve(selectedCurves[i])); 
    }
  }

  juce::Rectangle getFrameRectangle(const AffineTransform& transform) const
  {
    float x, y, width, height;
    getBounds(x, y, width, height);
    float x2 = x + width, y2 = y + height;
    transform.transformPoint(x, y);
    transform.transformPoint(x2, y2);
    return juce::Rectangle((int)(x + 0.5), (int)(y + 0.5), (int)(x2 - x + 0.5f), (int)(y2 - y + 0.5f));
  }

  void drawFrameMarker(Graphics& g, const AffineTransform& transform, double value, bool isHorizontal, bool isMainMarker) const
  {
    int dirh = isHorizontal ? 1 : 0;
    int dirv = isHorizontal ? 0 : 1;
    float frameMarkerHalfSize = (isMainMarker ? frameMarkerSize1 : frameMarkerSize2) / 2.f;

    float x, y;
    if (isHorizontal)
      x = (float)value, y = (float)(boundsY + boundsHeight);
    else
      x = (float)(boundsX + boundsWidth), y = (float)value;
    transform.transformPoint(x, y);

    g.drawLine(x - dirv * frameMarkerHalfSize, y - dirh * frameMarkerHalfSize, x + dirv * frameMarkerHalfSize, y + dirh * frameMarkerHalfSize);

    if (isHorizontal)
      x = (float)value, y = (float)boundsY;
    else
      x = (float)boundsX, y = (float)value;
    transform.transformPoint(x, y);
    g.drawLine(x - dirv * frameMarkerHalfSize, y - dirh * frameMarkerHalfSize, x + dirv * frameMarkerHalfSize, y + dirh * frameMarkerHalfSize);

    if (isMainMarker)
    {
      g.setFont(11);
      String str = Variable(value).toShortString();
      if (isHorizontal)
        g.drawText(str, (int)(x - 40), (int)(y + 5), 80, bottomValuesSize - 5, Justification::centred, false);
      else
        g.drawText(str, (int)(x - 60), (int)(y - 10), 80, 20, Justification::centred, false);
    }
  }

  void drawFrameMarkers(Graphics& g, const AffineTransform& transform, juce::Rectangle frameRectangle, bool isHorizontal) const
  {
    double minValue, maxValue, valuePerPixel;
    if (isHorizontal)
      minValue = boundsX, maxValue = boundsX + boundsWidth, valuePerPixel = boundsWidth / frameRectangle.getWidth();
    else
      minValue = boundsY, maxValue = boundsY + boundsHeight, valuePerPixel = boundsHeight / frameRectangle.getHeight();
    
    std::vector<double> mainMarkers, secondaryMarkers;
    getMarkerValues(minValue, maxValue, valuePerPixel, mainMarkers, secondaryMarkers);

    g.setColour(Colours::lightgrey);
    for (size_t i = 0; i < secondaryMarkers.size(); ++i)
      drawFrameMarker(g, transform, secondaryMarkers[i], isHorizontal, false);
    g.setColour(Colours::black);
    for (size_t i = 0; i < mainMarkers.size(); ++i)
      drawFrameMarker(g, transform, mainMarkers[i], isHorizontal, true);
  }

  void drawFrame(Graphics& g, const AffineTransform& transform) const
  {
    juce::Rectangle rect = getFrameRectangle(transform);
    
    drawFrameMarkers(g, transform, rect, true);
    drawFrameMarkers(g, transform, rect, false);

    g.setColour(Colours::black);
    g.drawRect(rect.getX(), rect.getY(), rect.getWidth() + 1, rect.getHeight() + 1, 1);

    g.setFont(16);
    g.drawText(getXAxisLabel(), rect.getX(), rect.getBottom() + bottomValuesSize, rect.getWidth(), 20, Justification::centred, false);

    String yAxisLabel = getYAxisLabel();
    float transx = (float)rect.getX() - 40.f;
    float transy = (float)rect.getY() + rect.getHeight() * (0.75f + juce::jmin(1.f, yAxisLabel.length() / 50.f)) / 2.f;
    g.drawTextAsPath(yAxisLabel, AffineTransform::rotation(-(float)(M_PI / 2.f)).translated(transx, transy));
  }
  
  String getXAxisLabel() const
  {
    String res = configuration->getXAxis()->getLabel();
    if (res.isEmpty())
      res = table->getElementsType()->getMemberVariableName(configuration->getKeyVariableIndex());
    return res;
  }

  String getYAxisLabel() const
  {
    String res = configuration->getYAxis()->getLabel();
    if (res.isEmpty())
    {
      for (size_t i = 0; i < selectedCurves.size(); ++i)
      {
        if (i > 0)
          res += T(", ");
        res += table->getElementsType()->getMemberVariableName(selectedCurves[i]);
      }
    }
    return res;
  }

  void drawZeroAxis(Graphics& g, const AffineTransform& transform, CurveAxisConfigurationPtr config, bool isHorizontalAxis) const
  {
    float x0 = 0.f, y0 = 0.f;
    transform.transformPoint(x0, y0);

    juce::Rectangle frameRect = getFrameRectangle(transform);

    g.setColour(Colours::lightgrey);
    if (isHorizontalAxis)
    {
      if (0 >= boundsY && 0 <= boundsY + boundsHeight)
        g.drawLine((float)frameRect.getX(), y0, (float)frameRect.getRight(), y0);
    }
    else
    {
      if (0 >= boundsX && 0 <= boundsX + boundsWidth)
        g.drawLine(x0, (float)frameRect.getY(), x0, (float)frameRect.getBottom());
    }
  }

  static bool isNumberValid(double number)
    {return lbcpp::isNumberValid(number) && fabs(number) < 1e20;}

  void drawCurveLine(Graphics& g, const AffineTransform& transform, size_t index, CurveVariableConfigurationPtr config) const
  {
    size_t n = table->getNumElements();
    size_t keyVariableIndex = configuration->getKeyVariableIndex();
    float x0, y0;
    bool isP0Valid = getPointPosition(0, keyVariableIndex, index, transform, x0, y0) && isNumberValid(x0) && isNumberValid(y0);

    g.setColour(config->getColour());
    for (size_t i = 1; i < n; ++i)
    {
      float x1, y1;
      bool isP1Valid = getPointPosition(i, keyVariableIndex, index, transform, x1, y1) && isNumberValid(x1) && isNumberValid(y1);
      if (isP0Valid && isP1Valid && fabs(y0) < 1e16 && fabs(y1) < 1e16)
        g.drawLine(x0, y0, x1, y1); // juce has a problem with very very large values, so we restrict y-values to 1e16
      x0 = x1;
      y0 = y1;
      isP0Valid = isP1Valid;
    }
  }

  void drawCurvePoint(Graphics& g, const AffineTransform& transform, size_t index, CurveVariableConfigurationPtr config) const
  {
    size_t n = table->getNumElements();
    size_t keyVariableIndex = configuration->getKeyVariableIndex();

    g.setColour(config->getColour());
    for (size_t i = 0; i < n; ++i)
    {
      float x, y;
      float crossHalfSize = pointCrossSize / 2.f;
      if (getPointPosition(i, keyVariableIndex, index, transform, x, y) && isNumberValid(x) && isNumberValid(y))
      {
        g.drawLine(x - crossHalfSize, y, x + crossHalfSize, y);
        g.drawLine(x, y - crossHalfSize, x, y + crossHalfSize);
      }
    }
  }

protected:
  ContainerPtr table;
  ContainerCurveEditorConfigurationPtr configuration;

  std::vector<size_t> selectedCurves;
  std::vector<size_t> order;
  double boundsX, boundsY, boundsWidth, boundsHeight;

  bool getPointPosition(size_t row, size_t columnX, size_t columnY, const AffineTransform& transform, float& x, float& y) const
  {
    ObjectPtr rowObject = table->getElement(order[row]).getObject();
    double dx = 0.0, dy = 0.0;
    if (!getTableScalarValue(rowObject, columnX, dx) || !getTableScalarValue(rowObject, columnY, dy))
      return false;

    x = (float)dx;
    y = (float)dy;
    transform.transformPoint(x, y);
    return true;
  }

  bool getTableScalarValue(const ObjectPtr& row, size_t column, double& scalarValue) const
  {
    Variable v = row->getVariable(column);
    if (!v.exists() || !v.isConvertibleToDouble())
      return false;
    scalarValue = v.toDouble();
    return true;
  }

  void getTableValueRange(size_t column, double& minValue, double& maxValue) const
  {
    double value = 0.0;
    for (size_t i = 0; i < table->getNumElements(); ++i)
      if (getTableScalarValue(table->getElement(i).getObject(), column, value))
      {
        if (value > maxValue)
          maxValue = value;
        if (value < minValue)
          minValue = value;
      }
  }

  void computeHorizontalBounds()
  {
    CurveAxisConfigurationPtr axis = configuration->getXAxis();
    if (axis->hasAutoRange())
    {
      double minValue = DBL_MAX, maxValue = -DBL_MAX;
      getTableValueRange(configuration->getKeyVariableIndex(), minValue, maxValue);

      if (maxValue > minValue)
      {
        double length = maxValue - minValue;
        boundsX = minValue - length * 0.1;
        boundsWidth = length * 1.2;
        axis->setRangeMin(boundsX);
        axis->setRangeMax(boundsX + boundsWidth);
        return;
      }
      else if (maxValue == minValue)
      {
        boundsX = minValue - 1.1;
        boundsWidth = 2.2;
        axis->setRangeMin(boundsX);
        axis->setRangeMax(boundsX + boundsWidth);
        return;
      }
    }
    boundsX = axis->getRangeMin();
    boundsWidth = axis->getRangeMax() - axis->getRangeMin();
  }

  void computeVerticalBounds()
  {
    CurveAxisConfigurationPtr axis = configuration->getYAxis();
    if (axis->hasAutoRange())
    {
      double minValue = DBL_MAX, maxValue = -DBL_MAX;
      for (size_t i = 0; i < selectedCurves.size(); ++i)
        getTableValueRange(selectedCurves[i], minValue, maxValue);
      if (maxValue > minValue)
      {
        double length = maxValue - minValue;
        boundsY = minValue - length * 0.1;
        boundsHeight = length * 1.2;
        axis->setRangeMin(boundsY);
        axis->setRangeMax(boundsY + boundsHeight);
        return;
      }
      else if (maxValue == minValue)
      {
        boundsY = minValue - 1.1;
        boundsHeight = 2.2;
        axis->setRangeMin(boundsY);
        axis->setRangeMax(boundsY + boundsHeight);
        return;
      }
    }
    boundsY = axis->getRangeMin();
    boundsHeight = axis->getRangeMax() - axis->getRangeMin();
  }

  static void getMarkerValues(double minValue, double maxValue, double valuePerPixel,
                              std::vector<double>& main, std::vector<double>& secondary)
  {
    int powerOfTen = (int)(log10(valuePerPixel) + 0.9);
    while (main.size() < 2)
    {
      main.clear();
      secondary.clear();

      double step = pow(10.0, (double)powerOfTen);
      int ibegin = (int)(minValue / step);
      int iend = (int)(maxValue / step);
      for (int i = ibegin; i <= iend; ++i)
      {
        double time = i * step;
        if (time >= minValue && time <= maxValue)
        {
          if (i % 10 == 0)
            main.push_back(time);
          else
            secondary.push_back(time);
        }
      }

      --powerOfTen; // if there are not enough main markers, retry with a more finner scale
    }
  }
};

class ContainerCurveEditorContentComponent : public Component
{
public:
  ContainerCurveEditorContentComponent(const ContainerPtr& table, const ContainerCurveEditorConfigurationPtr& configuration)
  {
    drawable = new ContainerCurveDrawable(table, configuration);
  }
  virtual ~ContainerCurveEditorContentComponent()
    {delete drawable;}

  enum
  {
    leftMargin = 60,
    rightMargin = 20,
    topMargin = 20,
    bottomMargin = 40
  };

  virtual void paint(Graphics& g)
    {drawable->drawWithin(g, leftMargin, topMargin, getWidth() - leftMargin - rightMargin, getHeight() - topMargin - bottomMargin, juce::RectanglePlacement::stretchToFit);}

protected:
  ContainerCurveDrawable* drawable;
};
//////////////////////////////////////////

class ContainerCurveSelectorConfigurationComponent : public BooleanButtonsComponent
{
public:
  ContainerCurveSelectorConfigurationComponent(ContainerCurveEditorConfigurationPtr configuration)
  {
    std::vector<ConfigurationButton* > buttonsColumn;

    TypePtr rowType = configuration->getRowType();
    for (size_t i = 0; i < rowType->getNumMemberVariables(); ++i)
    {
      CurveVariableConfigurationPtr curve = configuration->getCurve(i);
      if (curve)
        addToggleButton(buttonsColumn, rowType->getMemberVariableName(i), curve->isSelected(), 4, curve->getColour());
    }
    flushButtons(buttonsColumn);

    initialize();
  }
};

class CurveListBoxModel : public juce::ListBoxModel
{
public:
  CurveListBoxModel(const ContainerCurveEditorConfigurationPtr& configuration, juce::ChangeBroadcaster* callback)
    : configuration(configuration), callback(callback)
  {
    const size_t n = configuration->getNumCurves();
    for (size_t i = 0; i < n; ++i)
    {
      CurveVariableConfigurationPtr curve = configuration->getCurve(i);
      if (curve)
        curves.push_back(curve);
    }
  }

  virtual int getNumRows()
    {return curves.size();}

  virtual void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
  {
    if (curves[rowNumber]->isSelected())
    {
      g.setColour(juce::Colours::lightblue);
      g.fillRect(0, 0, width, height);
    }

    g.setColour(curves[rowNumber]->getColour());
    g.drawText(curves[rowNumber]->getLabel(), 0, 0, width, height, juce::Justification(juce::Justification::left), true);
  }

  virtual void listBoxItemClicked(int row, const juce::MouseEvent& e)
  {
    curves[row]->setSelected(!curves[row]->isSelected());
    listBox->deselectRow(row);
    callback->sendSynchronousChangeMessage(this);
  }

  void setListBox(juce::ListBox* listBox)
    {this->listBox = listBox;}

protected:
  ContainerCurveEditorConfigurationPtr configuration;
  juce::ChangeBroadcaster* callback;
  juce::ListBox* listBox;
  std::vector<CurveVariableConfigurationPtr> curves;
};

class ContainerCurveEditorConfigurationComponent : public Component, public juce::ChangeBroadcaster,
                                                   public juce::ComboBoxListener, public juce::ChangeListener,
                                                   public juce::ButtonListener, public juce::TextEditorListener
{
public:
  ContainerCurveEditorConfigurationComponent(ContainerCurveEditorConfigurationPtr configuration)
    : configuration(configuration)
  {
    addAndMakeVisible(xAxisLabel = new juce::Label(T("xaxis"), T("X-Axis")));
    addAndMakeVisible(keyComboBox = createVariableIndexComboBox(configuration->getKeyVariableIndex()));
    addAndMakeVisible(yAxisLabel = new juce::Label(T("yaxis"), T("Y-Axis")));
    // FIXME: Who will destroy curveListBox ?
    CurveListBoxModel* curveListBox = new CurveListBoxModel(configuration, this);
    addAndMakeVisible(yListBox = new juce::ListBox(T("yListBox"), curveListBox));
    curveListBox->setListBox(yListBox);
    yListBox->setOutlineThickness(1);
    yListBox->setRowHeight(15);
    yListBox->setMultipleSelectionEnabled(false);

    addAndMakeVisible(selectAllButton = new juce::TextButton(T("Select All")));
    selectAllButton->addButtonListener(this);
    addAndMakeVisible(noneButton = new juce::TextButton(T("None")));
    noneButton->addButtonListener(this);

    addAndMakeVisible(scalesLabel = new juce::Label(T("scales"), T("Scales")));
    addAndMakeVisible(xMinScaleLabel = new juce::Label(T("xMinScaleLabel"), T("x-Min")));
    addAndMakeVisible(xMaxScaleLabel = new juce::Label(T("xMaxScaleLabel"), T("x-Max")));
    addAndMakeVisible(yMinScaleLabel = new juce::Label(T("yMinScaleLabel"), T("y-Min")));
    addAndMakeVisible(yMaxScaleLabel = new juce::Label(T("yMaxScaleLabel"), T("y-Max")));

    addAndMakeVisible(xMinScaleTextEditor = new juce::TextEditor(T("xMinScaleTextEditor")));
    addAndMakeVisible(xMaxScaleTextEditor = new juce::TextEditor(T("xMaxScaleTextEditor")));
    addAndMakeVisible(yMinScaleTextEditor = new juce::TextEditor(T("yMinScaleTextEditor")));
    addAndMakeVisible(yMaxScaleTextEditor = new juce::TextEditor(T("yMaxScaleTextEditor")));

    xMinScaleTextEditor->addListener(this);
    xMaxScaleTextEditor->addListener(this);
    yMinScaleTextEditor->addListener(this);
    yMaxScaleTextEditor->addListener(this);

    addChangeListener(this);
  }

  virtual ~ContainerCurveEditorConfigurationComponent()
    {deleteAllChildren();}

  virtual void buttonClicked(juce::Button* button)
  {
    if (button == selectAllButton || button == noneButton)
    {
      const size_t n = configuration->getNumCurves();
      size_t index = 0;
      for (size_t i = 0; i < n; ++i)
      {
        if (configuration->getCurve(i))
        {
          configuration->getCurve(i)->setSelected(button == selectAllButton);
          yListBox->selectRow(index, true);
          ++index;
        }
      }
      configuration->getCurve(configuration->getKeyVariableIndex())->setSelected(false);
      sendSynchronousChangeMessage(this);
    }
  }

  virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged)
  {
    if (comboBoxThatHasChanged == keyComboBox)
      configuration->setKeyVariableIndex(keyComboBox->getSelectedId() - 1);
    sendSynchronousChangeMessage(this);
  }
  
  virtual void textEditorEscapeKeyPressed(juce::TextEditor& editor) {}
  virtual void textEditorTextChanged(juce::TextEditor &editor) {}
  virtual void textEditorFocusLost(juce::TextEditor& editor) {}

  virtual void textEditorReturnKeyPressed(juce::TextEditor& editor)
  {
    double value = editor.getText().getDoubleValue();
    if (&editor == xMinScaleTextEditor)
    {
        configuration->getXAxis()->setAutoRange(false);
        configuration->getXAxis()->setRangeMin(value);
    }
    else if (&editor == xMaxScaleTextEditor)
    {
        configuration->getXAxis()->setAutoRange(false);
        configuration->getXAxis()->setRangeMax(value);
    }
    else if (&editor == yMinScaleTextEditor)
    {
        configuration->getYAxis()->setAutoRange(false);
        configuration->getYAxis()->setRangeMin(value);
    }
    else if (&editor == yMaxScaleTextEditor)
    {
        configuration->getYAxis()->setAutoRange(false);
        configuration->getYAxis()->setRangeMax(value);
    }
    sendSynchronousChangeMessage(this);
  }

  virtual void changeListenerCallback(void* objectThatHasChanged)
  {
    updateAxisRanges();
  }

  virtual void resized()
  {
    xAxisLabel->setBoundsRelative(0, 0, 0.3f, 0.5f);
    xAxisLabel->setSize(xAxisLabel->getWidth(), 20);
    keyComboBox->setBoundsRelative(0, 0.22f, 0.3f, 0.5f);
    keyComboBox->setSize(keyComboBox->getWidth(), 20);

    yAxisLabel->setBoundsRelative(0.35f, 0, 0.3f, 0.5f);
    yAxisLabel->setSize(yAxisLabel->getWidth(), 20);
    yListBox->setBoundsRelative(0.35f, 0.22f, 0.3f, 1.f);
    yListBox->setSize(yListBox->getWidth(), (int)(yListBox->getHeight() - 0.22f * getHeight()));

    //selectedCurves->setBoundsRelative(0.4f, 0, 0.6f, 1.f);
    selectAllButton->setBoundsRelative(0.7f, 0.22f, 0.3f, 0.5f);
    selectAllButton->setSize(selectAllButton->getWidth(), 20);
    noneButton->setBoundsRelative(0.7f, 0.44f, 0.3f, 0.5f);
    noneButton->setSize(noneButton->getWidth(), 20);

    scalesLabel->setBoundsRelative(0, 0.44f, 0.3f, 0.5f);
    scalesLabel->setSize(scalesLabel->getWidth(), 20);
    xMinScaleLabel->setBoundsRelative(0.01f, 0.60f, 0.07f, 0.5f);
    xMinScaleLabel->setSize(xMinScaleLabel->getWidth(), 20);
    xMaxScaleLabel->setBoundsRelative(0.16f, 0.60f, 0.07f, 0.5f);
    xMaxScaleLabel->setSize(xMaxScaleLabel->getWidth(), 20);
    yMinScaleLabel->setBoundsRelative(0.01f, 0.80f, 0.07f, 0.5f);
    yMinScaleLabel->setSize(yMinScaleLabel->getWidth(), 20);
    yMaxScaleLabel->setBoundsRelative(0.16f, 0.80f, 0.07f, 0.5f);
    yMaxScaleLabel->setSize(yMaxScaleLabel->getWidth(), 20);

    xMinScaleTextEditor->setBoundsRelative(0.07f, 0.60f, 0.08f, 0.5f);
    xMinScaleTextEditor->setSize(xMinScaleTextEditor->getWidth(), 20);
    xMaxScaleTextEditor->setBoundsRelative(0.22f, 0.60f, 0.08f, 0.5f);
    xMaxScaleTextEditor->setSize(xMinScaleTextEditor->getWidth(), 20);
    yMinScaleTextEditor->setBoundsRelative(0.07f, 0.80f, 0.08f, 0.5f);
    yMinScaleTextEditor->setSize(xMinScaleTextEditor->getWidth(), 20);
    yMaxScaleTextEditor->setBoundsRelative(0.22f, 0.80f, 0.08f, 0.5f);
    yMaxScaleTextEditor->setSize(xMinScaleTextEditor->getWidth(), 20);
  }

  void updateAxisRanges()
  {
    xMinScaleTextEditor->setText(String(configuration->getXAxis()->getRangeMin()));
    xMaxScaleTextEditor->setText(String(configuration->getXAxis()->getRangeMax()));
    yMinScaleTextEditor->setText(String(configuration->getYAxis()->getRangeMin()));
    yMaxScaleTextEditor->setText(String(configuration->getYAxis()->getRangeMax()));
  }

protected:
  ContainerCurveEditorConfigurationPtr configuration;

  juce::Label* xAxisLabel;
  ComboBox* keyComboBox;
  juce::Label* yAxisLabel;
  juce::ListBox* yListBox;
  juce::Button* selectAllButton;
  juce::Button* noneButton;

  juce::Label* scalesLabel;
  juce::Label* xMinScaleLabel;
  juce::Label* xMaxScaleLabel;
  juce::Label* yMinScaleLabel;
  juce::Label* yMaxScaleLabel;

  juce::TextEditor* xMinScaleTextEditor;
  juce::TextEditor* xMaxScaleTextEditor;
  juce::TextEditor* yMinScaleTextEditor;
  juce::TextEditor* yMaxScaleTextEditor;

  juce::ComboBox* createVariableIndexComboBox(size_t selectedIndex)
  {
    TypePtr type = configuration->getRowType();
    ComboBox* res = new ComboBox(T("combo"));
    for (size_t i = 0; i < type->getNumMemberVariables(); ++i)
      if (configuration->getCurve(i))
      {
        res->addItem(type->getMemberVariableName(i), i + 1);
        if (selectedIndex == i)
          res->setSelectedId(i + 1);
      }
    res->addListener(this);
    return res;
  }
};

//////////////////////////////////////////
/*
** ContainerCurveEditorConfiguration
*/

using juce::Colour;
using juce::Colours;
inline Colour randomColour()
{
  RandomGeneratorPtr random = defaultExecutionContext().getRandomGenerator();
  return Colour(random->sampleByte(), random->sampleByte(), random->sampleByte(), (unsigned char)255);
}

ContainerCurveEditorConfiguration::ContainerCurveEditorConfiguration(ClassPtr rowType)
  : rowType(rowType), 
    xAxis(new CurveAxisConfiguration(0.0, 1000.0)),
    yAxis(new CurveAxisConfiguration(0.0, 1.0)),
    variables(rowType->getNumMemberVariables())
{
  const juce::Colour defaultColours[] = {
      Colours::red, Colours::green, Colours::blue, Colours::black,
      Colours::yellow.withBrightness(0.5f), Colours::cyan, Colours::violet, Colours::grey,
      Colours::darkred, Colours::darkgreen, Colours::darkblue,
  };
  const size_t numDefaultColours = sizeof (defaultColours) / sizeof (juce::Colour);

  keyVariableIndex = (size_t)-1;
  for (size_t i = 0; i < variables.size(); ++i)
  {
    TypePtr variableType = rowType->getMemberVariableType(i);
    if (variableType->isConvertibleToDouble())
    {
      if (keyVariableIndex == (size_t)-1)
        keyVariableIndex = i; // default x-variable
      variables[i] = new CurveVariableConfiguration(false, i < numDefaultColours ? defaultColours[i] : randomColour(), rowType->getMemberVariableName(i));
    }
  }
  // select y-variable
  for (size_t i = keyVariableIndex + 1; i < variables.size(); ++i)
    if (variables[i])
    {
      variables[i]->setSelected(true);
      break;
    }
}

std::vector<size_t> ContainerCurveEditorConfiguration::getSelectedCurves() const
{
  std::vector<size_t> res;
  for (size_t i = 0; i < variables.size(); ++i)
    if (variables[i] && variables[i]->isSelected())
      res.push_back(i);
  return res;
}

/*
** ContainerCurveEditor
*/
ContainerCurveEditor::ContainerCurveEditor(ExecutionContext& context, ContainerPtr table, ContainerCurveEditorConfigurationPtr configuration)
  : ObjectEditor(table, configuration, false, false), table(table)
{
  initialize();
}

Component* ContainerCurveEditor::createConfigurationComponent(const ObjectPtr& configuration)
{
  return new ContainerCurveEditorConfigurationComponent(configuration.staticCast<ContainerCurveEditorConfiguration>());
}

Component* ContainerCurveEditor::createContentComponent(const ObjectPtr& object, const ObjectPtr& configuration)
{
  return new ContainerCurveEditorContentComponent(object.staticCast<Container>(), configuration.staticCast<ContainerCurveEditorConfiguration>());
}
