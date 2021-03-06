/*===============================================================================
Copyright (c) 2012-2014 Qualcomm Connected Experiences, Inc. All Rights Reserved.

Vuforia is a trademark of QUALCOMM Incorporated, registered in the United States 
and other countries. Trademarks of QUALCOMM Incorporated are used with permission.
===============================================================================*/

#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <QCAR/QCAR.h>
#include <QCAR/CameraDevice.h>
#include <QCAR/Renderer.h>
#include <QCAR/VideoBackgroundConfig.h>
#include <QCAR/Trackable.h>
#include <QCAR/TrackableResult.h>
#include <QCAR/Tool.h>
#include <QCAR/Tracker.h>
#include <QCAR/TrackerManager.h>
#include <QCAR/ImageTracker.h>
#include <QCAR/CameraCalibration.h>
#include <QCAR/UpdateCallback.h>
#include <QCAR/DataSet.h>


#include "SampleUtils.h"
#include "Texture.h"
#include "CubeShaders.h"
#include "Teapot.h"
#include "Buildings.h"
#include "ag6000.h"
#include "ag6001.h"

#include "mbcr1953lqc_2.h"
#include "mba012.h"
#include "mlm007.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Textures:
int textureCount                = 0;
Texture** textures              = 0;

unsigned int shaderProgramID    = 0;
GLint vertexHandle              = 0;
GLint normalHandle              = 0;
GLint textureCoordHandle        = 0;
GLint mvpMatrixHandle           = 0;
GLint texSampler2DHandle        = 0;
GLint modelViewMatrixHandle     = 0;
GLint diffuseMaterialHandle     = 0;

// Screen dimensions:
unsigned int screenWidth        = 0;
unsigned int screenHeight       = 0;

// Indicates whether screen is in portrait (true) or landscape (false) mode
bool isActivityInPortraitMode   = false;

// The projection matrix used for rendering virtual objects:
QCAR::Matrix44F projectionMatrix;

// Constants:
//static const float kObjectScale = 3.f;
static const float kObjectScale = 3.5f;
static const float kBuildingsObjectScale = 6.f;

QCAR::DataSet* dataSetMbcr1953lqc   = 0;
QCAR::DataSet* dataSetTeapot        = 0;
QCAR::DataSet* dataSetMba012        = 0;
QCAR::DataSet* dataSetMlm007        = 0;
QCAR::DataSet* dataSetAg6000        = 0;
QCAR::DataSet* dataSetAg6001        = 0;


bool switchDataSetAsap            = false;
bool isExtendedTrackingActivated = false;
bool isDescActivated = false;

QCAR::CameraDevice::CAMERA currentCamera;

const int MBCR1953LQC_DATASET_ID = 0;
const int TEAPOT_DATASET_ID = 1;
const int MBA012_DATASET_ID = 2;
const int MLM007_DATASET_ID = 3;
const int AG6000_DATASET_ID = 4;
const int AG6001_DATASET_ID = 5;

//int selectedDataset = TEAPOT_DATASET_ID;
int selectedDataset = MBCR1953LQC_DATASET_ID;
// Object to receive update callbacks from QCAR SDK
class ImageTargets_UpdateCallback : public QCAR::UpdateCallback
{   
    virtual void QCAR_onUpdate(QCAR::State& /*state*/)
    {
        if (switchDataSetAsap)
        {
            switchDataSetAsap = false;

            // Get the image tracker:
            QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
            QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
                trackerManager.getTracker(QCAR::ImageTracker::getClassType()));
            if (imageTracker == 0 || dataSetTeapot == 0 || dataSetMbcr1953lqc == 0 || dataSetMba012 == 0 || dataSetMlm007 == 0 || dataSetAg6000 == 0 || dataSetAg6001 == 0 ||
                imageTracker->getActiveDataSet() == 0)
            {
                LOG("Failed to switch data set.");
                return;
            }
            
			switch( selectedDataset )
			{
				case MBCR1953LQC_DATASET_ID:
					if (imageTracker->getActiveDataSet() != dataSetMbcr1953lqc)
					{
						imageTracker->activateDataSet(dataSetMbcr1953lqc);
						imageTracker->deactivateDataSet(dataSetTeapot);
						imageTracker->deactivateDataSet(dataSetMba012);
						imageTracker->deactivateDataSet(dataSetMlm007);
						imageTracker->deactivateDataSet(dataSetAg6000);
						imageTracker->deactivateDataSet(dataSetAg6001);

					}
					break;
				case MBA012_DATASET_ID:
					if (imageTracker->getActiveDataSet() != dataSetMba012)
					{
						imageTracker->activateDataSet(dataSetMba012);
						imageTracker->deactivateDataSet(dataSetTeapot);
						imageTracker->deactivateDataSet(dataSetMbcr1953lqc);
						imageTracker->deactivateDataSet(dataSetMlm007);
						imageTracker->deactivateDataSet(dataSetAg6000);
						imageTracker->deactivateDataSet(dataSetAg6001);

					}
					break;
                case MLM007_DATASET_ID:
					if (imageTracker->getActiveDataSet() != dataSetMlm007)
					{
						imageTracker->activateDataSet(dataSetMlm007);
						imageTracker->deactivateDataSet(dataSetAg6000);
						imageTracker->deactivateDataSet(dataSetAg6001);
						imageTracker->deactivateDataSet(dataSetTeapot);
						imageTracker->deactivateDataSet(dataSetMbcr1953lqc);
						imageTracker->deactivateDataSet(dataSetMba012);

					}
					break;
				case AG6000_DATASET_ID:
                	if (imageTracker->getActiveDataSet() != dataSetAg6000)
                	{
                		imageTracker->activateDataSet(dataSetAg6000);
                		imageTracker->deactivateDataSet(dataSetAg6001);
                		imageTracker->deactivateDataSet(dataSetTeapot);
                		imageTracker->deactivateDataSet(dataSetMbcr1953lqc);
                		imageTracker->deactivateDataSet(dataSetMba012);
                		imageTracker->deactivateDataSet(dataSetMlm007);

                	}
                	break;

				case AG6001_DATASET_ID:
                	if (imageTracker->getActiveDataSet() != dataSetAg6001)
                	{
                		imageTracker->activateDataSet(dataSetAg6001);
                		imageTracker->deactivateDataSet(dataSetAg6000);
                		imageTracker->deactivateDataSet(dataSetTeapot);
                		imageTracker->deactivateDataSet(dataSetMbcr1953lqc);
                		imageTracker->deactivateDataSet(dataSetMba012);
                		imageTracker->deactivateDataSet(dataSetMlm007);

                	}
                	break;
				case TEAPOT_DATASET_ID:
					if (imageTracker->getActiveDataSet() != dataSetTeapot)
					{
						imageTracker->activateDataSet(dataSetTeapot);
						imageTracker->deactivateDataSet(dataSetMbcr1953lqc);
						imageTracker->deactivateDataSet(dataSetMba012);
						imageTracker->deactivateDataSet(dataSetMlm007);
						imageTracker->deactivateDataSet(dataSetAg6000);
						imageTracker->deactivateDataSet(dataSetAg6001);
					}
					break;
					

			}

			if(isExtendedTrackingActivated)
			{
				QCAR::DataSet* currentDataSet = imageTracker->getActiveDataSet();
				for (int tIdx = 0; tIdx < currentDataSet->getNumTrackables(); tIdx++)
				{
					QCAR::Trackable* trackable = currentDataSet->getTrackable(tIdx);
					trackable->startExtendedTracking();
				}
			}

        }
    }
};

