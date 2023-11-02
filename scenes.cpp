#include "scenes.h"

#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui/imgui.h"

#include "utils.h"

Scene::Scene(Resources& resources)
: m_res(resources)
{

}

Scene::~Scene()
{

}

void Scene::drawSky(glm::mat4& mvp)
{
    // DRAW SKYBOX
    glDepthFunc(GL_LEQUAL);
    m_res.skyboxShader.use();
    m_res.skyboxTexture.use();
    glUniformMatrix4fv(m_res.mvpLocationSky, 1, GL_FALSE, &mvp[0][0]);
    m_res.skybox.draw(GL_TRIANGLES, 6 * 6);
    glDepthFunc(GL_LESS);
}


WorldScene::WorldScene(Resources& resources, Window& w, bool& isFirstPersonCam, glm::vec3& position, glm::vec2& orientation)
: Scene(resources)
, m_isFirstPersonCam(isFirstPersonCam)
, m_position(position)
, m_orientation(orientation)
, m_w(w)
{
    for (int i = 0; i < N_GROUPS; ++i)
    {
        float x = 0, y = 0;
        getGroupRandomPos(i, N_ROWS, x, y);

        m_groupsTransform[i] = glm::mat4(1);
        m_groupsTransform[i] = glm::translate(m_groupsTransform[i], glm::vec3(x, -1, y));
        m_groupsTransform[i] = glm::rotate(m_groupsTransform[i], float(M_PI * 2 * rand01()), glm::vec3(0, 1, 0));
        const float GROUP_SCALE = 0.7f + rand01() * 0.6f;
        m_groupsTransform[i] = glm::scale(m_groupsTransform[i], glm::vec3(GROUP_SCALE));

        m_treeTransform[i] = glm::mat4(1);
        const float TREE_SCALE = 0.7f + rand01() * 0.6f;
        m_treeTransform[i] = glm::scale(m_treeTransform[i], glm::vec3(TREE_SCALE));

        m_rockTransform[i] = glm::mat4(1);
        m_rockTransform[i] = glm::rotate(m_rockTransform[i], float(M_PI * 2 * rand01()), glm::vec3(0, 1, 0));
        m_rockTransform[i] = glm::translate(m_rockTransform[i], glm::vec3(1 + rand01(), 0.2, 0));
        m_rockTransform[i] = glm::scale(m_rockTransform[i], glm::vec3(0.3f));

        m_shroomTransform[i] = glm::mat4(1);
        //shroomTransform[i] = glm::scale(shroomTransform[i], glm::vec3(TREE_SCALE));
        m_shroomTransform[i] = glm::translate(m_shroomTransform[i], glm::vec3(0.3 * TREE_SCALE, 0, 0.3 * TREE_SCALE));
        m_shroomTransform[i] = glm::scale(m_shroomTransform[i], glm::vec3(0.05f)); // /TREE_SCALE
    }
}

WorldScene::~WorldScene()
{

}

