/*  NAME:
        Geom Test.c
        
    DESCRIPTION:
        Geometry test.

    COPYRIGHT:
        Quesa Copyright � 1999-2003, Quesa Developers.
        
        For the list of Quesa Developers, and contact details, see:
        
            Documentation/contributors.html

        For the current version of Quesa, see:

        	<http://www.quesa.org/>

		This library is free software; you can redistribute it and/or
		modify it under the terms of the GNU Lesser General Public
		License as published by the Free Software Foundation; either
		version 2 of the License, or (at your option) any later version.

		This library is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
		Lesser General Public License for more details.

		You should have received a copy of the GNU Lesser General Public
		License along with this library; if not, write to the Free Software
		Foundation Inc, 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
    ___________________________________________________________________________
*/
//=============================================================================
//      Include files
//-----------------------------------------------------------------------------
#include "Qut.h"
#include <QuesaMemory.h>

#include <time.h>





//=============================================================================
//      Internal constants
//-----------------------------------------------------------------------------
enum {
	kMenuItemToggleLocalBoundingBox = 1,
	kMenuItemToggleWorldBoundingBox,
	kMenuItemToggleLocalBoundingSphere,
	kMenuItemToggleTexture,
	kMenuItemToggleLights,
	kMenuItemDivider0,
	kMenuItemLoadModel,
	kMenuItemSaveModel,
	kMenuItemDivider1,
	kMenuItemGeometryBox,
#if QUT_CAN_USE_TEXTURES
	kMenuItemGeometryTexturedBox,
#endif
	kMenuItemGeometryCone,
	kMenuItemGeometryCylinder,
	kMenuItemGeometryDisk,
	kMenuItemGeometryEllipse,
	kMenuItemGeometryEllipsoid,
	kMenuItemGeometryGeneralPolygon,
	kMenuItemGeometryLine,
	kMenuItemGeometryMarker,
	kMenuItemGeometryMesh,
	kMenuItemGeometryNURBCurve,
	kMenuItemGeometryNURBPatch,
	kMenuItemGeometryPixmapMarker,
	kMenuItemGeometryPoint,
	kMenuItemGeometryPolyLine,
	kMenuItemGeometryPolygon,
	kMenuItemGeometryPolyhedron,
	kMenuItemGeometryTorus,
	kMenuItemGeometryTriangle,
	kMenuItemGeometryTriGrid,
	kMenuItemGeometryTriMesh,
	kMenuItemDivider2,
	kMenuItemMultiBox,
	kMenuItemQuesaLogo,
	kMenuItemDivider3	
};

#define kTriGridRows										5
#define kTriGridCols										10
#define kTriGridPoints										(kTriGridRows * kTriGridCols)

const TQ3ColorARGB kColourARGBBackground = {1.0f, 0.0f, 0.0f, 0.1f};
const TQ3ColorARGB kColorARGBPickHit = {1.0f, 1.0f, 1.0f, 0.1f};
const TQ3ColorARGB kColorARGBPickMiss = {1.0f, 0.0f, 0.0f, 1.0f};




//=============================================================================
//      Internal globals
//-----------------------------------------------------------------------------
TQ3Object			gSceneGeometry       = NULL;
TQ3Object			gSceneBounds         = NULL;
TQ3Object			gSceneBoundingSphere = NULL;
TQ3ShaderObject		gSceneTexture        = NULL;
TQ3Boolean			gShowTexture         = kQ3False;
TQ3Boolean			gShowWorldBounds     = kQ3False;
TQ3Object			gWorldBounds         = NULL;
TQ3Uns32			gFlashStep           = 0;
TQ3Matrix4x4		gMatrixCurrent;
TQ3ColorARGB		gBackgroundColor;





//=============================================================================
//      Function prototypes
//-----------------------------------------------------------------------------
void appRender(TQ3ViewObject theView);





//=============================================================================
//      Internal functions
//-----------------------------------------------------------------------------
//      createUVsFromPoints : Create UV values from points.
//-----------------------------------------------------------------------------
//		Note :	We generate UVs by mapping the X/Y coordinates onto the UV
//				plane (bit of a hack, but works OK for this app).
//
//				The smallest coordinate value is mapped to 0, and the largest
//				coordinate value is mapped to 1.0.
//-----------------------------------------------------------------------------
static void
createUVsFromPoints(TQ3Uns32 numPoints, TQ3Point3D *thePoints, TQ3Param2D *theUVs)
{	float		minX, minY, maxX, maxY, diffX, diffY;
	TQ3Uns32	n;
	


	// Initialise ourselves
	minX = 0.0f;
	maxX = 1.0f;
	minY = 0.0f;
	maxY = 1.0f;



	// First find the minimum and maximum values
	for (n = 0; n < numPoints; n++)
		{
		if (thePoints[n].x < minX || n == 0)
			minX = thePoints[n].x;

		if (thePoints[n].x > maxX || n == 0)
			maxX = thePoints[n].x;

		if (thePoints[n].y < minY || n == 0)
			minY = thePoints[n].y;

		if (thePoints[n].y > maxY || n == 0)
			maxY = thePoints[n].y;
		}

	diffX = (maxX - minX);
	diffY = (maxY - minY);



	// Now generate the UVs
	for (n = 0; n < numPoints; n++)
		{
		theUVs[n].u = ((thePoints[n].x - minX) / diffX);
		theUVs[n].v = ((thePoints[n].y - minY) / diffY);
		}
}





//=============================================================================
//      createWorldBounds : Create the world-coordinate bounds.
//-----------------------------------------------------------------------------
static TQ3Object
createWorldBounds( TQ3ViewObject inView )
{
	TQ3BoundingBox		theBounds;
	TQ3GroupObject		theGroup = NULL;
	TQ3BoxData			boxData;
	TQ3GeometryObject	theBox;
	TQ3ColorRGB			boxColor  = { 0.3f, 0.3f, 0.3f };
	TQ3ColorRGB			edgeColor = { 0.0f, 0.3f, 1.0f };
	float           sizeX, sizeY, sizeZ;
	TQ3ShaderObject		theShader;
	TQ3StyleObject		theStyle;
	
	if (Q3View_StartBoundingBox(inView, kQ3ComputeBoundsExact) == kQ3Success)
	{
		do
		{
			// Submit the default state so that we are using the same subdivision
			// style as for rendering.  This keeps our cached trimesh from being
			// rebuilt unnecessarily.
			Qut_SubmitDefaultState( inView );
			Q3MatrixTransform_Submit(&gMatrixCurrent, inView);
			Q3Object_Submit(gSceneGeometry, inView);
			
		} while (Q3View_EndBoundingBox(inView, &theBounds) == kQ3ViewStatusRetraverse);
		

	    // If we have an empty bounding box, bump it up slightly
	    sizeX = theBounds.max.x - theBounds.min.x;
	    sizeY = theBounds.max.y - theBounds.min.y;
	    sizeZ = theBounds.max.z - theBounds.min.z;

	    if (sizeX <= kQ3RealZero && sizeY <= kQ3RealZero && sizeZ <= kQ3RealZero)
	        {
	        theBounds.max.x += 0.0001f;
	        theBounds.max.y += 0.0001f;
	        theBounds.max.z += 0.0001f;
	            
	        theBounds.min.x -= 0.0001f;
	        theBounds.min.y -= 0.0001f;
	        theBounds.min.z -= 0.0001f;
	        }
	
		theGroup = Q3DisplayGroup_New();
		
		boxData.origin = theBounds.min;
		
		Q3Vector3D_Set(&boxData.orientation, 0.0f, theBounds.max.y - theBounds.min.y, 0.0f);
		Q3Vector3D_Set(&boxData.majorAxis,   0.0f, 0.0f, theBounds.max.z - theBounds.min.z);
		Q3Vector3D_Set(&boxData.minorAxis,   theBounds.max.x - theBounds.min.x, 0.0f, 0.0f);

		boxData.faceAttributeSet = NULL;
		boxData.boxAttributeSet  = Q3AttributeSet_New();
		if (boxData.boxAttributeSet != NULL)
			Q3AttributeSet_Add( boxData.boxAttributeSet, kQ3AttributeTypeTransparencyColor,
				&boxColor );

		theBox = Q3Box_New( &boxData );
		if (theBox != NULL)
		{
			Q3Group_AddObject( theGroup, theBox );
			Q3Object_Dispose( theBox );
		}
		Q3Box_EmptyData( &boxData );
		
		theShader = Q3NULLIllumination_New();
		if (theShader != NULL)
			{
			Q3Group_AddObject(theGroup, theShader);
			Q3Object_Dispose(theShader);
			}

		theStyle = Q3FillStyle_New(kQ3FillStyleEdges);
		if (theStyle != NULL)
			{
			Q3Group_AddObject(theGroup, theStyle);
			Q3Object_Dispose(theStyle);
			}

		boxData.boxAttributeSet  = Q3AttributeSet_New();
		if (boxData.boxAttributeSet != NULL)
			Q3AttributeSet_Add( boxData.boxAttributeSet, kQ3AttributeTypeDiffuseColor,
				&edgeColor );
		theBox = Q3Box_New( &boxData );
		if (theBox != NULL)
		{
			Q3Group_AddObject( theGroup, theBox );
			Q3Object_Dispose( theBox );
		}
		Q3Box_EmptyData( &boxData );
		
	}
	
	return theGroup;
}





//=============================================================================
//      createLocalBounds : Create the local-coordinate bounds.
//-----------------------------------------------------------------------------
static TQ3GroupObject
createLocalBounds(TQ3GeometryObject theGeom)
{	TQ3ColorRGB			boxColour = { 0.0f, 1.0f, 0.0f };
	TQ3BoundingBox		theBounds;
	TQ3ShaderObject		theShader;
	TQ3StyleObject		theStyle;
	TQ3GroupObject		theGroup;
	TQ3BoxData			boxData;
	TQ3ViewObject		theView;
	TQ3GeometryObject	theBox;



	// Create the group
	theGroup = Q3OrderedDisplayGroup_New();
	if (theGroup == NULL)
		return(NULL);



	// Add the shader/fill style
	theShader = Q3NULLIllumination_New();
	if (theShader != NULL)
		{
		Q3Group_AddObject(theGroup, theShader);
		Q3Object_Dispose(theShader);
		}

	theStyle = Q3FillStyle_New(kQ3FillStyleEdges);
	if (theStyle != NULL)
		{
		Q3Group_AddObject(theGroup, theStyle);
		Q3Object_Dispose(theStyle);
		}

	theStyle = Q3BackfacingStyle_New(kQ3BackfacingStyleBoth);
	if (theStyle != NULL)
		{
		Q3Group_AddObject(theGroup, theStyle);
		Q3Object_Dispose(theStyle);
		}



	// Calculate the bounding box
	theView = Q3View_New();
	if (theView == NULL)
		return(NULL);

	Qut_CalcBounds(theView, theGeom, &theBounds);
	Q3Object_Dispose(theView);



	// Add the box geometry
	boxData.origin = theBounds.min;
	
	Q3Vector3D_Set(&boxData.orientation, 0.0f, theBounds.max.y - theBounds.min.y, 0.0f);
	Q3Vector3D_Set(&boxData.majorAxis,   0.0f, 0.0f, theBounds.max.z - theBounds.min.z);
	Q3Vector3D_Set(&boxData.minorAxis,   theBounds.max.x - theBounds.min.x, 0.0f, 0.0f);

	boxData.faceAttributeSet = NULL;
	boxData.boxAttributeSet  = Q3AttributeSet_New();
	if (boxData.boxAttributeSet != NULL)
		Q3AttributeSet_Add(boxData.boxAttributeSet, kQ3AttributeTypeDiffuseColor,      &boxColour);

	theBox = Q3Box_New(&boxData);
	if (theBox != NULL)
		{
		Q3Group_AddObject(theGroup, theBox);
		Q3Object_Dispose(theBox);
		}



	// Clean up
	if (boxData.boxAttributeSet != NULL)
		Q3Object_Dispose(boxData.boxAttributeSet);

	return(theGroup);
}									





