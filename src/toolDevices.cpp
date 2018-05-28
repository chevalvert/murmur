//
//  toolDevices.cpp
//  murmur
//
//  Created by Julien on 05/06/2014.
//
//

#include "toolDevices.h"
#include "globals.h"
#include "testApp.h"
#include "toolSurfaces.h"
#include "toolAnimations.h"
#include "toolScene.h"
#include "ofxSoundPlayerMultiOutput.h"

//--------------------------------------------------------------
toolDevices::toolDevices(toolManager* parent, DeviceManager* manager) : tool("Devices", parent)
{
	mp_ddlDevices 	= 0;
	mp_deviceManager= manager;
	mp_canvasDevice = 0;
	mp_lblDeviceTitle = 0;

	mp_teDeviceLedsLit = 0;
	
	mp_toggleDeviceUseRawVol = 0;
	mp_toggleDeviceMute = 0;
	mp_sliderDeviceVolMax = 0;
	mp_teDeviceVolMaxMin = 0;
	mp_teDeviceVolMaxMax = 0;
	mp_sliderDeviceVolHistorySize = 0;
	mp_toggleDeviceEnableStandby = 0;
	mp_sliderDeviceVolHistoryTh = 0;
	
	mp_toggleDevicePitchEnable = 0;
	mp_sliderDevicePitchMin = 0;
	mp_sliderDevicePitchMax = 0;
	
	mp_toggleDeviceBPMEnable = 0;
	mp_sliderDeviceBPM = 0;
	
	mp_sliderDeviceTimeStandby = 0;
	mp_sliderDeviceSampleVolStandby = 0;


	mp_toggleColorEnable = 0;
	mp_sliderColorManualHue = 0;
	mp_sliderColorManualSaturation = 0;
	mp_spacerColorManualHsb = 0;

	mp_togglePacketsInvert = 0;
	mp_togglePacketsReverse = 0;

	mp_sliderDeviceVolHistoryPingTh = 0;

	mp_toggleDeviceEnableStandup = 0;
	mp_sliderStandupVol = 0;
	
	mp_toggleDeviceGenerative = 0;

	
	mp_graphSoundValues = 0;
	
	mp_canvasColorMode = 0;
	
    setMidiName("tool Devices");
}

//--------------------------------------------------------------
void toolDevices::setup()
{
	if (GLOBALS->mp_app->isSimulation){
		selectDeviceWithIndex(0);
	}
}

//--------------------------------------------------------------
void toolDevices::update()
{
//	OFAPPLOG->begin("toolDevices::update");
	handleMidiMessages();
//	updateDeviceColorUI();
//	OFAPPLOG->end();
}

//--------------------------------------------------------------
void toolDevices::selectDeviceWithIndex(int index)
{
    if (mp_deviceManager && index>=0 && index < mp_deviceManager->m_listDevices.size())
		selectDevice(mp_deviceManager->m_listDevices[index]->m_id);
}


//--------------------------------------------------------------
void toolDevices::show(bool is)
{
	OFAPPLOG->begin("toolDevices::show("+ofToString(is)+")");

	tool::show(is);
	if (mp_canvasDevice)
	{
		if (is && mp_deviceManager->getDeviceCurrent())
			mp_canvasDevice->setVisible(is);
		else
			mp_canvasDevice->setVisible(is);

		mp_canvasDevice->disableAppDrawCallback();
	}
	OFAPPLOG->end();
}

//--------------------------------------------------------------
void  toolDevices::enableDrawCallback(bool is)
{
/*
	tool::enableDrawCallback(is);
	if (mp_canvasDevice){
		if (is){
			mp_canvasDevice->enableAppDrawCallback();
			mp_canvasDevice->enableMouseEventCallbacks();
		}else{
			mp_canvasDevice->disableAppDrawCallback();
			mp_canvasDevice->disableMouseEventCallbacks();
		}
	}
*/
}

//--------------------------------------------------------------
void toolDevices::createControlsCustom()
{
	if (mp_canvas)
	{
		float widthDefault = 320;
	
	    vector<string> listDevicesIds;
		mp_ddlDevices = new ofxUIDropDownList("Devices", listDevicesIds);
    	updateListDevices();
		mp_canvas->addWidgetDown(mp_ddlDevices);
	}
}