ImageTargets_UpdateCallback updateCallback;


JNIEXPORT void JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_setActivityPortraitMode(JNIEnv *, jobject, jboolean isPortrait)
{
    isActivityInPortraitMode = isPortrait;
}



JNIEXPORT void JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_switchDatasetAsap(JNIEnv *, jobject, jint datasetId)
{
    selectedDataset = datasetId;
    switchDataSetAsap = true;
}


JNIEXPORT int JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_initTracker(JNIEnv *, jobject)
{
    LOG("Java_com_daruni_QCAR_ImageTargets_ImageTargets_initTracker");
    
    // Initialize the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* tracker = trackerManager.initTracker(QCAR::ImageTracker::getClassType());
    if (tracker == NULL)
    {
        LOG("Failed to initialize ImageTracker.");
        return 0;
    }

    LOG("Successfully initialized ImageTracker.");
    return 1;
}


JNIEXPORT void JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_deinitTracker(JNIEnv *, jobject)
{
    LOG("Java_com_daruni_QCAR_ImageTargets_ImageTargets_deinitTracker");

    // Deinit the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    trackerManager.deinitTracker(QCAR::ImageTracker::getClassType());
}


JNIEXPORT int JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_loadTrackerData(JNIEnv *, jobject)
{
    LOG("Java_com_daruni_QCAR_ImageTargets_ImageTargets_loadTrackerData");
    
    // Get the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
                    trackerManager.getTracker(QCAR::ImageTracker::getClassType()));
    if (imageTracker == NULL)
    {
        LOG("Failed to load tracking data set because the ImageTracker has not"
            " been initialized.");
        return 0;
    }

    // Create the data sets:
    dataSetMbcr1953lqc = imageTracker->createDataSet();
    if (dataSetMbcr1953lqc == 0)
    {
        LOG("Failed to create a new tracking data.");
        return 0;
    }
    // Create the data sets:
    dataSetMba012 = imageTracker->createDataSet();
    if (dataSetMba012 == 0)
    {
        LOG("Failed to create a new tracking data.");
        return 0;
    }
    dataSetMlm007 = imageTracker->createDataSet();
    if (dataSetMlm007 == 0)
    {
        LOG("Failed to create a new tracking data.");
        return 0;
    }
    dataSetAg6000 = imageTracker->createDataSet();
     if (dataSetAg6000 == 0)
     {
         LOG("Failed to create a new tracking data.");
         return 0;
     }
    dataSetAg6001 = imageTracker->createDataSet();
     if (dataSetAg6001 == 0)
     {
         LOG("Failed to create a new tracking data.");
         return 0;
     }

    dataSetTeapot = imageTracker->createDataSet();
    if (dataSetTeapot == 0)
    {
        LOG("Failed to create a new tracking data.");
        return 0;
    }



    // Load the data sets:
    if (!dataSetMbcr1953lqc->load("Daruni.xml", QCAR::STORAGE_APPRESOURCE))
    {
         LOG("Failed to load data set.");
         return 0;
    }
    // Load the data sets:
    if (!dataSetMba012->load("Daruni.xml", QCAR::STORAGE_APPRESOURCE))
    {
         LOG("Failed to load data set.");
         return 0;
    }
    if (!dataSetMlm007->load("Daruni.xml", QCAR::STORAGE_APPRESOURCE))
    {
         LOG("Failed to load data set.");
         return 0;
    }
    if (!dataSetAg6000->load("Daruni.xml", QCAR::STORAGE_APPRESOURCE))
    {
         LOG("Failed to load data set.");
         return 0;
    }
    if (!dataSetAg6001->load("Daruni.xml", QCAR::STORAGE_APPRESOURCE))
    {
         LOG("Failed to load data set.");
         return 0;
    }

    if (!dataSetTeapot->load("Daruni.xml", QCAR::STORAGE_APPRESOURCE))
    {
        LOG("Failed to load data set.");
        return 0;
    }


    // Activate the data set: (Escolhe o produto que vai aparecer inicialmente)
    if (!imageTracker->activateDataSet(dataSetMba012))
    {
        LOG("Failed to activate data set.");
        return 0;
    }

    LOG("Successfully loaded and activated data set.");
    return 1;
}


JNIEXPORT int JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_destroyTrackerData(JNIEnv *, jobject)
{
    LOG("Java_com_daruni_QCAR_ImageTargets_ImageTargets_destroyTrackerData");

    // Get the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
        trackerManager.getTracker(QCAR::ImageTracker::getClassType()));
    if (imageTracker == NULL)
    {
        LOG("Failed to destroy the tracking data set because the ImageTracker has not"
            " been initialized.");
        return 0;
    }
    
    if (dataSetTeapot != 0)
    {
        if (imageTracker->getActiveDataSet() == dataSetTeapot &&
            !imageTracker->deactivateDataSet(dataSetTeapot))
        {
            LOG("Failed to destroy the tracking data set StonesAndChips because the data set "
                "could not be deactivated.");
            return 0;
        }

        if (!imageTracker->destroyDataSet(dataSetTeapot))
        {
            LOG("Failed to destroy the tracking data set StonesAndChips.");
            return 0;
        }

        LOG("Successfully destroyed the data set StonesAndChips.");
        dataSetTeapot = 0;
    }


    if (dataSetMbcr1953lqc != 0)
        {
            if (imageTracker->getActiveDataSet() == dataSetMbcr1953lqc &&
                !imageTracker->deactivateDataSet(dataSetMbcr1953lqc))
            {
                LOG("Failed to destroy the tracking data set Tarmac because the data set "
                    "could not be deactivated.");
                return 0;
            }

            if (!imageTracker->destroyDataSet(dataSetMbcr1953lqc))
            {
                LOG("Failed to destroy the tracking data set Tarmac.");
                return 0;
            }

            LOG("Successfully destroyed the data set Tarmac.");
            dataSetMbcr1953lqc = 0;
        }
        if (dataSetMba012 != 0)
        {
            if (imageTracker->getActiveDataSet() == dataSetMba012 &&
                !imageTracker->deactivateDataSet(dataSetMba012))
            {
                LOG("Failed to destroy the tracking data set Tarmac because the data set "
                    "could not be deactivated.");
                return 0;
            }

            if (!imageTracker->destroyDataSet(dataSetMba012))
            {
                LOG("Failed to destroy the tracking data set Tarmac.");
                return 0;
            }

            LOG("Successfully destroyed the data set Tarmac.");
            dataSetMba012 = 0;
        }
        if (dataSetMlm007 != 0)
        {
            if (imageTracker->getActiveDataSet() == dataSetMlm007 &&
                !imageTracker->deactivateDataSet(dataSetMlm007))
            {
                LOG("Failed to destroy the tracking data set Tarmac because the data set "
                    "could not be deactivated.");
                return 0;
            }

            if (!imageTracker->destroyDataSet(dataSetMlm007))
            {
                LOG("Failed to destroy the tracking data set Tarmac.");
                return 0;
            }

            LOG("Successfully destroyed the data set Tarmac.");
            dataSetMlm007 = 0;
        }
        if (dataSetAg6000 != 0)
        {
            if (imageTracker->getActiveDataSet() == dataSetAg6000 &&
                !imageTracker->deactivateDataSet(dataSetAg6000))
            {
                LOG("Failed to destroy the tracking data set Tarmac because the data set "
                    "could not be deactivated.");
                return 0;
            }

            if (!imageTracker->destroyDataSet(dataSetAg6000))
            {
                LOG("Failed to destroy the tracking data set Tarmac.");
                return 0;
            }

            LOG("Successfully destroyed the data set Tarmac.");
            dataSetAg6000 = 0;
        }
        if (dataSetAg6001 != 0)
        {
            if (imageTracker->getActiveDataSet() == dataSetAg6001 &&
                !imageTracker->deactivateDataSet(dataSetAg6001))
            {
                LOG("Failed to destroy the tracking data set Tarmac because the data set "
                    "could not be deactivated.");
                return 0;
            }

            if (!imageTracker->destroyDataSet(dataSetAg6001))
            {
                LOG("Failed to destroy the tracking data set Tarmac.");
                return 0;
            }

            LOG("Successfully destroyed the data set Tarmac.");
            dataSetAg6001 = 0;
        }

    return 1;
}