//=============================================================================
//      createLocalBoundingSphere : Create the local-coordinate  bounding sphere.
//-----------------------------------------------------------------------------
static TQ3GroupObject
createLocalBoundingSphere(TQ3GeometryObject theGeom)
{	TQ3ColorRGB			ellipsoidColour = { 0.5f, 0.5f, 0.0f };
	TQ3BoundingSphere	theBoundingSphere;
	TQ3ShaderObject		theShader;
	TQ3StyleObject		theStyle;
	TQ3GroupObject		theGroup;
	TQ3EllipsoidData	ellipsoidData;
	TQ3ViewObject		theView;
	TQ3GeometryObject	theEllipsoid;



	// Create the group
	theGroup = Q3OrderedDisplayGroup_New();
	if (theGroup == NULL)
		return(NULL);



	// Add the shader/fill style
	theShader = Q3NULLIllumination_New();
	if (theShader != NULL)
		{
		Q3Group_AddObject(theGroup, theShader);
		Q3Object_Dispose(theShader);
		}

	theStyle = Q3FillStyle_New(kQ3FillStyleEdges);
	if (theStyle != NULL)
		{
		Q3Group_AddObject(theGroup, theStyle);
		Q3Object_Dispose(theStyle);
		}

	theStyle = Q3BackfacingStyle_New(kQ3BackfacingStyleBoth);
	if (theStyle != NULL)
		{
		Q3Group_AddObject(theGroup, theStyle);
		Q3Object_Dispose(theStyle);
		}



	// Calculate the bounding sphere
	theView = Q3View_New();
	if (theView == NULL)
		return(NULL);

	Qut_CalcBoundingSphere(theView, theGeom, &theBoundingSphere);
	Q3Object_Dispose(theView);



	// Add the sphere (ellipsoid) geometry
	ellipsoidData.origin = theBoundingSphere.origin;
	Q3Vector3D_Set(&ellipsoidData.orientation, 0.0f, theBoundingSphere.radius, 0.0f);
	Q3Vector3D_Set(&ellipsoidData.majorRadius, 0.0f, 0.0f, theBoundingSphere.radius);
	Q3Vector3D_Set(&ellipsoidData.minorRadius, theBoundingSphere.radius, 0.0f, 0.0f);

	ellipsoidData.uMin = 0.0f;
	ellipsoidData.uMax = 1.0f;
	ellipsoidData.vMin = 0.0f;
	ellipsoidData.vMax = 1.0f;
	ellipsoidData.caps = kQ3EndCapNone;

	ellipsoidData.interiorAttributeSet = NULL;
	ellipsoidData.ellipsoidAttributeSet = Q3AttributeSet_New();
	if (ellipsoidData.ellipsoidAttributeSet != NULL)
		Q3AttributeSet_Add(ellipsoidData.ellipsoidAttributeSet, kQ3AttributeTypeDiffuseColor, &ellipsoidColour);

	theEllipsoid = Q3Ellipsoid_New(&ellipsoidData);
	if (theEllipsoid != NULL)
		{
		Q3Group_AddObject(theGroup, theEllipsoid);
		Q3Object_Dispose(theEllipsoid);
		}



	// Clean up
	if (ellipsoidData.interiorAttributeSet != NULL)
		Q3Object_Dispose(ellipsoidData.ellipsoidAttributeSet);
	if (ellipsoidData.ellipsoidAttributeSet != NULL)
		Q3Object_Dispose(ellipsoidData.ellipsoidAttributeSet);

	return(theGroup);
}									





//=============================================================================
//      createGeomBox : Create a Box object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomBox(void)
{	TQ3ColorRGB			faceColour[6] = { {1.0f, 0.0f, 0.0f},
										  {0.0f, 1.0f, 0.0f},
										  {0.0f, 0.0f, 1.0f},
										  {1.0f, 1.0f, 0.0f},
										  {1.0f, 0.0f, 1.0f},
										  {0.0f, 1.0f, 1.0f} };
	TQ3AttributeSet		faceAttributes[6];
	TQ3Matrix4x4		theMatrix;
	TQ3BoxData			boxData;
	TQ3GeometryObject	theBox;
	TQ3Uns32			n;



	// Set up the data
	Q3Point3D_Set(&boxData.origin,      -0.5f, -1.0f,  0.5f);
	Q3Vector3D_Set(&boxData.orientation, 0.0f,  2.0f,  0.0f);
	Q3Vector3D_Set(&boxData.majorAxis,   0.0f,  0.0f,  1.0f);
	Q3Vector3D_Set(&boxData.minorAxis,   1.0f,  0.0f,  0.0f);
	boxData.boxAttributeSet  = NULL;
	boxData.faceAttributeSet = faceAttributes;

	for (n = 0; n < 6; n++)
		{
		faceAttributes[n] = Q3AttributeSet_New();
		if (faceAttributes[n] != NULL)
			Q3AttributeSet_Add(faceAttributes[n], kQ3AttributeTypeDiffuseColor, &faceColour[n]);
		}



	// Rotate it so that we can see all the faces
	Q3Matrix4x4_SetRotate_XYZ(&theMatrix,
								 Q3Math_DegreesToRadians(45.0f),
								 Q3Math_DegreesToRadians(45.0f),
								 Q3Math_DegreesToRadians(45.0f));

	Q3Vector3D_Transform(&boxData.orientation, &theMatrix, &boxData.orientation);
	Q3Vector3D_Transform(&boxData.majorAxis,   &theMatrix, &boxData.majorAxis);
	Q3Vector3D_Transform(&boxData.minorAxis,   &theMatrix, &boxData.minorAxis);



	// Create the geometry
	theBox = Q3Box_New(&boxData);



	// Clean up
	for (n = 0; n < 6; n++)
		{
		if (faceAttributes[n] != NULL)
			Q3Object_Dispose(faceAttributes[n]);
		}

	return(theBox);
}





//=============================================================================
//      createGeomTexturedBox : Create a Box object with a texture on each face.
//-----------------------------------------------------------------------------
#if QUT_CAN_USE_TEXTURES
static TQ3GeometryObject
createGeomTexturedBox(void)
{
	TQ3AttributeSet		faceAttributes[6];
	TQ3BoxData			boxData;
	TQ3GeometryObject	theBox;
	TQ3Uns32			n;
	TQ3ShaderObject		texShader;
	ConstStringPtr		textureNames[6] = { "\p1.png", "\p2.png", "\p3.png",
											"\p4.png", "\p5.png", "\p6.png" };
	FSSpec				fileSpec;
	
	// Set up the data
	Q3Point3D_Set(&boxData.origin,      -0.5f, -1.0f,  0.5f);
	Q3Vector3D_Set(&boxData.orientation, 0.0f,  2.0f,  0.0f);
	Q3Vector3D_Set(&boxData.majorAxis,   0.0f,  0.0f,  1.0f);
	Q3Vector3D_Set(&boxData.minorAxis,   1.0f,  0.0f,  0.0f);
	boxData.boxAttributeSet  = NULL;
	boxData.faceAttributeSet = faceAttributes;
	
	HSetVol( "\p::Support Files:Images:", 0, 0 );

	for (n = 0; n < 6; n++)
	{
		faceAttributes[n] = Q3AttributeSet_New();
		if (faceAttributes[n] != NULL)
		{
			#if macintosh
			if (0 == FSMakeFSSpec( 0, 0, textureNames[n], &fileSpec ))
			{
				texShader = QutTexture_CreateTextureFromFile( &fileSpec,
					kQ3PixelTypeRGB16, kQ3True );
				if (texShader != NULL)
				{
					Q3AttributeSet_Add( faceAttributes[n], kQ3AttributeTypeSurfaceShader,
						&texShader );
					Q3Object_Dispose( texShader );
				}
			}
			#endif
			//Q3AttributeSet_Add(faceAttributes[n], kQ3AttributeTypeDiffuseColor, &faceColour[n]);
		}
	}

	// Create the geometry
	theBox = Q3Box_New(&boxData);



	// Clean up
	for (n = 0; n < 6; n++)
		{
		if (faceAttributes[n] != NULL)
			Q3Object_Dispose(faceAttributes[n]);
		}

	return(theBox);
}
#endif





//=============================================================================
//      createGeomCone : Create a Cone object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomCone(void)
{	TQ3ColorRGB			coneColour	 = { 1.0f, 1.0f, 1.0f };
	TQ3ColorRGB			bottomColour = { 1.0f, 0.0f, 0.0f };
	TQ3ColorRGB			faceTrans    = { 0.2f, 0.2f, 0.2f };
	TQ3ConeData			coneData;
	TQ3GeometryObject	theCone;



	// Set up the data
	Q3Point3D_Set(&coneData.origin,       0.0f, -1.0f,  0.0f);
	Q3Vector3D_Set(&coneData.orientation, 0.0f,  2.0f,  0.0f);
	Q3Vector3D_Set(&coneData.majorRadius, 0.0f,  0.0f,  1.0f);
	Q3Vector3D_Set(&coneData.minorRadius, 1.0f,  0.0f,  0.0f);
	coneData.uMin = 0.0f;
	coneData.uMax = 1.0f;
	coneData.vMin = 0.0f;
	coneData.vMax = 1.0f;
	coneData.caps = kQ3EndCapMaskBottom;

	coneData.coneAttributeSet     = NULL;
	coneData.faceAttributeSet     = NULL;
	coneData.interiorAttributeSet = NULL;
	coneData.bottomAttributeSet   = NULL;


	coneData.coneAttributeSet = Q3AttributeSet_New();
	if (coneData.coneAttributeSet != NULL)
		Q3AttributeSet_Add(coneData.coneAttributeSet, kQ3AttributeTypeDiffuseColor, &coneColour);

	coneData.faceAttributeSet = Q3AttributeSet_New();
	if (coneData.faceAttributeSet != NULL)
		Q3AttributeSet_Add(coneData.faceAttributeSet, kQ3AttributeTypeTransparencyColor, &faceTrans);

	coneData.bottomAttributeSet = Q3AttributeSet_New();
	if (coneData.bottomAttributeSet != NULL)
		Q3AttributeSet_Add(coneData.bottomAttributeSet, kQ3AttributeTypeDiffuseColor, &bottomColour);



	// Create the geometry
	theCone = Q3Cone_New(&coneData);



	// Clean up
	if (coneData.coneAttributeSet != NULL)
		Q3Object_Dispose(coneData.coneAttributeSet);
		
	if (coneData.interiorAttributeSet != NULL)
		Q3Object_Dispose(coneData.interiorAttributeSet);
		
	if (coneData.faceAttributeSet != NULL)
		Q3Object_Dispose(coneData.faceAttributeSet);
		
	if (coneData.bottomAttributeSet != NULL)
		Q3Object_Dispose(coneData.bottomAttributeSet);
		
	return(theCone);
}