//--------------------------------------------------------------
void toolDevices::createControlsCustomFinalize()
{

	float volMaxMax = 1.0f;

	// Load some properties for uis
	string fileConfig =  mp_toolManager->m_relPathData + "/Devices_UI_config.xml";
	ofxXmlSettings uiConfig;
	if (uiConfig.load(fileConfig))
	{
	 volMaxMax = uiConfig.getValue("volMax:max", 1.0f);
	}

	 float widthDefault = 400;
	 float dim = 16;
	 ofxUIWidgetFontType fontType = OFX_UI_FONT_SMALL;

	 ofVec2f pos(
		 getCanvas()->getRect()->getX() + getCanvas()->getRect()->getWidth()+10,
		 getCanvas()->getRect()->getY()
	 );

	mp_canvasDevice = new ofxUICanvas(pos.x,pos.y,widthDefault,300);
	
	mp_canvasDevice->setWidgetFontSize(OFX_UI_FONT_MEDIUM);
	mp_lblDeviceTitle = new ofxUILabel("Devices", OFX_UI_FONT_LARGE);
    mp_canvasDevice->addWidgetDown(mp_lblDeviceTitle);
    mp_canvasDevice->addWidgetDown(new ofxUISpacer(widthDefault, 2));

    mp_canvasDevice->addWidgetDown(new ofxUILabel("Micro", OFX_UI_FONT_MEDIUM));
    mp_canvasDevice->addWidgetDown(new ofxUISpacer(widthDefault, 1));

	
//	if (GLOBALS->mp_app->isSimulation)
	{
		mp_toggleDeviceMute = new ofxUIToggle("mute sound input", false, dim, dim);
		mp_canvasDevice->addWidgetDown( mp_toggleDeviceMute );

		// TODO : link with sound input buffer size
		// mp_graphSoundValues = new ofxUIMovingGraphThreshold("soundInputValues", );
	}

	mp_teDeviceLedsLit = new ofxUITextInput("nb leds", "40", widthDefault);

 
	mp_toggleDeviceUseRawVol = new ofxUIToggle("Use raw volume", false, dim,dim);
    mp_sliderDeviceVolMax = new ofxUISlider("Vol. max", 0.005f, volMaxMax, 0.02f, widthDefault-90, dim );
	mp_teDeviceVolMaxMin = new ofxUITextInput("Vol. max min", "0.00", 40 );
	mp_teDeviceVolMaxMax = new ofxUITextInput("Vol. max max", "0.04", 40 );
	mp_teDeviceVolMaxMin->setAutoClear(false);
	mp_teDeviceVolMaxMax->setAutoClear(false);
    mp_sliderDeviceVolHistorySize = new ofxUISlider("Vol. history size", 50, 500, 400, widthDefault-10, dim );
    mp_toggleDeviceEnableStandby = new ofxUIToggle("Enable standby", true, dim, dim);
    mp_sliderDeviceVolHistoryTh = new ofxUISlider( "Vol. history standby", 0.0f, 0.75f, 0.5f, widthDefault-10, dim );
	mp_sliderDeviceTimeStandby = new ofxUISlider("Time standby", 5.0f, 60.0f, 10.0f, widthDefault-10, dim );
	mp_sliderDeviceSampleVolStandby = new ofxUISlider( "Sample vol. standby", 0.0f, 1.0f, 0.35f, widthDefault-10, dim );
	mp_toggleDevicePitchEnable = new ofxUIToggle("Enable pitch", true, dim, dim);

	mp_sliderDevicePitchMin = new ofxUISlider( "Pitch min", 0.0f, 1200.0f, 30.0f, widthDefault-10, dim );
	mp_sliderDevicePitchMax = new ofxUISlider( "Pitch max", 0.0f, 1200.0f, 60.0f, widthDefault-10, dim );
 
	mp_canvasDevice->addWidgetDown(mp_teDeviceVolMaxMin);
	mp_canvasDevice->addWidgetRight(mp_sliderDeviceVolMax);
	mp_canvasDevice->addWidgetRight(mp_teDeviceVolMaxMax);
//    mp_canvasDevice->addWidgetDown(new ofxUISpacer(widthDefault, 1));
	mp_canvasDevice->addWidgetDown(mp_sliderDeviceVolHistorySize);
	mp_canvasDevice->addWidgetDown(mp_toggleDeviceUseRawVol);

    mp_canvasDevice->addWidgetDown(new ofxUILabel("Pitch", OFX_UI_FONT_MEDIUM));
    mp_canvasDevice->addWidgetDown(new ofxUISpacer(widthDefault, 1));
	mp_canvasDevice->addWidgetDown(mp_toggleDevicePitchEnable);
	mp_canvasDevice->addWidgetDown( mp_sliderDevicePitchMin );
	mp_canvasDevice->addWidgetDown( mp_sliderDevicePitchMax );
	
	
    mp_canvasDevice->addWidgetDown(new ofxUILabel("BPM", OFX_UI_FONT_MEDIUM));
    mp_canvasDevice->addWidgetDown(new ofxUISpacer(widthDefault, 1));


	mp_toggleDeviceBPMEnable = new ofxUIToggle("BPM enable", true, dim, dim);
	mp_sliderDeviceBPM = new ofxUIIntSlider( "BPM", 90, 160, 120, widthDefault-10, dim );

    mp_canvasDevice->addWidgetDown(mp_toggleDeviceBPMEnable);
    mp_canvasDevice->addWidgetDown(mp_sliderDeviceBPM);
	

    mp_canvasDevice->addWidgetDown(new ofxUILabel("Stand by", OFX_UI_FONT_MEDIUM));
    mp_canvasDevice->addWidgetDown(new ofxUISpacer(widthDefault, 1));

    mp_canvasDevice->addWidgetDown(mp_toggleDeviceEnableStandby);
	mp_canvasDevice->addWidgetDown(mp_sliderDeviceVolHistoryTh);
	mp_canvasDevice->addWidgetDown(mp_sliderDeviceTimeStandby);
	mp_canvasDevice->addWidgetDown(mp_sliderDeviceSampleVolStandby);

/*    mp_canvasDevice->addWidgetDown(new ofxUILabel("Stand up", OFX_UI_FONT_MEDIUM));
    mp_canvasDevice->addWidgetDown(new ofxUISpacer(widthDefault, 1));
	

    mp_toggleDeviceEnableStandup = new ofxUIToggle("Enable standup", true, dim, dim);
	mp_sliderStandupVol = new ofxUISlider("Vol. stand up", 0.5f, 1.0f, 0.5f, widthDefault-10, dim );


	mp_canvasDevice->addWidgetDown( mp_toggleDeviceEnableStandup );
	mp_canvasDevice->addWidgetDown( mp_sliderStandupVol );
*/

    mp_canvasDevice->addWidgetDown(new ofxUILabel("LEDs", OFX_UI_FONT_MEDIUM));
    mp_canvasDevice->addWidgetDown(new ofxUISpacer(widthDefault, 1));
	mp_canvasDevice->addWidgetDown(mp_teDeviceLedsLit);

    mp_canvasDevice->addWidgetDown(new ofxUILabel("Generativity", OFX_UI_FONT_MEDIUM));
    mp_canvasDevice->addWidgetDown(new ofxUISpacer(widthDefault, 1));
	mp_toggleDeviceGenerative = new ofxUIToggle("enableGenerative",	false, dim, dim);
	mp_canvasDevice->addWidgetDown(mp_toggleDeviceGenerative);


    mp_canvasDevice->addWidgetDown(new ofxUILabel("Color", OFX_UI_FONT_MEDIUM));
    mp_canvasDevice->addWidgetDown(new ofxUISpacer(widthDefault, 1));

//	mp_canvasColorMode = new ofxUICanvas(0,0,widthDefault,300);
	mp_toggleColorEnable			= new ofxUIToggle("enableColor",	false, dim, dim);
	mp_sliderColorManualHue 		= new ofxUISlider("hue",			1,254,127,(widthDefault-10)/2,dim);
	mp_sliderColorManualSaturation 	= new ofxUISlider("saturation", 	1,254,127,(widthDefault-10)/2,dim);

	mp_spacerColorManualHsb = new ofxUISpacer(0,0,widthDefault,dim/2,"toto");


	mp_canvasDevice->addWidgetDown(mp_toggleColorEnable);
	mp_canvasDevice->addWidgetDown(mp_spacerColorManualHsb);
	mp_canvasDevice->addWidgetDown(mp_sliderColorManualHue);
	mp_canvasDevice->addWidgetRight(mp_sliderColorManualSaturation);

	mp_spacerColorManualHsb->setToggleColor(true);
	mp_spacerColorManualHsb->setDrawFill(true);
	mp_spacerColorManualHsb->setColorFill( ofxUIColor(255,0,255) );

	//mp_canvasColorMode->autoSizeToFitWidgets();
	//mp_canvasDevice->addWidgetDown(mp_canvasColorMode);


    mp_canvasDevice->addWidgetDown(new ofxUILabel("Packets", OFX_UI_FONT_MEDIUM));
    mp_canvasDevice->addWidgetDown(new ofxUISpacer(widthDefault, 1));
	mp_togglePacketsInvert = new ofxUIToggle("invertColorPackets",	false, dim, dim);
    mp_canvasDevice->addWidgetDown(mp_togglePacketsInvert);

	mp_togglePacketsReverse = new ofxUIToggle("reverseDirPackets",	false, dim, dim);
    mp_canvasDevice->addWidgetRight(mp_togglePacketsReverse);

    mp_canvasDevice->addWidgetDown(new ofxUILabel("Ping", OFX_UI_FONT_MEDIUM));
    mp_canvasDevice->addWidgetDown(new ofxUISpacer(widthDefault, 1));
	
	
	mp_sliderDeviceVolHistoryPingTh = new ofxUISlider("ping threshold",	0.0,1.0,0.5,widthDefault,dim);
    mp_canvasDevice->addWidgetDown(mp_sliderDeviceVolHistoryPingTh);
    mp_canvasDevice->addWidgetDown( new ofxUILabelButton("ping reset", false, 0,0,widthDefault,dim,OFX_UI_FONT_SMALL) );


	
    mp_canvasDevice->addWidgetDown(new ofxUILabel("Speakers", OFX_UI_FONT_MEDIUM));
    mp_canvasDevice->addWidgetDown(new ofxUISpacer(widthDefault, 1));

	
	int speakerId=0;
	char tgSpeakerName[16];
	for (int speakerId=0; speakerId<ofFmodGetNumOutputs(); speakerId++)
	{
		sprintf(tgSpeakerName, "s%02d", speakerId);
		m_speakersId.push_back(string(tgSpeakerName));
		ofxUIToggle* pSpeakerToggle = new ofxUIToggle(string(tgSpeakerName),false,20,20);
		if (speakerId%6 == 0)
			mp_canvasDevice->addWidgetDown(pSpeakerToggle);
		else
			mp_canvasDevice->addWidgetRight(pSpeakerToggle);
	}
	
	mp_sliderDeviceVolMax->setLabelPrecision(4);


	mp_canvasDevice->autoSizeToFitWidgets();
	mp_canvasDevice->setVisible(false);
	
	ofAddListener(mp_canvasDevice->newGUIEvent, this, &toolDevices::handleEvents);
}

