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
#include <vtkTransform.h>
#include <vtkPngWriter.h>

// STD includes
#include <cassert>
#include <ctime>

// vnl include
#include <vnl/vnl_double_3.h>

// ==============================================
// Helpers - conversion functions
// ==============================================
vnl_matrix<double> convertVnlVectorToMatrix(const vnl_double_3& v)
{
  vnl_matrix<double> result(3,1);
  result(0,0) = v[0];
  result(1,0) = v[1];
  result(2,0) = v[2];
  return result;
}

vnl_double_3 convertVnlMatrixToVector(const vnl_matrix<double>& m)
{
  vnl_double_3 result;
  if(m.rows()==1 && m.cols()==3) {
    for(int i=0; i<3; i++)
      result[i] = m(0,i);
  }
  else if(m.rows()==3 && m.cols()==1) {
    for(int i=0; i<3; i++)
      result[i] = m(i,0);
  }
  return result;
}

vnl_double_3 arrayToVnlDouble(double arr[4])
{
  vnl_double_3 result;
  result[0]=arr[0];
  result[1]=arr[1];
  result[2]=arr[2];
  return result;
}

void vnlToArrayDouble(vnl_double_3 v, double arr[4])
{
  arr[0]=v[0];
  arr[1]=v[1];
  arr[2]=v[2];
  arr[3]=1.0;
}

std::vector<float> vtkToStdMatrix(vtkMatrix4x4* matrix)
{
  std::vector<float> result;
  for(int i=0; i<3; i++)
  {
    for(int j=0; j<4; j++)
      result.push_back(matrix->GetElement(i,j));
  }

  return result;
}

void vnlToVtkMatrix(const vnl_matrix<double> vnlMatrix , vtkMatrix4x4* vtkMatrix)
{
  vtkMatrix->Identity();
  int rows = vnlMatrix.rows();
  int cols = vnlMatrix.cols();
  if(rows > 4)
    rows = 4;
  if(cols > 4)
    cols = 4;
  for(int i=0; i<rows; i++)
  {
    for(int j=0; j<cols; j++)
      vtkMatrix->SetElement(i,j,vnlMatrix(i,j));
  }
}

void getVtkMatrixFromVector(const std::vector<float>& vec, vtkMatrix4x4* vtkMatrix)
{
  vtkMatrix->Identity();
  if(vec.size() < 12)
    return;
  for(int i=0; i<3; i++)
    for(int j=0; j<4; j++)
    vtkMatrix->SetElement(i,j,vec[i*4+j]);
}

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

void readImageTransforms_mha(const std::string& filename, std::vector<std::vector<float> >& transforms, set<string>& availableTransforms,  std::vector<bool>& transformsValidity, std::vector<std::string>& filenames)
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
      
    if(strstr(pch, "Seq_Frame") && strstr(pch, "Transform")){
      char* transformName = strstr(pch, "Seq_Frame")+string("Seq_Frame").size()+5;
      char* endTransformName = strstr(pch, "Transform");
      std::string transformNameCpy = string(transformName, endTransformName-transformName);
      availableTransforms.insert(transformNameCpy);
    }

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
  for(set<string>::iterator it=availableTransforms.begin(); it!=availableTransforms.end(); it++)
    cout << *it << endl;
}

