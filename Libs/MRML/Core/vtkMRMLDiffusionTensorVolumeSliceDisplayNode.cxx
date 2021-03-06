/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLDiffusionTensorVolumeSliceDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <vtkImageData.h>

#include "vtkDiffusionTensorGlyph.h"

#include "vtkTransformPolyDataFilter.h"

#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLDisplayableNode.h"

vtkCxxSetReferenceStringMacro(vtkMRMLDiffusionTensorVolumeSliceDisplayNode, DiffusionTensorDisplayPropertiesNodeID);

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLDiffusionTensorVolumeSliceDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeSliceDisplayNode::vtkMRMLDiffusionTensorVolumeSliceDisplayNode()
  :vtkMRMLGlyphableVolumeSliceDisplayNode()
{

  // Enumerated
  this->DiffusionTensorDisplayPropertiesNode = NULL;
  this->DiffusionTensorDisplayPropertiesNodeID = NULL;


  this->DiffusionTensorGlyphFilter = vtkDiffusionTensorGlyph::New();
  this->DiffusionTensorGlyphFilter->SetInput(this->SliceImage);
  this->DiffusionTensorGlyphFilter->SetResolution (1);

  this->ColorMode = this->colorModeScalar;

  this->UpdatePolyDataPipeline();
}


//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeSliceDisplayNode::~vtkMRMLDiffusionTensorVolumeSliceDisplayNode()
{
  this->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(NULL);
  this->DiffusionTensorGlyphFilter->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::WriteXML(ostream& of, int nIndent)
{

  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->DiffusionTensorDisplayPropertiesNodeID != NULL)
    {
    of << indent << " DiffusionTensorDisplayPropertiesNodeRef=\"" << this->DiffusionTensorDisplayPropertiesNodeID << "\"";
    }


}


//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "DiffusionTensorDisplayPropertiesNodeRef"))
      {
      this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(attValue);
      }
    }

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLDiffusionTensorVolumeSliceDisplayNode *node = (vtkMRMLDiffusionTensorVolumeSliceDisplayNode *) anode;

  this->SetDiffusionTensorDisplayPropertiesNodeID(node->DiffusionTensorDisplayPropertiesNodeID);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
 //int idx;

  Superclass::PrintSelf(os,indent);