//--------------------------------------------------------------
void toolDevices::drawUI()
{
	tool::drawUI();
	if (mp_canvasDevice)
		mp_canvasDevice->draw();


	ofPushStyle();
	Device* pDeviceCurrent = mp_deviceManager->getDeviceCurrent();
	if (pDeviceCurrent)
	{
		pDeviceCurrent->drawSoundInputVolume( 0.5f*(ofGetWidth()) ,ofGetHeight()-pDeviceCurrent->getHeightSoundInputVolume());
		pDeviceCurrent->drawSoundInputPitch( 0.5f*(ofGetWidth()) ,ofGetHeight()-2*pDeviceCurrent->getHeightSoundInputVolume());


		ofSetColor(ofColor::white);
		ofDrawBitmapString("color    = "+ofToString(pDeviceCurrent->m_color), 5, ofGetHeight()-2*15);
		ofDrawBitmapString("colorHsv = "+ofToString(pDeviceCurrent->m_colorHsv[0])+", "+ofToString(pDeviceCurrent->m_colorHsv[1])+", "+ofToString(pDeviceCurrent->m_colorHsv[2]), 5, ofGetHeight()-1*15);
    }


	ofPopStyle();
}

//--------------------------------------------------------------
void toolDevices::updateDeviceColorUI()
{
    Device* pDeviceCurrent = mp_deviceManager->getDeviceCurrent();
	if (pDeviceCurrent)
	{
		if (mp_spacerColorManualHsb)   		mp_spacerColorManualHsb->setColorFill( ofColor::fromHsb(pDeviceCurrent->m_colorHsv[0], pDeviceCurrent->m_colorHsv[1], 255.0f) );
		if (mp_sliderColorManualHue)		mp_sliderColorManualHue->setValue(pDeviceCurrent->m_colorHsv[0]);
		if (mp_sliderColorManualSaturation)	mp_sliderColorManualSaturation->setValue(pDeviceCurrent->m_colorHsv[1]);
	}
}

