/*
 *  surface.cpp
 *  murmur
 *
 *  Created by Julien on 10/04/13.
 *  Copyright 2013 2Roqs. All rights reserved.
 *
 */

#include "surface.h"
#include "globals.h"
#include "quadWarping.h"
#include "ofxXmlSettings.h"
#include "animationsFactory.h"


// TEMP
#include "animationComposition.h"

//--------------------------------------------------------------
Surface::Surface(string id, int wPixels, int hPixels)
{
	OFAPPLOG->begin("Surface::Surface("+id+","+ofToString(wPixels)+","+ofToString(hPixels)+")");
//    OFAPPLOG->println("[new Surface(%s,%d,%d)]\n", id.c_str(), wPixels, hPixels);

    m_id = id;
	zeroAll();

	setDimensions(wPixels,hPixels);
	#if MURMUR_DEFINE_SYPHON
		m_syphonServer.setName(m_id);
	#endif
	
	m_alphaLayer = 0.0f;

	OFAPPLOG->end();
}

//--------------------------------------------------------------
Surface::Surface(string id)
{
	OFAPPLOG->begin("Surface::Surface("+id+")");
//    printf("[new Surface(%s)]\n", id.c_str());

    m_id = id;
	zeroAll();

	#if MURMUR_DEFINE_SYPHON
		m_syphonServer.setName(m_id);
	#endif
	OFAPPLOG->end();
}

//--------------------------------------------------------------
void Surface::zeroAll()
{
    m_durationTransition = 1.0f;
    m_durationAnimation  = 5.0f;
    m_isTimelineActive   = false;
    m_isAnimationTransiting = false;
    m_timeAnimation = 0.0f;
	m_isPublishSyphon = false;
	m_isRenderTarget = false;
	m_renderTargetLineWidth = 2.0f;
	
	m_volumePacketsHistoryNb = 1024;
	m_volumePacketsHistory.assign(m_volumePacketsHistoryNb, 0.0f);
	
	m_stateActivity = EStandby_active;

	m_isEnableStandby = false;
	m_isLaunchPacket = false;
	m_timeLaunchPacket = 0.0f;
	m_durationLaunchPacket = 0.0f;
	m_durationNoLaunchPacket = 0.0f;
	m_durationPreStandby = 10.0f;
	
	m_fboNbSamples		= 0;
	
	setMask(0);
	m_isDrawMask = false;
	
	mpf_renderOffscreenCallback = 0;
	mp_renderOffscreenUserData = 0;
}

//--------------------------------------------------------------
string Surface::getStateActivity()
{
	if (m_stateActivity == EStandby_active) return "active";
	if (m_stateActivity == EStandby_pre_standby)
	{
		string s = "pre(";
		s+=ofToString((int)m_timeStandby);
		s+=")";
		return s;
	}
	if (m_stateActivity == EStandby_standby){
		if (m_isLaunchPacket)
			return "standby*";
		else
			return "standby";
	}
}

//--------------------------------------------------------------
void Surface::setTimelineActive(bool is)
{
	OFAPPLOG->begin("Surface::setTimelineActive("+ofToString(is)+")");
    m_isTimelineActive = is;
    m_timeAnimation = 0.0f;
    m_animationManager.m_animationInfoIndex = 0;
 	if (is)
		m_animationManager.setAnimationTimelineNext();
	OFAPPLOG->end();
}

//--------------------------------------------------------------
void Surface::setPublishSyphon(bool is)
{
	m_isPublishSyphon = is;
}

//--------------------------------------------------------------
void Surface::setRenderTarget(bool is)
{
	// printf(" - setRenderTarget %s\n", is ? "true" : "false");
	m_isRenderTarget = is;
}

//--------------------------------------------------------------
void Surface::setRenderTargetLineWidth(float w_)
{
	m_renderTargetLineWidth = w_;
}

