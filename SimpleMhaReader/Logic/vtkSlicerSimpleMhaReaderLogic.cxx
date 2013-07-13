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

// SimpleMhaReader Logic includes
#include "vtkSlicerSimpleMhaReaderLogic.h"

// MRML includes

// VTK includes
#include <vtkNew.h>
#include <vtkImageImport.h>

// STD includes
#include <cassert>

// =======================================================
// Reading functions
// =======================================================
std::string getDir(const std::string& filename)
{
  #ifdef WIN32
  const char dlmtr = '\\';
  #else
  const char dlmtr = '/';
  #endif

  std::string dirName;
  size_t pos = filename.rfind(dlmtr);
  dirName = pos == string::npos ? "" : filename.substr(0, pos) + dlmtr;
  return dirName;
}


void readTrainFilenames( const string& filename, string& dirName, vector<string>& trainFilenames )
{

  trainFilenames.clear();

  ifstream file( filename.c_str() );
  if ( !file.is_open() )
    return;

  dirName = getDir(filename);
  while( !file.eof() )
  {
    string str; getline( file, str );
    if( str.empty() ) break;
    trainFilenames.push_back(str);
  }
  file.close();
}

int readImageDimensions_mha(const std::string& filename, int& cols, int& rows, int& count)
{
  ifstream file( filename.c_str() );
  if ( !file.is_open() )
    return 1;

  // Read until get dimensions
  while( !file.eof() )
  {
    string str; getline( file, str );
    if( str.empty() ) break;
    char *pch = &(str[0]);
    if( !pch )
    {
      return 1;
      file.close();
    }

    if( strstr( pch, "DimSize =" ) )
    {
      if( sscanf( pch, "DimSize = %d %d %d", &cols, &rows, &count ) != 3 )
      {
        printf( "Error: could not read dimensions\n" );
        file.close();
        return 1;
      }
      file.close();
      return 0;
    }
  }
  file.close();
  return 1;
}

void readImageTransforms_mha(const std::string& filename, std::vector<std::vector<float> >& transforms, std::vector<bool>& transformsValidity, std::vector<std::string>& filenames)
{
  std::string dirName = getDir(filename);
  filenames.clear();
  transforms.clear();

  // Vector for reading in transforms
  vector< float > vfTrans;
  vfTrans.resize(12);
  std::string pngFilename;

  ifstream file( filename.c_str() );
  if ( !file.is_open() )
    return;

  while( !file.eof() )
  {
    string str; getline( file, str );
    if( str.empty() ) break;
    char *pch = &(str[0]);
    if( !pch )
      return;

    if( strstr( pch, "ProbeToTrackerTransform =" )
      || strstr( pch, "UltrasoundToTrackerTransform =" ) )
    {
       // Parse name and transform
       // Seq_Frame0000_ProbeToTrackerTransform = -0.224009 -0.529064 0.818481 212.75 0.52031 0.6452 0.559459 -14.0417 -0.824074 0.551188 0.130746 -26.1193 0 0 0 1 

      char *pcName = pch;
      char *pcTrans = strstr( pch, "=" );
      pcTrans[-1] = 0; // End file name string pcName
       //pcTrans++; // Increment to just after equal sign

      pngFilename = dirName + pcName + ".png";// + pcTrans;

      char *pch = pcTrans;

      for( int j =0; j < 12; j++ )
      {
        pch = strchr( pch + 1, ' ' );
        if( !pch )
          return;
        vfTrans[j] = atof( pch );
        pch++;
      }
      transforms.push_back( vfTrans );
      filenames.push_back(pngFilename);
    }
    else if(strstr(pch, "UltrasoundToTrackerTransformStatus") || strstr(pch, "ProbeToTrackerTransformStatus")) {
      if(strstr(pch, "OK")){
        transformsValidity.push_back(true);
      }
      else if(strstr(pch, "INVALID"))
        transformsValidity.push_back(false);
    }
    if( strstr( pch, "ElementDataFile = LOCAL" ) )
    {
       // Done reading
      break;
    }
  }
}