//--------------------------------------------------------------
void toolDevices::updateListDevices()
{
	OFAPPLOG->begin("toolDevices::updateListDevices");
    if (mp_deviceManager && mp_ddlDevices)
    {
		mp_ddlDevices->clearToggles();
	
        vector<string> listDevicesId;
        mp_deviceManager->getDevicesListId(listDevicesId);

        OFAPPLOG->println("Device manager has now "+ofToString(listDevicesId.size())+" device(s)");
     
		mp_ddlDevices->setAutoClose(false);
        //mp_ddlDevices->init("Devices",listDevicesId);
		mp_ddlDevices->addToggles(listDevicesId);
	}
	OFAPPLOG->end();
}

//--------------------------------------------------------------
void toolDevices::selectDevice(string id)
{
    if (mp_deviceManager)
    {
        Device* pDevice = mp_deviceManager->setDeviceCurrent( id );
        if (pDevice)
        {
            // Label of container
            if (mp_lblDeviceTitle) mp_lblDeviceTitle->setLabel(pDevice->m_id);
        
            // Data
            updateDeviceUI(pDevice);

			// Inform tool Scene
			toolScene* pToolScene = (toolScene*)toolManager::instance()->getTool("Scene");
			if (pToolScene)
				pToolScene->onDeviceChanged();
        }
    }
}