JNIEXPORT void JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_onQCARInitializedNative(JNIEnv *, jobject)
{
    // Register the update callback where we handle the data set swap:
    QCAR::registerCallback(&updateCallback);

    // Comment in to enable tracking of up to 2 targets simultaneously and
    // split the work over multiple frames:
    // QCAR::setHint(QCAR::HINT_MAX_SIMULTANEOUS_IMAGE_TARGETS, 2);
}


JNIEXPORT void JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargetsRenderer_renderFrame(JNIEnv *, jobject)
{
    //LOG("Java_com_daruni_QCAR_ImageTargets_GLRenderer_renderFrame");

    // Clear color and depth buffer 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Get the state from QCAR and mark the beginning of a rendering section
    QCAR::State state = QCAR::Renderer::getInstance().begin();
    
    // Explicitly render the Video Background
    QCAR::Renderer::getInstance().drawVideoBackground();
       
    glEnable(GL_DEPTH_TEST);

    // We must detect if background reflection is active and adjust the culling direction. 
    // If the reflection is active, this means the post matrix has been reflected as well,
    // therefore standard counter clockwise face culling will result in "inside out" models. 
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    if(QCAR::Renderer::getInstance().getVideoBackgroundConfig().mReflection == QCAR::VIDEO_BACKGROUND_REFLECTION_ON)
        glFrontFace(GL_CW);  //Front camera
    else
        glFrontFace(GL_CCW);   //Back camera


    // Did we find any trackables this frame?
    for(int tIdx = 0; tIdx < state.getNumTrackableResults(); tIdx++)
    {
        // Get the trackable:
        const QCAR::TrackableResult* result = state.getTrackableResult(tIdx);
        const QCAR::Trackable& trackable = result->getTrackable();

        // Get the image tracker:
        QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
        QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
                        trackerManager.getTracker(QCAR::ImageTracker::getClassType()));
        QCAR::Matrix44F modelViewMatrix =
            QCAR::Tool::convertPose2GLMatrix(result->getPose());        

        if(!isExtendedTrackingActivated)
        {
			// Choose the texture based on the target name:
			int textureIndex;
			if (strcmp(trackable.getName(), "chips") == 0)
			{
				textureIndex = 0;
			}
			else if (strcmp(trackable.getName(), "stones") == 0)
			{
				textureIndex = 3;
			}
			else if (strcmp(trackable.getName(), "tarmac") == 0)
			{
				textureIndex = 2;
			}
			else if (strcmp(trackable.getName(), "darunitargetingstones") == 0)
			{
				if (imageTracker->getActiveDataSet() == dataSetTeapot ){
					textureIndex = 3;
				}
				if (imageTracker->getActiveDataSet() == dataSetMbcr1953lqc ){
					textureIndex = 0;
				}
				if (imageTracker->getActiveDataSet() == dataSetMba012 ){
                	textureIndex = 2;
                }
                if (imageTracker->getActiveDataSet() == dataSetMlm007 ){
                	textureIndex = 3;
                }
                if (imageTracker->getActiveDataSet() == dataSetAg6000 ){
                	textureIndex = 4;
                }
                if (imageTracker->getActiveDataSet() == dataSetAg6001 ){
                	textureIndex = 5;
                }
			}
			else
			{
				textureIndex = 0;
			}

			const Texture* const thisTexture = textures[textureIndex];

			QCAR::Matrix44F modelViewProjection;

			SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
											 &modelViewMatrix.data[0]);
			SampleUtils::scalePoseMatrix(kObjectScale, kObjectScale, kObjectScale,
										 &modelViewMatrix.data[0]);
			SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
										&modelViewMatrix.data[0] ,
										&modelViewProjection.data[0]);

			glUseProgram(shaderProgramID);
			glUniformMatrix4fv(modelViewMatrixHandle, 1, GL_FALSE,
			                        (GLfloat*)&modelViewMatrix.data[0]);
			glUniform3f(diffuseMaterialHandle, 1.0f, 1.0f, 1.0f);

			if (imageTracker->getActiveDataSet() == dataSetTeapot ){
				glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
								  (const GLvoid*) &teapotVertices[0]);
				glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
								  (const GLvoid*) &teapotNormals[0]);
				glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
								  (const GLvoid*) &teapotTexCoords[0]);
				glEnableVertexAttribArray(vertexHandle);
				glEnableVertexAttribArray(normalHandle);
				glEnableVertexAttribArray(textureCoordHandle);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
				glUniform1i(texSampler2DHandle, 0 /*GL_TEXTURE0*/);
				glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
								   (GLfloat*)&modelViewProjection.data[0] );

				glDrawElements(GL_TRIANGLES, NUM_TEAPOT_OBJECT_INDEX, GL_UNSIGNED_SHORT,
							   (const GLvoid*) &teapotIndices[0]);

			}




			/***

                Cadeira de Rodas Alumínio MBCR1953LQC


             ***/
			if (imageTracker->getActiveDataSet() == dataSetMbcr1953lqc ){

			    static float kObjectScale = 4.0f;

				glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
								  (const GLvoid*) &polySurface519_001Vertices[0]);
				glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
								  (const GLvoid*) &polySurface519_001Normals[0]);
				glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
								  (const GLvoid*) &polySurface519_001TexCoords[0]);
				glEnableVertexAttribArray(vertexHandle);
				glEnableVertexAttribArray(normalHandle);
				glEnableVertexAttribArray(textureCoordHandle);


				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
				glUniform1i(texSampler2DHandle, 0 );
				glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
								(GLfloat*)&modelViewProjection.data[0] );

				glDrawElements(GL_TRIANGLES, NUM_POLYSURFACE519_001_OBJECT_INDEX, GL_UNSIGNED_SHORT,
							(const GLvoid*) &polySurface519_001Indices[0]);


				if ( isDescActivated ){

					const Texture* const thisTexture = textures[1];

					glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
								(const GLvoid*) &mbcr1953lqc_descVertices[0]);
					glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
								(const GLvoid*) &mbcr1953lqc_descNormals[0]);
					glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
								(const GLvoid*) &mbcr1953lqc_descTexCoords[0]);
					glEnableVertexAttribArray(vertexHandle);
					glEnableVertexAttribArray(normalHandle);
					glEnableVertexAttribArray(textureCoordHandle);

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
					glUniform1i(texSampler2DHandle, 0 );
					glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
							(GLfloat*)&modelViewProjection.data[0] );

					glDrawElements(GL_TRIANGLES, NUM_MBCR1953LQC_DESC_OBJECT_INDEX, GL_UNSIGNED_SHORT,
							(const GLvoid*) &mbcr1953lqc_descIndices[0]);

				}






			}


			/***

                Cadeira de Banho com Encosto MBA012


             ***/

			if (imageTracker->getActiveDataSet() == dataSetMba012 ){

			    static float kObjectScale = 0.7f;

                SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
            					        &modelViewMatrix.data[0]);
                SampleUtils::rotatePoseMatrix(0.0f, 1.0f, 0.0f, 0.0f,
                			            &modelViewMatrix.data[0]);
                SampleUtils::rotatePoseMatrix(0.0f, 0.0f, 1.0f, 0.0f,
			                            &modelViewMatrix.data[0]);
            	SampleUtils::scalePoseMatrix(kObjectScale, kObjectScale, kObjectScale,
            							&modelViewMatrix.data[0]);
            	SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
            						    &modelViewMatrix.data[0] ,
            							&modelViewProjection.data[0]);

            	glUseProgram(shaderProgramID);
            	glUniformMatrix4fv(modelViewMatrixHandle, 1, GL_FALSE,
            			                (GLfloat*)&modelViewMatrix.data[0]);
            	glUniform3f(diffuseMaterialHandle, 1.0f, 1.0f, 1.0f);


                // Drawing


            	glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &mba012Vertices[0]);
            	glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &mba012Normals[0]);
            	glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &mba012TexCoords[0]);
            	glEnableVertexAttribArray(vertexHandle);
            	glEnableVertexAttribArray(normalHandle);
            	glEnableVertexAttribArray(textureCoordHandle);


            	glActiveTexture(GL_TEXTURE0);
            	glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
            	glUniform1i(texSampler2DHandle, 0 );
            	glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
            					(GLfloat*)&modelViewProjection.data[0] );

  				glDrawElements(GL_TRIANGLES, NUM_MBA012_OBJECT_INDEX, GL_UNSIGNED_SHORT,
            				(const GLvoid*) &mba012Indices[0]);


            	if ( isDescActivated ){

                    // Posicionamento
                    //QCAR::Matrix44F modelViewProjection;
                    static float kObjectScale_desc = 1.4f;

            	    SampleUtils::translatePoseMatrix(-15.0f, 0.0f, kObjectScale_desc,
            					        &modelViewMatrix.data[0]);
                    SampleUtils::rotatePoseMatrix(0.0f, 1.0f, 0.0f, 0.0f,
                			            &modelViewMatrix.data[0]);
                    SampleUtils::rotatePoseMatrix(0.0f, 0.0f, 1.0f, 0.0f,
			                            &modelViewMatrix.data[0]);
            	    SampleUtils::scalePoseMatrix(kObjectScale_desc, kObjectScale_desc, kObjectScale_desc,
            							&modelViewMatrix.data[0]);
            	    SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
            						    &modelViewMatrix.data[0] ,
            							&modelViewProjection.data[0]);

            	    glUseProgram(shaderProgramID);
            	    glUniformMatrix4fv(modelViewMatrixHandle, 1, GL_FALSE,
            			                (GLfloat*)&modelViewMatrix.data[0]);
            	    glUniform3f(diffuseMaterialHandle, 1.0f, 1.0f, 1.0f);


                    // Drawing


            		const Texture* const thisTexture = textures[1];

            		glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
            					(const GLvoid*) &mba012_descVertices[0]);
            		glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
            					(const GLvoid*) &mba012_descNormals[0]);
            		glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
            					(const GLvoid*) &mba012_descTexCoords[0]);
            		glEnableVertexAttribArray(vertexHandle);
            		glEnableVertexAttribArray(normalHandle);
            		glEnableVertexAttribArray(textureCoordHandle);

            		glActiveTexture(GL_TEXTURE0);
            		glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
            		glUniform1i(texSampler2DHandle, 0 );
            		glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
            				(GLfloat*)&modelViewProjection.data[0] );

            		glDrawElements(GL_TRIANGLES, NUM_MBA012_DESC_OBJECT_INDEX, GL_UNSIGNED_SHORT,
            				(const GLvoid*) &mba012_descIndices[0]);

            	}
            }

            /***

             Bengala com Assendo MLM007 fechada e aberta


             ***/
            if (imageTracker->getActiveDataSet() == dataSetMlm007 ){

                const Texture* const thisTexture = textures[textureIndex];
                static float kObjectScale = 0.6f;

                // Bengala com assento fechada

                // Posicionamento
                QCAR::Matrix44F modelViewProjection;

            	SampleUtils::translatePoseMatrix(-25.0f, 0.0f, kObjectScale,
            					        &modelViewMatrix.data[0]);
                SampleUtils::rotatePoseMatrix(90.0f, 1.0f, 0.0f, 0.0f,
                			            &modelViewMatrix.data[0]);
                SampleUtils::rotatePoseMatrix(180.0f, 0.0f, 1.0f, 0.0f,
			                            &modelViewMatrix.data[0]);
            	SampleUtils::scalePoseMatrix(kObjectScale, kObjectScale, kObjectScale,
            							&modelViewMatrix.data[0]);
            	SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
            						    &modelViewMatrix.data[0] ,
            							&modelViewProjection.data[0]);

            	glUseProgram(shaderProgramID);
            	glUniformMatrix4fv(modelViewMatrixHandle, 1, GL_FALSE,
            			                (GLfloat*)&modelViewMatrix.data[0]);
            	glUniform3f(diffuseMaterialHandle, 1.0f, 1.0f, 1.0f);


            	// drawing
            	glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &mlm007_closedVertices[0]);
            	glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &mlm007_closedNormals[0]);
            	glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &mlm007_closedTexCoords[0]);
            	glEnableVertexAttribArray(vertexHandle);
            	glEnableVertexAttribArray(normalHandle);
            	glEnableVertexAttribArray(textureCoordHandle);


            	glActiveTexture(GL_TEXTURE0);
            	glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
            	glUniform1i(texSampler2DHandle, 0 );
            	glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
            					(GLfloat*)&modelViewProjection.data[0] );

  				glDrawElements(GL_TRIANGLES, NUM_MLM007_CLOSED_OBJECT_INDEX, GL_UNSIGNED_SHORT,
            				(const GLvoid*) &mlm007_closedIndices[0]);






                // Bengala com assento aberta

                // Posicionamento
                //QCAR::Matrix44F modelViewProjection;
                kObjectScale = 0.8f;

            	SampleUtils::translatePoseMatrix(-50.0f, 0.0f, kObjectScale,
            					        &modelViewMatrix.data[0]);
                SampleUtils::rotatePoseMatrix(0.0f, 1.0f, 0.0f, 0.0f,
                			            &modelViewMatrix.data[0]);
                SampleUtils::rotatePoseMatrix(0.0f, 0.0f, 1.0f, 0.0f,
			                            &modelViewMatrix.data[0]);
            	SampleUtils::scalePoseMatrix(kObjectScale, kObjectScale, kObjectScale,
            							&modelViewMatrix.data[0]);
            	SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
            						    &modelViewMatrix.data[0] ,
            							&modelViewProjection.data[0]);

            	glUseProgram(shaderProgramID);
            	glUniformMatrix4fv(modelViewMatrixHandle, 1, GL_FALSE,
            			                (GLfloat*)&modelViewMatrix.data[0]);
            	glUniform3f(diffuseMaterialHandle, 1.0f, 1.0f, 1.0f);


            	// drawing
            	glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &mlm007_openedVertices[0]);
            	glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &mlm007_openedNormals[0]);
            	glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &mlm007_openedTexCoords[0]);
            	glEnableVertexAttribArray(vertexHandle);
            	glEnableVertexAttribArray(normalHandle);
            	glEnableVertexAttribArray(textureCoordHandle);


            	glActiveTexture(GL_TEXTURE0);
            	glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
            	glUniform1i(texSampler2DHandle, 0 );
            	glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
            					(GLfloat*)&modelViewProjection.data[0] );

  				glDrawElements(GL_TRIANGLES, NUM_MLM007_OPENED_OBJECT_INDEX, GL_UNSIGNED_SHORT,
            				(const GLvoid*) &mlm007_openedIndices[0]);




            	if ( isDescActivated ){
                    // Posicionamento
                    //QCAR::Matrix44F modelViewProjection;
                    static float kObjectScale_desc = 1.5f;

            	    SampleUtils::translatePoseMatrix(50.0f, 0.0f, kObjectScale_desc,
            					        &modelViewMatrix.data[0]);
                    SampleUtils::rotatePoseMatrix(90.0f, 1.0f, 0.0f, 0.0f,
                			            &modelViewMatrix.data[0]);
                    SampleUtils::rotatePoseMatrix(180.0f, 0.0f, 1.0f, 0.0f,
			                            &modelViewMatrix.data[0]);
            	    SampleUtils::scalePoseMatrix(kObjectScale_desc, kObjectScale_desc, kObjectScale_desc,
            							&modelViewMatrix.data[0]);
            	    SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
            						    &modelViewMatrix.data[0] ,
            							&modelViewProjection.data[0]);

            	    glUseProgram(shaderProgramID);
            	    glUniformMatrix4fv(modelViewMatrixHandle, 1, GL_FALSE,
            			                (GLfloat*)&modelViewMatrix.data[0]);
            	    glUniform3f(diffuseMaterialHandle, 1.0f, 1.0f, 1.0f);


                    // Drawing
            		const Texture* const thisTexture = textures[1];

            		glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
            					(const GLvoid*) &mlm007_descVertices[0]);
            		glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
            					(const GLvoid*) &mlm007_descNormals[0]);
            		glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
            					(const GLvoid*) &mlm007_descTexCoords[0]);
            		glEnableVertexAttribArray(vertexHandle);
            		glEnableVertexAttribArray(normalHandle);
            		glEnableVertexAttribArray(textureCoordHandle);

            		glActiveTexture(GL_TEXTURE0);
            		glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
            		glUniform1i(texSampler2DHandle, 0 );
            		glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
            				(GLfloat*)&modelViewProjection.data[0] );

            		glDrawElements(GL_TRIANGLES, NUM_MLM007_DESC_OBJECT_INDEX, GL_UNSIGNED_SHORT,
            				(const GLvoid*) &mlm007_descIndices[0]);

            	}
            }
            /***

             Andador AG6000


             ***/
			if (imageTracker->getActiveDataSet() == dataSetAg6000 ){

			    static float kObjectScale = 0.7f;

                SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
            					        &modelViewMatrix.data[0]);
                SampleUtils::rotatePoseMatrix(90.0f, 1.0f, 0.0f, 0.0f, //rotate eixo X
                			            &modelViewMatrix.data[0]);
                SampleUtils::rotatePoseMatrix(-90.0f, 0.0f, 1.0f, 0.0f, //rotate eixo Z
			                            &modelViewMatrix.data[0]);
            	SampleUtils::scalePoseMatrix(kObjectScale, kObjectScale, kObjectScale,
            							&modelViewMatrix.data[0]);
            	SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
            						    &modelViewMatrix.data[0] ,
            							&modelViewProjection.data[0]);

            	glUseProgram(shaderProgramID);
            	glUniformMatrix4fv(modelViewMatrixHandle, 1, GL_FALSE,
            			                (GLfloat*)&modelViewMatrix.data[0]);
            	glUniform3f(diffuseMaterialHandle, 1.0f, 1.0f, 1.0f);


                // Drawing


            	glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &ag6000Vertices[0]);
            	glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &ag6000Normals[0]);
            	glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &ag6000TexCoords[0]);
            	glEnableVertexAttribArray(vertexHandle);
            	glEnableVertexAttribArray(normalHandle);
            	glEnableVertexAttribArray(textureCoordHandle);


            	glActiveTexture(GL_TEXTURE0);
            	glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
            	glUniform1i(texSampler2DHandle, 0 );
            	glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
            					(GLfloat*)&modelViewProjection.data[0] );

  				glDrawElements(GL_TRIANGLES, NUM_AG6000_OBJECT_INDEX, GL_UNSIGNED_SHORT,
            				(const GLvoid*) &ag6000Indices[0]);


            	if ( isDescActivated ){

                    // Posicionamento
                    //QCAR::Matrix44F modelViewProjection;
                    static float kObjectScale_desc = 1.5f;

            	    SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale_desc,
            					        &modelViewMatrix.data[0]);
                    SampleUtils::rotatePoseMatrix(-90.0f, 1.0f, 0.0f, 0.0f,
                			            &modelViewMatrix.data[0]);
                    SampleUtils::rotatePoseMatrix(0.0f, 0.0f, 1.0f, 0.0f,
			                            &modelViewMatrix.data[0]);
            	    SampleUtils::scalePoseMatrix(kObjectScale_desc, kObjectScale_desc, kObjectScale_desc,
            							&modelViewMatrix.data[0]);
            	    SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
            						    &modelViewMatrix.data[0] ,
            							&modelViewProjection.data[0]);

            	    glUseProgram(shaderProgramID);
            	    glUniformMatrix4fv(modelViewMatrixHandle, 1, GL_FALSE,
            			                (GLfloat*)&modelViewMatrix.data[0]);
            	    glUniform3f(diffuseMaterialHandle, 1.0f, 1.0f, 1.0f);


                    // Drawing


            		const Texture* const thisTexture = textures[1];

            		glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
            					(const GLvoid*) &ag6000_textVertices[0]);
            		glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
            					(const GLvoid*) &ag6000_textNormals[0]);
            		glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
            					(const GLvoid*) &ag6000_textTexCoords[0]);
            		glEnableVertexAttribArray(vertexHandle);
            		glEnableVertexAttribArray(normalHandle);
            		glEnableVertexAttribArray(textureCoordHandle);

            		glActiveTexture(GL_TEXTURE0);
            		glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
            		glUniform1i(texSampler2DHandle, 0 );
            		glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
            				(GLfloat*)&modelViewProjection.data[0] );

            		glDrawElements(GL_TRIANGLES, NUM_AG6000_TEXT_OBJECT_INDEX, GL_UNSIGNED_SHORT,
            				(const GLvoid*) &ag6000_textIndices[0]);

            	}
            }

            /***

             Início Andador AG6001

             ***/
			if (imageTracker->getActiveDataSet() == dataSetAg6001 ){

			    static float kObjectScale = 0.7f;

                SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
            					        &modelViewMatrix.data[0]);
                SampleUtils::rotatePoseMatrix(90.0f, 1.0f, 0.0f, 0.0f, //rotate eixo X
                			            &modelViewMatrix.data[0]);
                SampleUtils::rotatePoseMatrix(-90.0f, 0.0f, 1.0f, 0.0f, //rotate eixo Z
			                            &modelViewMatrix.data[0]);
            	SampleUtils::scalePoseMatrix(kObjectScale, kObjectScale, kObjectScale,
            							&modelViewMatrix.data[0]);
            	SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
            						    &modelViewMatrix.data[0] ,
            							&modelViewProjection.data[0]);

            	glUseProgram(shaderProgramID);
            	glUniformMatrix4fv(modelViewMatrixHandle, 1, GL_FALSE,
            			                (GLfloat*)&modelViewMatrix.data[0]);
            	glUniform3f(diffuseMaterialHandle, 1.0f, 1.0f, 1.0f);


                // Drawing


            	glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &ag6001Vertices[0]);
            	glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &ag6001Normals[0]);
            	glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
            							(const GLvoid*) &ag6001TexCoords[0]);
            	glEnableVertexAttribArray(vertexHandle);
            	glEnableVertexAttribArray(normalHandle);
            	glEnableVertexAttribArray(textureCoordHandle);


            	glActiveTexture(GL_TEXTURE0);
            	glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
            	glUniform1i(texSampler2DHandle, 0 );
            	glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
            					(GLfloat*)&modelViewProjection.data[0] );

  				glDrawElements(GL_TRIANGLES, NUM_AG6001_OBJECT_INDEX, GL_UNSIGNED_SHORT,
            				(const GLvoid*) &ag6001Indices[0]);


            	if ( isDescActivated ){

                    // Posicionamento
                    //QCAR::Matrix44F modelViewProjection;
                    static float kObjectScale_desc = 1.5f;

            	    //SampleUtils::translatePoseMatrix(0.0f, 100.0f, kObjectScale_desc,
            		//			        &modelViewMatrix.data[0]);
            	    SampleUtils::translatePoseMatrix(0.0f, 100.0f, -70.0f,
            					        &modelViewMatrix.data[0]);

                    SampleUtils::rotatePoseMatrix(0.0f, 1.0f, 0.0f, 0.0f,
                			            &modelViewMatrix.data[0]);
                    SampleUtils::rotatePoseMatrix(180.0f, 0.0f, 1.0f, 0.0f,
			                            &modelViewMatrix.data[0]);
            	    SampleUtils::scalePoseMatrix(kObjectScale_desc, kObjectScale_desc, kObjectScale_desc,
            							&modelViewMatrix.data[0]);
            	    SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
            						    &modelViewMatrix.data[0] ,
            							&modelViewProjection.data[0]);

            	    glUseProgram(shaderProgramID);
            	    glUniformMatrix4fv(modelViewMatrixHandle, 1, GL_FALSE,
            			                (GLfloat*)&modelViewMatrix.data[0]);
            	    glUniform3f(diffuseMaterialHandle, 1.0f, 1.0f, 1.0f);


                    // Drawing


            		const Texture* const thisTexture = textures[1];

            		glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
            					(const GLvoid*) &ag6001_textVertices[0]);
            		glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
            					(const GLvoid*) &ag6001_textNormals[0]);
            		glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
            					(const GLvoid*) &ag6001_textTexCoords[0]);
            		glEnableVertexAttribArray(vertexHandle);
            		glEnableVertexAttribArray(normalHandle);
            		glEnableVertexAttribArray(textureCoordHandle);

            		glActiveTexture(GL_TEXTURE0);
            		glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
            		glUniform1i(texSampler2DHandle, 0 );
            		glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
            				(GLfloat*)&modelViewProjection.data[0] );

            		glDrawElements(GL_TRIANGLES, NUM_AG6001_TEXT_OBJECT_INDEX, GL_UNSIGNED_SHORT,
            				(const GLvoid*) &ag6001_textIndices[0]);

            	}
            }
            /***

             Fim Andador AG6001

             ***/





			/* glEnableVertexAttribArray(vertexHandle);
			glEnableVertexAttribArray(normalHandle);
			glEnableVertexAttribArray(textureCoordHandle);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
			glUniform1i(texSampler2DHandle, 0);
			glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
							   (GLfloat*)&modelViewProjection.data[0] );
			if (imageTracker->getActiveDataSet() == dataSetTeapot ){
				glDrawElements(GL_TRIANGLES, NUM_TEAPOT_OBJECT_INDEX, GL_UNSIGNED_SHORT,
						   (const GLvoid*) &teapotIndices[0]);
			}
			if (imageTracker->getActiveDataSet() == dataSetMbcr1953lqc ){
				glDrawElements(GL_TRIANGLES, NUM_POLYSURFACE104_OBJECT_INDEX, GL_UNSIGNED_SHORT,
									   (const GLvoid*) &polySurface104Indices[0]);
				glDrawElements(GL_TRIANGLES, NUM_TEAPOT_OBJECT_INDEX, GL_UNSIGNED_SHORT,
										   (const GLvoid*) &teapotIndices[0]);
			}*/
			glDisableVertexAttribArray(vertexHandle);
			glDisableVertexAttribArray(normalHandle);
			glDisableVertexAttribArray(textureCoordHandle);

			SampleUtils::checkGlError("ImageTargets renderFrame");
        }
        else
        {
			const Texture* const thisTexture = textures[0];

			QCAR::Matrix44F modelViewProjection;

			SampleUtils::translatePoseMatrix(0.0f, 0.0f, kBuildingsObjectScale,
											 &modelViewMatrix.data[0]);
			SampleUtils::rotatePoseMatrix(90.0f, 1.0f, 0.0f, 0.0f,
			                                  &modelViewMatrix.data[0]);

			SampleUtils::scalePoseMatrix(kBuildingsObjectScale, kBuildingsObjectScale, kBuildingsObjectScale,
										 &modelViewMatrix.data[0]);
			SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
										&modelViewMatrix.data[0] ,
										&modelViewProjection.data[0]);


			/*SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
														 &modelViewMatrix.data[0]);

			SampleUtils::rotatePoseMatrix(0.0f, 1.0f, 0.0f, 0.0f,
									                      &modelViewMatrix.data[0])
			SampleUtils::scalePoseMatrix(kObjectScale, kObjectScale, kObjectScale,
													 &modelViewMatrix.data[0]);
			SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
													&modelViewMatrix.data[0] ,
													&modelViewProjection.data[0]);*/

			glUseProgram(shaderProgramID);
			glUniformMatrix4fv(modelViewMatrixHandle, 1, GL_FALSE,
			                        (GLfloat*)&modelViewMatrix.data[0]);
			glUniform3f(diffuseMaterialHandle, 1.0f, 1.0f, 1.0f);


			glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
								  (const GLvoid*) &buildingsVerts[0]);
			glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
								  (const GLvoid*) &buildingsNormals[0]);
			glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
								  (const GLvoid*) &buildingsTexCoords[0]);

			/*glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
											  (const GLvoid*) &mbcr1953lqcVertices[0]);
			glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
											  (const GLvoid*) &mbcr1953lqcNormals[0]);
			glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
											  (const GLvoid*) &mbcr1953lqcTexCoords[0]);*/

			glEnableVertexAttribArray(vertexHandle);
			glEnableVertexAttribArray(normalHandle);
			glEnableVertexAttribArray(textureCoordHandle);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
			glUniform1i(texSampler2DHandle, 0 /*GL_TEXTURE0*/);
			glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
							   (GLfloat*)&modelViewProjection.data[0] );
			glDrawArrays(GL_TRIANGLES, 0, buildingsNumVerts);
			/*glDrawElements(GL_TRIANGLES, NUM_MBCR1953LQC_OBJECT_INDEX, GL_UNSIGNED_SHORT,
												   (const GLvoid*) &mbcr1953lqcIndices[0]);*/


			glDisableVertexAttribArray(vertexHandle);
			glDisableVertexAttribArray(normalHandle);
			glDisableVertexAttribArray(textureCoordHandle);

			SampleUtils::checkGlError("ImageTargets renderFrame");
        }
    }

    glDisable(GL_DEPTH_TEST);

    QCAR::Renderer::getInstance().end();
}


