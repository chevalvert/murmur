//
//  toolMidi.cpp
//  murmur
//
//  Created by Julien on 10/09/2015.
//
//

#include "toolMidi.h"
#include "ofxMidi.h"
#include "midiInterface.h"
#include "ofAppLog.h"

int toolMidi::sm_timecodeMillis = 0;

//--------------------------------------------------------------
toolMidi::toolMidi(toolManager* parent) : tool("Midi", parent)
{
	mp_midiIns						= 0;
	mp_ddMidiInterfaces 			= 0;
	mp_canvasMidiInterfaceCurrent	= 0;
	mp_midiInterfaceCurrent			= 0;
	
	mp_lblTimecodeValue				= 0;
}

//--------------------------------------------------------------
toolMidi::~toolMidi()
{
	m_midiInterfaces.clear();
}

//--------------------------------------------------------------
void toolMidi::createControlsCustom()
{
	if (mp_canvas)
	{
		ofxUIWidgetFontType fontType = OFX_UI_FONT_SMALL;
		float dim = 16;
		int widthDefault = 320;

	    mp_canvas->addWidgetDown( new ofxUILabel("Midi",OFX_UI_FONT_LARGE) );
    	mp_canvas->addWidgetDown( new ofxUISpacer(widthDefault-8, 2) );

		if (mp_midiIns && mp_midiIns->size()>0)
		{
			int nbMidiIns = mp_midiIns->size();
			for (int i=0; i<nbMidiIns ; i++)
			{
				int port = (*mp_midiIns)[i]->getPort();
				string portName = ofxMidiIn::getPortName( port );
				if (portName != "")
				{
					string strPort = ofToString( port ) + " - " + portName;
			    	mp_canvas->addWidgetDown( new ofxUILabel(strPort,OFX_UI_FONT_SMALL) );
				}
			}
		}


		if (mp_midiIns && mp_midiIns->size()>0)
		{
			ofxUIRectangle* pCanvasRect = mp_canvas->getRect();
		
			int nbMidiInterfaces = m_midiInterfaces.size();
			float y = pCanvasRect->getY();
			for (int i=0; i<nbMidiInterfaces; i++)
			{
            	ofxUICanvas* pMidiCanvas = new ofxUICanvas(240, y, widthDefault, 300);
				midiInterface* pMidiInterface = m_midiInterfaces[i];

		    	pMidiCanvas->addWidgetDown( new ofxUILabel(pMidiInterface->m_name,OFX_UI_FONT_MEDIUM) );
				pMidiCanvas->addWidgetDown( new ofxUISpacer(widthDefault-8, 2) );

				bool hasProperties = false;
				for (int i=0; i<mp_midiIns->size(); i++)
				{
					midiPort* pMidiPort = pMidiInterface->getMidiPort(i);
					if (pMidiPort)
					{
						map<int,classProperty*>& mapProp = pMidiPort->m_mapMidiControlToProp;
						map<int,classProperty*>::iterator it = mapProp.begin();
						if(mapProp.size()>0)
						{
							hasProperties = true;
							for ( ; it!=mapProp.end(); ++it)
							{
						    	pMidiCanvas->addWidgetDown( new ofxUILabel(widthDefault/2, it->second->m_name,OFX_UI_FONT_SMALL) );

								ofxUITextInput* pTePort = new ofxUITextInput("tePort_"+it->second->m_name, ofToString( 0 ), 24, dim, widthDefault/2);
								ofxUITextInput* pTeControl = new ofxUITextInput("teCtrl_"+it->second->m_name, ofToString( it->first ), 24, dim, widthDefault/2);
								pTeControl->setAutoUnfocus(true);
								pTeControl->setAutoClear(false);
//								pTeControl->setInputTriggerType();
						    	pMidiCanvas->addWidgetRight( pTePort );
						    	pMidiCanvas->addWidgetRight( pTeControl );
							}
						}
					}
				}

				if (!hasProperties)
				{
					pMidiCanvas->addWidgetDown( new ofxUILabel("No midi properties defined.",OFX_UI_FONT_SMALL) );
				}

				//pMidiCanvas->autoSizeToFitWidgets();

				y+=pMidiCanvas->getRect()->getHeight();

				m_mapMidiCanvas[pMidiInterface] = pMidiCanvas;
				pMidiCanvas->setVisible(false);
				pMidiCanvas->disableAppDrawCallback();
				ofAddListener(pMidiCanvas->newGUIEvent, this, &toolMidi::handleEventsMidiInterface);
				//mp_canvas->addWidgetDown( pMidiCanvas );
			}

			vector<string> midiInterfacesNames;
			for (int i=0; i<nbMidiInterfaces; i++)
			{
				 midiInterfacesNames.push_back( m_midiInterfaces[i]->m_name );
			}
			
			mp_ddMidiInterfaces = new ofxUIDropDownList("interfaces", midiInterfacesNames);
			mp_ddMidiInterfaces->setAutoClose(true);
			mp_ddMidiInterfaces->setModal(true);
			mp_canvas->addWidgetDown( mp_ddMidiInterfaces );


			mp_canvas->autoSizeToFitWidgets();
		 }
		
		 mp_canvasTimecode = new ofxUICanvas(240+widthDefault+10, 0, widthDefault, 300);

	    mp_canvasTimecode->addWidgetDown( new ofxUILabel("Timecode",OFX_UI_FONT_LARGE) );
    	mp_canvasTimecode->addWidgetDown( new ofxUISpacer(widthDefault-8, 2) );
		mp_canvasTimecode->addWidgetDown( new ofxUILabel(100, "Port",OFX_UI_FONT_SMALL) );
		ofxUITextInput* pTePortTimecode = new ofxUITextInput("tePortTimecode", ofToString( 0 ), 24, dim, 100+5);
		mp_canvasTimecode->addWidgetRight( pTePortTimecode );
		mp_canvasTimecode->addWidgetDown( new ofxUILabel(100, "Value",OFX_UI_FONT_SMALL) );
		mp_lblTimecodeValue = new ofxUILabel(100, "1234",OFX_UI_FONT_SMALL);
	    mp_canvasTimecode->addWidgetRight( mp_lblTimecodeValue );
		

		m_portTimecode = getPortTimecode( pTePortTimecode->getTextString() );
	  	onPortTimecodeChanged();


		ofAddListener(mp_canvas->newGUIEvent, this, &toolMidi::handleEvents);
		ofAddListener(mp_canvasTimecode->newGUIEvent, this, &toolMidi::handleEventsTimecode);
	}
	
}