void readImage_mha(const std::string& filename, int& firstFrame, int& lastFrame)
{
  int iImgCols = -1;
  int iImgRows = -1;
  int iImgCount = -1;
  if(readImageDimensions_mha(filename, iImgCols, iImgRows, iImgCount))
    return;

  FILE *infile = fopen( filename.c_str(), "rb" );
  char buffer[400];
  while( fgets( buffer, 400, infile ) )
  {
    if( strstr( buffer, "ElementDataFile = LOCAL" ) )
    {
       // Done reading
      break;
    }
  }

  unsigned char *pucImgData = new unsigned char[iImgRows*iImgCols];

  if(firstFrame < 0)
    firstFrame = 0;
  if(firstFrame >= iImgCount)
    firstFrame = iImgCount-1;
  if(lastFrame < 0 || lastFrame >= iImgCount)
    lastFrame = iImgCount-1;
  if(firstFrame > lastFrame)
    firstFrame = lastFrame;
   // Read & write images
  void *ptr = NULL;
  for( int i = 0; i < iImgCount; i++ )
  {
    if(i<firstFrame)
      fseek(infile, iImgRows*iImgCols, SEEK_CUR);
    else if(i>=firstFrame && i <= lastFrame) {
      fread( pucImgData, 1, iImgRows*iImgCols, infile );
    }
    else if(i>lastFrame)
      break;
  }
  delete [] pucImgData;

  fclose( infile );
}

void readImage_mha(const string& filename, int& frame, vtkImageData* data, void* dataPointer)
{
  int iImgCols = -1;
  int iImgRows = -1;
  int iImgCount = -1;
  if(readImageDimensions_mha(filename, iImgCols, iImgRows, iImgCount))
    return;

  FILE *infile = fopen( filename.c_str(), "rb" );
  char buffer[400];
  while( fgets( buffer, 400, infile ) )
  {
    if( strstr( buffer, "ElementDataFile = LOCAL" ) )
    {
       // Done reading
      break;
    }
  } 

  if(frame < 0)
    frame = iImgCount-1;
  if(frame >= iImgCount)
    frame = iImgCount-1;

  fseek(infile, iImgRows*iImgCols*frame, SEEK_CUR);
  fread( dataPointer, 1, iImgRows*iImgCols, infile );

  fclose( infile );
}



//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerSimpleMhaReaderLogic);

//----------------------------------------------------------------------------
vtkSlicerSimpleMhaReaderLogic::vtkSlicerSimpleMhaReaderLogic()
{
  this->imgData = NULL;
  this->dataPointer = NULL;
  this->imageNode = vtkMRMLScalarVolumeNode::New();
  this->imageNode->SetName("mha image");
}

//----------------------------------------------------------------------------
vtkSlicerSimpleMhaReaderLogic::~vtkSlicerSimpleMhaReaderLogic()
{
  if(this->dataPointer)
    delete [] this->dataPointer;
}

//----------------------------------------------------------------------------
void vtkSlicerSimpleMhaReaderLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerSimpleMhaReaderLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerSimpleMhaReaderLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerSimpleMhaReaderLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerSimpleMhaReaderLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerSimpleMhaReaderLogic
  ::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

void vtkSlicerSimpleMhaReaderLogic::setMhaPath(string path)
{
  if(path != this->mhaPath){
    this->mhaPath = path;
    this->transforms.clear();
    this->filenames.clear();
    this->transformsValidity.clear();
    this->currentFrame = 0;
    int iImgCols = -1;
    int iImgRows = -1;
    int iImgCount = -1;
    if(readImageDimensions_mha(this->mhaPath, iImgCols, iImgRows, iImgCount))
      return;
    if(this->dataPointer)
      delete [] this->dataPointer;
    this->dataPointer = new unsigned char[iImgRows*iImgCols];
    readImageTransforms_mha(this->mhaPath, this->transforms, this->transformsValidity, this->filenames);
    this->updateImage();
    this->Modified();
  }
}

