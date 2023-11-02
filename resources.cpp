#include "resources.h"

#include "vertices_data.h"
#include "utils.h"

const char* pathes[] = {
    "../textures/skybox/Daylight Box_Right.bmp",
    "../textures/skybox/Daylight Box_Left.bmp",
    "../textures/skybox/Daylight Box_Top.bmp",
    "../textures/skybox/Daylight Box_Bottom.bmp",
    "../textures/skybox/Daylight Box_Front.bmp",
    "../textures/skybox/Daylight Box_Back.bmp",
};

Resources::Resources()
: ground(groundVertices, sizeof(groundVertices), groundIndexes, sizeof(groundIndexes))
, river(riverVertices, sizeof(riverVertices), groundIndexes, sizeof(groundIndexes))
, quad(quad2dVertices, sizeof(quad2dVertices), quad2dIndexes, sizeof(quad2dIndexes))
, skybox(skyboxVertices, sizeof(skyboxVertices))
, grassCount(0)
, rock("../models/rock.obj")
, shroom("../models/mushroom.obj")
, tree("../models/tree.obj")
, suzanne("../models/suzanne.obj")
, glass("../models/glass.obj")
, sphere("../models/icosphere.obj")
, cube("../models/cube.obj")
, spotlight("../models/spotlight.obj")
, rockTexture("../models/rockTexture.png", GL_CLAMP_TO_EDGE)
, shroomTexture("../models/mushroomTexture.png", GL_CLAMP_TO_EDGE)
, treeTexture("../models/treeTexture.png", GL_CLAMP_TO_EDGE)
, suzanneTexture("../models/suzanneTexture.png", GL_CLAMP_TO_EDGE)
, glassTexture("../models/glass.png", GL_CLAMP_TO_EDGE)
, groundTexture("../textures/groundSeamless.jpg", GL_REPEAT)
, riverTexture("../textures/waterSeamless.jpg", GL_REPEAT)
, grassCluterTexture("../textures/grassAtlas.png", GL_CLAMP_TO_EDGE)
, billboardTexture("../textures/treeBillboard.png", GL_CLAMP_TO_EDGE)
, heartTexture("../textures/heart.png", GL_CLAMP_TO_EDGE)
, whiteTexture("../textures/white.png", GL_CLAMP_TO_EDGE)
, diffuseMapTexture("../textures/metal_0029_color_1k.jpg", GL_CLAMP_TO_EDGE)
, specularMapTexture("../textures/metal_0029_metallic_1k.jpg", GL_CLAMP_TO_EDGE)
, skyboxTexture(pathes)
{
    // Simple meshes
    ground.enableAttribute(0, 3, 5, 0);
    ground.enableAttribute(1, 2, 5, 3);

    river.enableAttribute(0, 3, 5, 0);
    river.enableAttribute(1, 2, 5, 3);

    quad.enableAttribute(0, 3, 5, 0);
    quad.enableAttribute(1, 2, 5, 3);

    skybox.enableAttribute(0, 3, 0, 0);

    const int N_GRASS = 500;
    std::vector<GLfloat> grassData;
    grassData.reserve(N_GRASS * 5);

    for (int i = 0; i < N_GRASS; ++i)
    {
        float randomX = -25.0f + rand01() * 50.0f;
        float randomZ = -25.0f + rand01() * 50.0f;
        double rval = rand01();
        int randomTexture = rval > 0.05f ? rval > 0.10f ? 0 : 1 : 2;
        for (int nVert = 0; nVert < 18; ++nVert)
        {
            float x = grassPosition[nVert * 3 + 0];
            float y = grassPosition[nVert * 3 + 1];
            float z = grassPosition[nVert * 3 + 2];
            float u = grassTexCoords[nVert * 2 + 0];
            float v = grassTexCoords[nVert * 2 + 1];

            grassData.push_back(x + randomX);
            grassData.push_back(y - 1.0f);
            grassData.push_back(z + randomZ);
            grassData.push_back(u / 3.0f + randomTexture / 3.0f);
            grassData.push_back(v);
        }
    }
    grassCount = grassData.size() / 5;
    grass.setData(&grassData[0], grassData.size() * sizeof(float));
    grass.enableAttribute(0, 3, 5, 0);
    grass.enableAttribute(1, 2, 5, 3);
    grassData.clear(); grassData.resize(0); // desalloc array

    // Textures
    groundTexture.enableMipmap();
    riverTexture.enableMipmap();

    // Model shader
    {
        std::string vertexCode = readFile("shaders/model.vs.glsl");
        std::string fragmentCode = readFile("shaders/model.fs.glsl");

        Shader vertex(GL_VERTEX_SHADER, vertexCode.c_str());
        Shader fragment(GL_FRAGMENT_SHADER, fragmentCode.c_str());
        model.attachShader(vertex);
        model.attachShader(fragment);
        model.link();
        mvpLocationModel = model.getUniformLoc("mvp");
        
        std::string vertexGrassCode = readFile("shaders/grass.vs.glsl");
        Shader vertexGrass(GL_VERTEX_SHADER, vertexGrassCode.c_str());
        grassShader.attachShader(vertexGrass);
        grassShader.attachShader(fragment);
        grassShader.link();
        mvpLocationGrass = grassShader.getUniformLoc("mvp");
        timeLocationGrass = grassShader.getUniformLoc("time");
    }

    // Water shader
    {
        std::string vertexCode = readFile("shaders/water.vs.glsl");
        std::string fragmentCode = readFile("shaders/water.fs.glsl");
        Shader vertex(GL_VERTEX_SHADER, vertexCode.c_str());
        Shader fragment(GL_FRAGMENT_SHADER, fragmentCode.c_str());
        water.attachShader(vertex);
        water.attachShader(fragment);
        water.link();

        mvpLocationWater = water.getUniformLoc("mvp");
        timeLocationWater = water.getUniformLoc("time");
    }

    // Phong shader
    {
        std::string vertexCode = readFile("shaders/phong.vs.glsl");
        std::string fragmentCode = readFile("shaders/phong.fs.glsl");

        Shader vertex(GL_VERTEX_SHADER, vertexCode.c_str());
        Shader fragment(GL_FRAGMENT_SHADER, fragmentCode.c_str());
        phong.attachShader(vertex);
        phong.attachShader(fragment);
        phong.link();
        mvpLocationPhong = phong.getUniformLoc("mvp");
        modelViewLocationPhong = phong.getUniformLoc("modelView");
        viewLocationPhong = phong.getUniformLoc("view");
        normalLocationPhong = phong.getUniformLoc("normalMatrix");
        phong.use();
        glUniform1i(phong.getUniformLoc("diffuseSampler"), 0);
        glUniform1i(phong.getUniformLoc("specularSampler"), 1);
    }

    // Gouraud shader
    {
        std::string vertexCode = readFile("shaders/gouraud.vs.glsl");
        std::string fragmentCode = readFile("shaders/gouraud.fs.glsl");

        Shader vertex(GL_VERTEX_SHADER, vertexCode.c_str());
        Shader fragment(GL_FRAGMENT_SHADER, fragmentCode.c_str());
        gouraud.attachShader(vertex);
        gouraud.attachShader(fragment);
        gouraud.link();
        mvpLocationGouraud = gouraud.getUniformLoc("mvp");
        modelViewLocationGouraud = gouraud.getUniformLoc("modelView");
        viewLocationGouraud = gouraud.getUniformLoc("view");
        normalLocationGouraud = gouraud.getUniformLoc("normalMatrix");
        gouraud.use();
        glUniform1i(gouraud.getUniformLoc("diffuseSampler"), 0);
        glUniform1i(gouraud.getUniformLoc("specularSampler"), 1);
    }

    // Flat shader
    {
        std::string vertexCode = readFile("shaders/flat.vs.glsl");
        std::string geometryCode = readFile("shaders/flat.gs.glsl");
        std::string fragmentCode = readFile("shaders/gouraud.fs.glsl");

        Shader vertex(GL_VERTEX_SHADER, vertexCode.c_str());
        Shader geometry(GL_GEOMETRY_SHADER, geometryCode.c_str());
        Shader fragment(GL_FRAGMENT_SHADER, fragmentCode.c_str());
        flat.attachShader(vertex);
        flat.attachShader(geometry);
        flat.attachShader(fragment);
        flat.link();
        mvpLocationFlat = flat.getUniformLoc("mvp");
        modelViewLocationFlat = flat.getUniformLoc("modelView");
        viewLocationFlat = flat.getUniformLoc("view");
        normalLocationFlat = flat.getUniformLoc("normalMatrix");
        flat.use();
        glUniform1i(flat.getUniformLoc("diffuseSampler"), 0);
        glUniform1i(flat.getUniformLoc("specularSampler"), 1);
    }

    // Simple shader
    {
        std::string vertexCode = readFile("shaders/simple.vs.glsl");
        std::string fragmentCode = readFile("shaders/simple.fs.glsl");
        Shader vertex(GL_VERTEX_SHADER, vertexCode.c_str());
        Shader fragment(GL_FRAGMENT_SHADER, fragmentCode.c_str());
        simple.attachShader(vertex);
        simple.attachShader(fragment);
        simple.link();

        mvpLocationSimple = simple.getUniformLoc("mvp");
        colorLocationSimple = simple.getUniformLoc("color");
    }

    // Skybox shader
    {
        std::string vertexCode = readFile("shaders/skybox.vs.glsl");
        std::string fragmentCode = readFile("shaders/skybox.fs.glsl");
        Shader vertex(GL_VERTEX_SHADER, vertexCode.c_str());
        Shader fragment(GL_FRAGMENT_SHADER, fragmentCode.c_str());
        skyboxShader.attachShader(vertex);
        skyboxShader.attachShader(fragment);
        skyboxShader.link();

        mvpLocationSky = skyboxShader.getUniformLoc("mvp");
    }
}