//--------------------------------------------------------------
midiInterface* toolMidi::getMidiInterfaceByName(string name)
{
	vector<midiInterface*>::iterator it = m_midiInterfaces.begin();
	for ( ; it != m_midiInterfaces.end() ; ++it)
	{
		if ( (*it)->m_name == name)
			return *it;
	}
	return 0;
}

//--------------------------------------------------------------
void toolMidi::setMidiIns(vector<ofxMidiIn*>* pMidiIns)
{
	mp_midiIns = pMidiIns;
}

//--------------------------------------------------------------
void toolMidi::registerMidiInterface(midiInterface* pMidiInterface)
{
	m_midiInterfaces.push_back(pMidiInterface);
}


//--------------------------------------------------------------
void toolMidi::show(bool is)
{
	if (is == false)
		hideMidiInterfaces();
	else
	{
		if (mp_canvasMidiInterfaceCurrent)
		{
			mp_canvasMidiInterfaceCurrent->setVisible(true);
		}
		if (mp_canvasTimecode)
		{
			mp_canvasTimecode->setVisible(true);
		}

	}
	tool::show(is);

}
//--------------------------------------------------------------
void toolMidi::setup()
{
	ofAddListener( m_MTCReceiver.MTCEvent, this, &toolMidi::handleMTCMessage  );
}

//--------------------------------------------------------------
void toolMidi::handleMTCMessage(MTCEventArgs& e)
{
	int currentMillis = e.timeAsMillis;
	sm_timecodeMillis = currentMillis;
}


//--------------------------------------------------------------
void toolMidi::update()
{
	updateLayout();
}

//--------------------------------------------------------------
void toolMidi::updateLayout()
{
	if (mp_lblTimecodeValue)
	{
		mp_lblTimecodeValue->setLabel( m_MTCReceiver.timeAsString( sm_timecodeMillis ) );
	}
}

