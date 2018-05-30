#pragma once
#define NOMINMAX
#include "avisynth.h"
#include "avs\minmax.h"
#include <Windows.h>
#include <stdint.h>
#include <algorithm>

#define VS_RESTRICT __restrict

class Deblock : public GenericVideoFilter {
public:
	Deblock(PClip child, int quant, int a_offset, int b_offset, const char* planes, IScriptEnvironment* env);
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env);
	int __stdcall SetCacheHints(int cachehints, int frame_range);

private:
	template<typename T>
	void deblockHorEdge(T * VS_RESTRICT dstp, const unsigned stride) noexcept;
	template<>
	inline void deblockHorEdge(float * VS_RESTRICT dstp, const unsigned stride) noexcept;
	template<typename T>
	inline void deblockVerEdge(T * VS_RESTRICT dstp, const unsigned stride) noexcept;
	template<>
	inline void deblockVerEdge(float * VS_RESTRICT dstp, const unsigned stride) noexcept;
	template<typename T>
	void Process(PVideoFrame &dst, IScriptEnvironment *env) noexcept;
	template<typename T>
	void Process(PVideoFrame &dst, int plane, IScriptEnvironment *env) noexcept;

	bool _process[3];
	int _alpha, _beta, _c0, _c1;
	float _alphaF, _betaF, _c0F, _c1F;
	int _peak;
};