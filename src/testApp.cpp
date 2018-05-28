#include "testApp.h"
#include "ofxJSGlobalFunc.h"
#include "globals.h"
#include "data.h"
#include "js.h"
#include "ofAppLog.h"

#include "sceneVisualisation.h"
#include "deviceEchoSimulator.h"
#include "deviceNode.h"
#include "deviceInfo.h"
#include "soundInput.h"
#include "soundManager.h"
#include "animations.h"

#include "tools.h"
//#include "toolALB.h"
#include "toolTimeline.h"

#if MURMUR_MULTI_WINDOWS
#include "ofxMultiGLFWWindow.h"
#endif

//--------------------------------------------------------------
void testApp::setup()
{
	OFAPPLOG->begin("testApp::setup()");

	// Init
	m_isUpdateLayout = false;
	
    // Settings
    m_settings.loadFile("configuration.xml");
    int simulator = m_settings.getValue("murmur:simulator:enable",1);
 
    mp_deviceSimulator = 0;
    mp_deviceManager = new DeviceManager();
    isFullscreen = false;
    isShowDevicePointSurfaces = false;
    isAnimationSequence = false;
    isViewSimulation = true;
    isSimulation =  simulator == 1 ? true : false;
	m_bTurnoffDevices = false;
	m_bRebootDevices = false;
	
	// Turn off device
	m_bTurnoff = m_settings.getAttribute("murmur:turnoff", "enable", 0) > 0 ? true : false;
	
	
	string strTurnoff = m_settings.getValue("murmur:turnoff", "23:59:59");
	vector <string> strTurnoffParts = ofSplitString(strTurnoff, ":");
	if (strTurnoffParts.size() == 3)
	{
		m_hourTurnoff = ofToFloat( strTurnoffParts[0] );
		m_mnTurnoff = ofToFloat( strTurnoffParts[1] );
		m_secondTurnoff = ofToFloat( strTurnoffParts[2] );
	}
	else
	{
		m_bTurnoff = false;
	}
 
    // Globals
    Globals::instance()->mp_app = this;
    Globals::instance()->mp_deviceManager = mp_deviceManager;

	// Multi-windows
	#if MURMUR_MULTI_WINDOWS
    mp_glfw = (ofxMultiGLFWWindow*)ofGetWindowPtr();
    // vector of windows, count set in main
    mp_windows = &mp_glfw->windows;

    mp_glfw->setWindow(mp_windows->at(0));    // set window pointer
    mp_glfw->initializeWindow();       // initialize events (mouse, keyboard, etc) on window (optional)
 
	ofSetWindowPosition(0, 0);    // business as usual...
    ofSetWindowShape(1024, 900);
    ofSetWindowTitle("Murmur");

	int monitor =  m_settings.getValue("murmur:windows:surface:monitor", 0);
	int xSurface = m_settings.getValue("murmur:windows:surface:x", 0);
	int ySurface = m_settings.getValue("murmur:windows:surface:y", 0);
	int wSurface = m_settings.getValue("murmur:windows:surface:w", 800);
	int hSurface = m_settings.getValue("murmur:windows:surface:h", 600);
	bool bVsync = m_settings.getValue("murmur:windows:vsync", 0) > 0 ? true : false;
	bool bFramerate = m_settings.getValue("murmur:windows:framerate", 1) > 0 ? true : false;
	int valueFramerate = m_settings.getAttribute("murmur:windows:framerate", "value", 60);
	bool bHideToolWindow = m_settings.getValue("murmur:windows:tools:hide", 1) > 0 ? true : false;

	m_bTurnoffDevices = m_settings.getValue("murmur:turnoffDevices",0) > 0 ? true : false;
	m_bRebootDevices = ofToInt(m_settings.getAttribute("murmur:turnoffDevices", "reboot", "0")) > 0 ? true : false;

	OFAPPLOG->println(" - window surface ("+ofToString(xSurface)+","+ofToString(ySurface)+","+ofToString(wSurface)+","+ofToString(hSurface)+")");
	OFAPPLOG->println(" - turnoff devices =  "+ofToString(m_bTurnoffDevices) + " (reboot = "+ofToString(m_bRebootDevices)+")");
	OFAPPLOG->println(" - vsync = "+ofToString(bVsync));
	OFAPPLOG->println(" - framerate = "+ofToString(bFramerate));
	OFAPPLOG->println("    + value = "+ofToString(valueFramerate));
	OFAPPLOG->println(" - bHideToolWindow = "+ofToString(bHideToolWindow));

    mp_glfw->setWindow(mp_windows->at(1));    // set window pointer
    mp_glfw->initializeWindow();       // initialize events (mouse, keyboard, etc) on window (optional)

/*
	int nbMonitors = mp_glfw->getMonitorCount();
	OFAPPLOG->println(" - monitor = "+ofToString(monitor));
	OFAPPLOG->println(" - nbMonitors = "+ofToString(nbMonitors));

	ofRectangle monitorRect = mp_glfw->getMonitorRect(0);
	OFAPPLOG->println(" - monitor[0] = ("+ofToString(monitorRect.x)+","+ofToString(monitorRect.y)+","+ofToString(monitorRect.width)+","+ofToString(monitorRect.height)+")");
	if (monitor > 0 && monitor < nbMonitors)
	{
		monitorRect = mp_glfw->getMonitorRect(monitor);

		OFAPPLOG->println(" - monitor["+ofToString(monitor)+"] = ("+ofToString(monitorRect.x)+","+ofToString(monitorRect.y)+","+ofToString(monitorRect.width)+","+ofToString(monitorRect.height)+")");
	}

    ofSetWindowShape(wSurface, hSurface);
    ofSetWindowPosition(monitorRect.x+xSurface, monitorRect.y+ySurface);    // business as usual...
    ofSetWindowTitle("Surface");
*/

    mp_glfw->setWindow(mp_windows->at(0));
	#endif
	
    // Initialize JS
    initJS();
    
    // Load Data
    DATA->load();

    // Sounds
    SoundManager::instance()->setup(m_settings);
	
	// MIDI
	initMidi();

 
    // Devices + Surfaces
    initDevices();
    initSurfaces();

    // Devices <> Surfaces
    attachDevicesToSurfaces();
    
    // Scenes
    initScenes();
    
	// Controls
	toolConfiguration* 	pToolConfiguration 	= new toolConfiguration(&toolManager);
	toolAnimations* 	pToolAnimations 	= new toolAnimations(&toolManager);
	toolDevices*		pToolDevices		= new toolDevices(&toolManager, mp_deviceManager);
	toolNetwork* 		pToolNetwork 		= new toolNetwork(&toolManager);
	toolSurfaces*		pToolSurfaces		= new toolSurfaces(&toolManager, mp_surfaceMain);
	toolTimeline*		pToolTimeline		= new toolTimeline(&toolManager);
	toolMidi*			pToolMidi			= new toolMidi(&toolManager);

	toolManager.addTool( pToolConfiguration );
	toolManager.addTool( new toolNetwork(&toolManager) );
	toolManager.addTool( pToolDevices );
	toolManager.addTool( pToolSurfaces );
	toolManager.addTool( pToolAnimations );
	toolManager.addTool( new toolScene(&toolManager, mp_sceneVisualisation) );
	toolManager.addTool( new toolSound(&toolManager) );
	toolManager.addTool( pToolTimeline );
	toolManager.addTool( pToolMidi );

	toolManager.setLogo("murmur_logo.png");
	toolManager.setFontName("Fonts/NewMedia Fett.ttf");

	// Midi settings
	if (pToolAnimations)		pToolAnimations->initMidiAnimations(mp_surfaceMain);
	if (pToolTimeline)			pToolTimeline->loadMidiSettings();
	if (pToolDevices)			pToolDevices->loadMidiSettings();
	if (pToolSurfaces)			pToolSurfaces->loadMidiSettings();
	if (pToolMidi)
	{
		pToolMidi->registerMidiInterface( pToolTimeline );
		pToolMidi->registerMidiInterface( pToolDevices );
		pToolMidi->registerMidiInterface( pToolSurfaces );
		if (mp_surfaceMain)
		{
			for (int i=0; i< mp_surfaceMain->getAnimationManager().m_listAnimations.size(); i++)
			{
				pToolMidi->registerMidiInterface( mp_surfaceMain->getAnimationManager().m_listAnimations[i] );
			}
		}
		pToolMidi->setMidiIns(&m_midiIns);
	}

	toolManager.createControls(ofVec2f(100,100),ofVec2f(200,200));
	toolManager.loadData();
 
    // Run network
	// if (pToolNetwork) 			pToolNetwork->setup();
	m_oscReceiver.setup(1234);
	
	// Simulators
	initSimulators();

	// setup
	if (pToolConfiguration)		pToolConfiguration->setup();
	if (pToolAnimations)		pToolAnimations->createControlsAnimations(mp_surfaceMain);
	if (pToolDevices)			pToolDevices->setup();
	if (pToolSurfaces)			pToolSurfaces->setup();
	if (pToolAnimations)		pToolAnimations->setup();
	if (pToolTimeline)			pToolTimeline->setup();
	if (pToolMidi)				pToolMidi->setup();
	
	
	
	// GO
	ofSetVerticalSync(bVsync);
	if (bFramerate)
	{
		ofSetFrameRate(valueFramerate);
	}
	
	if (pToolConfiguration)
		pToolConfiguration->showToolWindow(!bHideToolWindow);

	m_windowSize.set(ofGetWidth(),ofGetHeight());

	OFAPPLOG->end();

}


