/*-----------------------------------------.---------------------------------.
| Filename: ObjectBrowser.h                | The Object Browser              |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_OBJECT_BROWSER_H_
# define EXPLORER_COMPONENTS_OBJECT_BROWSER_H_

# include "ObjectProxyComponent.h"
# include "../Utilities/PropertyListDisplayComponent.h"
# include "../Utilities/ComponentWithPreferedSize.h"
# include "../Utilities/ObjectSelector.h"

namespace lbcpp
{

class ObjectSelectorAndContentComponent : public Component, public ObjectSelectorCallback, public ComponentWithPreferedSize
{
public:
  ObjectSelectorAndContentComponent(ObjectPtr object, Component* selector)
    : object(object), selector(selector), content(new ObjectProxyComponent())
  {
    ObjectSelector* s = dynamic_cast<ObjectSelector* >(selector);
    jassert(s);
    s->addCallback(*this);

    addAndMakeVisible(properties = new PropertyListDisplayComponent(40));
    properties->addProperty(T("Class"), object->getClassName());
    String name = object->getName();
    if (name.indexOf(T("unimplemented")) < 0)
      properties->addProperty(T("Name"), name);

    addAndMakeVisible(selector);
    addAndMakeVisible(resizeBar = new juce::StretchableLayoutResizerBar(&layout, 1, true));
    addAndMakeVisible(content);

    layout.setItemLayout(0, 10, -1, 200);
    double size = 4;
    layout.setItemLayout(1, size, size, size);
    layout.setItemLayout(2, 10, -1, -1);   
  }

  virtual ~ObjectSelectorAndContentComponent()
    {deleteAllChildren();}

  virtual void resized()
  {
    Component* comps[] = {selector, resizeBar, content};
    layout.layOutComponents(comps, 3, 0, 0, getWidth(), getHeight(), false, true);
    
    enum {propertiesHeight = 25};
    properties->setBounds(selector->getX(), 0, selector->getWidth(), propertiesHeight);
    selector->setBounds(selector->getX(), propertiesHeight, selector->getWidth(), getHeight() - propertiesHeight);
  }

  virtual void objectSelectedCallback(ObjectPtr object)
    {if (object != this->object) content->setObject(object);}

  enum {selectorPreferedWidth = 200};

  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
  {
    ComponentWithPreferedSize* content = dynamic_cast<ComponentWithPreferedSize* >(this->content);
    if (content)
    {
      int res = selector->getWidth() + resizeBar->getWidth();
      availableWidth -= res;
      return res + content->getPreferedWidth(availableWidth, availableHeight);
    }
    else
      return juce::jmax(selectorPreferedWidth, availableWidth);
  }

private:
  ObjectPtr object;
  PropertyListDisplayComponent* properties;
  Component* selector;
  Component* resizeBar;
  ObjectProxyComponent* content;
  juce::StretchableLayoutManager layout;
};

class ObjectBrowser : public ViewportComponent
{
public:
  ObjectBrowser(ObjectSelectorAndContentComponent* content)
    : ViewportComponent(content, false, true) {}
};

// todo: ranger
class FileObject : public Object
{
public:
  FileObject(const File& file)
    : file(file)
  {
    if (file.isDirectory())
    {
      file.findChildFiles(subDirectories, File::findDirectories, false);
      file.findChildFiles(subFiles, File::findFiles, false);
    }
  }

  virtual String getName() const
    {return file.getFileName();}

  virtual String toString() const
    {return file.loadFileAsString();}

  File getFile() const
    {return file;}

  enum Type
  {
    textFile,
    binaryFile,
    classFile,
    directory,
    classDirectory,
    nonexistent,
  };

  Type getType() const
  {
    if (!file.exists())
      return nonexistent;
    if (file.isDirectory())
      return file.getChildFile(T(".classFile")).exists() ? classDirectory : directory;
    
    if (file.getSize() > 100 * 1024 * 1024) // do not open files that do more than 100 Mb
      return binaryFile;

    const juce::MemoryBlock& data = getData();
    
    bool isTextFile = true;
    int indexOfZero = -1;
    String beforeZero;
    for (int i = 0; i < data.getSize(); ++i)
      if (data[i] == 0)
      {
        indexOfZero = i;
        break;
      }
      else
        beforeZero += data[i];
    if (indexOfZero < 0)
      return textFile;
    if (Object::doClassNameExists(beforeZero))
      return classFile;
    return binaryFile;
  }

  bool isDirectory() const
    {return file.isDirectory();}

  virtual void getChildrenObjects(std::vector< std::pair<String, ObjectPtr> >& subObjects) const
  {
    subObjects.clear();
    subObjects.reserve(subDirectories.size() + subFiles.size());
    for (int i = 0; i < subDirectories.size(); ++i)
      addChildrenObject(*subDirectories[i], subObjects);
    for (int i = 0; i < subFiles.size(); ++i)
      addChildrenObject(*subFiles[i], subObjects);
  }

  static void addChildrenObject(const File& subFile, std::vector< std::pair<String, ObjectPtr> >& res)
    {res.push_back(std::make_pair(subFile.getFileName(), ObjectPtr(new FileObject(subFile))));}

  const juce::MemoryBlock& getData() const
  {
    jassert(!file.isDirectory());
    if (data.getSize() == 0)
      file.loadFileAsData(const_cast<FileObject* >(this)->data);
    return data;
  }

private:
  File file;
  juce::OwnedArray<File> subDirectories;
  juce::OwnedArray<File> subFiles;
  juce::MemoryBlock data;
};

typedef ReferenceCountedObjectPtr<FileObject> FileObjectPtr;

class HexadecimalMemoryViewer : public Component, public ComponentWithPreferedSize
{
public:
  HexadecimalMemoryViewer(const juce::MemoryBlock& data)
    : data(data) {}

  enum
  {
    letterWidth = 25, letterHeight = 14,
    leftMargin = 80, rightMargin = 10,
    verticalMargin = 10,
  };

  int getSizePerLine(int availableWidth) const
  {
    int res = (availableWidth - leftMargin - rightMargin) / letterWidth;
    return (res / 10) * 10;
  }

  int getNumLines(int sizePerLine, juce::int64 totalSize) const
    {return (int)ceil((double)totalSize / (double)sizePerLine);}

  virtual int getPreferedHeight(int availableWidth, int availableHeight) const
    {return 2 * verticalMargin + getNumLines(getSizePerLine(availableWidth), data.getSize());}

  virtual void paint(Graphics& g)
  {
    g.fillAll(Colours::white);

    int sizePerLine = getSizePerLine(getWidth());
    int numLines = getNumLines(sizePerLine, data.getSize());
    int x = leftMargin;
    int y = verticalMargin;
    int xEnd = x + sizePerLine * letterWidth;

    g.setColour(Colours::grey);
    g.setFont(8);
    for (int i = 0; i < numLines; ++i)
      g.drawText(String(i * sizePerLine), 5, y, leftMargin - 10, letterHeight, Justification::centredRight, false);
    
    g.setColour(Colours::black);
    g.setFont(12);
    unsigned char* bytes = (unsigned char* )data.getData();
    for (int i = 0; i < numLines; ++i)
    {
      g.drawLine((float)x, (float)y, (float)xEnd, (float)y);
      int offset = i * sizePerLine;
      int size = juce::jmin(sizePerLine, data.getSize() - offset);
      int yEnd = y + letterHeight;
      int xc = x;
      for (int j = 0; j < size; ++j)
      {
        g.drawLine((float)xc, (float)y, (float)xc, (float)yEnd);
        g.drawText(String::toHexString(bytes + offset + j, 1), xc, y, letterWidth, letterHeight, Justification::centred, false);
        xc += letterWidth;
      }
      g.drawLine((float)xc, (float)y, (float)xc, (float)yEnd);
      y = yEnd;
    }
    g.drawLine((float)x, (float)y, (float)xEnd, (float)y);
  }

protected:
  const juce::MemoryBlock& data;
};

class HexadecimalFileObjectComponent : public ViewportComponent
{
public:
  HexadecimalFileObjectComponent(FileObjectPtr file, const String& name)
    : ViewportComponent(new HexadecimalMemoryViewer(file->getData()), true, false)
    {}
};


}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_OBJECT_BROWSER_H_