//=============================================================================
//      createGeomCylinder : Create a Cylinder object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomCylinder(void)
{	TQ3ColorRGB			topColour    = { 0.0f, 0.0f, 1.0f };
	TQ3ColorRGB			bottomColour = { 0.0f, 1.0f, 0.0f };
	TQ3ColorRGB			faceColour   = { 1.0f, 0.0f, 0.0f };
	TQ3ColorRGB			specColour	 = { 1.0f, 1.0f, 0.5f };
	TQ3CylinderData		cylinderData;
	TQ3GeometryObject	theCylinder;



	// Set up the data
	Q3Point3D_Set(&cylinderData.origin,       0.0f, -1.0f,  0.0f);
	Q3Vector3D_Set(&cylinderData.orientation, 0.0f,  2.0f,  0.0f);
	Q3Vector3D_Set(&cylinderData.majorRadius, 0.0f,  0.0f,  1.0f);
	Q3Vector3D_Set(&cylinderData.minorRadius, 1.0f,  0.0f,  0.0f);
	cylinderData.uMin = 0.0f;
	cylinderData.uMax = 1.0f;
	cylinderData.vMin = 0.0f;
	cylinderData.vMax = 1.0f;
	cylinderData.caps = kQ3EndCapMaskBottom | kQ3EndCapMaskTop;

	cylinderData.cylinderAttributeSet     = NULL;
	cylinderData.faceAttributeSet     = NULL;
	cylinderData.interiorAttributeSet = NULL;
	cylinderData.topAttributeSet   = NULL;
	cylinderData.bottomAttributeSet   = NULL;


	cylinderData.cylinderAttributeSet = Q3AttributeSet_New();
	if (cylinderData.cylinderAttributeSet != NULL)
		Q3AttributeSet_Add(cylinderData.cylinderAttributeSet, kQ3AttributeTypeSpecularColor, &specColour);

	cylinderData.faceAttributeSet = Q3AttributeSet_New();
	if (cylinderData.faceAttributeSet != NULL)
		Q3AttributeSet_Add(cylinderData.faceAttributeSet, kQ3AttributeTypeDiffuseColor, &faceColour);

	cylinderData.topAttributeSet = Q3AttributeSet_New();
	if (cylinderData.topAttributeSet != NULL)
		Q3AttributeSet_Add(cylinderData.topAttributeSet, kQ3AttributeTypeDiffuseColor, &topColour);

	cylinderData.bottomAttributeSet = Q3AttributeSet_New();
	if (cylinderData.bottomAttributeSet != NULL)
		Q3AttributeSet_Add(cylinderData.bottomAttributeSet, kQ3AttributeTypeDiffuseColor, &bottomColour);



	// Create the geometry
	theCylinder = Q3Cylinder_New(&cylinderData);



	// Clean up
	if (cylinderData.cylinderAttributeSet != NULL)
		Q3Object_Dispose(cylinderData.cylinderAttributeSet);
		
	if (cylinderData.interiorAttributeSet != NULL)
		Q3Object_Dispose(cylinderData.interiorAttributeSet);
		
	if (cylinderData.faceAttributeSet != NULL)
		Q3Object_Dispose(cylinderData.faceAttributeSet);
		
	if (cylinderData.topAttributeSet != NULL)
		Q3Object_Dispose(cylinderData.topAttributeSet);

	if (cylinderData.bottomAttributeSet != NULL)
		Q3Object_Dispose(cylinderData.bottomAttributeSet);
		
	return(theCylinder);
}





//=============================================================================
//      createGeomDisk : Create a Disk object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomDisk(void)
{	TQ3ColorRGB			diskColour = { 1.0f, 1.0f, 0.0f };
	TQ3DiskData			diskData;
	TQ3GeometryObject	theDisk;



	// Set up the data
	Q3Point3D_Set(&diskData.origin,       0.0f,  0.0f,  0.0f);
	Q3Vector3D_Set(&diskData.majorRadius, 0.0f,  0.0f,  1.0f);
	Q3Vector3D_Set(&diskData.minorRadius, 1.0f,  0.0f,  0.0f);
	diskData.uMin = 0.0f;
	diskData.uMax = 1.0f;
	diskData.vMin = 0.0f;
	diskData.vMax = 1.0f;

	diskData.diskAttributeSet = Q3AttributeSet_New();
	if (diskData.diskAttributeSet != NULL)
		Q3AttributeSet_Add(diskData.diskAttributeSet, kQ3AttributeTypeDiffuseColor, &diskColour);



	// Create the geometry
	theDisk = Q3Disk_New(&diskData);



	// Clean up
	if (diskData.diskAttributeSet != NULL)
		Q3Object_Dispose(diskData.diskAttributeSet);
		
	return(theDisk);
}





//=============================================================================
//      createGeomEllipse : Create an Ellipse object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomEllipse(void)
{	TQ3ColorRGB			ellipseColour = { 1.0f, 0.0f, 0.0f };
	TQ3EllipseData		ellipseData;
	TQ3GeometryObject	theEllipse;



	// Set up the data
	Q3Point3D_Set(&ellipseData.origin,       0.0f,  0.0f,  0.0f);
	Q3Vector3D_Set(&ellipseData.majorRadius, 0.0f,  0.0f,  1.0f);
	Q3Vector3D_Set(&ellipseData.minorRadius, 1.0f,  0.0f,  0.0f);
	ellipseData.uMin = 0.0f;
	ellipseData.uMax = 0.75f;

	ellipseData.ellipseAttributeSet = Q3AttributeSet_New();
	if (ellipseData.ellipseAttributeSet != NULL)
		Q3AttributeSet_Add(ellipseData.ellipseAttributeSet, kQ3AttributeTypeDiffuseColor, &ellipseColour);



	// Create the geometry
	theEllipse = Q3Ellipse_New(&ellipseData);



	// Clean up
	if (ellipseData.ellipseAttributeSet != NULL)
		Q3Object_Dispose(ellipseData.ellipseAttributeSet);
		
	return(theEllipse);
}





//=============================================================================
//      createGeomEllipsoid : Create an Ellipsoid object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomEllipsoid(void)
{	TQ3ColorRGB			ellipsoidColour = { 1.0f, 0.0f, 0.0f };
	TQ3EllipsoidData	ellipsoidData   = { { 0.0f, 0.0f, 0.0f },
										    { 0.0f, 0.0f, 0.5f },
										    { 1.0f, 0.0f, 0.0f },
										    { 0.0f, 1.5f, 0.0f },
										    0.0f, 1.0f, 0.0f, 1.0f,
										    kQ3EndCapNone, NULL, NULL };
	TQ3GeometryObject	theEllipsoid;



	// Set up the data
	ellipsoidData.ellipsoidAttributeSet = Q3AttributeSet_New();
	if (ellipsoidData.ellipsoidAttributeSet != NULL)
		Q3AttributeSet_Add(ellipsoidData.ellipsoidAttributeSet, kQ3AttributeTypeDiffuseColor, &ellipsoidColour);



	// Create the geometry
	theEllipsoid = Q3Ellipsoid_New(&ellipsoidData);



	// Clean up
	if (ellipsoidData.ellipsoidAttributeSet != NULL)
		Q3Object_Dispose(ellipsoidData.ellipsoidAttributeSet);
		
	return(theEllipsoid);
}





//=============================================================================
//      createGeomGeneralPolygon : Create an General Polygon object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomGeneralPolygon(void)
{	TQ3Point3D						vertPoints[7] = { {-1.0f, -1.0f, 0.0f},
													  {-1.0f,  1.0f, 0.0f},
													  { 1.0f,  1.0f, 0.0f},
													  { 1.0f, -1.0f, 0.0f},
													  {-0.8f, -1.5f, 0.0f},
													  { 0.0f,  1.5f, 0.0f},
													  { 0.8f, -1.5f, 0.0f} };
	TQ3ColorRGB						vertColours[7] = { {1.0f, 0.0f, 0.0f},
													   {0.0f, 1.0f, 0.0f},
													   {0.0f, 1.0f, 0.0f},
													   {1.0f, 0.0f, 0.0f},
													   {0.0f, 0.0f, 1.0f},
													   {0.0f, 0.0f, 1.0f},
										 			   {0.0f, 0.0f, 1.0f} };
	TQ3GeneralPolygonData			generalPolygonData;
	TQ3GeneralPolygonContourData	theContours[2];
	TQ3GeometryObject				theGeneralPoly;
	TQ3Vertex3D						theVertices[7];
	TQ3Param2D						vertUVs[7];
	TQ3Uns32						n;



	// Set up the data
	generalPolygonData.numContours                = 2;
	generalPolygonData.contours                   = theContours;
	generalPolygonData.shapeHint                  = kQ3GeneralPolygonShapeHintComplex;
	generalPolygonData.generalPolygonAttributeSet = NULL;

	theContours[0].numVertices = 4;
	theContours[0].vertices    = &theVertices[0];

	theContours[1].numVertices = 3;
	theContours[1].vertices    = &theVertices[4];

	createUVsFromPoints(7, vertPoints, vertUVs);
	
	for (n = 0; n < 7; n++)
		{
		theVertices[n].point        = vertPoints[n];
		theVertices[n].attributeSet = Q3AttributeSet_New();

		if (theVertices[n].attributeSet != NULL)
			{
			Q3AttributeSet_Add(theVertices[n].attributeSet, kQ3AttributeTypeDiffuseColor, &vertColours[n]);
			Q3AttributeSet_Add(theVertices[n].attributeSet, kQ3AttributeTypeSurfaceUV,    &vertUVs[n]);
			}
		}



	// Create the geometry
	theGeneralPoly = Q3GeneralPolygon_New(&generalPolygonData);



	// Clean up
	for (n = 0; n < 7; n++)
		{
		if (theVertices[n].attributeSet != NULL)
			Q3Object_Dispose(theVertices[n].attributeSet);
		}
		
	return(theGeneralPoly);
}





//=============================================================================
//      createGeomLine : Create a Line object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomLine(void)
{	TQ3Point3D			vertPoints[2] = { {-1.0f, -1.0f, -1.0f},
										  { 1.0f,  1.0f,  1.0f} };
	TQ3ColorRGB			vertColours[2] = { {1.0f, 0.0f, 0.0f},
										   {0.0f, 0.0f, 1.0f} };
	TQ3LineData			lineData;
	TQ3GeometryObject	theLine;
	TQ3Uns32			n;



	// Set up the data
	lineData.lineAttributeSet = NULL;

	for (n = 0; n < 2; n++)
		{
		lineData.vertices[n].point        = vertPoints[n];
		lineData.vertices[n].attributeSet = Q3AttributeSet_New();

		if (lineData.vertices[n].attributeSet != NULL)
			Q3AttributeSet_Add(lineData.vertices[n].attributeSet,
								kQ3AttributeTypeDiffuseColor,
								&vertColours[n]);
		}



	// Create the geometry
	theLine = Q3Line_New(&lineData);



	// Clean up
	for (n = 0; n < 2; n++)
		{
		if (lineData.vertices[n].attributeSet != NULL)
			Q3Object_Dispose(lineData.vertices[n].attributeSet);
		}

	return(theLine);
}





//=============================================================================
//      createGeomMarker : Create a Marker object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomMarker(void)
{	TQ3Uns8				imageData[] = { 0x7E, 0x3C, 0x3C, 0x66, 0x7E, 0x7C, 0x18,
										0x60, 0x60, 0x66, 0x66, 0x60, 0x66, 0x18,
										0x7C, 0x3C, 0x60, 0x7E, 0x7C, 0x66, 0x18,
										0x60, 0x06, 0x60, 0x66, 0x60, 0x7C, 0x18,
										0x60, 0x06, 0x66, 0x66, 0x60, 0x66, 0x00,
										0x7E, 0x3C, 0x3C, 0x66, 0x7E, 0x66, 0x18 };
	TQ3ColorRGB			theColour  = { 0.0f, 1.0f, 1.0f };
	TQ3MarkerData		markerData = { { -1.0f, 0.5f, 0.5f },
									     -20, 20,
									   { NULL, 56, 6, 7, kQ3EndianBig },
									     NULL };
	TQ3GeometryObject	theMarker;



	// Set up the data
	markerData.bitmap.image       = imageData;
	markerData.markerAttributeSet = Q3AttributeSet_New();

	if (markerData.markerAttributeSet != NULL)
		Q3AttributeSet_Add(markerData.markerAttributeSet,
							kQ3AttributeTypeDiffuseColor,
							&theColour);



	// Create the geometry
	theMarker = Q3Marker_New(&markerData);



	// Clean up
	if (markerData.markerAttributeSet != NULL)
		Q3Object_Dispose(markerData.markerAttributeSet);

	return(theMarker);
}