void vtkSlicerSimpleMhaReaderLogic::readImage_mha()
{
  
  FILE *infile = fopen( this->mhaPath.c_str(), "rb" );
  char buffer[400];
  
  // Just move the pointer where data starts
  while( fgets( buffer, 400, infile ) )
  {
    if( strstr( buffer, "ElementDataFile = LOCAL" ) )
    {
       // Done reading
      break;
    }
  } 
  
  #ifdef WIN32
  _fseeki64(infile, (__int64)this->imageHeight*(__int64)this->imageWidth*(__int64)this->currentFrame, SEEK_CUR);
  #else
  fseek(infile, (long int)this->imageHeight*(long int)this->imageWidth*(long int)this->currentFrame, SEEK_CUR);
  #endif
  
  fread( this->dataPointer, 1, this->imageHeight*this->imageWidth, infile );
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
  this->imageWidth = 0;
  this->imageHeight = 0;
  this->numberOfFrames = 0;
  this->applyTransforms = false;
  this->playMode = "Forwards";
  
  // Initialize Image to Probe transform
  this->USToImageTransformNode = vtkMRMLLinearTransformNode::New();
  this->USToImageTransformNode->SetName("US to Image Transform");
  this->USToImageTransform = vtkSmartPointer<vtkMatrix4x4>::New();
  this->USToImageTransform->Identity();
  // this->USToImageTransform->SetElement(0,0,0.107535);
  //   this->USToImageTransform->SetElement(0,1,0.00094824);
  //   this->USToImageTransform->SetElement(0,2,0.0044213);
  //   this->USToImageTransform->SetElement(0,3,-65.9013);
  //   this->USToImageTransform->SetElement(1,0,0.0044901);
  //   this->USToImageTransform->SetElement(1,1,-0.00238041);
  //   this->USToImageTransform->SetElement(1,2,-0.106347);
  //   this->USToImageTransform->SetElement(1,3,-3.05698);
  //   this->USToImageTransform->SetElement(2,0,-0.000844189);
  //   this->USToImageTransform->SetElement(2,1,0.105271);
  //   this->USToImageTransform->SetElement(2,2,-0.00244457);
  //   this->USToImageTransform->SetElement(2,3,-17.1613);
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

void vtkSlicerSimpleMhaReaderLogic::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void * callData )
{
  if ( caller == NULL )
  {
    return;
  }
  if(event == vtkMRMLTransformableNode::TransformModifiedEvent) {
    vtkMRMLLinearTransformNode* tnode = vtkMRMLLinearTransformNode::SafeDownCast(caller);
    if(!tnode)
      return;
    this->console->insertPlainText("Changed Transform\n");
    this->updateImage();
  }
  else
    this->Superclass::ProcessMRMLNodesEvents( caller, event, callData );
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
    this->imageWidth = iImgCols;
    this->imageHeight = iImgRows;
    this->numberOfFrames = iImgCount;
    if(this->GetMRMLScene()) {
      if(!this->GetMRMLScene()->IsNodePresent(this->USToImageTransformNode))
        this->GetMRMLScene()->AddNode(this->USToImageTransformNode);
    }
    this->setUSToImageTransform();
    if(this->dataPointer)
      delete [] this->dataPointer;
    this->dataPointer = new unsigned char[iImgRows*iImgCols];
    readImageTransforms_mha(this->mhaPath, this->transforms, this->availableTransforms, this->transformsValidity, this->filenames);
    std::ostringstream oss;
    oss << "Number of transforms found: " << this->transforms.size() << endl;
    oss << "Number of transform validity: " << this->transformsValidity.size() <<endl;
    this->console->insertPlainText(oss.str().c_str());
    this->updateImage();
    this->Modified();
  }
}


string vtkSlicerSimpleMhaReaderLogic::getCurrentTransformStatus()
{
  if(this->transformsValidity.size() == 0)
    return "Unknown";
  if(this->transformsValidity[this->currentFrame])
    return "OK";
  else
    return "INVALID";
}