//--------------------------------------------------------------
void testApp::sM_timelineSimpleEvent(testApp* pThis, timelineSimpleEvent* pEvent)
{
	int delta = pEvent->m_timeTrigger-pEvent->m_time;
	ofLog() << "TIME EVENT '"+pEvent->m_id+"' time="+ofToString(pEvent->m_time)+" millis, time trigger="+ofToString(pEvent->m_timeTrigger)+", delta="+ofToString(delta);
}

//--------------------------------------------------------------
void testApp::exit()
{
	// printf("[testApp::exit()]\n");
	OFAPPLOG->begin("testApp::exit()");
	
	if (isSimulation)
	{
		OFAPPLOG->println(" - stopping sound stream input");
		m_soundStreamInput.stop();
	}

    if (mp_surfaceMain)
	{
		OFAPPLOG->println(" - surface main : saving animations properties");
        mp_surfaceMain->saveAnimationsProperties();
	}

	OFAPPLOG->println(" - tool manager : saving data");
	toolManager.saveData();

    if (mp_deviceManager)
	{
		OFAPPLOG->println(" - device manager : saving devices xml in Config/devices/");

        mp_deviceManager->saveDevicesXML("Config/devices/");
		if (m_bTurnoffDevices)
		{
			OFAPPLOG->println("- turning off devices with reboot = "+ofToString(m_bRebootDevices));
			mp_deviceManager->turnoffDevices( m_bRebootDevices );
		}
	}
	
	vector<ofxMidiIn*>::iterator itMidi = m_midiIns.begin();
	for ( ; itMidi != m_midiIns.end(); ++itMidi){
		delete *itMidi;
	}
	m_midiIns.clear();
	
    delete mp_sceneVisualisation;
    delete mp_deviceManager;
    delete Globals::instance();
    delete Data::instance();
	SoundManager::destroy();

	OFAPPLOG->end();
//	printf("[testApp::exit()] end\n");
}

