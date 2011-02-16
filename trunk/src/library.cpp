/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: library.cpp                    | LBcpp library functions         |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 18:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/library.h>
#include <lbcpp/Core/TypeManager.h>
#include <lbcpp/Core/Library.h>

#ifdef LBCPP_USER_INTERFACE
# include <lbcpp/UserInterface/UserInterfaceManager.h>
#endif

using namespace lbcpp;

namespace juce
{
  extern void juce_setCurrentExecutableFileName(const String& filename) throw();
};

namespace lbcpp
{

extern void coreLibraryCacheTypes(ExecutionContext& context);
extern void coreLibraryUnCacheTypes();
extern void lbCppLibraryCacheTypes(ExecutionContext& context);
extern void lbCppLibraryUnCacheTypes();

class LibraryManager
{
public:
  ~LibraryManager()
    {shutdown();}

  void preShutdown()
  {
    for (size_t i = 0; i < libraries.size(); ++i)
    {
      libraries[i].first->uncacheTypes();
      libraries[i].first = LibraryPtr();
      void* handle = libraries[i].second;
      if (handle)
      {
        typedef void (*DeinitializeLibraryFunction)();

        DeinitializeLibraryFunction deinitializeFunction = (DeinitializeLibraryFunction)juce::PlatformUtilities::getProcedureEntryPoint(handle, T("lbcppDeinitializeLibrary"));
        if (deinitializeFunction)
          deinitializeFunction();
      }
    }
  }

  void shutdown()
  {
    /*for (size_t i = 0; i < libraries.size(); ++i)
      if (libraries[i].second)
        juce::PlatformUtilities::freeDynamicLibrary(libraries[i].second);*/
    libraries.clear();
  }

  bool addLibrary(ExecutionContext& context, const LibraryPtr& library, void* handle)
  {
    if (library->getName().isEmpty())
    {
      context.errorCallback(T("Cannot add an unnamed library"));
      return false;
    }

    for (size_t i = 0; i < libraries.size(); ++i)
      if (libraries[i].first->getName() == library->getName())
      {
        context.errorCallback(T("The library called ") + library->getName() + T(" already exists"));
        return false;
      }

    libraries.push_back(std::make_pair(library, handle));
    return true;
  }

  const LibraryPtr& getLibrary(ExecutionContext& context, const String& name) const
  {
    for (size_t i = 0; i < libraries.size(); ++i)
      if (libraries[i].first->getName() == name)
        return libraries[i].first;

    static LibraryPtr empty;
    context.errorCallback(T("Could not find library ") + name.quoted());
    return empty;
  }

  size_t getNumLibraries() const
    {return libraries.size();}

  LibraryPtr getLibrary(size_t index) const
    {jassert(index < libraries.size()); return libraries[index].first;}

private:
  std::vector< std::pair<LibraryPtr, void* > > libraries;
};

struct ApplicationContext
{
  LibraryManager libraryManager;
  TypeManager typeManager;
  ExecutionContextPtr defaultExecutionContext;
#ifdef LBCPP_USER_INTERFACE
  UserInterfaceManager userInterfaceManager;
#endif
};

ApplicationContext* applicationContext = NULL;

extern lbcpp::LibraryPtr coreLibrary();
extern lbcpp::LibraryPtr lbCppLibrary();

}; /* namespace lbcpp */ 

void lbcpp::initialize(const char* executableName)
{
  // juce
  juce::initialiseJuce_NonGUI();
  // FIXME:
  //juce::juce_setCurrentExecutableFileName(String::fromUTF8((const juce::uint8* )executableName));

  // application context
  jassert(!applicationContext);
  applicationContext = new ApplicationContext();
  applicationContext->defaultExecutionContext = defaultConsoleExecutionContext();
  
  // types
  importLibrary(coreLibrary());
  importLibrary(lbCppLibrary());
  topLevelType = anyType = variableType;
}

void lbcpp::deinitialize()
{
  if (applicationContext)
  {
    applicationContext->defaultExecutionContext = ExecutionContextPtr();

    // pre shutdown types
    applicationContext->libraryManager.preShutdown();
    coreLibraryUnCacheTypes();
    lbCppLibraryUnCacheTypes();
    topLevelType = anyType = TypePtr();

    // shutdown types
    applicationContext->typeManager.shutdown();

    applicationContext->libraryManager.shutdown();
#ifdef LBCPP_USER_INTERFACE
    applicationContext->userInterfaceManager.shutdown();
#endif
    deleteAndZero(applicationContext);
    juce::shutdownJuce_NonGUI();
  }
}