void WorldScene::render(glm::mat4& view, glm::mat4& projPersp)
{
    glm::mat4 mvp;
    glm::mat4 projView = projPersp * view;

    float time = m_w.getTick() / 1000.0;

    // DRAW RIVER
    m_res.water.use();
    glUniformMatrix4fv(m_res.mvpLocationWater, 1, GL_FALSE, &projView[0][0]);
    glUniform1f(m_res.timeLocationWater, time);
    m_res.riverTexture.use();
    m_res.river.draw(GL_TRIANGLES, 6);

    // DRAW GRASS
    m_res.grassShader.use();
    glUniformMatrix4fv(m_res.mvpLocationGrass, 1, GL_FALSE, &projView[0][0]);
    glUniform1f(m_res.timeLocationGrass, time);
    m_res.grassCluterTexture.use();
    glDisable(GL_CULL_FACE);
    m_res.grass.draw(GL_TRIANGLES, m_res.grassCount);
    glEnable(GL_CULL_FACE);

    m_res.model.use();

    // DRAW CHARACTER
    if (!m_isFirstPersonCam)
    {
        glm::mat4 playerMat = glm::mat4(1);
        playerMat = glm::translate(playerMat, glm::vec3(0, -1.0f, 0));
        playerMat = glm::translate(playerMat, m_position);
        playerMat = glm::rotate(playerMat, m_orientation.y + float(M_PI), glm::vec3(0, 1, 0));
        playerMat = glm::scale(playerMat, glm::vec3(0.5));
        mvp = projView * playerMat;
        glUniformMatrix4fv(m_res.mvpLocationModel, 1, GL_FALSE, &mvp[0][0]);
        m_res.suzanneTexture.use();
        m_res.suzanne.draw();
    }

    // DRAW GROUND

    mvp = projView * glm::mat4(1.0);
    glUniformMatrix4fv(m_res.mvpLocationModel, 1, GL_FALSE, &mvp[0][0]);
    m_res.groundTexture.use();
    m_res.ground.draw(GL_TRIANGLES, 6);

    // DRAW MODELS

    // Draw per texture state
    for (int i = 0; i < N_GROUPS; ++i)
    {
        glm::mat4 groupTransformation = m_groupsTransform[i];
        // If further than 25 units from player, draw as billboard
        if (abs(glm::length(glm::vec3(groupTransformation[3]) - m_position)) > 25)
        {
            m_res.billboardTexture.use();

            glm::mat4 billboardMat = glm::mat4(1);
            const float BILLBOARD_3D_SCALE = 3.47f;
            billboardMat = glm::scale(billboardMat, glm::vec3(BILLBOARD_3D_SCALE * 0.96, BILLBOARD_3D_SCALE, 1));
            billboardMat = glm::translate(billboardMat, glm::vec3(0, 0.5, 0));
            glm::mat4 modelView = view * groupTransformation * m_treeTransform[i] * billboardMat;

            float scaleX = glm::length(glm::vec3(modelView[0]));
            float scaleY = glm::length(glm::vec3(modelView[1]));
            float scaleZ = glm::length(glm::vec3(modelView[2]));

            modelView[0][0] = scaleX; /*modelView[1][0] = 0;*/      modelView[2][0] = 0;
            modelView[0][1] = 0;      /*modelView[1][1] = scaleY;*/ modelView[2][1] = 0;
            modelView[0][2] = 0;      /*modelView[1][2] = 0;*/      modelView[2][2] = scaleZ;

            mvp = projPersp * modelView;
            glUniformMatrix4fv(m_res.mvpLocationModel, 1, GL_FALSE, &mvp[0][0]);
            m_res.quad.draw(GL_TRIANGLES, 6);
        }
        else
        {
            m_res.treeTexture.use();
            glm::mat4 modelMat = m_groupsTransform[i] * m_treeTransform[i];
            mvp = projView * modelMat;
            glUniformMatrix4fv(m_res.mvpLocationModel, 1, GL_FALSE, &mvp[0][0]);
            m_res.tree.draw();
        }
    }

    m_res.rockTexture.use();
    for (int i = 0; i < N_GROUPS; ++i)
    {
        glm::mat4 modelMat = m_groupsTransform[i] * m_rockTransform[i];
        mvp = projView * modelMat;
        glUniformMatrix4fv(m_res.mvpLocationModel, 1, GL_FALSE, &mvp[0][0]);
        m_res.rock.draw();
    }

    m_res.shroomTexture.use();
    for (int i = 0; i < N_GROUPS; ++i)
    {
        glm::mat4 modelMat = m_groupsTransform[i] * m_shroomTransform[i];
        mvp = projView * modelMat;
        glUniformMatrix4fv(m_res.mvpLocationModel, 1, GL_FALSE, &mvp[0][0]);
        m_res.shroom.draw();
    }

    // DRAW HUD
    glDepthFunc(GL_ALWAYS);
    glm::mat4 projOrtho = glm::ortho(0.0f, (float)m_w.getWidth(), 0.0f, (float)m_w.getHeight(), 1.0f, -1.0f);
    glm::mat4 quadMat = glm::mat4(1);
    const float QUAD_SIZE = 100;
    quadMat = glm::translate(quadMat, glm::vec3(3 * QUAD_SIZE / 4, 3 * QUAD_SIZE / 4, 0.0f));
    quadMat = glm::scale(quadMat, glm::vec3(QUAD_SIZE, QUAD_SIZE, 0));
    mvp = projOrtho * quadMat;
    glUniformMatrix4fv(m_res.mvpLocationModel, 1, GL_FALSE, &mvp[0][0]);
    m_res.heartTexture.use();
    m_res.quad.draw(GL_TRIANGLES, 6);

    mvp = projPersp * glm::mat4(glm::mat3(view));
    drawSky(mvp);
}