void vtkSlicerSimpleMhaReaderLogic::updateImage()
{
  checkFrame();
  
  ostringstream oss;
  clock_t beginTime = clock();

  readImage_mha();

  clock_t endTime = clock();
  double intervalInMiliSeconds = (double)(endTime - beginTime)/(double) CLOCKS_PER_SEC * 1000.;
  oss << "Reading took: " << intervalInMiliSeconds << " ms" << endl;
  this->console->insertPlainText(oss.str().c_str());
  oss.clear(); oss.str("");
  beginTime = endTime;


  vtkSmartPointer<vtkImageImport> importer = vtkSmartPointer<vtkImageImport>::New();
  importer->SetDataScalarTypeToUnsignedChar();
  importer->SetImportVoidPointer(dataPointer,1); // Save argument to 1 won't destroy the pointer when importer destroyed
  importer->SetWholeExtent(0,this->imageWidth-1,0, this->imageHeight-1, 0, 0);
  importer->SetDataExtentToWholeExtent();
  importer->Update();
  this->imgData = importer->GetOutput();
  
  if(this->transforms.size() > 0 && this->applyTransforms)
  {
    vtkSmartPointer<vtkMatrix4x4> transform = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkSmartPointer<vtkTransform> combinedTransform = vtkSmartPointer<vtkTransform>::New();
    vtkSmartPointer<vtkMatrix4x4> imageToUSTransform = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkMatrix4x4::Invert(this->USToImageTransform, imageToUSTransform);
    getVtkMatrixFromVector(this->transforms[this->currentFrame], transform);
    combinedTransform->Concatenate(transform);
    combinedTransform->Concatenate(imageToUSTransform);
    vtkSmartPointer<vtkMatrix4x4> matrix = combinedTransform->GetMatrix();
    // Makes a deep copy of the matrix
    this->imageNode->SetIJKToRASMatrix(matrix);
  }

  endTime = clock();
  intervalInMiliSeconds = (double)(endTime - beginTime)/(double) CLOCKS_PER_SEC * 1000.;
  oss << "Importing to vtk took : " << intervalInMiliSeconds << " ms" << endl;
  this->console->insertPlainText(oss.str().c_str());
  oss.clear(); oss.str("");
  beginTime = endTime;
  
  this->imageNode->SetAndObserveImageData(this->imgData);
  if(this->GetMRMLScene()) {
    if(!this->GetMRMLScene()->IsNodePresent(this->imageNode))
      this->GetMRMLScene()->AddNode(this->imageNode);
  }
  
  endTime = clock();
  intervalInMiliSeconds = (double)(endTime - beginTime)/(double) CLOCKS_PER_SEC * 1000.;
  oss << "Setting node's data took: " << intervalInMiliSeconds << " ms" << endl;
  this->console->insertPlainText(oss.str().c_str());
  oss.clear(); oss.str("");
  beginTime = endTime;
}

void vtkSlicerSimpleMhaReaderLogic::nextImage()
{
  this->currentFrame += 1;
  this->updateImage();
  this->Modified();
}

void vtkSlicerSimpleMhaReaderLogic::previousImage()
{
  this->currentFrame -= 1;
  this->updateImage();
  this->Modified();
}