//--------------------------------------------------------------
void toolDevices::updateDeviceAnimationTitle()
{
	toolSurfaces* pToolSurface = (toolSurfaces*) mp_toolManager->getTool("Surfaces");
	if (pToolSurface == 0) return;

	toolAnimations* pToolAnimation = (toolAnimations*) mp_toolManager->getTool("Animations");
	if (pToolAnimation == 0) return;

   Surface* pSurfaceCurrent = pToolSurface->getSurfaceForDeviceCurrent();
   if (pSurfaceCurrent)
   {
   	 if (pToolAnimation->mp_lblAnimDirJs)
	 {
			 pToolAnimation->mp_lblAnimDirJs->setLabel("> dir "+pSurfaceCurrent->m_strDirScripts);
	 }
   
	 if (pToolAnimation->mp_lblAnimTitle)
	 {
	 	if (pSurfaceCurrent->getAnimationManager().mp_animationCurrent)
	 		pToolAnimation->mp_lblAnimTitle->setLabel("> surface '"+pSurfaceCurrent->getId()+"' / playing '"+pSurfaceCurrent->getAnimationManager().mp_animationCurrent->m_name+"'");
	 }
	}

}

//--------------------------------------------------------------
void toolDevices::updateDeviceUI(Device* pDevice)
{
    if (pDevice)
    {
		if (mp_toggleDeviceUseRawVol) mp_toggleDeviceUseRawVol->setValue( pDevice->getSoundInputUseRawVolume() );
		if (mp_toggleDeviceMute) mp_toggleDeviceMute->setValue( pDevice->getSoundInputMute() );
		if (mp_teDeviceVolMaxMin) mp_teDeviceVolMaxMin->setTextString( ofToString( pDevice->getSoundInputVolumeMaxMin() ) );
		if (mp_teDeviceVolMaxMax) mp_teDeviceVolMaxMax->setTextString( ofToString( pDevice->getSoundInputVolumeMaxMax() ) );
		if (mp_sliderDeviceVolMax)
		{
			mp_sliderDeviceVolMax->setMax( pDevice->getSoundInputVolumeMaxMax() );
			mp_sliderDeviceVolMax->setValue( pDevice->getSoundInputVolumeMax() );
		}
		if (mp_sliderDeviceVolHistorySize) mp_sliderDeviceVolHistorySize->setValue( float(pDevice->getSoundInputVolHistorySize()) );
        if (mp_sliderDeviceVolHistoryTh) mp_sliderDeviceVolHistoryTh->setValue( float(pDevice->getSoundInputVolHistoryTh()) );
	 	if (mp_sliderDevicePitchMin) mp_sliderDevicePitchMin->setValue( float(pDevice->getSoundInputPitchMin()) );
	 	if (mp_sliderDevicePitchMax) mp_sliderDevicePitchMax->setValue( float(pDevice->getSoundInputPitchMax()) );
		
		if (mp_toggleDeviceEnableStandby) mp_toggleDeviceEnableStandby->setValue( pDevice->getEnableStandbyMode() );
        if (mp_sliderDeviceTimeStandby) mp_sliderDeviceTimeStandby->setValue( pDevice->getTimeStandby() );
        if (mp_sliderDeviceSampleVolStandby) mp_sliderDeviceSampleVolStandby->setValue( pDevice->getSampleVolStandby() );
		if (mp_sliderStandupVol) mp_sliderStandupVol->setValue( pDevice->getStandupVol() );
		if (mp_toggleColorEnable) mp_toggleColorEnable->setValue( pDevice->m_isEnableColor );
		if (mp_sliderColorManualHue) mp_sliderColorManualHue->setValue( pDevice->m_colorHsv[0] );
		if (mp_sliderColorManualSaturation) mp_sliderColorManualSaturation->setValue( pDevice->m_colorHsv[1] );
		if (mp_togglePacketsInvert) mp_togglePacketsInvert->setValue( pDevice->m_isInvertPacketsVolume );
		if (mp_togglePacketsReverse) mp_togglePacketsReverse->setValue( pDevice->m_isReverseDirPackets );
		if (mp_sliderDeviceVolHistoryPingTh) mp_sliderDeviceVolHistoryPingTh->setValue( pDevice->m_volHistoryPingTh );
		
		if (mp_toggleDeviceGenerative) mp_toggleDeviceGenerative->setValue( pDevice->isGenerative() );
		
		if (mp_toggleDeviceBPMEnable) mp_toggleDeviceBPMEnable->setValue( pDevice->isBPMEnabled() );
		if (mp_sliderDeviceBPM) mp_sliderDeviceBPM->setValue( pDevice->getBPM() );

		if (mp_canvasDevice)
		{
			for (int i=0;i<m_speakersId.size();i++)
			{
				ofxUIToggle* pSpeakerToggle = (ofxUIToggle*) mp_canvasDevice->getWidget( m_speakersId[i] );
				pSpeakerToggle->setValue(false);
			}

			for (int i=0;i<m_speakersId.size();i++)
			{
				ofxUIToggle* pSpeakerToggle = (ofxUIToggle*) mp_canvasDevice->getWidget( m_speakersId[i] );
				if (pSpeakerToggle)
				{
					for (int j=0; j<pDevice->m_listSpeakerIds.size() ;j++){
						if (pDevice->m_listSpeakerIds[j] == i)
						{
							pSpeakerToggle->setValue(true);
						}
					}
				}
			}
		}

		 updateDeviceAnimationTitle();
		 updateDeviceColorUI();
    }
}