//--------------------------------------------------------------
void testApp::initDevices()
{
	OFAPPLOG->begin("testApp::initDevices()");

    if (isSimulation)
    {
		m_settings.pushTag("simulator");
		m_settings.pushTag("devices");
		
		int nbDevicesSimulator = m_settings.getNumTags("device");
		OFAPPLOG->println("nbDevicesSimulator="+ofToString(nbDevicesSimulator));
		
		for (int i=0; i<nbDevicesSimulator ; i++)
		{
			m_settings.pushTag("device", i);
		
	        int nbLEDs = m_settings.getValue("nbLEDs", 129);
    	    float length = m_settings.getValue("length", 4.0f);
			string id = m_settings.getValue("id", "deviceEchoSimulator");

			DeviceEchoSimulator* pDeviceSimulator = new DeviceEchoSimulator(id, nbLEDs, length/float(nbLEDs-1));
			m_listDeviceSimulator.push_back(pDeviceSimulator);
	        mp_deviceManager->addDevice(pDeviceSimulator);

			m_settings.popTag();

		}
		m_settings.popTag();
		m_settings.popTag();
	
    }

	OFAPPLOG->end();
}


//--------------------------------------------------------------
void testApp::initSimulators()
{
	OFAPPLOG->begin("testApp::initSimulators");

    if (isSimulation && m_listDeviceSimulator.size()>0 /* && mp_deviceSimulator*/)
    {
        int deviceSoundInputId = m_settings.getValue("simulator:soundInput:device", -1);
        int nbChannels = m_settings.getValue("simulator:soundInput:nbChannels", 2);

        OFAPPLOG->println("deviceSoundInputId="+ofToString(deviceSoundInputId));
        OFAPPLOG->println("nbChannels="+ofToString(nbChannels));

		m_soundStreamInput.listDevices();
	    if (deviceSoundInputId>=0)
		{
    	    m_soundStreamInput.setDeviceID(deviceSoundInputId);
			m_soundStreamInput.setup(ofGetAppPtr(), 0, nbChannels, 44100, 256, 4); // ofBaseApp * app, int outChannels, int inChannels, int sampleRate, int bufferSize, int nBuffers
			m_soundStreamInput.start();
		}

		vector<DeviceEchoSimulator*>::iterator it = m_listDeviceSimulator.begin();
		for ( ; it != m_listDeviceSimulator.end() ; ++it)
		{
			DeviceEchoSimulator* pDeviceSim = *it;
			if (pDeviceSim)
			{
		        pDeviceSim->startSoundInput(nbChannels); // will not create an input stream
        		pDeviceSim->loadXML("Config/devices/");
				
				DeviceNode* pDeviceSimNode = mp_sceneVisualisation->getDeviceNode(pDeviceSim);
				if (pDeviceSimNode)
            		pDeviceSimNode->setPositionNodeSurface( mp_surfaceNodeMain->getGlobalPositionDevicePointSurface( pDeviceSim ) );
			}
		}
    }
	
	OFAPPLOG->end();
}