//--------------------------------------------------------------
void toolMidi::handleEvents(ofxUIEventArgs& e)
{
	string name = e.getName();
    if (name == "interfaces")
    {
        ofxUIDropDownList *ddlist = (ofxUIDropDownList *) e.widget;
        vector<ofxUIWidget *> &selected = ddlist->getSelected();
		vector<int>& selectedIndices = ddlist->getSelectedIndeces();
        if (selectedIndices.size()==1)
		{
			hideMidiInterfaces();
			
			 midiInterface* pMidiInterface = getMidiInterfaceByName( selected[0]->getName() );
			 if (pMidiInterface)
			 {
				 ofxUICanvas* pMidiInterfaceCanvas = m_mapMidiCanvas[pMidiInterface];
				 if (pMidiInterfaceCanvas)
				 {
					pMidiInterfaceCanvas->setPosition(mp_canvas->getRect()->getX(),mp_canvas->getRect()->getY() + mp_canvas->getRect()->getHeight());
					pMidiInterfaceCanvas->setVisible(true);
					
					mp_canvasMidiInterfaceCurrent = pMidiInterfaceCanvas;
					mp_midiInterfaceCurrent = pMidiInterface;
				}
				else
				{
					mp_canvasMidiInterfaceCurrent = 0;
					mp_midiInterfaceCurrent = 0;
				}
			}
		}
    }
}

//--------------------------------------------------------------
void toolMidi::handleEventsMidiInterface(ofxUIEventArgs& e)
{
  if (e.getKind() == OFX_UI_WIDGET_TEXTINPUT)
  {
	  ofxUITextInput* pTe = (ofxUITextInput*) e.widget;
	  if (pTe->getInputTriggerType() == OFX_UI_TEXTINPUT_ON_ENTER)
	  {
		bool doSave = false;
		
	  	// Port ou controle ?
		if ( e.getName().substr(0,7) == "teCtrl_" )
		{
			if (mp_midiInterfaceCurrent)
			{
				string propName = e.getName().substr(7);
				int control = atoi( pTe->getTextString().c_str() );
				// ofLog() << " - changing control value to " << ofToString( control ) << " for " << propName;
				mp_midiInterfaceCurrent->setControlForProperty(control, propName);
				doSave = true;
			}

		}
		else
		if ( e.getName().substr(0,7) == "tePort_" )
		{
			string propName = e.getName().substr(7);
		}
	  
		if (doSave)
		{
		  	vector<midiInterface*>::iterator it = m_midiInterfaces.begin();
			for ( ; it != m_midiInterfaces.end(); ++it)
			{
				(*it)->saveMidiSettings();
			}
	  
			ofLog() << " - saving all midi interfaces";

			// Load them again to refresh data
			for ( ; it != m_midiInterfaces.end(); ++it)
			{
				(*it)->loadMidiSettings();
			}
		}
	
	  }
  }
}

//--------------------------------------------------------------
void toolMidi::handleEventsTimecode(ofxUIEventArgs& e)
{
  if (e.getKind() == OFX_UI_WIDGET_TEXTINPUT)
  {
	  ofxUITextInput* pTe = (ofxUITextInput*) e.widget;
	  if (pTe->getInputTriggerType() == OFX_UI_TEXTINPUT_ON_ENTER)
	  {
		m_portTimecode = getPortTimecode( pTe->getTextString() );
	  	onPortTimecodeChanged();
	  }
   }
}


//--------------------------------------------------------------
void toolMidi::hideMidiInterfaces()
{
	map<midiInterface*, ofxUICanvas*>::iterator it = m_mapMidiCanvas.begin();
	for ( ; it!=m_mapMidiCanvas.end() ; ++it)
	{
		it->second->setVisible(false);
		it->second->DisableCallbacks();
	}
	
	if (mp_canvasTimecode){
		mp_canvasTimecode->setVisible(false);
		mp_canvasTimecode->DisableCallbacks();
	}
}

//--------------------------------------------------------------
void toolMidi::onPortTimecodeChanged()
{
	OFAPPLOG->begin("toolMidi::onPortTimecodeChanged()");
	OFAPPLOG->println(" - init receiver with port "+ofToString(m_portTimecode));
	sm_timecodeMillis = 0;
	m_MTCReceiver.close();
	m_MTCReceiver.init(m_portTimecode);
	OFAPPLOG->end();
}