//--------------------------------------------------------------
void toolDevices::handleEvents(ofxUIEventArgs& e)
{

    Device* pDeviceCurrent = mp_deviceManager->getDeviceCurrent();
	if (pDeviceCurrent == 0) return;

    string name = e.widget->getName();

    if (name == "Devices")
    {

        ofxUIDropDownList *ddlist = (ofxUIDropDownList *) e.widget;
        vector<ofxUIWidget *> &selected = ddlist->getSelected();
        if (selected.size()==1)
            selectDevice( selected[0]->getName() );

    }
	else if (name == "mute sound input")
	{
		if (pDeviceCurrent)
			pDeviceCurrent->setSoundInputMute( ((ofxUIToggle *) e.widget)->getValue() );

/*		if (pDeviceCurrent)
			pDeviceCurrent->mute( ((ofxUIToggle *) e.widget)->getValue() );
*/
	}
	else if (name == "nb leds")
	{
		int triggerType  = mp_teDeviceLedsLit->getInputTriggerType();
		if (triggerType == OFX_UI_TEXTINPUT_ON_ENTER)
		{
			int nbLeds = ofToInt(mp_teDeviceLedsLit->getTextString());
	        if (pDeviceCurrent)
    	        pDeviceCurrent->setNbLeds( nbLeds );
		}
	}
    else if (name == "Use raw volume")
	{
        if (pDeviceCurrent)
            pDeviceCurrent->setSoundInputUseRawVolume ( e.getToggle()->getValue() );
	}
    else if (name == "Vol. max min")
	{
		int triggerType  = mp_teDeviceVolMaxMin->getInputTriggerType();
		if (triggerType == OFX_UI_TEXTINPUT_ON_ENTER && mp_sliderDeviceVolMax)
		{
			float min = ofToFloat(mp_teDeviceVolMaxMin->getTextString());
			mp_sliderDeviceVolMax->setMin( min);

	        if (pDeviceCurrent)
    	        pDeviceCurrent->setSoundInputVolumeMaxMin( min );
		}
	}
    else if (name == "Vol. max max")
	{
		int triggerType  = mp_teDeviceVolMaxMax->getInputTriggerType();
		if (triggerType == OFX_UI_TEXTINPUT_ON_ENTER && mp_sliderDeviceVolMax)
		{
			float max = ofToFloat(mp_teDeviceVolMaxMax->getTextString());
			mp_sliderDeviceVolMax->setMax( max);

	        if (pDeviceCurrent)
    	        pDeviceCurrent->setSoundInputVolumeMaxMax( max );
		}
	}
    else if (name == "Vol. max")
    {
        if (pDeviceCurrent)
            pDeviceCurrent->setSoundInputVolumeMax( ((ofxUISlider *) e.widget)->getScaledValue() );
    }
    else if (name == "Vol. history size")
    {
        if (pDeviceCurrent)
            pDeviceCurrent->setSoundInputVolHistorySize( (int) ((ofxUISlider *) e.widget)->getScaledValue() );
    }
    else if (name == "Enable standby")
    {
        if (pDeviceCurrent)
            pDeviceCurrent->setEnableStandbyMode( ((ofxUIToggle *) e.widget)->getValue() );
    }
    else if (name == "Vol. history standby")
    {
        if (pDeviceCurrent)
            pDeviceCurrent->setSoundInputVolHistoryTh( ((ofxUISlider *) e.widget)->getScaledValue() );
    }
    else if (name == "Enable pitch")
    {
        if (pDeviceCurrent)
            pDeviceCurrent->setEnablePitch( ((ofxUIToggle *) e.widget)->getValue() );
    }
    else if (name == "Pitch min")
    {
        if (pDeviceCurrent)
            pDeviceCurrent->setSoundInputPitchMin( ((ofxUISlider *) e.widget)->getScaledValue() );
    }
    else if (name == "Pitch max")
    {
        if (pDeviceCurrent)
            pDeviceCurrent->setSoundInputPitchMax( ((ofxUISlider *) e.widget)->getScaledValue() );
    }
    else if (name == "Time standby")
    {
        if (pDeviceCurrent){
            pDeviceCurrent->setTimeStandby( ((ofxUISlider *) e.widget)->getScaledValue() );
        }
    }
    else if (name == "Sample vol. standby")
    {
        if (pDeviceCurrent){
            pDeviceCurrent->setSampleVolumeStandby( ((ofxUISlider *) e.widget)->getScaledValue() );
        }
    }
    else if (name == "Enable standup")
    {
        if (pDeviceCurrent)
            pDeviceCurrent->setEnableStandup( ((ofxUIToggle *) e.widget)->getValue() );
	}
    else if (name == "Vol. stand up")
    {
        if (pDeviceCurrent){
			pDeviceCurrent->setStandupVol( ((ofxUISlider *) e.widget)->getScaledValue() );
        }
    }
    else if (name == "enableColor")
    {
        if (pDeviceCurrent){
			pDeviceCurrent->enableColor( e.getToggle()->getValue() );
		}
	}
    else if (name == "hue")
    {
        if (pDeviceCurrent)
		{
			pDeviceCurrent->setColorHue( ((ofxUISlider *) e.widget)->getScaledValue() );
			updateDeviceColorUI();
        }
    }
    else if (name == "saturation")
    {
        if (pDeviceCurrent)
		{
			pDeviceCurrent->setColorSaturation( ((ofxUISlider *) e.widget)->getScaledValue() );
			updateDeviceColorUI();
        }
    }
	else if (name == "invertColorPackets")
	{
		if (pDeviceCurrent)
			pDeviceCurrent->invertPacketsVolume( e.getToggle()->getValue() );
	}
	else if (name == "reverseDirPackets")
	{
		if (pDeviceCurrent)
			pDeviceCurrent->reversePacketsDir( e.getToggle()->getValue() );
	}
	else if (name == "ping threshold")
	{
		if (pDeviceCurrent)
			pDeviceCurrent->setVolHistoryPingTh( e.getSlider()->getValue() );
	}
	else if (name == "ping reset")
	{
		ofxUILabelButton* pBtn = (ofxUILabelButton*) e.widget;
		if (pDeviceCurrent && pBtn->getValue())
			pDeviceCurrent->resetVolHistoryPing();
	}
	else if (name == "enableGenerative")
	{
        if (pDeviceCurrent){
			pDeviceCurrent->setGenerative( e.getToggle()->getValue() );
		}
	}
	else if (name == "BPM")
	{
        if (pDeviceCurrent){
			pDeviceCurrent->setBPM( ((ofxUIIntSlider*)e.widget)->getScaledValue() );
		}
	}
	else if (name == "BPM enable")
	{
        if (pDeviceCurrent){
			pDeviceCurrent->setBPMEnable( e.getToggle()->getValue() );
		}
	}
	else
	{
		if (pDeviceCurrent)
		{
			pDeviceCurrent->clearListSpeakers();
			for (int i=0;i<m_speakersId.size();i++)
			{
				ofxUIToggle* pToggle = (ofxUIToggle*) this->mp_canvasDevice->getWidget(m_speakersId[i]);
				if (pToggle->getValue()){
					pDeviceCurrent->addSpeakerId(i);
					// ofLog() << pDeviceCurrent->m_listSpeakerIds.size();
				}
			}
		}

	}
}

