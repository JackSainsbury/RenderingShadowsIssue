#include "shaderscene.h"
#include <QMouseEvent>

#include <glm/gtc/type_ptr.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/Image.h>


ShaderScene::ShaderScene() : Scene() {}

/**
 * @brief ObjLoaderScene::initGL
 */


void ShaderScene::resizeGL(GLint width, GLint height) noexcept {
    Scene::resizeGL(width,height);
    m_FBO = true;
}


void ShaderScene::initGL() noexcept {
    // Fire up the NGL machinary (not doing this will make it crash)
    ngl::NGLInit::instance();

    // Set background colour
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    // Enable 2D texturing
    glEnable(GL_TEXTURE_2D);

    // enable depth testing for drawing
    glEnable(GL_DEPTH_TEST);

    // enable multisampling for smoother drawing
    glEnable(GL_MULTISAMPLE);



    // Create and compile all of our shaders
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    shader->loadShader("GouraudProgram","../common/shaders/gouraud_vert.glsl","../common/shaders/gouraud_frag.glsl");
    shader->loadShader("PhongProgram","shaders/phong_vert.glsl","shaders/phong_frag.glsl");
    shader->loadShader("CookTorranceProgram","shaders/phong_vert.glsl","shaders/cooktorrance_frag.glsl");
    shader->loadShader("BaseProgram",
                       "shaders/base_vert.glsl",
                       "shaders/base_frag.glsl");




    shader->loadShader("SidesProgram","shaders/sides_vert.glsl","shaders/sides_frag.glsl");
    shader->loadShader("LockProgram","shaders/lock_vert.glsl","shaders/lock_frag.glsl");
    shader->loadShader("BackProgram","shaders/back_vert.glsl","shaders/back_frag.glsl");
    shader->loadShader("gBufferProgram","shaders/quad_vert.glsl","shaders/quad_frag.glsl");
    shader->loadShader("TestProgram","shaders/test_vert.glsl","shaders/test_frag.glsl");
    shader->loadShader("ShadowProgram","shaders/shadow_vert.glsl","shaders/shadow_frag.glsl");


    shader->loadShader("ScrewsProgram",
                       "shaders/screws_vert.glsl",
                       "shaders/screws_frag.glsl");
    (*shader)["ScrewsProgram"]->use();

    initTexture(4, m_colourTex, "images/cross.jpg");
    initTexture(5, m_normalTex, "images/crossnormal.jpg");
    GLint pid1 = shader->getProgramID("ScrewsProgram");
    glUniform1i(glGetUniformLocation(pid1, "ColourTexture1"), //location of uniform
                4); // texture unit for colour
    glUniform1i(glGetUniformLocation(pid1, "NormalTexture1"), //location of uniform
                5); // texture unit for normas


    //load in obj
    m_base.reset(  new ngl::Obj("models/base.obj"));
    m_base->createVAO();
    m_sides.reset(  new ngl::Obj("models/sides.obj"));
    m_sides->createVAO();
    m_lock.reset(  new ngl::Obj("models/lock.obj"));
    m_lock->createVAO();
    m_back.reset(  new ngl::Obj("models/back.obj"));
    m_back->createVAO();
    // m_screws.reset(  new ngl::Obj("models/screws.obj"));
    // m_screws->createVAO();
    m_screws.reset(  new ngl::Obj("models/test.obj"));
    m_screws->createVAO();

    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->createTrianglePlane("groundplane",8,8,1,1,ngl::Vec3(0,1,0));
    prim->createTrianglePlane("plane",2,2,1,1,ngl::Vec3(0,0,0));

}