//--------------------------------------------------------------
void testApp::initSurfaces()
{
	OFAPPLOG->begin("testApp::initSurfaces");

    // TODO : should have a list here
    mp_surfaceMain = new Surface("main", ofGetWidth(),ofGetHeight());
    mp_surfaceMain->setup();

	OFAPPLOG->end();

}

//--------------------------------------------------------------
void testApp::attachDevicesToSurfaces()
{
    // TEMP, should be somewhere in a config file
    if (mp_surfaceMain)
	{
        if (isSimulation)
		{
            //mp_surfaceMain->addDevice(mp_deviceSimulator);
			
        	vector<DeviceEchoSimulator*>::iterator it = m_listDeviceSimulator.begin();
	        for ( ; it != m_listDeviceSimulator.end() ; ++it)
        	{
            	DeviceEchoSimulator* pDeviceSim = *it;
				if (pDeviceSim)
				{
					mp_surfaceMain->addDevice(pDeviceSim);
				}
			}
		}
    }
}


//--------------------------------------------------------------
SurfaceNode* testApp::getSurfaceNode(Surface* pSurface)
{
	if (mp_sceneVisualisation)
		return mp_sceneVisualisation->getSurfaceNode(pSurface);
	return 0;
}

//--------------------------------------------------------------
AnimationManager* testApp::getAnimationManagerForDevice(string deviceId)
{
	if (GLOBALS->mp_deviceManager)
	{
		return getAnimationManagerForDevice( GLOBALS->mp_deviceManager->getDeviceById(deviceId) );
	}
	return 0;
}