//--------------------------------------------------------------
void Surface::setup()
{
	OFAPPLOG->begin("Surface::setup");

    // Cpp animations
    ofxXmlSettings surfaceSettings;
	string filename = "Config/surfaces/surface"+m_id+".xml";
	
    if ( surfaceSettings.loadFile(filename.c_str()) )
    {
        surfaceSettings.pushTag("surface");
		
		int nbAnimationsTag = surfaceSettings.getNumTags("animations");
		for (int i=0;i<nbAnimationsTag;i++)
		{
			string type = surfaceSettings.getAttribute("animations", "type", "", i);
			// printf(" - type=%s\n", type.c_str());
			OFAPPLOG->println("type="+type);

			// ----------------------------------------
			// CPP
			// ----------------------------------------
			if (type == "cpp")
			{

				surfaceSettings.pushTag("animations", i);

		        int nbAnimations = surfaceSettings.getNumTags("animation");
        		//printf(" - nbAnimations=%d\n", nbAnimations);
        		OFAPPLOG->println("nbAnimations="+ofToString(nbAnimations));
				for (int j=0;j<nbAnimations;j++)
				{
            		string animName = surfaceSettings.getAttribute("animation", "name", "", j);
		            OFAPPLOG->println("  - "+animName);
			
            		if (animName != "")
					{
                		Animation* pAnimation = AnimationsFactory::create( animName );
                		if (pAnimation)
						{
							surfaceSettings.pushTag("animation", j);
							pAnimation->readSurfaceSettings(surfaceSettings);
                    		m_animationManager.M_addAnimation(pAnimation);
							surfaceSettings.popTag();
						}
					}
        		}
				surfaceSettings.popTag();
			}
			// ----------------------------------------
			// JS
			// ----------------------------------------
			else if (type == "js")
			{
				string strDirNameScripts = surfaceSettings.getAttribute("animations", "folder", "",i);
				m_strDirScripts = "Scripts/"+strDirNameScripts;

				ofDirectory dirScripts(m_strDirScripts.c_str());
				if (dirScripts.exists())
				{
					dirScripts.listDir();
					// printf("    - DIR %s [%d file(s)]\n", dirScripts.path().c_str(),dirScripts.size());
					OFAPPLOG->println("    - DIR "+dirScripts.path()+" ["+ofToString( dirScripts.size() )+" file(s)]");
					
					vector<ofFile> files = dirScripts.getFiles();
					vector<ofFile>::iterator it;
					for (it = files.begin(); it != files.end(); ++it)
					{
						if ((*it).getExtension() == "js")
						{
							Animation* pAnimation = new Animation((*it).getFileName(), (*it).getAbsolutePath());
							
							// Eval script to extract informations from it
							if (pAnimation->M_loadScript( (*it).getAbsolutePath().c_str() ))
							{
								// Theme + autoclear
								ofxJSValue retValAutoClear;
								retValAutoClear = bool_TO_ofxJSValue(true);
								
								if (pAnimation->mp_obj)
								{
									ofxJSCallFunctionNameObject_NoArgs_IfExists(pAnimation->mp_obj,"getAutoClear",	retValAutoClear);
								}
								
								bool isAutoClear = ofxJSValue_TO_bool(retValAutoClear);
								
								// printf("    - js [%s], autoclear=%s\n", (*it).getFileName().c_str(), isAutoClear ? "true" : "false");
								OFAPPLOG->println("    - js ["+(*it).getFileName()+"], autoclear="+(isAutoClear ? "true" : "false"));
								
								pAnimation->m_isAutoClear = isAutoClear;

								m_animationManager.M_addAnimation(pAnimation);
							}
						}
					}
				
				}
				dirScripts.close();
			}
			
		}

        surfaceSettings.popTag();
    }
    else
	{
        // printf("   - cannot load %s\n", "surfaceMain.xml");
		OFAPPLOG->println("cannot load 'surfaceMain.xml'");
    }

	
	// Scripts
	ofDirectory dirScripts("Scripts");
   // m_animationManager.M_readSettings(surfaceSettings);

	
	OFAPPLOG->println("setting animation index 0");
	m_animationManager.M_setAnimationDirect(0);
	
	OFAPPLOG->end();
}

//--------------------------------------------------------------
void Surface::setDimensions(int w, int h, int nbSamples)
{
	if (m_fbo.getWidth() != w || m_fbo.getHeight() != h || m_fboNbSamples != nbSamples)
	{
		m_fboNbSamples = nbSamples;
		if (m_fboNbSamples > ofFbo::maxSamples())
			m_fboNbSamples = ofFbo::maxSamples();

		m_fbo.allocate(w,h,GL_RGBA, m_fboNbSamples); // TODO : GL_RGBA ???
		ofVec2f dims = ofVec2f(m_fbo.getWidth(),m_fbo.getHeight());
		ofNotifyEvent(m_eventOffscreenChanged, dims, this);
	}
}