StencilTestScene::StencilTestScene(Resources& resources, bool& isFirstPersonCam, glm::vec3& position, glm::vec2& orientation)
: Scene(resources)
, m_isFirstPersonCam(isFirstPersonCam)
, m_position(position)
, m_orientation(orientation)
{
    const GLfloat GROUND_SIZE = 30.0f/2.0f;
    // CALCULATE MATRIX TRANSFORMS FOR GROUPS AND OBJECTS
    for (int i = 0; i < N_ALLY_MONKEE; ++i)
    {
        float x = -GROUND_SIZE + 2 * GROUND_SIZE * rand01();
        float y = -GROUND_SIZE + 2 * GROUND_SIZE * rand01();
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(x, -1, y));
        model = glm::rotate(model, float(M_PI * 2 * rand01()), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.5));
        allyTransform[i] = model;
    }

    for (int i = 0; i < N_ENEMY_MONKEE; ++i)
    {
        float x = -GROUND_SIZE + 2 * GROUND_SIZE * rand01();
        float y = -GROUND_SIZE + 2 * GROUND_SIZE * rand01();
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(x, -1, y));
        model = glm::rotate(model, float(M_PI * 2 * rand01()), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.5));
        enemyTransform[i] = model;
    }
}

StencilTestScene::~StencilTestScene()
{

}