//=============================================================================
//      createGeomMesh : Create a Mesh object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomMesh(void)
{	TQ3Vertex3D			theVertices[9] = { { { -1.5f,  1.5f, 0.0f }, NULL },
										   { { -1.5f, -1.5f, 0.0f }, NULL },
										   { {  0.0f, -1.5f, 0.9f }, NULL },
										   { {  1.5f, -1.5f, 0.0f }, NULL },
										   { {  1.5f,  1.5f, 0.0f }, NULL },
										   { {  0.0f,  1.5f, 0.9f }, NULL },
										   { { -1.2f,  0.6f, 0.0f }, NULL },
										   { {  0.0f,  0.0f, 0.0f }, NULL },
										   { { -1.2f, -0.6f, 0.0f }, NULL } };
	TQ3Param2D			vertUVs[9] = {
		{0.0f, 1.0f}, {0.0f, 0.0f}, {0.5f, 0.0f}, {1.0f, 0.0f},
		{1.0f, 1.0f}, {0.5f, 1.0f}, {0.1f, 0.8f}, {0.5f, 0.5f},
		{0.1f, 0.4f}};
	TQ3ColorRGB			theColour = { 0.3f, 0.9f, 0.5f };
	TQ3MeshVertex		meshVertices[9];
	TQ3AttributeSet		theAttributes;
	TQ3MeshFace			theFace;
	TQ3GeometryObject	theMesh;
	TQ3Uns32			n;



	// Create the mesh
	theMesh = Q3Mesh_New();
	Q3Mesh_DelayUpdates(theMesh);



	// Create the vertices
	for (n = 0; n < 9; n++)
		{
		meshVertices[n] = Q3Mesh_VertexNew(theMesh, &theVertices[n]);

		theAttributes = Q3AttributeSet_New();
		if (theAttributes != NULL)
			{
			Q3AttributeSet_Add(theAttributes, kQ3AttributeTypeSurfaceUV, &vertUVs[n]);
			Q3Mesh_SetVertexAttributeSet(theMesh, meshVertices[n], theAttributes);
			Q3Object_Dispose(theAttributes);
			}
		}



	// Add the face
	theAttributes = Q3AttributeSet_New();
	if (theAttributes != NULL)
		Q3AttributeSet_Add(theAttributes, kQ3AttributeTypeDiffuseColor, &theColour);

	theFace = Q3Mesh_FaceNew(theMesh, 6, meshVertices, theAttributes);
	if (theFace != NULL)
		Q3Mesh_FaceToContour(theMesh, theFace, Q3Mesh_FaceNew(theMesh, 3, &meshVertices[6], NULL));



	// Resume updates and clean up
	Q3Mesh_ResumeUpdates(theMesh);
	
	Q3Object_Dispose(theAttributes);

	return(theMesh);
}





//=============================================================================
//      createGeomMultiBox : Create the multi-box geometry.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomMultiBox(void)
{	TQ3ColorRGB				faceColour[6] = { {1.0f, 0.0f, 0.0f},
											  {0.0f, 1.0f, 0.0f},
											  {0.0f, 0.0f, 1.0f},
											  {1.0f, 1.0f, 0.0f},
											  {1.0f, 0.0f, 1.0f},
											  {0.0f, 1.0f, 1.0f} };
	TQ3AttributeSet			faceAttributes[6];
	TQ3Uns32				n, i, j;
	TQ3DisplayGroupObject	theGroup;
	TQ3BoxData				boxData;
	TQ3GeometryObject		theBox;



	// Create the group to hold the boxes
	theGroup = Q3DisplayGroup_New();
	if (theGroup == NULL)
		return(NULL);



	// Set up the data
	Q3Vector3D_Set(&boxData.orientation, 0.0f,  0.1f,  0.0f);
	Q3Vector3D_Set(&boxData.majorAxis,   0.0f,  0.0f,  0.1f);
	Q3Vector3D_Set(&boxData.minorAxis,   0.1f,  0.0f,  0.0f);
	boxData.boxAttributeSet  = NULL;
	boxData.faceAttributeSet = faceAttributes;

	for (n = 0; n < 6; n++)
		{
		faceAttributes[n] = Q3AttributeSet_New();
		if (faceAttributes[n] != NULL)
			Q3AttributeSet_Add(faceAttributes[n], kQ3AttributeTypeDiffuseColor, &faceColour[n]);
		}



	// Create the boxes
	for (i = 0; i < 10; ++i)
		{
		boxData.origin.x = -0.95f + i * 0.2f;
		
		for (j = 0; j < 10; ++j)
			{
			boxData.origin.y = -0.95f + j * 0.2f;
			
			for (n = 0; n < 10; ++n)
				{
				boxData.origin.z = -0.95f + n * 0.2f;

				theBox = Q3Box_New(&boxData);
				if (theBox != NULL)
					{
					Q3Group_AddObject(theGroup, theBox);
					Q3Object_Dispose(theBox);
					}
				}
			}
		}



	// Clean up
	for (n = 0; n < 6; n++)
		{
		if (faceAttributes[n] != NULL)
			Q3Object_Dispose(faceAttributes[n]);
		}

	return(theGroup);
}





//=============================================================================
//      createGeomNURBCurve : Create a NURB curve object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomNURBCurve(void)
{	TQ3ColorRGB			theColour = { 0.8f, 0.2f, 0.6f };
	TQ3RationalPoint4D	thePoints[7] = { { -2.0f,  0.0f,  0.0f, 1.0f },
										 { -1.0f,  1.0f,  0.0f, 1.0f },
										 { -0.5f,  0.0f,  0.0f, 1.0f },
										 {  0.0f,  1.0f,  0.0f, 1.0f },
										 {  0.5f,  0.0f,  0.0f, 1.0f },
										 {  1.0f,  1.0f,  0.0f, 1.0f },
										 {  2.0f,  0.0f,  0.0f, 1.0f } };
	float				theKnots[11] = { 0.0f,  0.0f, 0.0f,  0.0f,
										 0.25f, 0.5f, 0.75f, 1.0f,
										 1.0f,  1.0f, 1.0f };
	TQ3NURBCurveData	curveData;
	TQ3GeometryObject	theCurve;



	// Set up the data
	curveData.order 			= 4;
	curveData.numPoints			= 7;
	curveData.controlPoints		= thePoints;
	curveData.knots 			= theKnots;
	curveData.curveAttributeSet = Q3AttributeSet_New();
	
	if (curveData.curveAttributeSet != NULL)
		Q3AttributeSet_Add(curveData.curveAttributeSet, kQ3AttributeTypeDiffuseColor, &theColour);



	// Create the geometry
	theCurve = Q3NURBCurve_New(&curveData);



	// Clean up
	if (curveData.curveAttributeSet != NULL)
		Q3Object_Dispose(curveData.curveAttributeSet);

	return(theCurve);
}





//=============================================================================
//      createGeomNURBPatch : Create a NURB patch object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomNURBPatch(void)
{	TQ3RationalPoint4D		thePoints[12] = { { -1.5f, -1.0f,  0.0f, 1.0f },
											  { -0.5f, -1.0f,  2.0f, 1.0f },
											  {  0.5f, -1.0f,  0.0f, 1.0f },
											  {  1.5f, -1.0f,  0.0f, 1.0f },

											  { -1.5f, -0.0f,  0.0f, 1.0f },
											  { -0.5f, -0.0f,  2.0f, 1.0f },
											  {  0.5f, -0.0f, -2.0f, 1.0f },
											  {  1.5f, -0.0f, -2.0f, 1.0f },

											  { -1.5f,  1.0f,  0.0f, 1.0f },
											  { -0.5f,  1.0f,  0.0f, 1.0f },
											  {  0.5f,  1.0f,  0.0f, 1.0f },
											  {  1.5f,  1.0f,  0.0f, 1.0f } };

	float					vKnots[6] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
	float					uKnots[8] = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };

	TQ3RationalPoint3D		trimPointsZero[5] = { { 0.1f, 0.1f, 1.0f },
												  { 0.9f, 0.1f, 1.0f },
												  { 0.4f, 0.4f, 1.0f },
												  { 0.1f, 0.4f, 1.0f },
												  { 0.1f, 0.1f, 1.0f } };
	TQ3RationalPoint3D		trimPointsOne[5] =  { { 0.3f, 0.6f, 1.0f },
												  { 0.9f, 0.6f, 1.0f },
												  { 0.4f, 0.9f, 1.0f },
												  { 0.2f, 0.9f, 1.0f },
												  { 0.3f, 0.6f, 1.0f } };

	float					trimKnotsZero[9] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.5f,
												 1.0f, 1.0f, 1.0f, 1.0f };
	float					trimKnotsOne[9]  = { 0.0f, 0.0f, 0.0f, 0.0f, 0.5f,
												 1.0f, 1.0f, 1.0f, 1.0f };

	TQ3ColorRGB							theColour = { 0.9f, 0.2f, 0.9f };
	TQ3NURBPatchTrimCurveData			trimCurveZero, trimCurveOne;
	TQ3NURBPatchTrimLoopData			trimLoops[2];
	TQ3NURBPatchData					patchData;
	TQ3GeometryObject					thePatch;



	// Set up the data
	trimLoops[0].numTrimCurves 	= 1;
	trimLoops[0].trimCurves		= &trimCurveZero;	
	trimLoops[1].numTrimCurves 	= 1;
	trimLoops[1].trimCurves		= &trimCurveOne;	

	trimCurveZero.order			= 4;
	trimCurveZero.numPoints		= 5;
	trimCurveZero.knots			= trimKnotsZero;
	trimCurveZero.controlPoints = trimPointsZero;

	trimCurveOne.order		   = 4;
	trimCurveOne.numPoints	   = 5;
	trimCurveOne.knots		   = trimKnotsOne;
	trimCurveOne.controlPoints = trimPointsOne;
							
	patchData.uOrder		= 4;
	patchData.vOrder		= 3;
	patchData.numColumns	= 4;
	patchData.numRows		= 3;
	patchData.uKnots		= uKnots;
	patchData.vKnots		= vKnots;
	patchData.controlPoints = thePoints;
	patchData.numTrimLoops	= 0;
	patchData.trimLoops 	= NULL;

	patchData.patchAttributeSet = Q3AttributeSet_New();
	if (patchData.patchAttributeSet != NULL)
		Q3AttributeSet_Add(patchData.patchAttributeSet, kQ3AttributeTypeDiffuseColor, &theColour);



	// Create the geometry
	thePatch = Q3NURBPatch_New(&patchData);



	// Clean up
	Q3Object_Dispose(patchData.patchAttributeSet);

	return(thePatch);
}