void
configureVideoBackground()
{
    // Get the default video mode:
    QCAR::CameraDevice& cameraDevice = QCAR::CameraDevice::getInstance();
    QCAR::VideoMode videoMode = cameraDevice.
                                getVideoMode(QCAR::CameraDevice::MODE_DEFAULT);


    // Configure the video background
    QCAR::VideoBackgroundConfig config;
    config.mEnabled = true;
    config.mSynchronous = true;
    config.mPosition.data[0] = 0.0f;
    config.mPosition.data[1] = 0.0f;
    
    if (isActivityInPortraitMode)
    {
        //LOG("configureVideoBackground PORTRAIT");
        config.mSize.data[0] = videoMode.mHeight
                                * (screenHeight / (float)videoMode.mWidth);
        config.mSize.data[1] = screenHeight;

        if(config.mSize.data[0] < screenWidth)
        {
            LOG("Correcting rendering background size to handle missmatch between screen and video aspect ratios.");
            config.mSize.data[0] = screenWidth;
            config.mSize.data[1] = screenWidth * 
                              (videoMode.mWidth / (float)videoMode.mHeight);
        }
    }
    else
    {
        //LOG("configureVideoBackground LANDSCAPE");
        config.mSize.data[0] = screenWidth;
        config.mSize.data[1] = videoMode.mHeight
                            * (screenWidth / (float)videoMode.mWidth);

        if(config.mSize.data[1] < screenHeight)
        {
            LOG("Correcting rendering background size to handle missmatch between screen and video aspect ratios.");
            config.mSize.data[0] = screenHeight
                                * (videoMode.mWidth / (float)videoMode.mHeight);
            config.mSize.data[1] = screenHeight;
        }
    }

    LOG("Configure Video Background : Video (%d,%d), Screen (%d,%d), mSize (%d,%d)", videoMode.mWidth, videoMode.mHeight, screenWidth, screenHeight, config.mSize.data[0], config.mSize.data[1]);

    // Set the config:
    QCAR::Renderer::getInstance().setVideoBackgroundConfig(config);
}