//--------------------------------------------------------------
AnimationManager* testApp::getAnimationManagerForDevice(Device* pDevice)
{
	Surface* pSurface = getSurfaceForDevice(pDevice);
	if (pSurface)
		return &pSurface->getAnimationManager();
	return 0;
}

//--------------------------------------------------------------
Surface* testApp::getSurfaceForDevice(Device* pDevice)
{
    // TODO : iterate through a liste of surfaces to find the surface that has the device attached to it
    return mp_surfaceMain;
}

//--------------------------------------------------------------
Surface* testApp::getSurfaceForDeviceCurrent()
{
    return getSurfaceForDevice( mp_deviceManager->getDeviceCurrent() );
}

//--------------------------------------------------------------
void testApp::initScenes()
{
    // TEMP
    // Surface
    mp_surfaceNodeMain = new SurfaceNode( mp_surfaceMain,4.0,3.0 );
    mp_surfaceNodeMain->setPosition(-0.5f*mp_surfaceNodeMain->getWidth(),0,0);
 
    // Construct the scene
    mp_sceneVisualisation = new SceneVisualisation();
	mp_sceneVisualisation->loadSettings();
    mp_sceneVisualisation->setup();
    mp_sceneVisualisation->addSurfaceNode(mp_surfaceNodeMain);

    if (isSimulation)
	{
        // mp_sceneVisualisation->addDeviceNode(mp_deviceNodeSim, mp_surfaceNodeMain);
		vector<DeviceEchoSimulator*>::iterator it = m_listDeviceSimulator.begin();
		for ( ; it != m_listDeviceSimulator.end() ; ++it)
		{
			DeviceEchoSimulator* pDeviceSim = *it;
			if (pDeviceSim)
			{
				DeviceNode* pDeviceSimNode = new DeviceNode(pDeviceSim);
				//m_listDeviceSimNode.push_back( pDeviceSimNode );
	        	mp_sceneVisualisation->addDeviceNode(pDeviceSimNode, mp_surfaceNodeMain);
			}
		}
	
	}
	
    // Add to a list
    m_listScenes.push_back(mp_sceneVisualisation);
}

//--------------------------------------------------------------
void testApp::createDeviceNodeInScene(Device* pDevice)
{
    if (mp_sceneVisualisation)
    {
        mp_sceneVisualisation->createDeviceNode(pDevice, mp_surfaceNodeMain); // TODO : select another surface ?
    }
}

//--------------------------------------------------------------
/*void testApp::updateControls()
{
	if (mp_lblSurfaceActivity)
	{
	    Surface* pSurfaceCurrent = getSurfaceForDeviceCurrent();
		if (pSurfaceCurrent)
			mp_lblSurfaceActivity->setLabel(pSurfaceCurrent->getStateActivity() + " - " + ofToString( pSurfaceCurrent->getVolumePacketsHistoryMean() ));
	}

}
*/

//--------------------------------------------------------------
void testApp::initJS()
{
	int err = ofxJSInitialize();
    if (err == 0)
	{
		// Create some new functions
        setupJS();
	}
}

//--------------------------------------------------------------
void testApp::initMidi()
{
	OFAPPLOG->begin("testApp::initMidi()");
	//m_midiIn.listPorts();

	m_settings.pushTag("midi");
	
	int nbMidiPorts = m_settings.getNumTags("port");
	OFAPPLOG->println("- nb ports defined =  "+ofToString(nbMidiPorts));

	for (int i=0;i<nbMidiPorts;i++)
	{
		int midiInPort = m_settings.getValue("port", 0, i);
		
		string name = ofxMidiIn::getPortName(midiInPort);
		//if (name != "")
		{
			OFAPPLOG->println("- opening port "+ofToString(midiInPort)+" / "+(name=="" ? "INVALID" : name));

			ofxMidiIn* pMidiIn = new ofxMidiIn();
			if (pMidiIn->openPort( midiInPort ))
			{
				pMidiIn->addListener( this );
				pMidiIn->setVerbose( true );
			}
			
			// Push it anyway
			m_midiIns.push_back( pMidiIn );
		}
	}

	m_settings.popTag();
	
	OFAPPLOG->end();
}