int vtkSlicerSimpleMhaReaderLogic::getNumberOfFrames()
{
  return this->transformsValidity.size();
}

string vtkSlicerSimpleMhaReaderLogic::getCurrentTransformStatus()
{
  if(this->transformsValidity[this->currentFrame])
    return "OK";
  else
    return "INVALID";
}

void vtkSlicerSimpleMhaReaderLogic::updateImage()
{
  int iImgCols = -1;
  int iImgRows = -1;
  int iImgCount = -1;
  readImageDimensions_mha(this->mhaPath, iImgCols, iImgRows, iImgCount);
  readImage_mha(this->mhaPath, this->currentFrame, this->imgData, this->dataPointer);
  vtkSmartPointer<vtkImageImport> importer = vtkSmartPointer<vtkImageImport>::New();
  importer->SetDataScalarTypeToUnsignedChar();
  importer->SetImportVoidPointer(dataPointer,1); // Save argument to 1 won't destroy the pointer when importer destroyed
  importer->SetWholeExtent(0,iImgCols-1,0, iImgRows-1, 0, 0);
  importer->SetDataExtentToWholeExtent();
  importer->Update();
  this->imgData = importer->GetOutput();
  
  this->imageNode->SetAndObserveImageData(this->imgData);


  if(this->GetMRMLScene()) {
    if(!this->GetMRMLScene()->IsNodePresent(this->imageNode))
      this->GetMRMLScene()->AddNode(this->imageNode);
  }
}

void vtkSlicerSimpleMhaReaderLogic::nextImage()
{
  this->currentFrame += 1;
  if(this->currentFrame >= this->getNumberOfFrames())
    this->currentFrame = 0;
  this->updateImage();
  this->Modified();
}

void vtkSlicerSimpleMhaReaderLogic::previousImage()
{
  this->currentFrame -= 1;
  if(this->currentFrame < 0)
    this->currentFrame = this->getNumberOfFrames()-1;
  this->updateImage();
  this->Modified();
}

void vtkSlicerSimpleMhaReaderLogic::goToFrame(int frame)
{
  this->currentFrame = frame;
  if(this->currentFrame < 0)
    this->currentFrame = this->getNumberOfFrames()-1;
  else if(this->currentFrame >= this->getNumberOfFrames())
    this->currentFrame = 0;
  this->updateImage();
  this->Modified();
}

void vtkSlicerSimpleMhaReaderLogic::nextValidFrame()
{
  int frame = 0;
  for(int i=0; i<this->getNumberOfFrames(); i++)
  {
    frame = (this->currentFrame + i + 1)%this->getNumberOfFrames();
    if(this->transformsValidity[frame])
      break;
  }
  this->currentFrame = frame;
  this->updateImage();
  this->Modified();
}

void vtkSlicerSimpleMhaReaderLogic::previousValidFrame()
{
  int frame = 0;
  for(int i=0; i<this->getNumberOfFrames(); i++)
  {
    frame = this->currentFrame-i-1;
    if(frame < 0)
      frame = this->getNumberOfFrames() + frame;
    if(this->transformsValidity[frame])
      break;
  }
  this->currentFrame = frame;
  this->updateImage();
  this->Modified();
}

void vtkSlicerSimpleMhaReaderLogic::nextInvalidFrame()
{
  int frame = 0;
  for(int i=0; i<this->getNumberOfFrames(); i++)
  {
    frame = (this->currentFrame + i + 1)%this->getNumberOfFrames();
    if(!this->transformsValidity[frame])
      break;
  }
  this->currentFrame = frame;
  this->updateImage();
  this->Modified();
}

void vtkSlicerSimpleMhaReaderLogic::previousInvalidFrame()
{
  int frame = 0;
  for(int i=0; i<this->getNumberOfFrames(); i++)
  {
    frame = this->currentFrame-i-1;
    if(frame < 0)
      frame = this->getNumberOfFrames() + frame;
    if(!this->transformsValidity[frame])
      break;
  }
  this->currentFrame = frame;
  this->updateImage();
  this->Modified();
}