JNIEXPORT void JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_initApplicationNative(
                            JNIEnv* env, jobject obj, jint width, jint height)
{
    LOG("Java_com_daruni_QCAR_ImageTargets_ImageTargets_initApplicationNative");
    
    // Store screen dimensions
    screenWidth = width;
    screenHeight = height;
        
    // Handle to the activity class:
    jclass activityClass = env->GetObjectClass(obj);

    jmethodID getTextureCountMethodID = env->GetMethodID(activityClass,
                                                    "getTextureCount", "()I");
    if (getTextureCountMethodID == 0)
    {
        LOG("Function getTextureCount() not found.");
        return;
    }

    textureCount = env->CallIntMethod(obj, getTextureCountMethodID);    
    if (!textureCount)
    {
        LOG("getTextureCount() returned zero.");
        return;
    }

    textures = new Texture*[textureCount];

    jmethodID getTextureMethodID = env->GetMethodID(activityClass,
        "getTexture", "(I)Lcom/daruni/QCAR/ImageTargets/Texture;");

    if (getTextureMethodID == 0)
    {
        LOG("Function getTexture() not found.");
        return;
    }

    // Register the textures
    for (int i = 0; i < textureCount; ++i)
    {

        jobject textureObject = env->CallObjectMethod(obj, getTextureMethodID, i); 
        if (textureObject == NULL)
        {
            LOG("GetTexture() returned zero pointer");
            return;
        }

        textures[i] = Texture::create(env, textureObject);
    }
    LOG("Java_com_daruni_QCAR_ImageTargets_ImageTargets_initApplicationNative finished");
}