//--------------------------------------------------------------
void testApp::newMidiMessage(ofxMidiMessage& midiMessage)
{
	if (mp_surfaceMain)
		mp_surfaceMain->getAnimationManager().newMidiMessage(midiMessage);

	toolTimeline* ptoolTimeline = (toolTimeline*) toolManager.getTool("Timeline");
	if (ptoolTimeline)
		ptoolTimeline->newMidiMessage(midiMessage);

	toolDevices* ptoolDevices = (toolDevices*) toolManager.getTool("Devices");
	if (ptoolDevices)
		ptoolDevices->newMidiMessage(midiMessage);
}

//--------------------------------------------------------------
void testApp::update()
{
	// exit time ?
	if ( m_bTurnoff && ofGetHours() >= m_hourTurnoff && ofGetMinutes() >= m_mnTurnoff && ofGetSeconds() >= m_secondTurnoff)
	{
		ofExit();
	}

	// dt
	float dt = (float) ofGetLastFrameTime();

	// Surface
	// TODO : put this in toolSurfaces
	if (mp_surfaceMain)
		mp_surfaceMain->update(dt);

	// Tools
 	toolManager.update();
 	m_oscReceiver.update();
 
    // Devices
	if (isSimulation)
	{
	   vector<DeviceEchoSimulator*>::iterator it = m_listDeviceSimulator.begin();
	   for ( ; it != m_listDeviceSimulator.end() ; ++it)
	   {
		   DeviceEchoSimulator* pDeviceSim = *it;
		   if (pDeviceSim)
			   pDeviceSim->update(dt);
	   }
	}
 
    // Scenes
    if (mp_sceneVisualisation)
        mp_sceneVisualisation->update(dt);
	
	// updateControls();
}

//--------------------------------------------------------------
void testApp::draw()
{
	#if MURMUR_MULTI_WINDOWS
    m_windowIndex = mp_glfw->getWindowIndex();
	if (m_windowIndex==0)
	{
	#endif
	
	 if (isViewSimulation)
	 {
		// Draw simulation ?
		 toolScene* pToolScene = (toolScene*) toolManager.getTool("Scene");

		 // Draw Scene
		 if (mp_sceneVisualisation && pToolScene && pToolScene->isDrawScene())
			 mp_sceneVisualisation->draw();

		toolManager.drawUI();

		 if (m_isUpdateLayout)
		 {
			 guiUpdateListDevices();
			 m_isUpdateLayout = false;
		 }

	 }
	 else
	 {
	 	ofPushStyle();
		ofSetColor(255);
		ofDrawBitmapString("Click on this window and press key 's' to show the interface again.", 5,20);
		ofPopStyle();
	 }

	#if MURMUR_MULTI_WINDOWS
	}
	else if (m_windowIndex == 1)
	{
		 toolSurfaces* pToolSurfaces = (toolSurfaces*) toolManager.getTool("Surfaces");
		 if (pToolSurfaces){
			 pToolSurfaces->draw();
		 }

	}
	#endif
}

//--------------------------------------------------------------
void testApp::selectDeviceWithIndex(int index)
{
	toolDevices* pToolDevices = (toolDevices*) toolManager.getTool("Devices");
	if (pToolDevices)
		pToolDevices->selectDeviceWithIndex(index);
}

//--------------------------------------------------------------
void testApp::selectDevice(string id)
{
	toolDevices* pToolDevices = (toolDevices*) toolManager.getTool("Devices");
	if (pToolDevices)
		pToolDevices->selectDevice(id);
}

//--------------------------------------------------------------
void testApp::guiUpdateListDevices()
{
	toolDevices* pToolDevices = (toolDevices*) toolManager.getTool("Devices");
	if (pToolDevices)
	{
		pToolDevices->updateListDevices();
	}
}

