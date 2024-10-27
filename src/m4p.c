#include "m4p.h"
#include <string.h>

// IT/S3M
extern bool IT_InitMusic(int32_t mixingFrequency, int32_t mixingBufferSize);
extern bool IT_LoadMusicfromData(uint8_t *Data, uint32_t DataLen);
extern void IT_PlaySong(uint16_t order);
extern void IT_MixAudio(int16_t *buffer, int32_t numSamples);
extern void IT_MixAudioFloat(float *buffer, int32_t numSamples);
extern void IT_CloseDriver(void);
extern void IT_StopPlayback(void);
extern void IT_FreeMusic(void);

// XM/MOD/FT
extern bool FT2_InitMusic(int32_t audioFrequency, int32_t audioBufferSize, bool interpolation, bool volumeRamping);
extern bool FT2_LoadMusicfromData(const uint8_t *data, uint32_t dataLength);
extern void FT2_StartPlayback(void);
extern void FT2_StopPlayback(void);
extern void FT2_MixAudio(int16_t *buffer, int32_t numSamples);
extern void FT2_MixAudioFloat(float *buffer, int32_t numSamples);
extern void FT2_StopMusic();
extern void FT2_FreeMusic(void);

extern const char *MODSig[16]; // For format checking

enum
{
	FORMAT_UNKNOWN = 0,
	FORMAT_IT_S3M = 1,
	FORMAT_XM_MOD = 2
};

int current_format = FORMAT_UNKNOWN;

int m4p_TestFromData(uint8_t *Data, uint32_t DataLen)
{
	if ((DataLen >= 4 && (Data[0] == 'I' && Data[1] == 'M' &&
		Data[2] == 'P' && Data[3] == 'M')) || (DataLen >= 48 &&
		(Data[44] == 'S' && Data[45] == 'C' &&
		Data[46] == 'R' && Data[47] == 'M')))
	{
		return FORMAT_IT_S3M;
	}
	if (DataLen >= 17)
	{
		bool is_xm_mod = true;
		const char *hdrtxt = "Extended Module:";
		for (int i = 0; i < 16; i++)
		{
			if (Data[i] != *hdrtxt++)
			{
				is_xm_mod = false;
				break;
			}
		}
		if (is_xm_mod) return FORMAT_XM_MOD;
	}
	if (DataLen >= 1084)
	{
		for (uint8_t i = 0; i < 16; i++)
		{
			if (Data[1080] == MODSig[i][0] && Data[1081] == MODSig[i][1] &&
				Data[1082] == MODSig[i][2] && Data[1083] == MODSig[i][3])
				return FORMAT_XM_MOD;
		}
	}

	return FORMAT_UNKNOWN;
}

bool m4p_LoadFromData(uint8_t *Data, uint32_t DataLen, int32_t mixingFrequency, int32_t mixingBufferSize)
{
	current_format = m4p_TestFromData(Data, DataLen);

	if (current_format == FORMAT_IT_S3M)
	{
		if (IT_InitMusic(mixingFrequency, mixingBufferSize))
			return IT_LoadMusicfromData(Data, DataLen);
		else
			return false;
	}
	else if (current_format == FORMAT_XM_MOD)
	{
		if (FT2_InitMusic(mixingFrequency, mixingBufferSize, true, true))
			return FT2_LoadMusicfromData(Data, DataLen);
		else
			return false;
	}

	return false;
}

void m4p_PlaySong(void)
{
	if (current_format == FORMAT_IT_S3M)
		IT_PlaySong(0);
	else
		FT2_StartPlayback();	
}

void m4p_GenerateSamples(int16_t *buffer, int32_t numSamples)
{
	if (current_format == FORMAT_IT_S3M)
		IT_MixAudio(buffer, numSamples);
	else
		FT2_MixAudio(buffer, numSamples);
}

void m4p_GenerateFloatSamples(float *buffer, int32_t numSamples)
{
	if (current_format == FORMAT_IT_S3M)
		IT_MixAudioFloat(buffer, numSamples);
	else
		FT2_MixAudioFloat(buffer, numSamples);
}

void m4p_Stop(void)
{
	if (current_format == FORMAT_IT_S3M)
		IT_StopPlayback();
	else
		FT2_StopPlayback();
}

void m4p_Close(void)
{
	if (current_format == FORMAT_IT_S3M)
		IT_CloseDriver();
	else
		FT2_StopMusic();
}

void m4p_FreeSong(void)
{
	if (current_format == FORMAT_IT_S3M)
		IT_FreeMusic();
	else
		FT2_FreeMusic();
}