void StencilTestScene::render(glm::mat4& view, glm::mat4& projPersp)
{
    glm::mat4 mvp;
    glm::mat4 projView = projPersp * view;

    // DRAW CHARACTER

    m_res.model.use();
    m_res.suzanneTexture.use();

    if (!m_isFirstPersonCam)
    {
        glm::mat4 playerMat = glm::mat4(1);
        playerMat = glm::translate(playerMat, glm::vec3(0, -1.0f, 0));
        playerMat = glm::translate(playerMat, m_position);
        playerMat = glm::rotate(playerMat, m_orientation.y + float(M_PI), glm::vec3(0, 1, 0));
        playerMat = glm::scale(playerMat, glm::vec3(0.5));
        mvp = projView * playerMat;
        glUniformMatrix4fv(m_res.mvpLocationModel, 1, GL_FALSE, &mvp[0][0]);
        m_res.suzanne.draw();
    }

    m_res.rockTexture.use();

    glm::mat4 rockMat = glm::mat4(1);
    rockMat = glm::translate(rockMat, glm::vec3(-10, -0.2f, 0));
    mvp = projView * rockMat;
    glUniformMatrix4fv(m_res.mvpLocationModel, 1, GL_FALSE, &mvp[0][0]);
    m_res.rock.draw();


    // DRAW GROUND
    mvp = projView * glm::mat4(1.0);
    glUniformMatrix4fv(m_res.mvpLocationModel, 1, GL_FALSE, &mvp[0][0]);
    m_res.groundTexture.use();
    m_res.ground.draw(GL_TRIANGLES, 6);


    // TODO: Précalcul des matrices mvp des singes,
	// utilisable pour les modèles et halos.
    glm::mat4 mvpAlly[N_ALLY_MONKEE];
    glm::mat4 mvpEnemy[N_ENEMY_MONKEE];

    for (int i = 0; i < N_ALLY_MONKEE; i++) {
        mvp = projView * allyTransform[i];
        mvpAlly[i] = mvp;
    }

    for (int i = 0; i < N_ENEMY_MONKEE; i++) {
        mvp = projView * enemyTransform[i];
        mvpEnemy[i] = mvp;
    }

	m_res.suzanneTexture.use();
    // TODO: Remplir le stencil en dessinant les singes
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0xff, 0xff); 
    glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
    //Ally
    int mask = 1;
    
    
    for (int i = 0; i < N_ALLY_MONKEE; i++) {
        glStencilMask(mask);
        glUniformMatrix4fv(m_res.mvpLocationModel, 1, GL_FALSE, &mvpAlly[i][0][0]);
        m_res.suzanne.draw();
        mask = mask * 2;
    }
    //Enemy
    
    for (int i = 0; i < N_ENEMY_MONKEE; i++) {
        glStencilMask(mask);
        glUniformMatrix4fv(m_res.mvpLocationModel, 1, GL_FALSE, &mvpEnemy[i][0][0]);
        m_res.suzanne.draw();
        mask = mask * 2;
    }

    glDisable(GL_STENCIL_TEST);


	// On dessine le ciel un peu plus tôt
    mvp = projPersp * glm::mat4(glm::mat3(view));
    drawSky(mvp);

    m_res.model.use();
    m_res.glassTexture.use();
    
	// TODO: Dessin du mur vitrée
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glm::mat4 wallmat = glm::mat4(1.0f);
    wallmat = glm::translate(wallmat, glm::vec3(0.0f, -1.0f, 0.0f));
    mvp = projView * wallmat;
    glUniformMatrix4fv(m_res.mvpLocationModel, 1, GL_FALSE, &mvp[0][0]);
    m_res.glass.draw();
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
 

    // TODO: Dessiner les halos    
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, 0, 0xff);
    m_res.simple.use();
    glm::vec3 allyColor = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 enemycolor = glm::vec3(1.0f, 0.0f, 0.0f);

    glDepthFunc(GL_ALWAYS);
    for (int i = 0; i < N_ALLY_MONKEE; i++) {
        glUniformMatrix4fv(m_res.mvpLocationSimple, 1, GL_FALSE, &mvpAlly[i][0][0]);
        glUniform3fv(m_res.colorLocationSimple, 1, &allyColor[0]);
        m_res.suzanne.draw();
    }

    glDepthFunc(GL_LESS);
    for (int i = 0; i < N_ENEMY_MONKEE; i++) {
        glUniformMatrix4fv(m_res.mvpLocationSimple, 1, GL_FALSE, &mvpEnemy[i][0][0]);
        glUniform3fv(m_res.colorLocationSimple, 1, &enemycolor[0]);
        m_res.suzanne.draw();
    }
    
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glDisable(GL_STENCIL_TEST);
}