//=============================================================================
//      createGeomPixmapMarker : Create a Pixmap marker object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomPixmapMarker(void)
{	TQ3PixmapMarkerData		pixmapMarkerData = { { -1.0f, 0.5f, 0.5f },
											       -50, -50,
											   { NULL, 0, 0, 0, 32,
											   	 kQ3PixelTypeRGB32,
											   	 kQ3EndianBig,
											   	 kQ3EndianBig },
											     NULL };
	TQ3Uns32				theImage[64][64];
	TQ3Uns32				x, y, r, g;
	TQ3GeometryObject		theMarker;



	// Create an image
	for (y = 0; y < 64; y++)
		{
		g = (y * 4);
		for (x = 0; x < 64; x++)
			{
			r = (x * 4);
			theImage[y][x] = (r << 16) | (g << 8);
			}
		}



	// Initialise the pixmap
	pixmapMarkerData.pixmap.width    = 64;
	pixmapMarkerData.pixmap.height   = 64;
	pixmapMarkerData.pixmap.rowBytes = pixmapMarkerData.pixmap.width * sizeof(TQ3Uns32);
	pixmapMarkerData.pixmap.image    = Q3MemoryStorage_New((TQ3Uns8 *) theImage, sizeof(theImage));

	if (pixmapMarkerData.pixmap.image == NULL)
		return(NULL);



	// Create the geometry
	theMarker = Q3PixmapMarker_New(&pixmapMarkerData);



	// Clean up
	Q3Object_Dispose(pixmapMarkerData.pixmap.image);

	return(theMarker);
}





//=============================================================================
//      createGeomPoint : Create a Point object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomPoint(void)
{	TQ3PointData		pointData = { {-1.0f, -1.0f, -1.0f}, NULL };
	TQ3GeometryObject	thePoint;



	// Create the geometry
	thePoint = Q3Point_New(&pointData);

	return(thePoint);
}





//=============================================================================
//      createGeomPolygon : Create a Polygon object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomPolygon(void)
{	TQ3Point3D			vertPoints[5] = { {-0.5f, -1.0f, 0.0f},
										  {-1.0f,  0.0f, 0.0f},
										  { 0.0f,  1.5f, 0.0f},
										  { 1.0f,  0.0f, 0.0f},
										  { 0.5f, -1.0f, 0.0f} };
	TQ3ColorRGB			vertColours[5] = { {1.0f, 0.0f, 0.0f},
										   {0.0f, 1.0f, 0.0f},
										   {0.0f, 0.0f, 1.0f},
										   {1.0f, 1.0f, 0.0f},
										   {0.0f, 1.0f, 1.0f} };
	TQ3Vertex3D			theVertices[5];
	TQ3PolygonData		polygonData;
	TQ3GeometryObject	thePolygon;
	TQ3Param2D			vertUVs[5];
	TQ3Uns32			n;



	// Set up the data
	polygonData.numVertices         = 5;
	polygonData.vertices            = theVertices;
	polygonData.polygonAttributeSet = NULL;

	createUVsFromPoints(polygonData.numVertices, vertPoints, vertUVs);

	for (n = 0; n < 5; n++)
		{
		polygonData.vertices[n].point        = vertPoints[n];
		polygonData.vertices[n].attributeSet = Q3AttributeSet_New();

		if (polygonData.vertices[n].attributeSet != NULL)
			{
			Q3AttributeSet_Add(polygonData.vertices[n].attributeSet,
								kQ3AttributeTypeDiffuseColor,
								&vertColours[n]);

			Q3AttributeSet_Add(polygonData.vertices[n].attributeSet,
								kQ3AttributeTypeSurfaceUV,
								&vertUVs[n]);
			}
		}



	// Create the geometry
	thePolygon = Q3Polygon_New(&polygonData);



	// Clean up
	for (n = 0; n < 5; n++)
		{
		if (polygonData.vertices[n].attributeSet != NULL)
			Q3Object_Dispose(polygonData.vertices[n].attributeSet);
		}

	return(thePolygon);
}





//=============================================================================
//      createGeomPolyhedron : Create a Polyhedron object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomPolyhedron(void)
{	TQ3PolyhedronTriangleData	theTriangles[14] = { { {0,  2,  3}, kQ3PolyhedronEdge01,   NULL },
													 { {0,  3,  4}, kQ3PolyhedronEdge12,   NULL },
													 { {0,  4,  1}, kQ3PolyhedronEdge20,   NULL },
													 { {1,  4,  5}, kQ3PolyhedronEdge20,   NULL },
													 { {2,  6,  7}, kQ3PolyhedronEdge01,   NULL },
													 { {2,  7,  3}, kQ3PolyhedronEdge12,   NULL },
													 { {3,  7,  8}, kQ3PolyhedronEdgeNone, NULL },
													 { {3,  8,  4}, kQ3PolyhedronEdge12,   NULL },
													 { {4,  8,  9}, kQ3PolyhedronEdgeNone, NULL },
													 { {4,  9,  5}, kQ3PolyhedronEdge12,   NULL },
													 { {6, 10,  7}, kQ3PolyhedronEdge01,   NULL },
													 { {7, 10, 11}, kQ3PolyhedronEdge12,   NULL },
													 { {7, 11,  8}, kQ3PolyhedronEdge20,   NULL },
													 { {8, 11,  9}, kQ3PolyhedronEdge12,   NULL } };
	TQ3Point3D			vertPoints[12] = { {-0.5f, -1.0f, 0.0f},
										   { 0.5f, -1.0f, 0.0f},
										   {-1.0f, -0.5f, 0.0f},
										   {-0.5f, -0.5f, 0.0f},
										   { 0.5f, -0.5f, 0.0f},
										   { 1.0f, -0.5f, 0.0f},
										   {-1.0f,  0.5f, 0.0f},
										   {-0.5f,  0.5f, 0.0f},
										   { 0.5f,  0.5f, 0.0f},
										   { 1.0f,  0.5f, 0.0f},
										   {-0.5f,  1.0f, 0.0f},
										   { 0.5f,  1.0f, 0.0f} };
	TQ3ColorRGB			vertColours[12] = { {0.0f, 0.0f, 1.0f},
										    {1.0f, 0.0f, 0.0f},
										    {1.0f, 0.0f, 0.0f},
										    {1.0f, 1.0f, 1.0f},
										    {1.0f, 1.0f, 1.0f},
										    {0.0f, 0.0f, 1.0f},
										    {0.0f, 0.0f, 1.0f},
										    {1.0f, 1.0f, 1.0f},
										    {1.0f, 1.0f, 1.0f},
										    {1.0f, 0.0f, 0.0f},
										    {1.0f, 0.0f, 0.0f},
										    {0.0f, 0.0f, 1.0f} };
	TQ3Vertex3D					theVertices[12];
	TQ3PolyhedronData			polyhedronData;
	TQ3GeometryObject			thePolyhedron;
	TQ3Param2D					vertUVs[12];
	TQ3Uns32					n;



	// Set up the data
	polyhedronData.numVertices            = 12;
	polyhedronData.vertices               = theVertices;
	polyhedronData.numEdges               = 0;
	polyhedronData.edges                  = NULL;
	polyhedronData.numTriangles           = 14;
	polyhedronData.triangles              = theTriangles;
	polyhedronData.polyhedronAttributeSet = NULL;

	createUVsFromPoints(polyhedronData.numVertices, vertPoints, vertUVs);
	
	for (n = 0; n < 12; n++)
		{
		polyhedronData.vertices[n].point        = vertPoints[n];
		polyhedronData.vertices[n].attributeSet = Q3AttributeSet_New();

		if (polyhedronData.vertices[n].attributeSet != NULL)
			{
			Q3AttributeSet_Add(polyhedronData.vertices[n].attributeSet,
								kQ3AttributeTypeDiffuseColor,
								&vertColours[n]);

			Q3AttributeSet_Add(polyhedronData.vertices[n].attributeSet,
								kQ3AttributeTypeSurfaceUV,
								&vertUVs[n]);
			}
		}



	// Create the geometry
	thePolyhedron = Q3Polyhedron_New(&polyhedronData);



	// Clean up
	for (n = 0; n < 12; n++)
		{
		if (polyhedronData.vertices[n].attributeSet != NULL)
			Q3Object_Dispose(polyhedronData.vertices[n].attributeSet);
		}

	return(thePolyhedron);
}





//=============================================================================
//      createGeomPolyLine : Create a PolyLine object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomPolyLine(void)
{	TQ3Point3D			vertPoints[5] = { {-1.0f, -1.0f, -1.0f},
										  {-1.0f, -0.5f,  1.0f},
										  { 1.0f,  0.0f,  1.0f},
										  { 1.0f,  0.5f, -1.0f},
										  { 0.0f,  1.0f,  0.0f} };
	TQ3ColorRGB			vertColours[5] = { {1.0f, 0.0f, 0.0f},
										   {0.0f, 1.0f, 0.0f},
										   {0.0f, 0.0f, 1.0f},
										   {1.0f, 1.0f, 0.0f},
										   {0.0f, 1.0f, 1.0f} };
	TQ3Vertex3D			theVertices[5];
	TQ3PolyLineData		polyLineData;
	TQ3GeometryObject	thePolyLine;
	TQ3Uns32			n;



	// Set up the data
	polyLineData.numVertices          = 5;
	polyLineData.vertices             = theVertices;
	polyLineData.polyLineAttributeSet = NULL;
	polyLineData.segmentAttributeSet  = NULL;

	for (n = 0; n < 5; n++)
		{
		polyLineData.vertices[n].point        = vertPoints[n];
		polyLineData.vertices[n].attributeSet = Q3AttributeSet_New();

		if (polyLineData.vertices[n].attributeSet != NULL)
			Q3AttributeSet_Add(polyLineData.vertices[n].attributeSet,
								kQ3AttributeTypeDiffuseColor,
								&vertColours[n]);
		}



	// Create the geometry
	thePolyLine = Q3PolyLine_New(&polyLineData);



	// Clean up
	for (n = 0; n < 5; n++)
		{
		if (polyLineData.vertices[n].attributeSet != NULL)
			Q3Object_Dispose(polyLineData.vertices[n].attributeSet);
		}

	return(thePolyLine);
}





