/*===============================================================================
Copyright (c) 2012-2014 Qualcomm Connected Experiences, Inc. All Rights Reserved.

Vuforia is a trademark of QUALCOMM Incorporated, registered in the United States 
and other countries. Trademarks of QUALCOMM Incorporated are used with permission.
===============================================================================*/

#ifndef _QCAR_CUBE_SHADERS_H_
#define _QCAR_CUBE_SHADERS_H_


static const char* cubeMeshVertexShader = " \
\
attribute vec4 vertexPosition; \
attribute vec4 vertexNormal; \
attribute vec2 vertexTexCoord; \
varying vec2 texCoord; \
varying vec4 normal; \
\
uniform mat4 modelViewMatrix; \
uniform mat4 modelViewProjectionMatrix; \
\
void main()  { \
   gl_Position = modelViewProjectionMatrix * vertexPosition; \
   normal = modelViewMatrix * vec4(vertexNormal.xyz, 0.0); \
   normal = normalize(normal); \
   texCoord = vertexTexCoord; \
}";
\
\
static const char* cubeFragmentShader = " \
precision mediump float; \
varying vec2 texCoord; \
varying vec4 normal; \
uniform vec3 diffuseMaterial; \
uniform sampler2D texSampler2D; \
\
\
void main()  { \
   vec3 n = normalize(normal.xyz); \
   vec3 lightDir = vec3(0.0, 0.0, -1.0); \
   vec3 ambient = vec3(0, 0, 0); \
   vec3 diffuseLight = vec3(1.2, 1.2, 1.2); \
   float diffuseFactor = max(0.0, dot(n, lightDir)); \
   vec3 diffuse = diffuseFactor * diffuseMaterial * diffuseLight; \
   vec3 shadedColor = ambient + diffuse; \
\
   gl_FragColor = vec4(shadedColor, 1.0) * texture2D(texSampler2D, texCoord); \
}";

#endif // _QCAR_CUBE_SHADERS_H_