TypeManager& lbcpp::typeManager()
  {jassert(applicationContext); return applicationContext->typeManager;}

#ifdef LBCPP_USER_INTERFACE
UserInterfaceManager& lbcpp::userInterfaceManager()
  {jassert(applicationContext); return applicationContext->userInterfaceManager;}
#endif

ExecutionContext& lbcpp::defaultExecutionContext()
  {jassert(applicationContext); return *applicationContext->defaultExecutionContext;}

void lbcpp::setDefaultExecutionContext(ExecutionContextPtr defaultContext)
  {jassert(applicationContext); applicationContext->defaultExecutionContext = defaultContext;}

size_t lbcpp::getNumLibraries()
  {jassert(applicationContext); return applicationContext->libraryManager.getNumLibraries();}
  
LibraryPtr lbcpp::getLibrary(size_t index)
  {jassert(applicationContext); return applicationContext->libraryManager.getLibrary(index);}

bool lbcpp::importLibrariesFromDirectory(ExecutionContext& executionContext, const File& directory)
{
  juce::OwnedArray<File> files;
  directory.findChildFiles(files, File::findFiles | File::ignoreHiddenFiles, false, T("*.dll"));
  directory.findChildFiles(files, File::findFiles | File::ignoreHiddenFiles, false, T("*.so"));
  directory.findChildFiles(files, File::findFiles | File::ignoreHiddenFiles, false, T("*.dylib"));
  ProgressionStatePtr progression(new ProgressionState(0.0, (double)files.size(), T("Dynamic Libraries")));
  for (int i = 0; i < files.size(); ++i)
  {
    File file = *files[i];
    progression->setValue((double)i);
    executionContext.informationCallback(T("Loading dynamic library ") + file.getFullPathName());
    executionContext.progressCallback(progression);
    importLibraryFromFile(executionContext, file);
  }
  return true;
}

LibraryPtr lbcpp::importLibraryFromFile(ExecutionContext& context, const File& file)
{
  void* handle = juce::PlatformUtilities::loadDynamicLibrary(file.getFullPathName());
  if (!handle)
  {
    context.errorCallback(T("Could not open dynamic library ") + file.getFullPathName());
    return LibraryPtr();
  }
 
  typedef Library* (*InitializeLibraryFunction)(lbcpp::ApplicationContext& applicationContext);

  InitializeLibraryFunction initializeFunction = (InitializeLibraryFunction)juce::PlatformUtilities::getProcedureEntryPoint(handle, T("lbcppInitializeLibrary"));
  if (!initializeFunction)
  {
    context.informationCallback(T("Skipping ") + file.getFileName());
    juce::PlatformUtilities::freeDynamicLibrary(handle);
    return LibraryPtr();
  }

  LibraryPtr res = (*initializeFunction)(*applicationContext);
  res->decrementReferenceCounter();
  jassert(res->getReferenceCount() == 1);
  if (!res)
  {
    context.errorCallback(T("Load ") + file.getFileName(), T("Could not find create library"));
    return LibraryPtr();
  }

  if (!importLibrary(context, res, handle))
  {
    juce::PlatformUtilities::freeDynamicLibrary(handle);
    return LibraryPtr();
  }

  return res;
}

bool lbcpp::importLibrary(ExecutionContext& context, LibraryPtr library, void* dynamicLibraryHandle)
{
  if (!library->initialize(context))
    return false;
  if (!applicationContext->libraryManager.addLibrary(context, library, dynamicLibraryHandle))
    return false;
  library->cacheTypes(context);
  typeManager().finishDeclarations(context);
  return true;
}

// called from dynamic library
void lbcpp::initializeDynamicLibrary(lbcpp::ApplicationContext& applicationContext)
{
#ifdef JUCE_WIN32
  lbcpp::applicationContext = &applicationContext;
  ExecutionContext& context = defaultExecutionContext();

  coreLibraryCacheTypes(context);
  lbCppLibraryCacheTypes(context);
  topLevelType = anyType = variableType;
#else
  jassert(lbcpp::applicationContext == &applicationContext);
#endif
}

void lbcpp::deinitializeDynamicLibrary()
{
#ifdef JUCE_WIN32
  coreLibraryUnCacheTypes();
  lbCppLibraryUnCacheTypes();
  topLevelType = anyType = TypePtr();
  jassert(lbcpp::applicationContext);
  lbcpp::applicationContext = NULL;
#endif // JUCE_WIN32
}