//--------------------------------------------------------------
void Surface::addDevice(Device* pDevice)
{
    m_listDevices.push_back(pDevice);
}


//--------------------------------------------------------------
void Surface::checkActivity(float dt)
{
	float now = ofGetElapsedTimeMillis();
	if (!m_isEnableStandby) return;
	
	
	if (m_stateActivity == EStandby_active)
	{
		if (getVolumePacketsHistoryMean() <= m_volumePacketsHistoryMeanTh_goStandby)
		{
			m_stateActivity = EStandby_pre_standby;
			m_timeStandby = 0.0f;
		}
	}
	else
	if (m_stateActivity == EStandby_pre_standby)
	{

		m_timeStandby+=dt;
		
		if (m_timeStandby >= m_durationPreStandby)
		{
			if (getVolumePacketsHistoryMean() >= m_volumePacketsHistoryMeanTh_goActive)
			{
				m_stateActivity = EStandby_active;
			}
			else
			{
				m_stateActivity = EStandby_standby;

				m_isLaunchPacket = true;
				m_timeLaunchPacket = now;
				m_durationLaunchPacket = ofRandom(0.25f,0.75f);
			}
		}
	}
	else
	if (m_stateActivity == EStandby_standby)
	{

		if (getVolumePacketsHistoryMean() >= m_volumePacketsHistoryMeanTh_goActive)
		{
			m_stateActivity = EStandby_active;
		}
		else
		{
			//printf("m_isLaunchPacket=%s\n", m_isLaunchPacket ? "true":"false");
		
		
			Animation* pAnimCurrent = getAnimationManager().mp_animationCurrent;
			if (pAnimCurrent && m_listDevices.size()>0)
			{
				Device* pDevice = m_listDevices[0];
				
				if (m_isLaunchPacket)
				{

					// Generate pulse (call directly)
					DevicePacket packet;
					packet.m_volume = ofRandom(1.0f)/* * pDevice->getSoundInputVolumeMax()*/;
					if (pAnimCurrent->m_name == "particlesMega2")
					{
						packet.m_volume = 0.2f;
					}
										
										
					pAnimCurrent->onNewPacket(&packet, pDevice->m_id, pDevice->m_pointSurface.x*m_fbo.getWidth(), pDevice->m_pointSurface.y*m_fbo.getHeight());
				
					if (now-m_timeLaunchPacket>=m_durationLaunchPacket*1000.0f){
						m_isLaunchPacket = false;
						m_durationNoLaunchPacket = ofRandom(8.0f,12.0f);
						m_timeLaunchPacket = now;
					}
				}
				else
				{
					// Generate pulse (call directly)
					DevicePacket packet;
					packet.m_volume = 0.0f;
					
					pAnimCurrent->onNewPacket(&packet, pDevice->m_id, pDevice->m_pointSurface.x*m_fbo.getWidth(), pDevice->m_pointSurface.y*m_fbo.getHeight());

					if (now-m_timeLaunchPacket>=m_durationNoLaunchPacket*1000.0f)
					{
						m_isLaunchPacket = true;
						m_durationLaunchPacket = ofRandom(0.25f,0.75f);
						m_timeLaunchPacket = now;
					}
				}
												
				
			}
		}
	}
	
}

//--------------------------------------------------------------
void Surface::update(float dt)
{
    m_animationManager.M_update(dt);
	checkActivity(dt);
	

	//printf("%.3f - ", m_volumePacketsHistoryMean);
	
    if (m_isTimelineActive)
    {
        m_timeAnimation += dt;
        // printf("tAnim=%.2f / tAnimDuration=%.2f\n", m_timeAnimation, m_durationAnimation);
        
        if (m_timeAnimation>=m_durationAnimation && !m_animationManager.M_isTransiting())
        {
           m_animationManager.setAnimationTimelineNext();
            m_isAnimationTransiting = true;
            m_timeAnimation = 0.0f;
        }

        if (/*m_isAnimationTransiting && */m_animationManager.M_isTransiting() == false){
//            printf(">>> end of transition\n");
//            m_isAnimationTransiting = false;
        }
    
    }
}

