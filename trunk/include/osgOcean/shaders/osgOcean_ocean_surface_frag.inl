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

// ------------------------------------------------------------------------------
// -- THIS FILE HAS BEEN CREATED AS PART OF THE BUILD PROCESS -- DO NOT MODIFY --
// ------------------------------------------------------------------------------

static const char osgOcean_ocean_surface_frag[] =
	"uniform bool osgOcean_EnableReflections;\n"
	"uniform bool osgOcean_EnableRefractions;\n"
	"uniform bool osgOcean_EnableCrestFoam;\n"
	"\n"
	"uniform bool osgOcean_EnableDOF;\n"
	"uniform bool osgOcean_EnableGlare;\n"
	"\n"
	"uniform float osgOcean_DOF_Near;\n"
	"uniform float osgOcean_DOF_Focus;\n"
	"uniform float osgOcean_DOF_Far;\n"
	"uniform float osgOcean_DOF_Clamp;\n"
	"uniform float osgOcean_FresnelMul;\n"
	"\n"
	"uniform samplerCube osgOcean_EnvironmentMap;\n"
	"uniform sampler2D   osgOcean_ReflectionMap;\n"
	"uniform sampler2D   osgOcean_RefractionMap;\n"
	"uniform sampler2D   osgOcean_RefractionDepthMap;\n"
	"uniform sampler2D   osgOcean_FoamMap;\n"
	"uniform sampler2D   osgOcean_NoiseMap;\n"
	"uniform sampler2D   osgOcean_Heightmap;\n"
	"\n"
	"uniform float osgOcean_UnderwaterFogDensity;\n"
	"uniform float osgOcean_AboveWaterFogDensity;\n"
	"uniform vec4  osgOcean_UnderwaterFogColor;\n"
	"uniform vec4  osgOcean_AboveWaterFogColor;\n"
	"\n"
	"uniform mat4 osg_ViewMatrixInverse;\n"
	"\n"
	"uniform mat4 osgOcean_RefractionInverseTransformation;\n"
	"\n"
	"uniform vec2 osgOcean_ViewportDimensions;\n"
	"\n"
	"uniform float osgOcean_WaterHeight;\n"
	"uniform float osgOcean_FoamCapBottom;\n"
	"uniform float osgOcean_FoamCapTop;\n"
	"\n"
	"varying vec3 vNormal;\n"
	"varying vec3 vViewerDir;\n"
	"varying vec3 vLightDir;\n"
	"varying vec4 vVertex;\n"
	"varying vec4 vWorldVertex;\n"
	"\n"
	"varying vec3 vWorldViewDir;\n"
	"varying vec3 vWorldNormal;\n"
	"\n"
	"varying float height;\n"
	"\n"
	"mat4 worldObjectMatrix;\n"
	"\n"
	"const float shininess = 2000.0;\n"
	"\n"
	"// Extinction level for red, green and blue light in ocean water\n"
	"// (maybe this should be changed into a user configurable shader uniform?)\n"
	"// Values are taken from 'Rendering Water as Post-process Effect', Wojciech Toman\n"
	"// http://www.gamedev.net/reference/programming/features/ppWaterRender/\n"
	"// vec4 colorExtinction = vec4(4.5, 75.0, 300.0, 1.0) * 5.0;\n"
	"const vec4 oneOverColorExtinction = vec4(1.0/22.5, 1.0/375.0, 1.0/1500, 1.0/5.0);\n"
	"\n"
	"// The amount of light extinction,\n"
	"// higher values means that less light is transmitted through the water\n"
	"const float oneOverLightExtinction = 1.0/60.0;\n"
	"\n"
	"vec4 distortGen( vec4 v, vec3 N )\n"
	"{\n"
	"    // transposed\n"
	"    const mat4 mr =\n"
	"        mat4( 0.5, 0.0, 0.0, 0.0,\n"
	"              0.0, 0.5, 0.0, 0.0,\n"
	"              0.0, 0.0, 0.5, 0.0,\n"
	"              0.5, 0.5, 0.5, 1.0 );\n"
	"\n"
	"    mat4 texgen_matrix = mr * gl_ProjectionMatrix * gl_ModelViewMatrix;\n"
	"\n"
	"    //float disp = 8.0;\n"
	"    float disp = 4.0;\n"
	"\n"
	"    vec4 tempPos;\n"
	"\n"
	"    tempPos.xy = v.xy + disp * N.xy;\n"
	"    tempPos.z  = v.z;\n"
	"    tempPos.w  = 1.0;\n"
	"\n"
	"    return texgen_matrix * tempPos;\n"
	"}\n"
	"\n"
	"vec3 reorientate( vec3 v )\n"
	"{\n"
	"    float y = v.y;\n"
	"\n"
	"    v.y = -v.z;\n"
	"    v.z = y;\n"
	"\n"
	"    return v;\n"
	"}\n"
	"\n"
	"mat3 getLinearPart( mat4 m )\n"
	"{\n"
	"    mat3 result;\n"
	"\n"
	"    result[0][0] = m[0][0];\n"
	"    result[0][1] = m[0][1];\n"
	"    result[0][2] = m[0][2];\n"
	"\n"
	"    result[1][0] = m[1][0];\n"
	"    result[1][1] = m[1][1];\n"
	"    result[1][2] = m[1][2];\n"
	"\n"
	"    result[2][0] = m[2][0];\n"
	"    result[2][1] = m[2][1];\n"
	"    result[2][2] = m[2][2];\n"
	"\n"
	"    return result;\n"
	"}\n"
	"\n"
	"vec4 computeCubeMapColor( vec3 N, vec4 V, vec3 E )\n"
	"{\n"
	"    mat3 worldObjectMat3x3 = getLinearPart( worldObjectMatrix );\n"
	"    vec4 world_pos = worldObjectMatrix *  V;\n"
	"\n"
	"    vec3 normal = normalize( worldObjectMat3x3 * N );\n"
	"    vec3 eye = normalize( world_pos.xyz - E );\n"
	"\n"
	"    vec3 coord = reflect( eye, normal );\n"
	"\n"
	"    vec3 reflection_vector = vec3( coord.x, coord.y, -coord.z );\n"
	"\n"
	"    return textureCube(osgOcean_EnvironmentMap, reflection_vector.xzy);\n"
	"}\n"
	"\n"
	"float calcFresnel( float dotEN, float mul )\n"
	"{\n"
	"    float fresnel = clamp( dotEN, 0.0, 1.0 ) + 1.0;\n"
	"    return pow(fresnel, -8.0) * mul;\n"
	"}\n"
	"\n"
	"float alphaHeight( float min, float max, float val)\n"
	"{\n"
	"    if(max-min == 0.0)\n"
	"        return 1.0;\n"
	"\n"
	"    return (val - min) / (max - min);\n"
	"}\n"
	"\n"
	"float computeDepthBlur(float depth, float focus, float near, float far, float clampval )\n"
	"{\n"
	"   float f;\n"
	"\n"
	"   if (depth < focus){\n"
	"      // scale depth value between near blur distance and focal distance to [-1, 0] range\n"
	"      f = (depth - focus)/(focus - near);\n"
	"   }\n"
	"   else{\n"
	"      // scale depth value between focal distance and far blur\n"
	"      // distance to [0, 1] range\n"
	"      f = (depth - focus)/(far - focus);\n"
	"\n"
	"      // clamp the far blur to a maximum blurriness\n"
	"      f = clamp(f, 0.0, clampval);\n"
	"   }\n"
	"\n"
	"   // scale and bias into [0, 1] range\n"
	"   return f * 0.5 + 0.5;\n"
	"}\n"
	"\n"
	"float luminance( vec4 color )\n"
	"{\n"
	"    return (0.3*color.r) + (0.59*color.g) + (0.11*color.b);\n"
	"}\n"
	"\n"
	"float computeFogFactor( float density, float fogCoord )\n"
	"{\n"
	"    return exp2(density * fogCoord * fogCoord );\n"
	"}\n"
	"\n"
	"// -------------------------------\n"
	"//          Main Program\n"
	"// -------------------------------\n"
	"\n"
	"void main( void )\n"
	"{\n"
	"    vec4 final_color;\n"
	"\n"
	"    vec3 noiseNormal = vec3( texture2D( osgOcean_NoiseMap, gl_TexCoord[0].xy ) * 2.0 - 1.0 );\n"
	"    noiseNormal += vec3( texture2D( osgOcean_NoiseMap, gl_TexCoord[0].zw ) * 2.0 - 1.0 );\n"
	"\n"
	"    worldObjectMatrix = osg_ViewMatrixInverse * gl_ModelViewMatrix;\n"
	"\n"
	"    if(gl_FrontFacing)\n"
	"    {\n"
	"        vec3 N = normalize( vNormal + noiseNormal );\n"
	"        vec3 L = normalize( vLightDir );\n"
	"        vec3 E = normalize( vViewerDir );\n"
	"        vec3 R = reflect( -L, N );\n"
	"\n"
	"        vec4 specular_color;\n"
	"\n"
	"        float lambertTerm = dot(N,L);\n"
	"\n"
	"        if( lambertTerm > 0.0 )\n"
	"        {\n"
	"            float specCoeff = pow( max( dot(R, E), 0.0 ), shininess );\n"
	"            specular_color = gl_LightSource[osgOcean_LightID].diffuse * specCoeff * 6.0;\n"
	"        }\n"
	"\n"
	"        float dotEN = dot(E, N);\n"
	"        float dotLN = dot(L, N);\n"
	"\n"
	"        // Fade out the distortion along the screen edges this reduces artifacts\n"
	"        // caused by texture coordinates that are distorted out of the [0, 1] range.\n"
	"        // At very close distance to the surface the distortion artifacts still appear.\n"
	"        vec2 fade_xy = pow(abs(gl_FragCoord.xy / (osgOcean_ViewportDimensions.xy * 0.5) - 1.0), 10.0);\n"
	"        float fade = 1.0 - max(fade_xy.x , fade_xy.y);\n"
	"\n"
	"        vec4 distortedVertex = distortGen(vVertex, N*fade);\n"
	"\n"
	"        // Calculate the position in world space of the pixel on the ocean floor\n"
	"        vec4 refraction_ndc = vec4(gl_FragCoord.xy / osgOcean_ViewportDimensions, texture2DProj(osgOcean_RefractionDepthMap, distortGen(vVertex, 0.0 * N)).x, 1.0);\n"
	"        vec4 refraction_screen = refraction_ndc * 2.0 - 1.0;\n"
	"        vec4 refraction_world = osgOcean_RefractionInverseTransformation * refraction_screen;\n"
	"        refraction_world = refraction_world / refraction_world.w;\n"
	"\n"
	"        // The amount of water behind the pixel\n"
	"        // (water depth as seen from the camera position)\n"
	"        float waterDepth = distance(vWorldVertex, refraction_world);\n"
	"\n"
	"#if SHORETOSINUS\n"
	"        // The vertical distance between the ocean surface and ocean floor, this uses the projected heightmap\n"
	"        float waterHeight = (texture2DProj(osgOcean_Heightmap, distortGen(vVertex, 0.0 * N)).x) * 500.0;\n"
	"#endif\n"
	"\n"
	"        // Determine refraction color\n"
	"        vec4 refraction_color = vec4( gl_Color.rgb, 1.0 );\n"
	"\n"
	"        if(osgOcean_EnableRefractions)\n"
	"        {\n"
	"            vec4 refractionmap_color = texture2DProj(osgOcean_RefractionMap, distortedVertex );\n"
	"			\n"
	"            vec4 waterColor = mix(refractionmap_color, refraction_color, clamp(pow(waterDepth * oneOverLightExtinction, 0.3), 0.0, 1.0));\n"
	"\n"
	"#if SHORETOSINUS\n"
	"            refraction_color = mix(waterColor, refraction_color, clamp(waterHeight * oneOverColorExtinction, 0.0, 1.0));\n"
	"#else\n"
	"            refraction_color = waterColor;\n"
	"#endif\n"
	"        }\n"
	"\n"
	"        // To cubemap or not to cubemap that is the question\n"
	"        // projected reflection looks pretty nice anyway\n"
	"        // cubemap looks wrong with fixed skydome\n"
	"        //vec4 env_color = computeCubeMapColor(N, vWorldVertex, osgOcean_EyePosition);\n"
	"\n"
	"        float fresnel = calcFresnel(dotEN, osgOcean_FresnelMul );\n"
	"\n"
	"        vec4 env_color;\n"
	"\n"
	"        if(osgOcean_EnableReflections)\n"
	"        {\n"
	"            env_color = texture2DProj( osgOcean_ReflectionMap, distortedVertex );\n"
	"        }\n"
	"        else\n"
	"        {\n"
	"            env_color = gl_LightSource[osgOcean_LightID].diffuse;\n"
	"        }\n"
	"\n"
	"        final_color = mix(refraction_color, env_color, fresnel) + specular_color;\n"
	"\n"
	"        // Store the color here to compute luminance later, we don't want\n"
	"        // foam or fog to be taken into account for this calculation.\n"
	"        vec4 lumColor = final_color;\n"
	"\n"
	"        if(osgOcean_EnableCrestFoam)\n"
	"        {\n"
	"#if SHORETOSINUS\n"
	"            if( vVertex.z > osgOcean_FoamCapBottom || waterHeight < 10.0)\n"
	"            {\n"
	"                vec4 foam_color = texture2D( osgOcean_FoamMap, gl_TexCoord[1].st / 10.0);\n"
	"\n"
	"                float alpha = max(alphaHeight( osgOcean_FoamCapBottom, osgOcean_FoamCapTop, vVertex.z ) * (fresnel*2.0),\n"
	"                                  0.8 - clamp(waterHeight / 10.0, 0.0, 0.8));\n"
	"\n"
	"                final_color = final_color + (foam_color * alpha);\n"
	"            }\n"
	"#else\n"
	"            if( vVertex.z > osgOcean_FoamCapBottom )\n"
	"            {\n"
	"                vec4 foam_color = texture2D( osgOcean_FoamMap, gl_TexCoord[1].st / 10.0);\n"
	"                float alpha = alphaHeight( osgOcean_FoamCapBottom, osgOcean_FoamCapTop, vVertex.z ) * (fresnel*2.0);\n"
	"                final_color = final_color + (foam_color * alpha);\n"
	"            }\n"
	"#endif\n"
	"        }\n"
	"\n"
	"\n"
	"        // exp2 fog\n"
	"        float fogFactor = computeFogFactor( osgOcean_AboveWaterFogDensity, gl_FogFragCoord );\n"
	"\n"
	"        final_color = mix( osgOcean_AboveWaterFogColor, final_color, fogFactor );\n"
	"\n"
	"        if(osgOcean_EnableGlare)\n"
	"        {\n"
	"            float lum = luminance(lumColor);\n"
	"            gl_FragData[1] = vec4(lum);\n"
	"        }\n"
	"\n"
	"        gl_FragData[0] = final_color;\n"
	"    }\n"
	"    else\n"
	"    {\n"
	"        vec3 E = normalize( vViewerDir );\n"
	"        vec3 N = -normalize( (vWorldNormal + noiseNormal) );\n"
	"\n"
	"        vec3 incident = normalize( vWorldViewDir );\n"
	"\n"
	"        //------ Find the reflection\n"
	"        // not really usable as we would need to use cubemap again..\n"
	"        // the ocean is blue not much to reflect back\n"
	"        //vec3 reflected = reflect( incident, -N );\n"
	"        //reflected      = reorientate( reflected );\n"
	"        //vec3 reflVec   = normalize( reflected );\n"
	"\n"
	"        //------ Find the refraction from cubemap\n"
	"        vec3 refracted = refract( incident, N, 1.3333333333 );   // 1.1 looks better? - messes up position of godrays though\n"
	"        refracted.z = refracted.z - 0.015;                       // on the fringes push it down to show base texture color\n"
	"        refracted = reorientate( refracted );\n"
	"\n"
	"        vec4 refractColor = textureCube( osgOcean_EnvironmentMap, refracted );\n"
	"\n"
	"        //------ Project texture where the light isn't internally reflected\n"
	"        if(osgOcean_EnableRefractions)\n"
	"        {\n"
	"            // if alpha is 1.0 then it's a sky pixel\n"
	"            if(refractColor.a == 1.0 )\n"
	"            {\n"
	"                vec4 env_color = texture2DProj( osgOcean_RefractionMap, distortGen(vVertex, N) );\n"
	"                refractColor.rgb = mix( refractColor.rgb, env_color.rgb, env_color.a );\n"
	"            }\n"
	"        }\n"
	"\n"
	"        // if it's not refracting in, add a bit of highlighting with fresnel\n"
	"        if( refractColor.a == 0.0 )\n"
	"        {\n"
	"            float fresnel = calcFresnel( dot(E, N), 0.7 );\n"
	"            refractColor.rgb = osgOcean_UnderwaterFogColor.rgb*fresnel + (1.0-fresnel)* refractColor.rgb;\n"
	"        }\n"
	"\n"
	"        float fogFactor = computeFogFactor( osgOcean_UnderwaterFogDensity, gl_FogFragCoord );\n"
	"        final_color = mix( osgOcean_UnderwaterFogColor, refractColor, fogFactor );\n"
	"\n"
	"        if(osgOcean_EnableDOF)\n"
	"        {\n"
	"            float depthBlur = computeDepthBlur( gl_FogFragCoord, osgOcean_DOF_Focus, osgOcean_DOF_Near, osgOcean_DOF_Far, osgOcean_DOF_Clamp );\n"
	"            gl_FragData[1] = vec4(depthBlur);\n"
	"        }\n"
	"\n"
	"        gl_FragData[0] = final_color;\n"
	"    }\n"
	"}\n"
	"\n"
	"\n"
	"\n"
	"\n";