//--------------------------------------------------------------
/*void testApp::guiUpdateDeviceAnimationTitle()
{
   Surface* pSurfaceCurrent = this->getSurfaceForDeviceCurrent();
   if (pSurfaceCurrent)
   {
   	 if (mp_lblAnimDirJs)
	 {
			 mp_lblAnimDirJs->setLabel("> dir "+pSurfaceCurrent->m_strDirScripts);
	 }
   
	 if (mp_lblAnimTitle)
	 {
	 	if (pSurfaceCurrent->getAnimationManager().mp_animationCurrent)
	 		mp_lblAnimTitle->setLabel("> surface '"+pSurfaceCurrent->getId()+"' / playing '"+pSurfaceCurrent->getAnimationManager().mp_animationCurrent->m_name+"'");
	 }
	}
}
*/


//--------------------------------------------------------------
void testApp::guiUpdateViewSimulation()
{
/*    if (mp_tgViewSimu)
        mp_tgViewSimu->setValue( isViewSimulation );
*/
}



//--------------------------------------------------------------
void testApp::audioIn(float * input, int bufferSize, int nChannels)
{
   vector<DeviceEchoSimulator*>::iterator it = m_listDeviceSimulator.begin();
   for ( ; it != m_listDeviceSimulator.end() ; ++it)
   {
	   DeviceEchoSimulator* pDeviceSim = *it;
	   if (pDeviceSim)
		   pDeviceSim->audioIn(input, bufferSize, nChannels);
   }
}

//--------------------------------------------------------------
void testApp::setViewSimulation(bool is)
{
	OFAPPLOG->begin("testApp::setViewSimulation()");

	toolSurfaces* 		pToolSurfaces 		= (toolSurfaces*) 		toolManager.getTool("Surfaces");
	toolConfiguration* 	pToolConfiguration 	= (toolConfiguration*) 	toolManager.getTool("Configuration");

	OFAPPLOG->println("- isViewSimulation="+ofToString(is));
	OFAPPLOG->println("- isShowDevicePointSurfaces="+ofToString(isShowDevicePointSurfaces));
	OFAPPLOG->println("- pToolSurfaces="+ofToString(pToolSurfaces == 0 ? "NULL" : "valid pointer"));

	isViewSimulation = is;

	if (pToolSurfaces)
		OFAPPLOG->println("- pToolSurfaces->m_isDrawHandles="+ofToString(pToolSurfaces->m_isDrawHandles));

	if (pToolConfiguration){
 		pToolConfiguration->isViewSimulation = is;
		pToolConfiguration->updateUI();
	}
	
	if (isViewSimulation)
	{
		ofShowCursor();
		toolManager.showUI();
		OFAPPLOG->println("- showing cursor");
	}
	else
	{
		if (isShowDevicePointSurfaces || (pToolSurfaces && pToolSurfaces->m_isDrawHandles) ){
			OFAPPLOG->println("- showing cursor");
			ofShowCursor();
		}
		else{
			ofHideCursor();
			OFAPPLOG->println("- hiding cursor");
		}

		toolManager.hideUI();
	}
	
	OFAPPLOG->end();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key)
{
	#if MURMUR_MULTI_WINDOWS
    if (mp_glfw->getEventWindow() == mp_windows->at(0))
	{
	#endif
	
	toolConfiguration* 	pToolConfiguration 	= (toolConfiguration*) 		toolManager.getTool("Configuration");
	toolAnimations* 	pToolAnimations 	= (toolAnimations*) 		toolManager.getTool("Animations");
	
	 
	if (toolManager.keyPressed(key) == false && toolManager.hasKeyboardFocus() == false)
	{
	  if (key == 's')
	  {
		if (!isViewSimulation)
			setViewSimulation(true);
	  }
	  else
	  if (key == OF_KEY_LEFT)
	  {
		  if (pToolAnimations && !pToolAnimations->isSequenceActive()){
			  pToolAnimations->showPrevAnimation();
		  }
	  }
	  else if (key == OF_KEY_RIGHT)
	  {
		  if (pToolAnimations && !pToolAnimations->isSequenceActive()){
			  pToolAnimations->showNextAnimation();
		  }
	  }
	}

	#if MURMUR_MULTI_WINDOWS
	}
    else if (mp_glfw->getEventWindow() == mp_windows->at(1))
	{
/*		toolSurfaces* pToolSurfaces = (toolSurfaces*) toolManager.getTool("Surfaces");
		if (pToolSurfaces && !pToolSurfaces->keyPressed(key))
		{
			if (key == 'f' || key == 'F')
				ofToggleFullscreen();
		}
*/
	}
	#endif

}