JNIEXPORT void JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_deinitApplicationNative(
                                                        JNIEnv* env, jobject obj)
{
    LOG("Java_com_daruni_QCAR_ImageTargets_ImageTargets_deinitApplicationNative");

    isExtendedTrackingActivated = false;

    // Release texture resources
    if (textures != 0)
    {    
        for (int i = 0; i < textureCount; ++i)
        {
            delete textures[i];
            textures[i] = NULL;
        }
    
        delete[]textures;
        textures = NULL;
        
        textureCount = 0;
    }
}


JNIEXPORT void JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_startCamera(JNIEnv *,
                                                                         jobject, jint camera)
{
    LOG("Java_com_daruni_QCAR_ImageTargets_ImageTargets_startCamera");
    
	currentCamera = static_cast<QCAR::CameraDevice::CAMERA> (camera);

    // Initialize the camera:
    if (!QCAR::CameraDevice::getInstance().init(currentCamera))
        return;

    // Configure the video background
    configureVideoBackground();

    // Select the default mode:
    if (!QCAR::CameraDevice::getInstance().selectVideoMode(
                                QCAR::CameraDevice::MODE_DEFAULT))
        return;

    // Start the camera:
    if (!QCAR::CameraDevice::getInstance().start())
        return;

    // Uncomment to enable flash
    //if(QCAR::CameraDevice::getInstance().setFlashTorchMode(true))
    //    LOG("IMAGE TARGETS : enabled torch");

    // Uncomment to enable infinity focus mode, or any other supported focus mode
    // See CameraDevice.h for supported focus modes
    //if(QCAR::CameraDevice::getInstance().setFocusMode(QCAR::CameraDevice::FOCUS_MODE_INFINITY))
    //    LOG("IMAGE TARGETS : enabled infinity focus");

    // Start the tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* imageTracker = trackerManager.getTracker(QCAR::ImageTracker::getClassType());
    if(imageTracker != 0)
        imageTracker->start();
}