//--------------------------------------------------------------
bool toolDevices::isHit(int x, int y)
{
	return ( tool::isHit(x,y) || mp_canvasDevice->isHit(x, y));
}

//--------------------------------------------------------------
bool toolDevices::keyPressed(int key)
{
 	int nbDevices = mp_deviceManager->getDevicesNb();
 
	if (key == OF_KEY_UP)
	{
		if (nbDevices>0)
		{
	  		this->selectDeviceWithIndex( (mp_deviceManager->indexOfCurrent()+1)%nbDevices );
			return true;
  		}
	}
  	else if (key == OF_KEY_DOWN)
  	{
		if (nbDevices>0)
		{
			int index = mp_deviceManager->indexOfCurrent()-1;
			if (index <0){
				index = nbDevices-1;
			}
	  		this->selectDeviceWithIndex( index );
			return true;
  		}
  	}

	return false;
}

//--------------------------------------------------------------
void toolDevices::enableWindowCallbacks()
{
	tool::enableWindowCallbacks();
	if (mp_canvasDevice) mp_canvasDevice->EnableCallbacks();
}

//--------------------------------------------------------------
void toolDevices::disableWindowCallbacks()
{
	tool::disableWindowCallbacks();
	if (mp_canvasDevice) mp_canvasDevice->DisableCallbacks();
}

