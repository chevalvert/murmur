/*
 *  surface.h
 *  murmur
 *
 *  Created by Julien on 10/04/13.
 *  Copyright 2013 2Roqs. All rights reserved.
 *
 */

#pragma once
#include "murmur.h"
#include "ofFbo.h"
#include "animations.h"
#include "device.h"
#if MURMUR_DEFINE_SYPHON
	#include "ofxSyphon.h"
#endif
class quadWarping;
class Surface
{
	public:
		Surface		(string id, int wPixels, int hPixels);
		Surface		(string id);

        enum{
            EStandby_active              = 0,
            EStandby_pre_standby         = 1,
            EStandby_standby             = 2
        };
 
		typedef void (*FRenderOffscreen)(Surface*,void*);
		ofEvent<ofVec2f> m_eventOffscreenChanged;


		void			zeroAll					();
    
        ofFbo&          getOffscreen    		(){return m_fbo;}
        void            setDimensions			(int w, int h, int nbSamples=0);
		int				getQuality				(){return getNbSamples();}
		int				getNbSamples			(){return m_fboNbSamples;}

        void            setup           		();
        void            update          		(float dt);
        void            renderOffscreen 		(bool isRenderDevicePoints=false);
		void			setRenderOffscreenCb	(FRenderOffscreen pFunction, void* pUserData){mpf_renderOffscreenCallback = pFunction; mp_renderOffscreenUserData=pUserData;}
        void            addDevice       		(Device*);
        vector<Device*>&getListDevices 			(){return m_listDevices;}
        void            onNewPacket     		(DevicePacket*, string deviceId);
//        void            drawDevicePointSurface	(ofRectangle&, ofMatrix4x4* mWarping=0);
        void            drawDevicePointSurface	(ofRectangle&, quadWarping* pWarping=0);
        void            drawCacheLEDs   		(float d);
        float           getWidthPixels  		(){return m_fbo.getWidth();}
        float           getHeightPixels 		(){return m_fbo.getHeight();}
        string          getId           		(){return m_id;}
    
        void            setDurationTransition	(float d){m_durationTransition=d;}
        void            setDurationAnimation	(float d){m_durationAnimation=d;}
        void            setTimelineActive		(bool is);
        bool            isTimelineActive		(){return m_isTimelineActive;}
    
        AnimationManager& getAnimationManager	(){return m_animationManager;}
    
        void            saveAnimationsProperties();
        void            loadAnimationsProperties();
	
		void			setPublishSyphon		(bool is=true);
		void			publishSyphon			();
	
		void			drawRenderTarget		();
		void			setRenderTarget			(bool is=true);
		void			setRenderTargetLineWidth(float w=2.0f);

		void			setDrawMask				(bool is=true){m_isDrawMask = is;}
		void			setMask					(ofImage* image){mp_mask = image;}
		void			drawMask				();

		void			setLayerAlpha			(float alpha){m_alphaLayer=alpha;}
		void			drawLayerAlpha			();
	
		float			getVolumePacketsHistoryMean	(){return m_volumePacketsHistoryMean;}
		string			getStateActivity			();
		void			setGoStandbyTh				(float v){m_volumePacketsHistoryMeanTh_goStandby=v;}
		void			setGoActiveTh				(float v){m_volumePacketsHistoryMeanTh_goActive=v;}
		void			checkActivity				(float dt);
		void			setEnableStandby			(bool is){m_isEnableStandby=is; if (!m_isEnableStandby) m_stateActivity=EStandby_active;}
		void			setDurationPreStandby		(float v){m_durationPreStandby=v;}
	
		string			m_strDirScripts;
		bool			m_isEnableStandby;
		bool			m_isLaunchPacket;
		float			m_timeLaunchPacket;
		float			m_durationLaunchPacket;
		float			m_durationNoLaunchPacket;
		float			m_volumePacketsHistoryMeanTh_goStandby,m_volumePacketsHistoryMeanTh_goActive;
		float			m_timeStandby;
		float			m_durationPreStandby;
	

        string          m_id;
		int				m_fboNbSamples;
#if MURMUR_DEFINE_SYPHON
		ofxSyphonServer 	m_syphonServer;
#endif

    private:
        int                 m_stateActivity;
		ofFbo               m_fbo;
        AnimationManager    m_animationManager;
        vector<Device*>     m_listDevices;
        ofVec2f             m_devicePoint;
 
		FRenderOffscreen	mpf_renderOffscreenCallback;
		void*				mp_renderOffscreenUserData;
    
        float               m_durationTransition;
        float               m_durationAnimation;
        bool                m_isTimelineActive;
        bool                m_isAnimationTransiting;
        float               m_timeAnimation;
		bool				m_isPublishSyphon;
		bool				m_isRenderTarget;
		float				m_renderTargetLineWidth;
		bool				m_isDrawMask;
	
		void				computeVolumePacketsMean(DevicePacket* pDevicePacket);
		float				m_volumePacketsHistoryMean;
        vector <float>      m_volumePacketsHistory;
		int					m_volumePacketsHistoryNb;
	
		ofImage*			mp_mask;
		float				m_alphaLayer;

	
};