JNIEXPORT void JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_stopCamera(JNIEnv *, jobject)
{
    LOG("Java_com_daruni_QCAR_ImageTargets_ImageTargets_stopCamera");

    // Stop the tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* imageTracker = trackerManager.getTracker(QCAR::ImageTracker::getClassType());
    if(imageTracker != 0)
        imageTracker->stop();
    
    QCAR::CameraDevice::getInstance().stop();
    QCAR::CameraDevice::getInstance().deinit();
}


JNIEXPORT void JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_setProjectionMatrix(JNIEnv *, jobject)
{
    LOG("Java_com_daruni_QCAR_ImageTargets_ImageTargets_setProjectionMatrix");

    // Cache the projection matrix:
    const QCAR::CameraCalibration& cameraCalibration =
                                QCAR::CameraDevice::getInstance().getCameraCalibration();
    projectionMatrix = QCAR::Tool::getProjectionGL(cameraCalibration, 10.0f, 5000.0f);
}

// ----------------------------------------------------------------------------
// Activates Camera Flash
// ----------------------------------------------------------------------------
JNIEXPORT jboolean JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_activateFlash(JNIEnv*, jobject, jboolean flash)
{
    return QCAR::CameraDevice::getInstance().setFlashTorchMode((flash==JNI_TRUE)) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_autofocus(JNIEnv*, jobject)
{
    return QCAR::CameraDevice::getInstance().setFocusMode(QCAR::CameraDevice::FOCUS_MODE_TRIGGERAUTO) ? JNI_TRUE : JNI_FALSE;
}


JNIEXPORT jboolean JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_setFocusMode(JNIEnv*, jobject, jint mode)
{
    int qcarFocusMode;

    switch ((int)mode)
    {
        case 0:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_NORMAL;
            break;
        
        case 1:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_CONTINUOUSAUTO;
            break;
            
        case 2:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_INFINITY;
            break;
            
        case 3:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_MACRO;
            break;
    
        default:
            return JNI_FALSE;
    }
    
    return QCAR::CameraDevice::getInstance().setFocusMode(qcarFocusMode) ? JNI_TRUE : JNI_FALSE;
}


JNIEXPORT jboolean JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_startExtendedTracking(JNIEnv*, jobject)
{
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
          trackerManager.getTracker(QCAR::ImageTracker::getClassType()));

    QCAR::DataSet* currentDataSet = imageTracker->getActiveDataSet();
    if (imageTracker == 0 || currentDataSet == 0)
    	return JNI_FALSE;

    for (int tIdx = 0; tIdx < currentDataSet->getNumTrackables(); tIdx++)
    {
        QCAR::Trackable* trackable = currentDataSet->getTrackable(tIdx);
        if(!trackable->startExtendedTracking())
        	return JNI_FALSE;
    }

    isExtendedTrackingActivated = true;
    return JNI_TRUE;
}