LightingTestScene::LightingTestScene(Resources& resources)
: Scene(resources)
, m_lightingData(nullptr, sizeof(m_lightModel) + sizeof(m_material) + sizeof(m_lights))
, m_currentModel(0)
, m_currentShading(2)
{
    m_lightModel =
    {
        glm::vec3(0.2f),
        false,
        false,
        false,
        1.0f,
        20.0f
    };

    m_material =
    {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.8f, 0.8f, 0.8f, 0.0f},
        {0.8f, 0.8f, 0.8f, 0.0f},
        {1.0f, 1.0f, 1.0f},
        100.0f
    };

    m_lights[0] =
    {
        {0.1f, 0.0f, 0.0f, 0.0f},
        {1.0f, 0.2f, 0.2f, 0.0f},
        {0.5f, 0.1f, 0.1f, 0.0f},
        {1.5f, 2.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f}
    };    
    m_lights[1] =
    {
        {0.0f, 0.1f, 0.0f, 0.0f},
        {0.2f, 1.0f, 0.2f, 0.0f},
        {0.1f, 0.5f, 0.1f, 0.0f},
        {-1.5f, 1.0f, 1.5f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f}
    };
    m_lights[2] =
    {
        {0.0f, 0.0f, 0.1f, 0.0f},
        {0.2f, 0.2f, 1.0f, 0.0f},
        {0.1f, 0.1f, 0.5f, 0.0f},
        {0.0f, 1.0f, -2.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    orientation[0] = glm::vec2(45.0f, 55.0f);
    orientation[1] = glm::vec2(45.0f, -45.0f);
    orientation[2] = glm::vec2(45.0f, 180.0f);

    m_res.phong.setUniformBlockBinding("LightingBlock", 0);
    m_lightingData.setBindingIndex(0);
}

LightingTestScene::~LightingTestScene()
{

}

void LightingTestScene::render(glm::mat4& view, glm::mat4& projPersp)
{
    drawMenu();

    GLintptr offset = 0;
    m_lightingData.updateData(&m_material  , offset, sizeof(m_material));   offset += sizeof(m_material);
    m_lightingData.updateData(m_lights     , offset, sizeof(m_lights));     offset += sizeof(m_lights);
    m_lightingData.updateData(&m_lightModel, offset, sizeof(m_lightModel)); offset += sizeof(m_lightModel);

    glm::mat4 mvp;
    glm::mat4 projView = projPersp * view;
    glm::mat4 modelView;

    GLint mvpMatrixLocation = -1;
    GLint viewMatrixLocation = -1;
    GLint modelViewMatrixLocation = -1;
    GLint normalMatrixLocation = -1;

    switch (m_currentShading)
    {
    case 0: 
        m_res.flat.use();
        viewMatrixLocation = m_res.viewLocationFlat;
        mvpMatrixLocation = m_res.mvpLocationFlat;
        modelViewMatrixLocation = m_res.modelViewLocationFlat;
        normalMatrixLocation = m_res.normalLocationFlat;
        break;
    case 1:
        m_res.gouraud.use();
        viewMatrixLocation = m_res.viewLocationGouraud;
        mvpMatrixLocation = m_res.mvpLocationGouraud;
        modelViewMatrixLocation = m_res.modelViewLocationGouraud;
        normalMatrixLocation = m_res.normalLocationGouraud;
        break;
    case 2: 
        m_res.phong.use();
        viewMatrixLocation = m_res.viewLocationPhong;
        mvpMatrixLocation = m_res.mvpLocationPhong;
        modelViewMatrixLocation =  m_res.modelViewLocationPhong;
        normalMatrixLocation = m_res.normalLocationPhong;
        break;
    }
    m_res.diffuseMapTexture.use(0);
    m_res.specularMapTexture.use(1);

    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &view[0][0]);
    glm::mat4 sphereModel = glm::mat4(1.0f);
    mvp = projView * sphereModel;
    modelView = view * sphereModel;
    glUniformMatrix4fv(mvpMatrixLocation, 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(modelViewMatrixLocation, 1, GL_FALSE, &modelView[0][0]);
    glUniformMatrix3fv(normalMatrixLocation, 1, GL_TRUE, glm::value_ptr(glm::inverse(glm::mat3(modelView))));

    switch (m_currentModel)
    {
    case 0: m_res.sphere.draw(); break;
    case 1: m_res.cube.draw(); break;
    case 2: m_res.suzanne.draw(); break;
    }

    m_res.whiteTexture.use(0);
    m_res.whiteTexture.use(1);
    for (size_t i = 0; i < 3; ++i)
    {
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, glm::vec3(m_lights[i].position));
        lightModel = glm::rotate(lightModel, glm::radians(orientation[i].y), glm::vec3(0.0f, 1.0f, 0.0f));
        lightModel = glm::rotate(lightModel, glm::radians(orientation[i].x), glm::vec3(1.0f, 0.0f, 0.0f));
        m_lights[i].spotDirection = lightModel * glm::vec4(0, -1, 0, 0) - m_lights[i].position;

        mvp = projView * lightModel;
        modelView = view * lightModel;
        glUniformMatrix4fv(mvpMatrixLocation, 1, GL_FALSE, &mvp[0][0]);
        glUniformMatrix4fv(modelViewMatrixLocation, 1, GL_FALSE, &modelView[0][0]);
        glUniformMatrix3fv(normalMatrixLocation, 1, GL_TRUE, glm::value_ptr(glm::inverse(glm::mat3(modelView))));

        Material lightMaterial =
        {
            m_lights[i].diffuse,
            glm::vec4(0.0f),
            glm::vec4(0.0f),
            glm::vec3(0.0f),
            1.0f
        };
        m_lightingData.updateData(&lightMaterial, 0, sizeof(lightMaterial));
        m_res.spotlight.draw();
    }

    mvp = projPersp * glm::mat4(glm::mat3(view));
    drawSky(mvp);
}

