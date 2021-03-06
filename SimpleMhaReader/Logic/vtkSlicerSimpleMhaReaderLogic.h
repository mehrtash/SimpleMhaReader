/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkSlicerSimpleMhaReaderLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerSimpleMhaReaderLogic_h
#define __vtkSlicerSimpleMhaReaderLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLLinearTransformNode.h>

// STD includes
#include <cstdlib>
#include <stdio.h>
#include <set>

// VTK includes
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>

// QT includes
#include <QTextEdit>

#include "vtkSlicerSimpleMhaReaderModuleLogicExport.h"

#include "util_macros.h"

using namespace std;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_SIMPLEMHAREADER_MODULE_LOGIC_EXPORT vtkSlicerSimpleMhaReaderLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerSimpleMhaReaderLogic *New();
  vtkTypeMacro(vtkSlicerSimpleMhaReaderLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkSlicerSimpleMhaReaderLogic();
  virtual ~vtkSlicerSimpleMhaReaderLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
  virtual void ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void * callData );
private:

  vtkSlicerSimpleMhaReaderLogic(const vtkSlicerSimpleMhaReaderLogic&); // Not implemented
  void operator=(const vtkSlicerSimpleMhaReaderLogic&);               // Not implemented
  void printUSToImageTransform();
  void checkFrame();
  
  // Attributes
private:
  string mhaPath;
  vector<vector<float> > transforms;
  vector<string> filenames;
  vector<bool> transformsValidity;
  set<string> availableTransforms;
  
  vtkMRMLLinearTransformNode* USToImageTransformNode;
  vtkSmartPointer<vtkMatrix4x4> USToImageTransform;
  vtkSmartPointer<vtkImageData> imgData;
  unsigned char* dataPointer;
  vtkMRMLScalarVolumeNode* imageNode;
  int imageWidth;
  int imageHeight;
  int currentFrame;
  int numberOfFrames;
  bool applyTransforms;
  string playMode;
  
  QTextEdit* console;
  
public:
  // Read image logic
  void readImage_mha();
  void setTransformToIdentity();
  void setApplyTransforms(bool);
  void setUSToImageTransform();
  void setMhaPath(string path);
  string getCurrentTransformStatus();
  void updateImage();
  void nextImage();
  void nextValidFrame();
  void goToFrame(int);
  void previousValidFrame();
  void nextInvalidFrame();
  void previousInvalidFrame();
  void randomFrame();
  void previousImage();
  void playNext();
  void saveToPng(const std::string filepath);
  
  // Getters and Setters
  string getMhaPath();
  GET(int, imageWidth, ImageWidth);
  GET(int, imageHeight, ImageHeight);
  GET(int, currentFrame, CurrentFrame);
  GET(int, numberOfFrames, NumberOfFrames);
  GET(set<string>, availableTransforms, AvailableTransforms);
  GET(vtkMatrix4x4*, USToImageTransform, USToImageTransform);
  GETSET(QTextEdit*, console, Console);
  GETSET(string, playMode, PlayMode);
  GET(bool, applyTransforms, ApplyTransforms);
  
};

#endif