JNIEXPORT jboolean JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_stopExtendedTracking(JNIEnv*, jobject)
{
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
          trackerManager.getTracker(QCAR::ImageTracker::getClassType()));

    QCAR::DataSet* currentDataSet = imageTracker->getActiveDataSet();
    if (imageTracker == 0 || currentDataSet == 0)
    	return JNI_FALSE;

    for (int tIdx = 0; tIdx < currentDataSet->getNumTrackables(); tIdx++)
    {
    	QCAR::Trackable* trackable = currentDataSet->getTrackable(tIdx);
        if(!trackable->stopExtendedTracking())
        	return JNI_FALSE;
    }
    
    isExtendedTrackingActivated = false;
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_stopDesc(JNIEnv*, jobject)
{
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
          trackerManager.getTracker(QCAR::ImageTracker::getClassType()));

    QCAR::DataSet* currentDataSet = imageTracker->getActiveDataSet();
    if (imageTracker == 0 || currentDataSet == 0)
    	return JNI_FALSE;

    for (int tIdx = 0; tIdx < currentDataSet->getNumTrackables(); tIdx++)
    {
    	QCAR::Trackable* trackable = currentDataSet->getTrackable(tIdx);
        if(!trackable->stopExtendedTracking())
        	return JNI_FALSE;
    }

    isDescActivated = false;
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargets_startDesc(JNIEnv*, jobject)
{
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
          trackerManager.getTracker(QCAR::ImageTracker::getClassType()));

    QCAR::DataSet* currentDataSet = imageTracker->getActiveDataSet();
    if (imageTracker == 0 || currentDataSet == 0)
    	return JNI_FALSE;

    for (int tIdx = 0; tIdx < currentDataSet->getNumTrackables(); tIdx++)
    {
    	QCAR::Trackable* trackable = currentDataSet->getTrackable(tIdx);
        if(!trackable->stopExtendedTracking())
        	return JNI_FALSE;
    }

    isDescActivated = true;
    return JNI_TRUE;
}


JNIEXPORT void JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargetsRenderer_initRendering(
                                                    JNIEnv* env, jobject obj)
{
    LOG("Java_com_daruni_QCAR_ImageTargets_ImageTargetsRenderer_initRendering");

    // Define clear color
    glClearColor(0.0f, 0.0f, 0.0f, QCAR::requiresAlpha() ? 0.0f : 1.0f);
    
    // Now generate the OpenGL texture objects and add settings
    for (int i = 0; i < textureCount; ++i)
    {
        glGenTextures(1, &(textures[i]->mTextureID));
        glBindTexture(GL_TEXTURE_2D, textures[i]->mTextureID);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textures[i]->mWidth,
                textures[i]->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                (GLvoid*)  textures[i]->mData);
    }
  
    shaderProgramID     = SampleUtils::createProgramFromBuffer(cubeMeshVertexShader,
                                                            cubeFragmentShader);

    vertexHandle        = glGetAttribLocation(shaderProgramID,
                                                "vertexPosition");
    normalHandle        = glGetAttribLocation(shaderProgramID,
                                                "vertexNormal");
    textureCoordHandle  = glGetAttribLocation(shaderProgramID,
                                                "vertexTexCoord");
    mvpMatrixHandle     = glGetUniformLocation(shaderProgramID,
                                                "modelViewProjectionMatrix");
    texSampler2DHandle  = glGetUniformLocation(shaderProgramID, 
                                                "texSampler2D");
    modelViewMatrixHandle = glGetUniformLocation(shaderProgramID, "modelViewMatrix");
    diffuseMaterialHandle = glGetUniformLocation(shaderProgramID, "diffuseMaterial");
                                                
}


JNIEXPORT void JNICALL
Java_com_daruni_QCAR_ImageTargets_ImageTargetsRenderer_updateRendering(
                        JNIEnv* env, jobject obj, jint width, jint height)
{
    LOG("Java_com_daruni_QCAR_ImageTargets_ImageTargetsRenderer_updateRendering");

    // Update screen dimensions
    screenWidth = width;
    screenHeight = height;

    // Reconfigure the video background
    configureVideoBackground();
}


#ifdef __cplusplus
}
#endif