void LightingTestScene::drawMenu()
{
    const char* modelList[] = { "Sphere", "Cube", "Monkey" };
    const char* shadingList[] = { "Flat", "Gouraud", "Phong" };

    ImGui::Begin("Scene Parameters");

    ImGui::Combo("Model", &m_currentModel, modelList, sizeof(modelList) / sizeof(modelList[0]));
    ImGui::SeparatorText("Material");
    ImGui::ColorEdit3("Emission##m", &m_material.emission[0]);
    ImGui::ColorEdit3("Ambient##m", &m_material.ambient[0]);
    ImGui::ColorEdit3("Diffuse##m", &m_material.diffuse[0]);
    ImGui::ColorEdit3("Specular##m", &m_material.specular[0]);
    ImGui::DragFloat("Shininess##m", &m_material.shininess, 1.0f, 0.0f, 1000.0f);

    ImGui::SeparatorText("Light Model");
    ImGui::Combo("Shading", &m_currentShading, shadingList, sizeof(shadingList) / sizeof(shadingList[0]));
    ImGui::ColorEdit3("Global ambient", &m_lightModel.lightModelAmbient[0]);
    ImGui::Checkbox("Use blinn?", (bool*)&m_lightModel.useBlinn);
    ImGui::Checkbox("Use spotlight?", (bool*)&m_lightModel.useSpotlight); ImGui::SameLine();
    ImGui::Checkbox("Use Direct3D?", (bool*)&m_lightModel.useDirect3D);
    ImGui::DragFloat("Spot Exponent", &m_lightModel.spotExponent, 0.5f, 0.0f, 500.0f);
    ImGui::DragFloat("Spot Opening", &m_lightModel.spotOpeningAngle, 0.5f, 0.0f, 360.0f);
    if (ImGui::TreeNode("Light 1"))
    {
        ImGui::ColorEdit3("Ambient##l1", &m_lights[0].ambient[0]);
        ImGui::ColorEdit3("Diffuse##l1", &m_lights[0].diffuse[0]);
        ImGui::ColorEdit3("Specular##l1", &m_lights[0].specular[0]);
        ImGui::DragFloat3("Position##l1", &m_lights[0].position[0], 0.1f, 0.0f, 0.0f);
        ImGui::DragFloat2("Orientation##l1", &orientation[0][0], 0.1f, -360.0f, 360.0f);
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Light 2"))
    {
        ImGui::ColorEdit3("Ambient##l2", &m_lights[1].ambient[0]);
        ImGui::ColorEdit3("Diffuse##l2", &m_lights[1].diffuse[0]);
        ImGui::ColorEdit3("Specular##l2", &m_lights[1].specular[0]);
        ImGui::DragFloat3("Position##l2", &m_lights[1].position[0], 0.1f, 0.0f, 0.0f);
        ImGui::DragFloat2("Orientation##l2", &orientation[1][0], 0.1f, -360.0f, 360.0f);
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Light 3"))
    {
        ImGui::ColorEdit3("Ambient##l3", &m_lights[2].ambient[0]);
        ImGui::ColorEdit3("Diffuse##l3", &m_lights[2].diffuse[0]);
        ImGui::ColorEdit3("Specular##l3", &m_lights[2].specular[0]);
        ImGui::DragFloat3("Position##l3", &m_lights[2].position[0], 0.1f, 0.0f, 0.0f);
        ImGui::DragFloat2("Orientation##l3", &orientation[2][0], 0.1f, -360.0f, 360.0f);
        ImGui::TreePop();
    }

    ImGui::End();
}