//--------------------------------------------------------------
void Surface::drawRenderTarget()
{
	if (m_isRenderTarget)
	{
		ofPushStyle();
		
		 ofSetLineWidth(m_renderTargetLineWidth);
	     ofSetColor(255,255,255,255);
		 
		 int nbGrids = 30;
		 float stepx = m_fbo.getWidth() / (float)nbGrids;
		 for (float x=0; x<=m_fbo.getWidth(); x+=stepx)
		 {
			ofLine(x,0,x,m_fbo.getHeight());
		 }

		 float stepy = stepx; // m_fbo.getHeight() / nbGrids;
		 for (float y=0; y<=m_fbo.getHeight(); y+=stepy)
		 {
			ofLine(0,y,m_fbo.getWidth(),y);
		 }
		 
	     ofSetColor(0,255,0,255);
		 ofNoFill();
//		 ofSetLineWidth(10);
		 ofCircle(0.5*m_fbo.getWidth(),0.5*m_fbo.getHeight(), 0.5*min(m_fbo.getWidth(),m_fbo.getHeight()));

		ofPopStyle();

	}
}

//--------------------------------------------------------------
void Surface::drawMask()
{
	if (m_isDrawMask && mp_mask)
	{
		ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
		mp_mask->draw(0,0,m_fbo.getWidth(),m_fbo.getHeight());
		ofDisableBlendMode();
	}
}

//--------------------------------------------------------------
void Surface::drawLayerAlpha()
{
	ofPushStyle();
	if (m_alphaLayer>0.0f)
	{
		ofEnableAlphaBlending();
		ofSetColor(0.0f,255.0f*m_alphaLayer);
		ofRect(0,0,m_fbo.getWidth(),m_fbo.getHeight());
		ofDisableAlphaBlending();
	}
//		ofSetColor(0.0f,200.0f);
	ofPopStyle();
}


//--------------------------------------------------------------
void Surface::renderOffscreen(bool isRenderDevicePoints)
{
	m_animationManager.M_drawCanvasBefore(m_fbo.getWidth(),m_fbo.getHeight());
	
	if (mpf_renderOffscreenCallback == 0)
	{
	    m_fbo.begin();
		    m_animationManager.M_drawCanvas(m_fbo.getWidth(),m_fbo.getHeight());
    	m_fbo.end();
	}
	else
	{
		if (mpf_renderOffscreenCallback)
			(*mpf_renderOffscreenCallback)(this, mp_renderOffscreenUserData); // responsible for filling the fbo
	    m_fbo.begin();
		m_animationManager.M_drawTransition(m_fbo.getWidth(),m_fbo.getHeight());
    	m_fbo.end();
	}

	m_fbo.begin();
	drawRenderTarget();
	drawMask();
	drawLayerAlpha();
	m_fbo.end();

}

//--------------------------------------------------------------
void Surface::publishSyphon()
{
	#if MURMUR_DEFINE_SYPHON
		if (m_isPublishSyphon)
		{
			m_syphonServer.publishFBO(&m_fbo);
//			m_syphonServer.publishScreen();
//			m_syphonServer.publishTexture(&m_fbo.getTextureReference());
		}
	#endif
}

//--------------------------------------------------------------
/*
void Surface::drawDevicePointSurface(ofRectangle& rect, ofMatrix4x4* mWarping)
{
    ofPushStyle();
    ofSetColor(255,255,255,255);
	ofEnableAlphaBlending();
    
    vector<Device*>::iterator itDevices = m_listDevices.begin();
    ofVec2f posSurface;
    Device* pDevice=0;
	for (;itDevices!=m_listDevices.end();++itDevices)
    {
        pDevice = *itDevices;

		if (mWarping == 0)
		{
			posSurface.set(rect.getX()+pDevice->m_pointSurface.x*rect.getWidth(), rect.getY()+pDevice->m_pointSurface.y*rect.getHeight());
		}
		else
		{
			ofVec3f tmp = *mWarping * ofVec3f( pDevice->m_pointSurface.x*rect.getWidth(), pDevice->m_pointSurface.y*rect.getHeight(), 0.0f );
			posSurface.set(tmp.x,tmp.y);
		}

		if (GLOBALS->mp_deviceManager->getDeviceCurrent() == pDevice)
			ofSetColor(255,255,255,255);
		else
			ofSetColor(255,255,254,100);
	 
		ofLine(posSurface.x,posSurface.y-40, posSurface.x, posSurface.y+40);
		ofLine(posSurface.x-40,posSurface.y, posSurface.x+40,posSurface.y);
		ofDrawBitmapString(pDevice->m_id, posSurface.x+4, posSurface.y-4);
	}

    ofPopStyle();
}
*/