void vtkSlicerSimpleMhaReaderLogic::goToFrame(int frame)
{
  this->currentFrame = frame;
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

void vtkSlicerSimpleMhaReaderLogic::randomFrame()
{
  this->currentFrame = rand()%this->getNumberOfFrames();
  this->updateImage();
  this->Modified();
}

void vtkSlicerSimpleMhaReaderLogic::checkFrame()
{
  if(this->currentFrame >= this->numberOfFrames)
    this->currentFrame = 0;
  if(this->currentFrame < 0)
    this->currentFrame = this->getNumberOfFrames()-1;
}


void vtkSlicerSimpleMhaReaderLogic::playNext()
{
  cout << this->playMode;
  if(this->playMode == "Forwards")
    this->nextImage();
  else if(this->playMode == "Backwards")
    this->previousImage();
  else if(this->playMode == "Random")
    this->randomFrame();
}

void vtkSlicerSimpleMhaReaderLogic::setTransformToIdentity()
{
  if(this->imageNode)
  {
    vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
    matrix->Identity();
    this->imageNode->SetIJKToRASMatrix(matrix);
  }
}

void vtkSlicerSimpleMhaReaderLogic::setUSToImageTransform()
{
  this->USToImageTransform->Identity();
  if(this->imageWidth == 1280 && this->imageHeight == 1024) {
    // Transform as calculated by Andras
    //this->USToImageTransform->SetElement(0,0,9.4);
    //this->USToImageTransform->SetElement(0,1,0);
    //this->USToImageTransform->SetElement(0,2,0);
    //this->USToImageTransform->SetElement(0,3,613.);
    //this->USToImageTransform->SetElement(1,0,0);
    //this->USToImageTransform->SetElement(1,1,0);
    //this->USToImageTransform->SetElement(1,2,9.4);
    //this->USToImageTransform->SetElement(1,3,165.);
    //this->USToImageTransform->SetElement(2,0,0);
    //this->USToImageTransform->SetElement(2,1,-9.4);
    //this->USToImageTransform->SetElement(2,2,0);
    //this->USToImageTransform->SetElement(2,3,0);

    // transform as calculated by me...
    //this->USToImageTransform->SetElement(0,0,9.4);
    //this->USToImageTransform->SetElement(0,1,0);
    //this->USToImageTransform->SetElement(0,2,0);
    //this->USToImageTransform->SetElement(0,3,612.);
    //this->USToImageTransform->SetElement(1,0,0);
    //this->USToImageTransform->SetElement(1,1,0);
    //this->USToImageTransform->SetElement(1,2,9.4);
    //this->USToImageTransform->SetElement(1,3,150.);
    //this->USToImageTransform->SetElement(2,0,0);
    //this->USToImageTransform->SetElement(2,1,-9.4);
    //this->USToImageTransform->SetElement(2,2,0);
    //this->USToImageTransform->SetElement(2,3,0);

    // transform as calculated by Plus...
    this->USToImageTransform->SetElement(0,0,9.2825);
    this->USToImageTransform->SetElement(0,1,0.38763);
    this->USToImageTransform->SetElement(0,2,-0.074848);
    this->USToImageTransform->SetElement(0,3,611.63);
    this->USToImageTransform->SetElement(1,0,0.083496);
    this->USToImageTransform->SetElement(1,1,-0.21476);
    this->USToImageTransform->SetElement(1,2,9.4937);
    this->USToImageTransform->SetElement(1,3,167.77);
    this->USToImageTransform->SetElement(2,0,0.39005);
    this->USToImageTransform->SetElement(2,1,-9.382);
    this->USToImageTransform->SetElement(2,2,-0.21566);
    this->USToImageTransform->SetElement(2,3,-6.6768);
    
  }
  else if(this->imageWidth == 1920 && this->imageHeight == 1200) {
    this->USToImageTransform->SetElement(0,0,11);
    this->USToImageTransform->SetElement(0,1,0.);
    this->USToImageTransform->SetElement(0,2,0.);
    this->USToImageTransform->SetElement(0,3,932);
    this->USToImageTransform->SetElement(1,0,0.);
    this->USToImageTransform->SetElement(1,1,0.);
    this->USToImageTransform->SetElement(1,2,11);
    this->USToImageTransform->SetElement(1,3,172.);
    this->USToImageTransform->SetElement(2,0,0.);
    this->USToImageTransform->SetElement(2,1,-11);
    this->USToImageTransform->SetElement(2,2,0.);
    this->USToImageTransform->SetElement(2,3,0.);
  }
  
  this->USToImageTransformNode->SetAndObserveMatrixTransformToParent(this->USToImageTransform);
  
  int wasModifying = this->StartModify();
  vtkMRMLLinearTransformNode* newNode = NULL;
  vtkSmartPointer< vtkIntArray > events = vtkSmartPointer< vtkIntArray >::New();
  events->InsertNextValue( vtkMRMLTransformableNode::TransformModifiedEvent );
  vtkSetAndObserveMRMLNodeEventsMacro( newNode, this->USToImageTransformNode, events );
  this->USToImageTransformNode = newNode;
  this->EndModify( wasModifying );
  
  this->printUSToImageTransform();
}

void vtkSlicerSimpleMhaReaderLogic::printUSToImageTransform()
{
  for(int i=0; i<4; i++) {
    ostringstream oss;
    for(int j=0; j<4; j++){
      oss <<this->USToImageTransform->GetElement(i,j) << " ";
    }
    oss << "\n";
    this->console->insertPlainText(oss.str().c_str());
  }
}

void vtkSlicerSimpleMhaReaderLogic::setApplyTransforms(bool value)
{
  if(value != this->applyTransforms) {
    this->applyTransforms = value;
    this->updateImage();
  }
  this->applyTransforms = value;
  
}

string vtkSlicerSimpleMhaReaderLogic::getMhaPath()
{
  return this->mhaPath;
}

void vtkSlicerSimpleMhaReaderLogic::saveToPng(const std::string filepath)
{
  if(!this->imgData)
    return;
  vtkSmartPointer<vtkPNGWriter> writer =vtkSmartPointer<vtkPNGWriter>::New();
  writer->SetFileName(filepath.c_str());
  writer->SetInput(this->imgData);
  writer->Write();
}