//=============================================================================
//      createGeomQuesa : Create the Quesa logo geometry.
//-----------------------------------------------------------------------------
static TQ3GroupObject
createGeomQuesa(void)
{	TQ3GeometryObject		theTorus, theSphere, theCone;
	TQ3TransformObject		theTransform;
	TQ3GroupObject			theGroup;
	TQ3RotateTransformData	transformData = { kQ3AxisZ, Q3Math_DegreesToRadians(45.0f) };
	TQ3ColorRGB				colourTorus   = { 0.19f, 0.21f, 0.60f };
	TQ3ColorRGB				colourSphere  = { 0.66f, 0.01f, 0.01f };
	TQ3ColorRGB				colourCone    = { 0.14f, 0.42f, 0.18f };
	TQ3TorusData			torusData = { { 0.0f, 0.3f, 0.0f },		// Origin
										  { 0.0f, 0.0f, 0.3f },		// Orientation
										  { 1.0f, 0.0f, 0.0f },		// Major radius
										  { 0.0f, 1.0f, 0.0f },		// Minor radius
										    1.0f,					// Ratio
										    0.0f, 1.0f, 0.0f, 1.0f,
										    kQ3EndCapNone, NULL, NULL };
	TQ3EllipsoidData		sphereData = { { 0.0f, 0.3f, 0.0f },	// Origin
										   { 0.0f, 0.0f, 0.3f },	// Orientation
										   { 0.3f, 0.0f, 0.0f },	// Major radius
										   { 0.0f, 0.3f, 0.0f },	// Minor radius
										   0.0f, 1.0f, 0.0f, 1.0f,
										   kQ3EndCapNone, NULL, NULL };
	TQ3ConeData				coneData   = { { 0.0f, -1.4f,  0.0f },	// Origin
										   { 0.0f,  1.5f,  0.0f },	// Orientation
										   { 0.0f,  0.0f,  0.3f },	// Major radius
										   { 0.3f,  0.0f,  0.0f },	// Minor radius
										   0.0f, 1.0f, 0.0f, 1.0f,
										   kQ3EndCapMaskBottom, NULL, NULL, NULL, NULL };



	// Create the group
	theGroup = Q3OrderedDisplayGroup_New();
	if (theGroup == NULL)
		return(NULL);



	// Create the transform
	theTransform = Q3RotateTransform_New(&transformData);
	if (theTransform != NULL)
		{
		Q3Group_AddObject(theGroup, theTransform);
		Q3Object_Dispose(theTransform);
		}



	// Create the Torus
	torusData.torusAttributeSet = Q3AttributeSet_New();
	if (torusData.torusAttributeSet != NULL)
		Q3AttributeSet_Add(torusData.torusAttributeSet, kQ3AttributeTypeDiffuseColor, &colourTorus);

	theTorus = Q3Torus_New(&torusData);
	if (theTorus != NULL)
		{
		Q3Group_AddObject(theGroup, theTorus);
		Q3Object_Dispose(theTorus);
		}



	// Create the Sphere
	sphereData.ellipsoidAttributeSet = Q3AttributeSet_New();
	if (sphereData.ellipsoidAttributeSet != NULL)
		Q3AttributeSet_Add(sphereData.ellipsoidAttributeSet, kQ3AttributeTypeDiffuseColor, &colourSphere);

	theSphere = Q3Ellipsoid_New(&sphereData);
	if (theSphere != NULL)
		{
		Q3Group_AddObject(theGroup, theSphere);
		Q3Object_Dispose(theSphere);
		}



	// Create the Cone
	coneData.coneAttributeSet = Q3AttributeSet_New();
	if (coneData.coneAttributeSet != NULL)
		Q3AttributeSet_Add(coneData.coneAttributeSet, kQ3AttributeTypeDiffuseColor, &colourCone);

	theCone = Q3Cone_New(&coneData);
	if (theCone != NULL)
		{
		Q3Group_AddObject(theGroup, theCone);
		Q3Object_Dispose(theCone);
		}



	// Clean up
	if (torusData.torusAttributeSet != NULL)
		Q3Object_Dispose(torusData.torusAttributeSet);

	if (sphereData.ellipsoidAttributeSet != NULL)
		Q3Object_Dispose(sphereData.ellipsoidAttributeSet);

	if (coneData.coneAttributeSet != NULL)
		Q3Object_Dispose(coneData.coneAttributeSet);

	return(theGroup);
}									





//=============================================================================
//      createGeomTorus : Create a Torus object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomTorus(void)
{	TQ3ColorRGB			color = {1.0f, 0.7f, 0.4f};
	TQ3TorusData		torusData = {
		{0.0f, 0.0f, 0.0f},		// origin
		{0.0f, 0.0f, 0.1f},		// orientation
		{2.0f, 0.0f, 0.0f},		// majorRadius
		{0.0f, 0.8f, 0.0f},		// minorRadius
		1.2f,					// ratio
		0.0f, 1.0f, 0.0f, 1.0f,	// uMin, uMax, vMin, vMax
		kQ3EndCapNone,			// caps
		NULL,					// interiorAttributeSet
		NULL					// torusAttributeSet
	};
	TQ3GeometryObject	theTorus;



	// Set up the data
	torusData.torusAttributeSet = Q3AttributeSet_New();
	if (torusData.torusAttributeSet != NULL)
		Q3AttributeSet_Add(torusData.torusAttributeSet,
							kQ3AttributeTypeDiffuseColor,
							&color);



	// Create the geometry
	theTorus = Q3Torus_New(&torusData);



	// Clean up
	if (torusData.torusAttributeSet != NULL)
		Q3Object_Dispose(torusData.torusAttributeSet);

	return(theTorus);
}





//=============================================================================
//      createGeomTriangle : Create a Triangle object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomTriangle(void)
{	TQ3Point3D			vertPoints[3] = { {-1.5f, -1.5f, 0.0f},
										  { 0.0f,  1.5f, 0.0f},
										  { 1.5f, -1.5f, 0.0f} };
	TQ3ColorRGB			vertColours[3] = { {1.0f, 0.0f, 0.0f},
										   {0.0f, 1.0f, 0.0f},
										   {0.0f, 0.0f, 1.0f} };
	TQ3TriangleData		triangleData;
	TQ3GeometryObject	theTriangle;
	TQ3Param2D			vertUVs[3];
	TQ3Uns32			n;



	// Set up the data
	triangleData.triangleAttributeSet = NULL;

	createUVsFromPoints(3, vertPoints, vertUVs);

	for (n = 0; n < 3; n++)
		{
		triangleData.vertices[n].point        = vertPoints[n];
		triangleData.vertices[n].attributeSet = Q3AttributeSet_New();

		if (triangleData.vertices[n].attributeSet != NULL)
			{
			Q3AttributeSet_Add(triangleData.vertices[n].attributeSet,
								kQ3AttributeTypeDiffuseColor,
								&vertColours[n]);

			Q3AttributeSet_Add(triangleData.vertices[n].attributeSet,
								kQ3AttributeTypeSurfaceUV,
								&vertUVs[n]);
			}
		}



	// Create the geometry
	theTriangle = Q3Triangle_New(&triangleData);



	// Clean up
	for (n = 0; n < 3; n++)
		{
		if (triangleData.vertices[n].attributeSet != NULL)
			Q3Object_Dispose(triangleData.vertices[n].attributeSet);
		}

	return(theTriangle);
}





//=============================================================================
//      createGeomTriGrid : Create a TriGrid object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomTriGrid(void)
{	TQ3ColorRGB					theColour = {0.8f, 0.5f, 0.2f};
	TQ3Point3D					vertPoints[kTriGridPoints];
	TQ3Vertex3D					theVerts[kTriGridPoints];
	TQ3Param2D					vertUVs[kTriGridPoints];
	TQ3Uns32					row, col, i;
	TQ3TriGridData				triGridData;
	TQ3GeometryObject			theTriGrid;



	// initialize the random number generator
	srand(12345);	



	// Initialise the points	
	for (i=0; i<kTriGridPoints; i++) {
		row = i / kTriGridCols;
		col = i % kTriGridCols;
		Q3Point3D_Set(&vertPoints[i],
			3.0f/kTriGridCols * col - 1.5f + (float)rand() / RAND_MAX / 10.0f,
			3.0f/kTriGridRows * row - 1.5f + (float)rand() / RAND_MAX / 10.0f,
			(float)rand() / RAND_MAX / 5.0f );
	}



	// Initialise the vertices	
	createUVsFromPoints(kTriGridPoints, vertPoints, vertUVs);

	for (i=0; i<kTriGridPoints; i++) {
		theVerts[i].point        = vertPoints[i];
		theVerts[i].attributeSet = Q3AttributeSet_New();
		
		if (theVerts[i].attributeSet != NULL)
			Q3AttributeSet_Add(theVerts[i].attributeSet,
								kQ3AttributeTypeSurfaceUV,
								&vertUVs[i]);
	}


	// Set up the data
	triGridData.numRows             = kTriGridRows;
	triGridData.numColumns          = kTriGridCols;
	triGridData.vertices            = theVerts;
	triGridData.facetAttributeSet   = NULL;
	triGridData.triGridAttributeSet = Q3AttributeSet_New();

	if (triGridData.triGridAttributeSet != NULL)
		Q3AttributeSet_Add(triGridData.triGridAttributeSet, kQ3AttributeTypeDiffuseColor, &theColour);



	// Create the geometry
	theTriGrid = Q3TriGrid_New(&triGridData);



	// Clean up
	if (triGridData.triGridAttributeSet != NULL)
		Q3Object_Dispose(triGridData.triGridAttributeSet);
	
	for (i=0; i<kTriGridPoints; ++i)
	{
		if (theVerts[i].attributeSet != NULL)
			Q3Object_Dispose( theVerts[i].attributeSet );
	}

	return(theTriGrid);
}





//=============================================================================
//      createGeomTriMesh : Create a TriMesh object.
//-----------------------------------------------------------------------------
static TQ3GeometryObject
createGeomTriMesh(void)
{	TQ3Point3D			vertPoints[4] = { {-1.5f, -1.5f, 0.0f},
										  { 0.0f,  1.5f, 0.0f},
										  { 1.5f, -1.5f, 0.0f},
										  { 0.0f, -1.5f, -1.0f} };
	TQ3ColorRGB			vertColours[4] = { {1.0f, 0.0f, 0.0f},
										   {0.0f, 1.0f, 0.0f},
										   {0.0f, 0.0f, 1.0f},
										   {1.0f, 1.0f, 0.0f}};
	TQ3TriMeshTriangleData		triangles[2] = {{{1, 0, 3}}, {{3, 2, 1}}};
	TQ3Param2D					vertUVs[4];
	TQ3TriMeshData				triMeshData;
	TQ3GeometryObject			theTriMesh;
	TQ3TriMeshAttributeData		attrData[2];



	// Set up the data
	triMeshData.triMeshAttributeSet       = NULL;
	triMeshData.numPoints                 = 4;
	triMeshData.points                    = vertPoints;
	triMeshData.numTriangles              = 2;
	triMeshData.triangles                 = triangles;
	triMeshData.numTriangleAttributeTypes = 0;
	triMeshData.triangleAttributeTypes    = NULL;
	triMeshData.numEdges                  = 0;
	triMeshData.edges                     = NULL;
	triMeshData.numEdgeAttributeTypes     = 0;
	triMeshData.edgeAttributeTypes        = NULL;
	triMeshData.numVertexAttributeTypes   = 2;
	triMeshData.vertexAttributeTypes      = attrData;

	createUVsFromPoints(triMeshData.numPoints, vertPoints, vertUVs);

	attrData[0].attributeType     = kQ3AttributeTypeDiffuseColor;
	attrData[0].data              = vertColours;
	attrData[0].attributeUseArray = NULL;

	attrData[1].attributeType     = kQ3AttributeTypeSurfaceUV;
	attrData[1].data              = vertUVs;
	attrData[1].attributeUseArray = NULL;

	Q3BoundingBox_SetFromPoints3D(&triMeshData.bBox, triMeshData.points, triMeshData.numPoints, sizeof(TQ3Point3D));



	// Create the geometry
	theTriMesh = Q3TriMesh_New(&triMeshData);

	return(theTriMesh);
}





//=============================================================================
//      doPicktest : Return kQ3True if the given point hits the geometry in
//				     gSceneGeometry, kQ3False otherwise.
//-----------------------------------------------------------------------------
static TQ3Boolean
doPicktest(TQ3ViewObject theView, TQ3Point2D mousePoint)
{	TQ3Boolean					hitGood = kQ3False;
	TQ3WindowPointPickData		pickData;
	TQ3Uns32					numHits;
	TQ3PickObject 				thePick;



	// Create a window-point pick object
	pickData.data.sort            = kQ3PickSortNearToFar;		// Request sorted results
	pickData.data.mask            = kQ3PickDetailMaskObject |	// Return the object
									kQ3PickDetailMaskXYZ;		// Return the XYZ point hit
	pickData.data.numHitsToReturn = kQ3ReturnAllHits;			// Give me back everything
	pickData.point.x              = mousePoint.x;				// Set the point to pick on
	pickData.point.y              = mousePoint.y;
	pickData.vertexTolerance      = 2.0;						// Set fudge-factor for vertices
	pickData.edgeTolerance        = 2.0;						// Set fudge-factor for edges

	thePick = Q3WindowPointPick_New(&pickData);



	// Submit the scene for picking - note that we submit exactly what's submitted
	// for rendering, including things like styles. This is so the scene picked
	// is exactly the same as the scene rendered (so that geometries are picked
	// with the same subdivision as is used to render them on-screen).
	if (Q3View_StartPicking(theView, thePick) == kQ3Success)
		{
		do
			{
			Qut_SubmitDefaultState(theView);
			appRender(theView);
			}
		while (Q3View_EndPicking(theView) == kQ3ViewStatusRetraverse);
		}



	// Test for hits
	if (Q3Pick_GetNumHits(thePick, &numHits) != kQ3Failure && numHits > 0)
		hitGood = kQ3True;



	// Clean up. Note that we don't need to explicitly dispose of the pick list,
	// as disposing of the pick object will release that for us. If we did want
	// to re-use the pick list, we would call Q3Pick_EmptyHitList to empty it.
	Q3Object_Dispose(thePick);

	return(hitGood);
}