//--------------------------------------------------------------
void toolDevices::loadMidiSettings()
{
	midiInterface::setClassProperties(&m_properties) ;
	midiInterface::loadMidiSettings();
}

//--------------------------------------------------------------
void toolDevices::readMidiSettingsExtraBegin(int which, string propName)
{
	classProperty_float* pProp = new classProperty_float(propName, 0.0f,1.0f);
	pProp->enableEvents();
	ofAddListener(pProp->onValueChanged, this, &toolDevices::onDevicePropChanged);


	m_properties.add( pProp );
}

//--------------------------------------------------------------
void  toolDevices::newMidiMessage(ofxMidiMessage& eventArgs)
{
//	OFAPPLOG->println("toolDevices::newMidiMessage()");
	midiInterface::newMidiMessage(eventArgs);
}

//--------------------------------------------------------------

void toolDevices::handleMidiMessages()
{
	midiInterface::handleMidiMessages();
}


//--------------------------------------------------------------
void toolDevices::onDevicePropChanged(const void* sender, float& value)
{
	if (mp_deviceManager)
	{
		classProperty_float* pProperty = (classProperty_float*) sender;
		vector <string> parts = ofSplitString(pProperty->m_name, ".");
		if (parts.size()==2){
			Device* pDevice = mp_deviceManager->getDeviceById(parts[0]);
			if (pDevice)
			{
				if (parts[1] == "soundInputVolEmpiricalMax")
				{
					pDevice->setSoundInputVolumeMax( value*pDevice->getSoundInputVolumeMaxMax() );
					if (pDevice == mp_deviceManager->getDeviceCurrent())
						updateDeviceUI(pDevice);
				}
			}
		}
	
	}
}