//  os << indent << "ColorMode:             " << this->ColorMode << "\n";
}
//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SetSliceGlyphRotationMatrix(vtkMatrix4x4 *matrix)
{
  this->DiffusionTensorGlyphFilter->SetTensorRotationMatrix(matrix);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SetSlicePositionMatrix(vtkMatrix4x4 *matrix)
{
  // We need to call vtkDiffusionTensorGlyph::SetVolumePositionMatrix BEFORE
  // calling Superclass::SetSlicePositionMatrix(matrix)
  // because the later fire the even Modified() wich will update the pipeline
  // and execute the filter that needs to be up-to-date.
  this->DiffusionTensorGlyphFilter->SetVolumePositionMatrix(matrix);
  Superclass::SetSlicePositionMatrix(matrix);
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SetSliceImage(vtkImageData *image)
{
  this->DiffusionTensorGlyphFilter->SetInput(image);
  //this->DiffusionTensorGlyphFilter->SetDimensions(image ? image->GetDimensions(): 0);

  Superclass::SetSliceImage(image);
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLDiffusionTensorVolumeSliceDisplayNode::GetOutputPort()
{
  return this->DiffusionTensorGlyphFilter->GetOutputPort();
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::UpdatePolyDataPipeline()
{
  // set display properties according to the tensor-specific display properties node for glyphs
  vtkMRMLDiffusionTensorDisplayPropertiesNode * DiffusionTensorDisplayNode = this->GetDiffusionTensorDisplayPropertiesNode( );

  this->Superclass::UpdatePolyDataPipeline();

  if (DiffusionTensorDisplayNode == NULL ||
      this->SliceImage == NULL ||
      DiffusionTensorDisplayNode->GetGlyphGeometry( ) == vtkMRMLDiffusionTensorDisplayPropertiesNode::Superquadrics)
    {
    this->ScalarVisibilityOff();
    return;
    }

  // TO DO: need filter to calculate FA, average FA, etc. as requested

  // get tensors from the fiber bundle node and glyph them
  // TO DO: include superquadrics
  // if glyph type is other than superquadrics, get glyph source
  this->DiffusionTensorGlyphFilter->ClampScalingOff();

  // TO DO: implement max # ellipsoids, random sampling features
  this->DiffusionTensorGlyphFilter->SetResolution(1);
  this->DiffusionTensorGlyphFilter->SetDimensionResolution( DiffusionTensorDisplayNode->GetLineGlyphResolution(), DiffusionTensorDisplayNode->GetLineGlyphResolution());
  this->DiffusionTensorGlyphFilter->SetScaleFactor( DiffusionTensorDisplayNode->GetGlyphScaleFactor( ) );

  this->DiffusionTensorGlyphFilter->SetSource( DiffusionTensorDisplayNode->GetGlyphSource( ) );

  vtkDebugMacro("setting glyph geometry" << DiffusionTensorDisplayNode->GetGlyphGeometry( ) );

  // set glyph coloring
  if (this->GetColorMode ( ) == colorModeSolid)
    {
    this->ScalarVisibilityOff( );
    }
  else if (this->GetColorMode ( ) == colorModeScalar)
    {
    switch ( DiffusionTensorDisplayNode->GetColorGlyphBy( ))
      {
      case vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy:
        {
        vtkDebugMacro("coloring with FA==============================");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByFractionalAnisotropy( );
        }
        break;
      case vtkMRMLDiffusionTensorDisplayPropertiesNode::LinearMeasure:
        {
        vtkDebugMacro("coloring with Cl=============================");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByLinearMeasure( );
        }
        break;
      case vtkMRMLDiffusionTensorDisplayPropertiesNode::Trace:
        {
        vtkDebugMacro("coloring with trace =================");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByTrace( );
        }
        break;
      case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation:
        {
        vtkDebugMacro("coloring with direction (re-implement)");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByOrientation( );
        vtkMRMLNode* colorNode = this->GetScene()->GetNodeByID("vtkMRMLColorTableNodeFullRainbow");
        if (colorNode)
          {
          this->SetAndObserveColorNodeID(colorNode->GetID());
          }
        }
        break;
      case vtkMRMLDiffusionTensorDisplayPropertiesNode::PlanarMeasure:
        {
        vtkDebugMacro("coloring with planar");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByPlanarMeasure( );
        }
        break;
      case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalue:
        {
        vtkDebugMacro("coloring with max eigenval");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByMaxEigenvalue( );
        }
        break;
      case vtkMRMLDiffusionTensorDisplayPropertiesNode::MidEigenvalue:
        {
        vtkDebugMacro("coloring with mid eigenval");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByMidEigenvalue( );
        }
        break;
      case vtkMRMLDiffusionTensorDisplayPropertiesNode::MinEigenvalue:
        {
        vtkDebugMacro("coloring with min eigenval");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByMinEigenvalue( );
        }
        break;
      case vtkMRMLDiffusionTensorDisplayPropertiesNode::RelativeAnisotropy:
        {
        vtkDebugMacro("coloring with relative anisotropy");
        this->ScalarVisibilityOn( );
        this->DiffusionTensorGlyphFilter->ColorGlyphsByRelativeAnisotropy( );
        }
        break;
      default:
        {
        vtkDebugMacro("coloring with relative anisotropy");
        this->ScalarVisibilityOff( );
        }
        break;
      }
    }

  // Updating the filter can be time consuming, we want to refrain from updating
  // as much as possible. Not updating the filter may result into an out-of-date
  // scalar range if AutoScalarRange is true. We infer here that the user doesn't
  // care of an unsync scalar range value if the display node is invisible, or if
  // the glyphs are invisible (Visibility vs ScalarVisibility).
  // Moreover, if the input is null the filter would generate an error.
  if (this->GetVisibility() &&
      this->GetScalarVisibility() &&
      this->GetAutoScalarRange() &&
      this->SliceImage != NULL)
    {
          int ScalarInvariant =  0;
          if ( DiffusionTensorDisplayPropertiesNode )
          {
            ScalarInvariant = DiffusionTensorDisplayPropertiesNode->GetColorGlyphBy();
          }

          double range[2];
          if (DiffusionTensorDisplayPropertiesNode && vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantHasKnownScalarRange(ScalarInvariant))
          {
            vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantKnownScalarRange(ScalarInvariant, range);
          } else {
            this->DiffusionTensorGlyphFilter->Update();
            this->DiffusionTensorGlyphFilter->GetOutput()->GetScalarRange(range);
          }
          this->ScalarRange[0] = range[0];
          this->ScalarRange[1] = range[1];
    }
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode* vtkMRMLDiffusionTensorVolumeSliceDisplayNode::GetDiffusionTensorDisplayPropertiesNode ( )
{
  vtkMRMLDiffusionTensorDisplayPropertiesNode* node = NULL;

  // Find the node corresponding to the ID we have saved.
  if  ( this->GetScene ( ) && this->GetDiffusionTensorDisplayPropertiesNodeID ( ) )
    {
    vtkMRMLNode* cnode = this->GetScene ( ) -> GetNodeByID ( this->DiffusionTensorDisplayPropertiesNodeID );
    node = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast ( cnode );
    }

  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SetAndObserveDiffusionTensorDisplayPropertiesNodeID ( const char *id )
{
  vtkDebugMacro(<< this->GetClassName() << ": Setting and Observing Diffusion Tensor Display Properties ID: " << id  );

  if (
      (id != this->GetDiffusionTensorDisplayPropertiesNodeID())
      && id != NULL && this->GetDiffusionTensorDisplayPropertiesNodeID() != NULL
      && (strcmp(id, this->GetDiffusionTensorDisplayPropertiesNodeID()) == 0)
      )
    {
    return;
    }

  // Stop observing any old node
  vtkSetAndObserveMRMLObjectMacro ( this->DiffusionTensorDisplayPropertiesNode, NULL );

  // Set the ID. This is the "ground truth" reference to the node.
  this->SetDiffusionTensorDisplayPropertiesNodeID ( id );

  // Get the node corresponding to the ID. This pointer is only to observe the object.
  vtkMRMLNode *cnode = this->GetDiffusionTensorDisplayPropertiesNode ( );

  // Observe the node using the pointer.
  vtkSetAndObserveMRMLObjectMacro ( this->DiffusionTensorDisplayPropertiesNode , cnode );

  //The new DiffusionTensorDisplayPropertiesNode can have a different setting on the properties
  //so we emit the event that the polydata has been modified
  if (cnode && this->SliceImage)
    {
    this->Modified();
    }

}
//---------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  // Calls "UpdatePolyDataPipeline"
  this->Superclass::ProcessMRMLEvents(caller, event, callData);

  // Let everyone know that the "display" has changed.
  vtkMRMLDiffusionTensorDisplayPropertiesNode* propertiesNode =
    vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(caller);
  if (propertiesNode != NULL &&
      this->DiffusionTensorDisplayPropertiesNodeID != NULL &&
      strcmp(this->DiffusionTensorDisplayPropertiesNodeID,
             propertiesNode->GetID()) == 0 &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->Modified();
    }
}

//-----------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(this->GetDiffusionTensorDisplayPropertiesNodeID());
}

//-----------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->DiffusionTensorDisplayPropertiesNodeID != NULL && this->Scene->GetNodeByID(this->DiffusionTensorDisplayPropertiesNodeID) == NULL)
    {
    this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(NULL);
    }
}


//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  if (this->DiffusionTensorDisplayPropertiesNodeID && !strcmp(oldID, this->DiffusionTensorDisplayPropertiesNodeID))
    {
    this->SetDiffusionTensorDisplayPropertiesNodeID(newID);
    }
}

//----------------------------------------------------------------------------
std::vector<int> vtkMRMLDiffusionTensorVolumeSliceDisplayNode::GetSupportedColorModes()
{
  std::vector<int> modes;
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::Trace);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::LinearMeasure);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::PlanarMeasure);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::SphericalMeasure);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::RelativeAnisotropy);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::ParallelDiffusivity);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::PerpendicularDiffusivity);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalue);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::MidEigenvalue);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::MinEigenvalue);
  return modes;
}

//----------------------------------------------------------------------------
int vtkMRMLDiffusionTensorVolumeSliceDisplayNode::GetNumberOfScalarInvariants()
{
  static std::vector<int> modes = vtkMRMLDiffusionTensorVolumeSliceDisplayNode::GetSupportedColorModes();
  return modes.size();
}

//----------------------------------------------------------------------------
int vtkMRMLDiffusionTensorVolumeSliceDisplayNode::GetNthScalarInvariant(int i)
{
  static std::vector<int> modes = vtkMRMLDiffusionTensorVolumeSliceDisplayNode::GetSupportedColorModes();
  return modes[i];
}