//=============================================================================
//      doSaveModel : write the currently displayed model to a file
//-----------------------------------------------------------------------------
static void
doSaveModel(TQ3ViewObject theView)
{	TQ3Status			submitStatus;
	TQ3ViewStatus		viewStatus;
	TQ3StorageObject	storage;
	TQ3FileObject		file;



	// Prompt the user for somewhere to save the file
	storage = Qut_SelectMetafileToSaveTo();



	// Save the model into the selected file
	if (storage != NULL){

		file = Q3File_New();

		if(file){
			submitStatus = Q3File_SetStorage(file, storage);

			if(submitStatus == kQ3Success){
				viewStatus = kQ3ViewStatusRetraverse;
				submitStatus = Q3File_OpenWrite(file, kQ3FileModeNormal);

				if(submitStatus == kQ3Success){
					submitStatus = Q3View_StartWriting(theView,file);
		
					while (viewStatus == kQ3ViewStatusRetraverse && submitStatus == kQ3Success){
	
						// Submit the scene
						Qut_SubmitDefaultState(theView);
						if (gShowTexture && gSceneTexture != NULL)
							Q3Shader_Submit(gSceneTexture, theView);
							
						Q3Object_Submit(gSceneGeometry, theView);

						viewStatus = Q3View_EndWriting(theView);
						}

					}
	
				Q3File_Close(file);
			}
			Q3Object_Dispose(file);
		}
		Q3Object_Dispose(storage);
		}
}





//=============================================================================
//      doLoadModel : Loads a Model from File
//-----------------------------------------------------------------------------
static TQ3Object
doLoadModel(TQ3ViewObject theView)
{	TQ3StorageObject	storageObj;
	TQ3Object 			theModel;
	TQ3GroupObject 		normalizedModel;
	TQ3Object 			tempObj;
	float				xBounds, yBounds, zBounds, scaleFactor;
	TQ3BoundingBox		theBounds;
	TQ3Vector3D			translateToOrigin = { 0.0f, 0.0f, 0.0f };
	TQ3Vector3D			scale     = { 1.0f, 1.0f, 1.0f };


	// Get the file
	storageObj = Qut_SelectMetafileToOpen();
	if( storageObj == NULL )
		return NULL;
	
	// Read the file (note, this disposes of storageObj)
	theModel = Qut_ReadModel(storageObj);
	
	
		// Adjust the scale and translation required for the model
		
	if(theModel)
		{
		Qut_CalcBounds(theView, theModel, &theBounds);
		xBounds = (theBounds.max.x - theBounds.min.x);
		yBounds = (theBounds.max.y - theBounds.min.y);
		zBounds = (theBounds.max.z - theBounds.min.z);

		scaleFactor = (xBounds > yBounds)     ? xBounds : yBounds;
		scaleFactor = (zBounds > scaleFactor) ? zBounds : scaleFactor;
		scaleFactor = 1.0f / (scaleFactor * 0.3f);

		if (xBounds <= 0.0003f && yBounds <= 0.0003f && zBounds <= 0.0003f)
				scaleFactor = 1.0f;

		translateToOrigin.x = -(theBounds.min.x + (xBounds * 0.5f));
		translateToOrigin.y = -(theBounds.min.y + (yBounds * 0.5f));
		translateToOrigin.z = -(theBounds.min.z + (zBounds * 0.5f));

		scale.x = scaleFactor;
		scale.y = scaleFactor;
		scale.z = scaleFactor;
	    
	    normalizedModel = Q3DisplayGroup_New();
	    if(normalizedModel != NULL)
	    	{
	    		tempObj = Q3ScaleTransform_New(&scale);
	    		if(tempObj)
	    			{
                    Q3Group_AddObject(normalizedModel, tempObj);
               		Q3Object_Dispose(tempObj);
	    			}
	    		tempObj = Q3TranslateTransform_New(&translateToOrigin);
	    		if(tempObj)
	    			{
                    Q3Group_AddObject(normalizedModel, tempObj);
               		Q3Object_Dispose(tempObj);
	    			}
	    			
             Q3Group_AddObject(normalizedModel, theModel);
             Q3Object_Dispose(theModel);
             
             return (normalizedModel);
	    	}
		}
	return NULL;
}





//=============================================================================
//      toggleLights : Turn the lights on or off.
//-----------------------------------------------------------------------------
static void
toggleLights( TQ3ViewObject				theView )
{
	TQ3GroupObject	lightGroup = NULL;
	
	if ( (kQ3Success == Q3View_GetLightGroup( theView, &lightGroup )) &&
		(lightGroup != NULL) )
	{
		TQ3GroupPosition	pos;
		TQ3LightObject		theLight;
		TQ3Boolean			theState;
		
		if ( (kQ3Success == Q3Group_GetFirstPosition( lightGroup, &pos )) &&
			(pos != NULL) )
		{
			do
			{
				Q3Group_GetPositionObject( lightGroup, pos, &theLight );
				Q3Light_GetState( theLight, &theState );
				theState = (theState == kQ3True) ? kQ3False : kQ3True;
				Q3Light_SetState( theLight, theState );
				Q3Object_Dispose( theLight );
				
				Q3Group_GetNextPosition( lightGroup, &pos );
			}	while (pos != NULL);
		}
		
		Q3Object_Dispose( lightGroup );
	}
}





//=============================================================================
//      appConfigureView : Configure the view.
//-----------------------------------------------------------------------------
static void
appConfigureView(TQ3ViewObject				theView,
					TQ3DrawContextObject	theDrawContext,
					TQ3CameraObject			theCamera)
{
#pragma unused(theView)
#pragma unused(theCamera)



	// Adjust the background colour
	Q3DrawContext_SetClearImageColor(theDrawContext, &kColourARGBBackground);
}





//=============================================================================
//      appMouseDown : Handle mouse clicks.
//					   Here, we use them to test Picking.
//-----------------------------------------------------------------------------
static void
appMouseDown(TQ3ViewObject theView, TQ3Point2D mousePoint)
{
	gFlashStep = 5;
	gBackgroundColor = doPicktest(theView, mousePoint) ? kColorARGBPickHit : kColorARGBPickMiss;
	gBackgroundColor.r *= 2.0f;
	gBackgroundColor.g *= 2.0f;
	gBackgroundColor.b *= 2.0f;
}





//=============================================================================
//      appMenuSelect : Handle menu selections.
//-----------------------------------------------------------------------------
static void
appMenuSelect(TQ3ViewObject theView, TQ3Uns32 menuItem)
{	TQ3Object		theGeom;
#pragma unused(theView)



	// Create the new geometry type
	theGeom = NULL;
	switch (menuItem) {
		case kMenuItemToggleLocalBoundingBox:
			// Create or dispose of the bounding geometry
			if (gSceneBounds == NULL)
				gSceneBounds = createLocalBounds(gSceneGeometry);
			else
				{
				Q3Object_Dispose(gSceneBounds);
				gSceneBounds = NULL;
				}
			break;
			
		case kMenuItemToggleWorldBoundingBox:
			gShowWorldBounds = !gShowWorldBounds;
			break;
			
		case kMenuItemToggleLocalBoundingSphere:
			// Create or dispose of the bounding sphere geometry
			if (gSceneBoundingSphere == NULL)
				gSceneBoundingSphere = createLocalBoundingSphere(gSceneGeometry);
			else
				{
				Q3Object_Dispose(gSceneBoundingSphere);
				gSceneBoundingSphere = NULL;
				}
			break;
		
		case kMenuItemToggleTexture:
			gShowTexture = (TQ3Boolean) !gShowTexture;
			break;
		
		case kMenuItemToggleLights:
			toggleLights( theView );
			break;

		case kMenuItemLoadModel:
			theGeom = doLoadModel(theView);
			break;

		case kMenuItemSaveModel:
			doSaveModel(theView);
			break;

		case kMenuItemGeometryBox:
			theGeom = createGeomBox();
			break;
			
	#if QUT_CAN_USE_TEXTURES
		case kMenuItemGeometryTexturedBox:
			theGeom = createGeomTexturedBox();
			break;
	#endif

		case kMenuItemGeometryCone:
			theGeom = createGeomCone();
			break;

		case kMenuItemGeometryCylinder:
			theGeom = createGeomCylinder();
			break;

		case kMenuItemGeometryDisk:
			theGeom = createGeomDisk();
			break;

		case kMenuItemGeometryEllipse:
			theGeom = createGeomEllipse();
			break;

		case kMenuItemGeometryEllipsoid:
			theGeom = createGeomEllipsoid();
			break;

		case kMenuItemGeometryGeneralPolygon:
			theGeom = createGeomGeneralPolygon();
			break;

		case kMenuItemGeometryLine:
			theGeom = createGeomLine();
			break;

		case kMenuItemGeometryMarker:
			theGeom = createGeomMarker();
			break;

		case kMenuItemGeometryMesh:
			theGeom = createGeomMesh();
			break;

		case kMenuItemGeometryNURBCurve:
			theGeom = createGeomNURBCurve();
			break;

		case kMenuItemGeometryNURBPatch:
			theGeom = createGeomNURBPatch();
			break;

		case kMenuItemGeometryPixmapMarker:
			theGeom = createGeomPixmapMarker();
			break;

		case kMenuItemGeometryPoint:
			theGeom = createGeomPoint();
			break;

		case kMenuItemGeometryPolyLine:
			theGeom = createGeomPolyLine();
			break;

		case kMenuItemGeometryPolygon:
			theGeom = createGeomPolygon();
			break;
		
		case kMenuItemGeometryPolyhedron:
			theGeom = createGeomPolyhedron();
			break;
		
		case kMenuItemGeometryTriangle:
			theGeom = createGeomTriangle();
			break;

		case kMenuItemGeometryTorus:
			theGeom = createGeomTorus();
			break;

		case kMenuItemGeometryTriGrid:
			theGeom = createGeomTriGrid();
			break;

		case kMenuItemGeometryTriMesh:
			theGeom = createGeomTriMesh();
			break;

		case kMenuItemMultiBox:
			theGeom = createGeomMultiBox();
			break;

		case kMenuItemQuesaLogo:
			theGeom = createGeomQuesa();
			break;

		default:
			break;
		}



	// If we created it, replace the current geometry
	if (theGeom != NULL)
		{
		if (gSceneGeometry != NULL)
			Q3Object_Dispose(gSceneGeometry);
		
		gSceneGeometry = theGeom;

		if (gSceneBounds != NULL)
			{
			Q3Object_Dispose(gSceneBounds);
			gSceneBounds = createLocalBounds(gSceneGeometry);
			}

		if (gSceneBoundingSphere != NULL)
			{
			Q3Object_Dispose(gSceneBoundingSphere);
			gSceneBoundingSphere = createLocalBoundingSphere(gSceneGeometry);
			}
			
		Q3Matrix4x4_SetIdentity(&gMatrixCurrent);
		}
}