//--------------------------------------------------------------
void testApp::keyReleased(int key)
{
	#if MURMUR_MULTI_WINDOWS
    if (mp_glfw->getEventWindow() != mp_windows->at(0)) return;
	#endif

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y)
{
	#if MURMUR_MULTI_WINDOWS
    if (mp_glfw->getEventWindow() == mp_windows->at(0))
	{
		toolManager.mouseMoved(x,y);
	}
	#endif

}

//--------------------------------------------------------------
static bool init_mouseDragged = false; // dirty hack otherwise mouse dragged at start
void testApp::mouseDragged(int x, int y, int button)
{
	#if MURMUR_MULTI_WINDOWS
    if (mp_glfw->getEventWindow() != mp_windows->at(0)) return;
	#endif


    if (isViewSimulation && init_mouseDragged)
    {
        if (m_isUserControls) return;
        if (mp_sceneVisualisation)
            mp_sceneVisualisation->mouseDragged(x, y);
    }
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{
	#if MURMUR_MULTI_WINDOWS
	
	// UI Window
    if (mp_glfw->getEventWindow() == mp_windows->at(0))
	{

	#endif

		if (isViewSimulation)
		{
			toolManager.mousePressed(x,y,button);

		
        	m_isUserControls = toolManager.isHit(x,y);
        	if (m_isUserControls == false)
			{
            	mp_sceneVisualisation->mousePressed(x, y);
			}
		}
		else
		{
/*			toolSurfaces* pToolSurfaces = (toolSurfaces*) toolManager.getTool("Surfaces");
			if (pToolSurfaces)
			{
			// BOF ...
				#if MURMUR_MULTI_WINDOWS
				pToolSurfaces->setWindowCurrent( mp_glfw->getEventWindow() );
				pToolSurfaces->mousePressed(x, y, button);
				pToolSurfaces->setWindowCurrent(0);
				#else
				pToolSurfaces->mousePressed(x, y, button);
				#endif
			}
*/
		}

	#if MURMUR_MULTI_WINDOWS
	}
    else if (mp_glfw->getEventWindow() == mp_windows->at(1))
	{
		if (!init_mouseDragged)
		{
			init_mouseDragged = true;
			return;
		}

		toolSurfaces* pToolSurfaces = (toolSurfaces*) toolManager.getTool("Surfaces");
		if (pToolSurfaces)
		{
			// BOF ...
			pToolSurfaces->setWindowCurrent( mp_glfw->getEventWindow() );
			pToolSurfaces->mousePressed(x, y, button);
			pToolSurfaces->setWindowCurrent(0);
		}
	}
	#endif
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{
	#if MURMUR_MULTI_WINDOWS
		toolManager.mouseReleased(x,y,button);
	#endif

	if (isViewSimulation)
    {
        if (!m_isUserControls)
        {
            if (mp_sceneVisualisation)
                mp_sceneVisualisation->mouseReleased(x, y);
        }
        m_isUserControls = false;
    }
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{
    if (mp_glfw->getEventWindow() == mp_windows->at(1))
	{
		float wnew = (float)w;
		float hnew = (float)h;
		if (wnew != m_windowSize.x && hnew != m_windowSize.y)
		{
			int wold = (int) m_windowSize.x;
			int hold = (int) m_windowSize.y;

			// toolManager.windowResized(w,h);
			toolSurfaces* pToolSurfaces = (toolSurfaces*) toolManager.getTool("Surfaces");
			if (pToolSurfaces)
				pToolSurfaces->windowResized(wold, hold, w,h);

			m_windowSize.set(w,h);
		}
	}
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo)
{
	toolManager.dragEvent(dragInfo);
}