//--------------------------------------------------------------
void Surface::drawDevicePointSurface(ofRectangle& rect, quadWarping* pWarping)
{
    ofPushStyle();
    ofSetColor(255,255,255,255);
	ofEnableAlphaBlending();

    vector<Device*>::iterator itDevices = m_listDevices.begin();
    ofVec2f posSurface;
    Device* pDevice=0;
	for (;itDevices!=m_listDevices.end();++itDevices)
    {
        pDevice = *itDevices;

		if (pWarping == 0)
		{
			posSurface.set(rect.getX()+pDevice->m_pointSurface.x*rect.getWidth(), rect.getY()+pDevice->m_pointSurface.y*rect.getHeight());
		}
		else
		{
			posSurface = pWarping->getPointInQuad( pDevice->m_pointSurface );
		}

		if (GLOBALS->mp_deviceManager->getDeviceCurrent() == pDevice)
			ofSetColor(255,255,255,255);
		else
			ofSetColor(255,255,254,100);


		ofLine(posSurface.x,posSurface.y-40, posSurface.x, posSurface.y+40);
		ofLine(posSurface.x-40,posSurface.y, posSurface.x+40,posSurface.y);
		ofDrawBitmapString(pDevice->m_id, posSurface.x+4, posSurface.y-4);
	}




    ofPopStyle();
}


//--------------------------------------------------------------
void Surface::drawCacheLEDs(float d)
{
    // TEMP
    return;
    
    ofPushStyle();
    ofSetColor(255,255,255,255);

    vector<Device*>::iterator itDevices = m_listDevices.begin();
    ofVec2f posSurface;
    Device* pDevice=0;
    for (;itDevices!=m_listDevices.end();++itDevices)
    {
        pDevice = *itDevices;
        posSurface.set(pDevice->m_pointSurface.x*m_fbo.getWidth(), pDevice->m_pointSurface.y*m_fbo.getHeight());
        
        ofEllipse(posSurface.x,posSurface.y, d,d);
    }
    
    ofPopStyle();
}

//--------------------------------------------------------------
void Surface::onNewPacket(DevicePacket* pDevicePacket, string deviceId)
{
	computeVolumePacketsMean(pDevicePacket);


	if (m_stateActivity == EStandby_active || m_stateActivity == EStandby_pre_standby)
	{
    	if (m_animationManager.mp_animationCurrent)
    	{
			Device* pDevice = Globals::instance()->mp_deviceManager->getDeviceById(deviceId);
        	if (pDevice)
           	m_animationManager.mp_animationCurrent->onNewPacket(pDevicePacket, pDevice->m_id, pDevice->m_pointSurface.x*m_fbo.getWidth(), pDevice->m_pointSurface.y*m_fbo.getHeight());
		}
	}

}

//--------------------------------------------------------------
void Surface::computeVolumePacketsMean(DevicePacket* pDevicePacket)
{
	if (pDevicePacket == 0) return;
	
	   //if we are bigger the the size we want to record - lets drop the oldest value
	   if( m_volumePacketsHistory.size() >= m_volumePacketsHistoryNb ){
		   m_volumePacketsHistory.erase(m_volumePacketsHistory.end()-1, m_volumePacketsHistory.end());
	   }

	   //lets record the volume into an array
	   m_volumePacketsHistory.insert( m_volumePacketsHistory.begin(), pDevicePacket->m_volume );

	   // Compute mean
	   m_volumePacketsHistoryMean = 0.0f;
	   int nb=m_volumePacketsHistory.size();
	   for (int i=0; i<nb ; i++)
		   m_volumePacketsHistoryMean+= m_volumePacketsHistory[i];
	   m_volumePacketsHistoryMean /= (float)nb;

}

//--------------------------------------------------------------
void Surface::saveAnimationsProperties()
{
    vector<Animation*>::iterator it = m_animationManager.m_listAnimations.begin();
    Animation* pAnim;
    for ( ; it != m_animationManager.m_listAnimations.end(); ++it)
    {
        (*it)->saveProperties("surface"+this->m_id);
    }
}

//--------------------------------------------------------------
void Surface::loadAnimationsProperties()
{
    vector<Animation*>::iterator it = m_animationManager.m_listAnimations.begin();
    Animation* pAnim;
    for ( ; it != m_animationManager.m_listAnimations.end(); ++it)
    {
        (*it)->loadProperties("surface"+this->m_id);
    }
}