//=============================================================================
//      appPreRender : Prepare to render another frame.
//-----------------------------------------------------------------------------
static void
appPreRender(TQ3ViewObject theView)
{


	// Update the world bounds
	if (gWorldBounds != NULL)
	{
		Q3Object_Dispose( gWorldBounds );
		gWorldBounds = NULL;
	}
	
	if (gShowWorldBounds)
		gWorldBounds = createWorldBounds( theView );
}





//=============================================================================
//      appRender : Render another frame.
//-----------------------------------------------------------------------------
static void
appRender(TQ3ViewObject theView)
{
	static clock_t	sPrevRenderTime = 0;
	clock_t			renderTime;
	float			timeFactor;
	TQ3Matrix4x4	rotationMatrix;
	static TQ3Vector3D	sAxis = { 0.6f, 0.8f, 0.0f };
	const TQ3Point3D	kOrigin = { 0.0f, 0.0f, 0.0f };
	TQ3DrawContextObject context;

	// If we're flashing the background color, update it now
	if (gFlashStep) {
		gFlashStep--;
		if (0 == gFlashStep) {
			gBackgroundColor = kColourARGBBackground;
		} else {
			gBackgroundColor.r = (gBackgroundColor.r + kColourARGBBackground.r) / 2.0f;
			gBackgroundColor.g = (gBackgroundColor.g + kColourARGBBackground.r) / 2.0f;
			gBackgroundColor.b = (gBackgroundColor.b + kColourARGBBackground.r) / 2.0f;
		}
		Q3View_GetDrawContext(theView, &context);
		Q3DrawContext_SetClearImageColor(context, &gBackgroundColor);
		Q3Object_Dispose(context);
	}



	// Submit the scene
	if (gWorldBounds != NULL)
		Q3Object_Submit(gWorldBounds, theView);

	if (gShowTexture && gSceneTexture != NULL)
		Q3Shader_Submit(gSceneTexture, theView);

	Q3MatrixTransform_Submit(&gMatrixCurrent, theView);
	Q3Object_Submit(gSceneGeometry, theView);

	if (gSceneBounds != NULL)
		{
		Q3Push_Submit(theView);
		Q3BackfacingStyle_Submit(kQ3BackfacingStyleBoth, theView);
		Q3FillStyle_Submit(kQ3FillStyleEdges,            theView);
		Q3Object_Submit(gSceneBounds, theView);
		Q3Pop_Submit(theView);
		}

	if (gSceneBoundingSphere != NULL)
		{
		Q3Push_Submit(theView);
		Q3BackfacingStyle_Submit(kQ3BackfacingStyleBoth, theView);
		Q3FillStyle_Submit(kQ3FillStyleEdges,            theView);
		Q3Object_Submit(gSceneBoundingSphere, theView);
		Q3Pop_Submit(theView);
		}



	// Update the rotation matrix, in a such a way that the rate of rotation
	// remains approximately constant in spite of changes in frame rate.
	renderTime = clock();
	if (sPrevRenderTime != 0)
		{
		timeFactor = (renderTime - sPrevRenderTime) / ((float) CLOCKS_PER_SEC);
		
		// precession of the axis
		Q3Matrix4x4_SetRotate_Y( &rotationMatrix, timeFactor * 0.1f );
		Q3Vector3D_Transform( &sAxis, &rotationMatrix, &sAxis );
		Q3Vector3D_Normalize( &sAxis, &sAxis );
		
		Q3Matrix4x4_SetRotateAboutAxis( &rotationMatrix, &kOrigin, &sAxis, timeFactor );
		Q3Matrix4x4_Multiply(&gMatrixCurrent, &rotationMatrix, &gMatrixCurrent);
		}
	sPrevRenderTime = renderTime;
}





#if QUESA_OS_MACINTOSH
//=============================================================================
//      showMessage : Display a message in a debugger or console.
//-----------------------------------------------------------------------------
static void showMessage( const char* inMessage )
{
	unsigned char	msg[256];
	int		msgLen;
	
	msgLen = strlen( inMessage );
	if (msgLen > 255)
	{
		msgLen = 255;
	}
	msg[0] = msgLen;
	memcpy( &msg[1], inMessage, msgLen );
	DebugStr( msg );
}
#endif





//=============================================================================
//      errorCallback : Receive notification of errors.
//-----------------------------------------------------------------------------
static void errorCallback( TQ3Error            firstError,
                            TQ3Error            lastError,
                            TQ3Int32            userData )
{
#pragma unused( lastError, userData )
	const char*	theText;
	
#if QUESA_OS_MACINTOSH && TARGET_API_MAC_OS8	// if using QuickDraw 3D
	char	msg[100];
	
	sprintf( msg, "QD3D error number %d", (int)firstError );
	theText = msg;
#else
	theText = Q3Error_ToString( kQ3LanguageDefault, firstError );
#endif

#if QUESA_OS_MACINTOSH
	showMessage( theText );
#endif
}





//=============================================================================
//      warningCallback : Receive notification of warnings.
//-----------------------------------------------------------------------------
static void warningCallback( TQ3Warning          firstWarning,
                            TQ3Warning          lastWarning,
                            TQ3Int32            userData )
{
#pragma unused( lastWarning, userData )
	const char*	theText;
	
#if QUESA_OS_MACINTOSH && TARGET_API_MAC_OS8	// if using QuickDraw 3D
	char	msg[100];
	
	sprintf( msg, "QD3D warning number %d", (int)firstWarning );
	theText = msg;
#else
	theText = Q3Warning_ToString( kQ3LanguageDefault, firstWarning );
#endif

#if QUESA_OS_MACINTOSH
	showMessage( theText );
#endif
}





//=============================================================================
//      noticeCallback : Receive notification of notices.
//-----------------------------------------------------------------------------
static void noticeCallback( TQ3Notice           firstNotice,
                            TQ3Notice           lastNotice,
                            TQ3Int32            userData )
{
#pragma unused( lastNotice, userData )
	const char*	theText;
	
#if QUESA_OS_MACINTOSH && TARGET_API_MAC_OS8	// if using QuickDraw 3D
	char	msg[100];
	
	sprintf( msg, "QD3D notice number %d", (int)firstNotice );
	theText = msg;
#else
	theText = Q3Notice_ToString( kQ3LanguageDefault, firstNotice );
#endif

#if QUESA_OS_MACINTOSH
	showMessage( theText );
#endif
}





//=============================================================================
//      App_Initialise : Initialise the app.
//-----------------------------------------------------------------------------
void
App_Initialise(void)
{
	// Install error handlers.
	Q3Error_Register( errorCallback, 0 );
	Q3Warning_Register( warningCallback, 0 );
	Q3Notice_Register( noticeCallback, 0 );



	// Watch for leaks (Quesa only)
	#if !defined(QUT_MAC_HDR) || TARGET_API_MAC_CARBON
	Q3Memory_StartRecording();
	#endif



	// Initialise Qut
	Qut_CreateWindow("Geom Test", 300, 300, kQ3True);
	Qut_CreateView(appConfigureView);
	Qut_SetRenderFunc(appRender);
	Qut_SetRenderPreFunc(appPreRender);
	Qut_SetMouseDownFunc(appMouseDown);
	


	// Create the initial scene
	gSceneGeometry = createGeomQuesa();



	// If we can use textures, try and load a texture
#if QUT_CAN_USE_TEXTURES
	{	FSSpec		theFSSpec;
	
	FSMakeFSSpec(0, 0, "\p::Support Files:Images:Quesa.jpg", &theFSSpec);
	gSceneTexture = QutTexture_CreateTextureFromFile(&theFSSpec, kQ3PixelTypeRGB16, kQ3False);
	if (gSceneTexture == NULL)
		{
		FSMakeFSSpec(0, 0, "\p:Quesa.jpg", &theFSSpec);
		gSceneTexture = QutTexture_CreateTextureFromFile(&theFSSpec, kQ3PixelTypeRGB16, kQ3False);
		}
	}
#endif



	// Initialise the matrices
	Q3Matrix4x4_SetIdentity(&gMatrixCurrent);



	// Set up the menu bar
	Qut_CreateMenu(appMenuSelect);
			
	Qut_CreateMenuItem(kMenuItemLast, "Toggle Local Bounding Box");
	Qut_CreateMenuItem(kMenuItemLast, "Toggle World Bounding Box");
	Qut_CreateMenuItem(kMenuItemLast, "Toggle Local Bounding Sphere");
	Qut_CreateMenuItem(kMenuItemLast, "Toggle Texture");
	Qut_CreateMenuItem(kMenuItemLast, "Toggle Lights");
	Qut_CreateMenuItem(kMenuItemLast, kMenuItemDivider);
	Qut_CreateMenuItem(kMenuItemLast, "Load Model...");
	Qut_CreateMenuItem(kMenuItemLast, "Save Model...");
	Qut_CreateMenuItem(kMenuItemLast, kMenuItemDivider);
	Qut_CreateMenuItem(kMenuItemLast, "Box");
#if QUT_CAN_USE_TEXTURES
	Qut_CreateMenuItem(kMenuItemLast, "Box (textured)");
#endif
	Qut_CreateMenuItem(kMenuItemLast, "Cone");
	Qut_CreateMenuItem(kMenuItemLast, "Cylinder");
	Qut_CreateMenuItem(kMenuItemLast, "Disk");
	Qut_CreateMenuItem(kMenuItemLast, "Ellipse");
	Qut_CreateMenuItem(kMenuItemLast, "Ellipsoid");
	Qut_CreateMenuItem(kMenuItemLast, "General Polygon");
	Qut_CreateMenuItem(kMenuItemLast, "Line");
	Qut_CreateMenuItem(kMenuItemLast, "Marker");
	Qut_CreateMenuItem(kMenuItemLast, "Mesh");
	Qut_CreateMenuItem(kMenuItemLast, "NURB Curve");
	Qut_CreateMenuItem(kMenuItemLast, "NURB Patch");
	Qut_CreateMenuItem(kMenuItemLast, "Pixmap Marker");
	Qut_CreateMenuItem(kMenuItemLast, "Point");
	Qut_CreateMenuItem(kMenuItemLast, "PolyLine");
	Qut_CreateMenuItem(kMenuItemLast, "Polygon");
	Qut_CreateMenuItem(kMenuItemLast, "Polyhedron");
	Qut_CreateMenuItem(kMenuItemLast, "Torus");
	Qut_CreateMenuItem(kMenuItemLast, "Triangle");
	Qut_CreateMenuItem(kMenuItemLast, "TriGrid");
	Qut_CreateMenuItem(kMenuItemLast, "TriMesh");
	Qut_CreateMenuItem(kMenuItemLast, kMenuItemDivider);
	Qut_CreateMenuItem(kMenuItemLast, "MultiBox");
	Qut_CreateMenuItem(kMenuItemLast, "Quesa Logo");
}





//=============================================================================
//      App_Terminate : Terminate the app.
//-----------------------------------------------------------------------------
void
App_Terminate(void)
{


	// Clean up
	if (gSceneBounds != NULL)
		Q3Object_Dispose(gSceneBounds);
	
	if (gSceneBoundingSphere != NULL)
		Q3Object_Dispose(gSceneBoundingSphere);
	
	if (gWorldBounds != NULL)
		Q3Object_Dispose(gWorldBounds);
	
	if (gSceneGeometry != NULL)
		Q3Object_Dispose(gSceneGeometry);
	
	if (gSceneTexture != NULL)
		Q3Object_Dispose( gSceneTexture );
}
