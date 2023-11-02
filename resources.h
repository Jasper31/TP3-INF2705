#ifndef RESOURCES_H
#define RESOURCES_H

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "shader_program.h"
#include "shapes.h"
#include "model.h"
#include "textures.h"

class Resources
{
public:
    Resources();

    // #############
    // SHADERS
    // #############
    ShaderProgram model;
    GLint mvpLocationModel;
    // ---
    ShaderProgram water;
    GLint mvpLocationWater;
    GLint timeLocationWater;
    // ---
    ShaderProgram grassShader;
    GLint mvpLocationGrass;
    GLint timeLocationGrass;
    // ---
    ShaderProgram phong;
    GLint mvpLocationPhong;
    GLint modelViewLocationPhong;
    GLint viewLocationPhong;
    GLint normalLocationPhong;
    // ---
    ShaderProgram gouraud;
    GLint mvpLocationGouraud;
    GLint modelViewLocationGouraud;
    GLint viewLocationGouraud;
    GLint normalLocationGouraud;
    // ---
    ShaderProgram flat;
    GLint mvpLocationFlat;
    GLint modelViewLocationFlat;
    GLint viewLocationFlat;
    GLint normalLocationFlat;
    // ---
    ShaderProgram simple;
    GLint mvpLocationSimple;
    GLint colorLocationSimple;
    // ---
    ShaderProgram skyboxShader;
    GLint mvpLocationSky;
    // #############
    // SIMPLE GEOMETRY MESHES
    // #############
    BasicShapeElements ground;
    BasicShapeElements river;
    BasicShapeElements quad;
    BasicShapeArrays grass;
    GLsizei grassCount;
    BasicShapeArrays skybox;
    // #############
    // MODELS
    // #############
    Model rock;
    Model shroom;
    Model tree;
    Model suzanne;
    Model glass;
    Model sphere;
    Model cube;
    Model spotlight;
    // #############
    // TEXTURES
    // #############
    Texture2D rockTexture;
    Texture2D shroomTexture;
    Texture2D treeTexture;
    Texture2D suzanneTexture;
    Texture2D glassTexture;
    Texture2D groundTexture;
    Texture2D riverTexture;
    Texture2D grassCluterTexture;
    Texture2D billboardTexture;
    Texture2D heartTexture;
    Texture2D whiteTexture;
    Texture2D diffuseMapTexture;
    Texture2D specularMapTexture;
    TextureCubeMap skyboxTexture;
};

#endif // RESOURCES_H
