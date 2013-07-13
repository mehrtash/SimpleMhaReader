cmake_minimum_required(VERSION 2.8.9)

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  set(EXTENSION_NAME SimpleMhaReader)
  set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/SimpleMhaReader")
  set(EXTENSION_CATEGORY "Examples")
  set(EXTENSION_CONTRIBUTORS "Jean-Christophe Fillion-Robin (Kitware)")
  set(EXTENSION_DESCRIPTION "This is an example of extension bundling a loadable module")
  set(EXTENSION_ICONURL "http://viewvc.slicer.org/viewvc.cgi/Slicer4/trunk/Extensions/Testing/SimpleMhaReader/SimpleMhaReader.png?revision=21746&view=co")
  set(EXTENSION_SCREENSHOTURLS "http://wiki.slicer.org/slicerWiki/images/4/42/Slicer-r19441-SimpleMhaReader-screenshot.png")
endif()

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
endif()

#-----------------------------------------------------------------------------
add_subdirectory(SimpleMhaReader)

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  include(${Slicer_EXTENSION_CPACK})
endif()