const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
void ShaderScene::initFBO() {

    // First delete the FBO if it has been created previously
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE) {
        glDeleteTextures(1, &m_fboTextureId);
        glDeleteTextures(1, &m_fboPositionId);
        glDeleteTextures(1, &m_fboDepthId);
        glDeleteFramebuffers(1, &m_fboId);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Generate a texture to write the FBO result to
    glGenTextures(1, &m_fboTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_fboTextureId);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 m_width,
                 m_height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Generate a texture to write the FBO result to
    glGenTextures(1, &m_fboPositionId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_fboPositionId);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 m_width,
                 m_height,
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // The depth buffer is rendered to a texture buffer too
    glGenTextures(1, &m_fboDepthId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_fboDepthId);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT,
                 m_width,
                 m_height,
                 0,
                 GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_BYTE,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create the frame buffer
    glGenFramebuffers(1, &m_fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           m_fboTextureId, 0);
    //position
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT1,
                           GL_TEXTURE_2D,
                           m_fboPositionId, 0);
    //if you want to output another colour, you copy that and change 'm_fboTextured'
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D,
                           m_fboDepthId, 0);

    // Set the fragment shader output targets (DEPTH_ATTACHMENT is done automatically)
    GLenum drawBufs[] = {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBufs);

    // Check it is ready to rock and roll
    CheckFrameBuffer();

    // Unbind the framebuffer to revert to default render pipeline
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // First delete the FBO if it has been created previously
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboShadowId);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE) {
        glDeleteTextures(1, &m_fboShadowDepthId);
        glDeleteFramebuffers(1, &m_fboShadowId);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    // The depth buffer is rendered to a texture buffer too
    glGenTextures(1, &m_fboShadowDepthId);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_fboShadowDepthId);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH,
                 SHADOW_HEIGHT,
                 0,
                 GL_DEPTH_COMPONENT,
                 GL_FLOAT,
                 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Create the frame buffer
    glGenFramebuffers(1, &m_fboShadowId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboShadowId);

    //if you want to output another colour, you copy that and change 'm_fboTextured'
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D,
                           m_fboShadowDepthId, 0);

    // Set the fragment shader output targets (DEPTH_ATTACHMENT is done automatically)
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);


    // Check it is ready to rock and roll
    CheckFrameBuffer();

    // Unbind the framebuffer to revert to default render pipeline
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void ShaderScene::loadMatricesToShader(glm::vec3 _move){
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    GLint pid;
    glGetIntegerv(GL_CURRENT_PROGRAM, &pid);

    // Our MVP matrices
    glm::mat4 M = glm::translate(glm::mat4(1.0f),_move);
    glm::mat4 MVP, MV;
    glm::mat3 N;

    // Note the matrix multiplication order as we are in COLUMN MAJOR storage
    MV = m_V * M;
    N = glm::inverse(glm::mat3(MV));
    MVP = m_P * MV;

    // Set this MVP on the GPU
    glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data
    glUniformMatrix4fv(glGetUniformLocation(pid, "MV"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MV)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data



}

void ShaderScene::initTexture(const GLuint& texUnit, GLuint &texId, const char *filename) {
    // Set our active texture unit
    glActiveTexture(GL_TEXTURE0 + texUnit);

    // Load up the image using NGL routine
    ngl::Image img(filename);

    // Create storage for our new texture
    glGenTextures(1, &texId);

    // Bind the current texture
    glBindTexture(GL_TEXTURE_2D, texId);

    // Transfer image data onto the GPU using the teximage2D call
    glTexImage2D (
                GL_TEXTURE_2D,    // The target (in this case, which side of the cube)
                0,                // Level of mipmap to load
                img.format(),     // Internal format (number of colour components)
                img.width(),      // Width in pixels
                img.height(),     // Height in pixels
                0,                // Border
                GL_RGB,          // Format of the pixel data
                GL_UNSIGNED_BYTE, // Data type of pixel data
                img.getPixels()); // Pointer to image data in memory

    // Set up parameters for our texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void ShaderScene::paintGL() noexcept {

    // Check if the FBO needs to be recreated. This occurs after a resize.
    if (m_FBO) {
        initFBO();
        m_FBO = false;
    }

    // Bind the FBO to specify an alternative render target
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboShadowId);


    glEnable(GL_CULL_FACE);
    // Clear the screen (fill with our glClearColor)
    glClear (GL_DEPTH_BUFFER_BIT);

    // Set up the viewport
    glViewport(0,0,SHADOW_WIDTH,SHADOW_HEIGHT);
    //glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glCullFace(GL_BACK);
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    glm::mat4 MVP, MV;
    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();

    (*shader)["TestProgram"]->use();
    GLint pid = shader->getProgramID("TestProgram");

    // Our MVP matrices
    glm::mat4 M = glm::mat4(1.0f);

    glGetIntegerv(GL_CURRENT_PROGRAM, &pid);

    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    GLfloat near_plane = -5.0f;
    GLfloat far_plane = 10.0f;
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

    lightSpaceMatrix = lightProjection * lightView; //light mvp

    glUniformMatrix4fv((glGetUniformLocation(pid, "lightSpaceMatrix") ), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    glUniformMatrix4fv(glGetUniformLocation(pid, "M"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(M)); // a raw pointer to the data

    m_base->draw();
    m_screws->draw();
    prim->draw("groundplane");


    glBindFramebuffer(GL_FRAMEBUFFER,m_fboId);

    // Find the depth of field shader
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,m_width,m_height);
    //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    (*shader)["PhongProgram"]->use();
    pid = shader->getProgramID("PhongProgram");
    loadMatricesToShader(glm::vec3(0));

    glUniformMatrix4fv((glGetUniformLocation(pid, "lightSpaceMatrix") ), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    m_base->draw();
    m_screws->draw();
    glCullFace(GL_FRONT);

    // loadMatricesToShader(glm::vec3(0,-1.875,0));
    prim->draw("groundplane");

    glBindFramebuffer(GL_FRAMEBUFFER,0);

    // Now bind our rendered image which should be in the frame buffer for the next render pass
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_fboTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_fboDepthId);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_fboShadowDepthId);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_fboPositionId);

    // Find the depth of field shader
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,m_width,m_height);

    (*shader)["ShadowProgram"]->use();
    pid = shader->getProgramID("ShadowProgram");

    // These set the uniforms on our shader
    glUniform1i(glGetUniformLocation(pid, "colourTex"), 1);
    glUniform1i(glGetUniformLocation(pid, "depthTex"), 2);
    glUniform1i(glGetUniformLocation(pid, "shadowTex"), 3);
    glUniform1i(glGetUniformLocation(pid, "positionTex"), 4);

    glUniformMatrix4fv((glGetUniformLocation(pid, "lightSpaceMatrix") ), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    loadMatricesToShader(glm::vec3(0));

//    // Our MVP matrices
//    glm::mat4 M = glm::mat4(1.0f);
//    glm::mat3 N;
//    N = glm::inverse(glm::mat3(MV));

//    // Note the matrix multiplication order as we are in COLUMN MAJOR storage
//    MV = m_V * M;
//    MVP = m_P * MV;

    auto SSMVP = glm::rotate(glm::mat4(1.0f), -glm::pi<float>() * 0.5f, glm::vec3(1.0f,0.0f,0.0f));
    glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), 1, false, glm::value_ptr(SSMVP));

