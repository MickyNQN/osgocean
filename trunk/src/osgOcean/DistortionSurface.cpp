/*
* This source file is part of the osgOcean library
* 
* Copyright (C) 2009 Kim Bale
* Copyright (C) 2009 The University of Hull, UK
* 
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free Software
* Foundation; either version 3 of the License, or (at your option) any later
* version.

* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
* http://www.gnu.org/copyleft/lesser.txt.
*/


#include <osgOcean/DistortionSurface>
#include <osgDB/Registry>
#include <osgOcean/ShaderManager>

#include <osgOcean/shaders/osgOcean_water_distortion_vert.inl>
#include <osgOcean/shaders/osgOcean_water_distortion_frag.inl>

using namespace osgOcean;

DistortionSurface::DistortionSurface( void )
{
    addResourcePaths();
}

DistortionSurface::DistortionSurface( const osg::Vec3f& corner, const osg::Vec2f& dims, osg::TextureRectangle* texture )
{
    build(corner,dims,texture);
    addResourcePaths();
}

DistortionSurface::DistortionSurface( const DistortionSurface &copy, const osg::CopyOp &copyop ):
    osg::Geode(copy,copyop)
{
}

void DistortionSurface::build( const osg::Vec3f& corner, const osg::Vec2f& dims, osg::TextureRectangle* texture )
{
    osg::notify(osg::INFO) << "DistortionSurface::build()"<< std::endl;

    removeDrawables( 0, getNumDrawables() );

    osg::Geometry* geom = new ScreenAlignedQuad(corner,dims,texture);
    addDrawable(geom);

    osg::StateSet* ss = new osg::StateSet;

    osg::ref_ptr<osg::Program> program = createShader();

    if(program.valid())
        ss->setAttributeAndModes( program.get(), osg::StateAttribute::ON );
    else
        osg::notify(osg::WARN) << "DistortionSurface::build() Invalid Shader"<< std::endl;
    
    ss->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON);

    ss->addUniform( new osg::Uniform( "osgOcean_FrameBuffer",  0 ) );
    ss->addUniform( new osg::Uniform( "osgOcean_Frequency",    2.f ) );
    ss->addUniform( new osg::Uniform( "osgOcean_Offset",       0.f ) );
    ss->addUniform( new osg::Uniform( "osgOcean_Speed",        1.f ) );
    ss->addUniform( new osg::Uniform( "osgOcean_ScreenRes",    dims ) );

    setStateSet(ss);

    setUserData( new DistortionDataType(*this) );
    setUpdateCallback( new DistortionCallback );
}

osg::Program* DistortionSurface::createShader(void)
{
    static const char osgOcean_water_distortion_vert_file[] = "osgOcean_water_distortion.vert";
    static const char osgOcean_water_distortion_frag_file[] = "osgOcean_water_distortion.frag";

    return ShaderManager::instance().createProgram("distortion_surface", 
                                                   osgOcean_water_distortion_vert_file, osgOcean_water_distortion_frag_file, 
                                                   osgOcean_water_distortion_vert,      osgOcean_water_distortion_frag);
}

void DistortionSurface::update( const double& dt )
{
    static float val = 0.f;
    static float inc = 1.39624444f; //(2PI/4.5f;)

    val += inc * dt; 

    if(val >= 6.2831f) 
        val = 0.f;

    getOrCreateStateSet()->getOrCreateUniform("osgOcean_Offset", osg::Uniform::FLOAT)->set(val);
}

void DistortionSurface::addResourcePaths(void)
{
    const std::string shaderPath  = "resources/shaders/";
    const std::string texturePath = "resources/textures/";

    osgDB::FilePathList& pathList = osgDB::Registry::instance()->getDataFilePathList();

    bool shaderPathPresent = false;
    bool texturePathPresent = false;

    for(unsigned int i = 0; i < pathList.size(); ++i )
    {
        if( pathList.at(i).compare(shaderPath) == 0 )
            shaderPathPresent = true;

        if( pathList.at(i).compare(texturePath) == 0 )
            texturePathPresent = true;
    }

    if(!texturePathPresent)
        pathList.push_back(texturePath);

    if(!shaderPathPresent)
        pathList.push_back(shaderPath);
}

// --------------------------------------------
//          Callback implementations
// --------------------------------------------

DistortionSurface::DistortionDataType::DistortionDataType(DistortionSurface& surface):
_surface( surface ),
_oldTime(0.0),
_newTime(0.0)
{}

void DistortionSurface::DistortionDataType::update( const double& time )
{
    _oldTime = _newTime;
    _newTime = time;

    _surface.update(_newTime-_oldTime);
}

void DistortionSurface::DistortionCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    osg::ref_ptr<DistortionSurface::DistortionDataType> data 
        = dynamic_cast<DistortionSurface::DistortionDataType*> ( node->getUserData() );

    if(data.valid())
    {
        if(nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR )
        {
            data->update( nv->getFrameStamp()->getSimulationTime() );
        }
    }

    traverse(node, nv); 
}
