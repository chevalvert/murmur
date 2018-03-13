//
//  animationWaveShader.h
//  murmur
//
//  Created by Julien on 16/05/13.
//
//

#pragma once
#include "animations.h"


class ShaderWave
{
	public:
	
		ShaderWave				();

		void					update				(float dt);
		void					setSize				(int w);
		void					clearSoundInput		();

       	ofFloatImage            m_imgSoundInput;
 		//ofImage					 m_imgSoundInput;
		ofVec2f                	m_anchor,m_anchorNorm;
        float                  	m_volume;

		ofFloatColor			m_colorWhite;
		ofFloatColor			m_colorDevice;
		ofFloatColor			m_color, m_colorTarget;
		float					m_fColor;
 
		bool					m_bUpdateTexture;
};

class AnimationShaderWave : public Animation
{
    public:
    
        AnimationShaderWave(string name);
		~AnimationShaderWave();
 
        virtual	void			VM_enter				();
        virtual void			VM_update				(float dt);
        virtual void			VM_draw					(float w, float h);
        virtual	void			VM_exit					();
		virtual	void			onVolumAccumEvent		(string deviceId);
        virtual void            onNewPacket             (DevicePacket*, string deviceId, float xNorm, float yNorm);
		virtual	void			createUICustom			();
		virtual	void			guiEvent				(ofxUIEventArgs &e);
				void			setSizeVolumeTexture	();

        ofFloatImage            m_imgSoundInput;
        ofVec2f                 m_anchor,m_anchorNorm;
        float                   m_volume;
		float					m_waveIntensity;
		float					m_waveAmpSine, m_waveFreqSine,m_waveFreqCosine;
		bool					m_isBlend;
		int						m_waveVolumeTexture;
		float					m_time;
		float					m_pitchCurrent, m_pitchRelax;
		float					m_volRetainPitch;
	
	private:
		map<string, ShaderWave*>	m_mapShaderWaves;
		map<string, bool>			m_mapVolMeanAbove;
 
		ofxUISlider*				mp_sliderAmpSine;


};
