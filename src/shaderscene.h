#ifndef SHADERSCENE_H
#define SHADERSCENE_H

// The parent class for this scene
#include "scene.h"

#include <ngl/Obj.h>
#include <ngl/Transformation.h>

#include <ngl/Camera.h>
#include <ngl/Light.h>
class ShaderScene : public Scene
{
public:
    /// This enumerated type allows the user to flip between the shader methods
    typedef enum {
        SHADER_GOURAUD,
        SHADER_PHONG,
        SHADER_COOKTORRANCE,
        SHADER_TOON
    } ShaderMethod;

    ShaderScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

    /// Allow the user to set the currently active shader method
    void setShaderMethod(ShaderMethod method) {m_shaderMethod = method;}

    /// Load Matrices
    void loadMatricesToShader(glm::vec3 _move);

    void resizeGL(GLint /*width*/, GLint /*height*/) noexcept;
    void ConfigureShaderAndMatrices();


    inline void setCameraPos(glm::vec3 _setPos){ CameraPos = _setPos; }
    inline void setLightPos(glm::vec3 _setLightPos){ lightPos = _setLightPos; }

private:
    ///Initialise FBO
    void initFBO();


    ///whether FBO needs to be recreated
    bool m_FBO = true;
    /// Keep track of the currently active shader method

    ShaderMethod m_shaderMethod = SHADER_GOURAUD;
    std::unique_ptr<ngl::Obj> m_base, m_sides, m_lock, m_back, m_screws,m_ground;
    GLuint m_colourTex, m_normalTex, m_fboPositionId;
    GLuint m_fboShadowId, m_fboShadowDepthId,  m_fboId, m_fboDepthId,m_fboTextureId;
    ngl::Transformation m_transform;
    ngl::Camera m_lightCamera;

    glm::vec3 CameraPos;
    glm::vec3 lightPos;

    glm::mat4 _lightSpaceMatrix;

    void initTexture(const GLuint& /*texUnit*/, GLuint &/*texId*/, const char */*filename*/);


};

#endif // SHADERSCENE_H