//    // Set this MVP on the GPU
//    glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), //location of uniform
//                       1, // how many matrices to transfer
//                       false, // whether to transpose matrix
//                       glm::value_ptr(SSMVP)); // a raw pointer to the data
//    glUniformMatrix4fv(glGetUniformLocation(pid, "MV"), //location of uniform
//                       1, // how many matrices to transfer
//                       false, // whether to transpose matrix
//                       glm::value_ptr(MV)); // a raw pointer to the data
//    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
//                       1, // how many matrices to transfer
//                       true, // whether to transpose matrix
//                       glm::value_ptr(N)); // a raw pointer to the data
//    glUniformMatrix4fv(glGetUniformLocation(pid, "M"), //location of uniform
//                       1, // how many matrices to transfer
//                       false, // whether to transpose matrix
//                       glm::value_ptr(M)); // a raw pointer to the data
//    glUniformMatrix4fv(glGetUniformLocation(pid, "V"), //location of uniform
//                       1, // how many matrices to transfer
//                       false, // whether to transpose matrix
//                       glm::value_ptr(m_V)); // a raw pointer to the data
//    glUniformMatrix3fv(glGetUniformLocation(pid, "P"), //location of uniform
//                       1, // how many matrices to transfer
//                       true, // whether to transpose matrix
//                       glm::value_ptr(m_P)); // a raw pointer to the data



    prim->draw("plane");

    glBindTexture(GL_TEXTURE_2D, 0);

}

