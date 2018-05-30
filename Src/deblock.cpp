#include "deblock.h"

static constexpr int QUANT_MAX = 60; // generalized by Fizick (was max=51)

static constexpr int alphas[] = {
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 4, 4,
	5, 6, 7, 8, 9, 10,
	12, 13, 15, 17, 20,
	22, 25, 28, 32, 36,
	40, 45, 50, 56, 63,
	71, 80, 90, 101, 113,
	127, 144, 162, 182,
	203, 226, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255 // added by Fizick 
};

static constexpr int betas[] = {
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 2, 2,
	2, 3, 3, 3, 3, 4,
	4, 4, 6, 6,
	7, 7, 8, 8, 9, 9,
	10, 10, 11, 11, 12,
	12, 13, 13, 14, 14,
	15, 15, 16, 16, 17,
	17, 18, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27 // added by Fizick 
};

static constexpr int cs[] = {
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 1, 1,
	1, 1, 1, 1, 1, 1,
	1, 2, 2, 2, 2, 3,
	3, 3, 4, 4, 5, 5,
	6, 7, 8, 8, 10,
	11, 12, 13, 15, 17,
	19, 21, 23, 25, 27, 29, 31, 33, 35 // added by Fizick for really strong deblocking :)
};

template<typename T>
inline void Deblock::deblockHorEdge(T * VS_RESTRICT dstp, const unsigned stride) noexcept {
	const int alpha = _alpha;
	const int beta = _beta;
	const int c0 = _c0;
	const int c1 = _c1;

	T * VS_RESTRICT sq0 = dstp;
	T * VS_RESTRICT sq1 = dstp + stride;
	const T * sq2 = dstp + stride * 2;
	T * VS_RESTRICT sp0 = dstp - stride;
	T * VS_RESTRICT sp1 = dstp - stride * 2;
	const T * sp2 = dstp - stride * 3;

	for (unsigned i = 0; i < 4; i++) {
		if (std::abs(sp0[i] - sq0[i]) < alpha && std::abs(sp1[i] - sp0[i]) < beta && std::abs(sq0[i] - sq1[i]) < beta) {
			const int ap = std::abs(sp2[i] - sp0[i]);
			const int aq = std::abs(sq2[i] - sq0[i]);

			int c = c0;
			if (aq < beta)
				c += c1;
			if (ap < beta)
				c += c1;

			const int avg = (sp0[i] + sq0[i] + 1) >> 1;
			const int delta = min(max(((sq0[i] - sp0[i]) * 4 + sp1[i] - sq1[i] + 4) >> 3, -c), c);
			const int deltap1 = min(max((sp2[i] + avg - sp1[i] * 2) >> 1, -c0), c0);
			const int deltaq1 = min(max((sq2[i] + avg - sq1[i] * 2) >> 1, -c0), c0);

			sp0[i] = min(max(sp0[i] + delta, 0), _peak);
			sq0[i] = min(max(sq0[i] - delta, 0), _peak);
			if (ap < beta)
				sp1[i] += deltap1;
			if (aq < beta)
				sq1[i] += deltaq1;
		}
	}
}

template<>
inline void Deblock::deblockHorEdge(float * VS_RESTRICT dstp, const unsigned stride) noexcept {
	const float alpha = _alphaF;
	const float beta = _betaF;
	const float c0 = _c0F;
	const float c1 = _c1F;

	float * VS_RESTRICT sq0 = dstp;
	float * VS_RESTRICT sq1 = dstp + stride;
	const float * sq2 = dstp + stride * 2;
	float * VS_RESTRICT sp0 = dstp - stride;
	float * VS_RESTRICT sp1 = dstp - stride * 2;
	const float * sp2 = dstp - stride * 3;

	for (unsigned i = 0; i < 4; i++) {
		if (std::abs(sp0[i] - sq0[i]) < alpha && std::abs(sp1[i] - sp0[i]) < beta && std::abs(sq0[i] - sq1[i]) < beta) {
			const float ap = std::abs(sp2[i] - sp0[i]);
			const float aq = std::abs(sq2[i] - sq0[i]);

			float c = c0;
			if (aq < beta)
				c += c1;
			if (ap < beta)
				c += c1;

			const float avg = (sp0[i] + sq0[i]) / 2.f;
			const float delta = min(max(((sq0[i] - sp0[i]) * 4.f + sp1[i] - sq1[i]) / 8.f, -c), c);
			const float deltap1 = min(max((sp2[i] + avg - sp1[i] * 2.f) / 2.f, -c0), c0);
			const float deltaq1 = min(max((sq2[i] + avg - sq1[i] * 2.f) / 2.f, -c0), c0);

			sp0[i] += delta;
			sq0[i] -= delta;
			if (ap < beta)
				sp1[i] += deltap1;
			if (aq < beta)
				sq1[i] += deltaq1;
		}
	}
}

template<typename T>
inline void Deblock::deblockVerEdge(T * VS_RESTRICT dstp, const unsigned stride) noexcept {
	const int alpha = _alpha;
	const int beta = _beta;
	const int c0 = _c0;
	const int c1 = _c1;

	for (unsigned i = 0; i < 4; i++) {
		if (std::abs(dstp[0] - dstp[-1]) < alpha && std::abs(dstp[1] - dstp[0]) < beta && std::abs(dstp[-1] - dstp[-2]) < beta) {
			const int ap = std::abs(dstp[2] - dstp[0]);
			const int aq = std::abs(dstp[-3] - dstp[-1]);

			int c = c0;
			if (aq < beta)
				c += c1;
			if (ap < beta)
				c += c1;

			const int avg = (dstp[0] + dstp[-1] + 1) >> 1;
			const int delta = min(max(((dstp[0] - dstp[-1]) * 4 + dstp[-2] - dstp[1] + 4) >> 3, -c), c);
			const int deltaq1 = min(max((dstp[2] + avg - dstp[1] * 2) >> 1, -c0), c0);
			const int deltap1 = min(max((dstp[-3] + avg - dstp[-2] * 2) >> 1, -c0), c0);

			dstp[0] = min(max(dstp[0] - delta, 0), _peak);
			dstp[-1] = min(max(dstp[-1] + delta, 0), _peak);
			if (ap < beta)
				dstp[1] += deltaq1;
			if (aq < beta)
				dstp[-2] += deltap1;
		}

		dstp += stride;
	}
}

template<>
inline void Deblock::deblockVerEdge(float * VS_RESTRICT dstp, const unsigned stride) noexcept {
	const float alpha = _alphaF;
	const float beta = _betaF;
	const float c0 = _c0F;
	const float c1 = _c1F;

	for (unsigned i = 0; i < 4; i++) {
		if (std::abs(dstp[0] - dstp[-1]) < alpha && std::abs(dstp[1] - dstp[0]) < beta && std::abs(dstp[-1] - dstp[-2]) < beta) {
			const float ap = std::abs(dstp[2] - dstp[0]);
			const float aq = std::abs(dstp[-3] - dstp[-1]);

			float c = c0;
			if (aq < beta)
				c += c1;
			if (ap < beta)
				c += c1;

			const float avg = (dstp[0] + dstp[-1]) / 2.f;
			const float delta = min(max(((dstp[0] - dstp[-1]) * 4.f + dstp[-2] - dstp[1]) / 8.f, -c), c);
			const float deltaq1 = min(max((dstp[2] + avg - dstp[1] * 2.f) / 2.f, -c0), c0);
			const float deltap1 = min(max((dstp[-3] + avg - dstp[-2] * 2.f) / 2.f, -c0), c0);

			dstp[0] -= delta;
			dstp[-1] += delta;
			if (ap < beta)
				dstp[1] += deltaq1;
			if (aq < beta)
				dstp[-2] += deltap1;
		}

		dstp += stride;
	}
}

template<typename T>
void Deblock::Process(PVideoFrame &dst, IScriptEnvironment *env) noexcept {
	if (_process[0])
		Process<T>(dst, PLANAR_Y, env);
	if (!vi.IsY()) {
		if (_process[1])
			Process<T>(dst, PLANAR_U, env);
		if (_process[2])
			Process<T>(dst, PLANAR_V, env);
	}
}

template<typename T>
void Deblock::Process(PVideoFrame &dst, int plane, IScriptEnvironment *env) noexcept {
	const unsigned width = dst->GetRowSize(plane) / sizeof(T);
	const unsigned height = dst->GetHeight(plane);
	const unsigned stride = dst->GetPitch(plane) / sizeof(T);
	T * VS_RESTRICT dstp = reinterpret_cast<T *>(dst->GetWritePtr(plane));

	for (unsigned x = 4; x < width; x += 4)
		deblockVerEdge(dstp + x, stride);

	dstp += stride * 4;

	for (unsigned y = 4; y < height; y += 4) {
		deblockHorEdge(dstp, stride);

		for (unsigned x = 4; x < width; x += 4) {
			deblockHorEdge(dstp + x, stride);
			deblockVerEdge(dstp + x, stride);
		}

		dstp += stride * 4;
	}
}


Deblock::Deblock(PClip child, int quant, int aOffset, int bOffset, const char* planes, IScriptEnvironment* env)
	: GenericVideoFilter(child) {
	if (quant < 0 || quant > QUANT_MAX)
		env->ThrowError("Deblock: quant must be between 0 and %i", QUANT_MAX);
	if (!vi.IsPlanar())
		env->ThrowError("Deblock: only planar input is supported");
	if ((vi.width % 8 != 0) || (vi.height % 8 != 0))
		env->ThrowError("Deblock: input clip width and height must be mod 8");

	int m = planes != '\0' ? strlen(planes) : 0;
	for (int i = 0; i < 3; i++)
		_process[i] = m <= 0;

	for (int i = 0; i < m; i++) {
		const char n = planes[i];
		int ni = -1;
		if (n == 'Y' || n == 'y')
			ni = 0;
		else if (n == 'U' || n == 'u')
			ni = 1;
		else if (n == 'V' || n == 'v')
			ni = 2;

		if (ni < 0 || ni >= (vi.IsPlanar() ? 3 : 1))
			env->ThrowError("Deblock: plane index out of range");

		if (_process[ni])
			env->ThrowError("Deblock: plane specified twice");

		_process[ni] = true;
	}

	aOffset = min(max(aOffset, -quant), QUANT_MAX - quant);
	bOffset = min(max(bOffset, -quant), QUANT_MAX - quant);
	const int aIndex = min(max(quant + aOffset, 0), QUANT_MAX);
	const int bIndex = min(max(quant + bOffset, 0), QUANT_MAX);
	_alpha = alphas[aIndex];
	_beta = betas[bIndex];
	_c0 = cs[aIndex];

	if (vi.BitsPerComponent() <= 16) {
		_peak = (1 << vi.BitsPerComponent()) - 1;
		_alpha = _alpha * _peak / 255;
		_beta = _beta * _peak / 255;
		_c0 = _c0 * _peak / 255;
		_c1 = 1 * _peak / 255;
	}
	else {
		_alphaF = _alpha / 255.f;
		_betaF = _beta / 255.f;
		_c0F = _c0 / 255.f;
		_c1F = 1.f / 255.f;
	}
}

PVideoFrame Deblock::GetFrame(int n, IScriptEnvironment *env) {
	PVideoFrame src = child->GetFrame(n, env);
	env->MakeWritable(&src);

	if (vi.BitsPerComponent() <= 8)
		Process<uint8_t>(src, env);
	else if (vi.BitsPerComponent() <= 16)
		Process<uint16_t>(src, env);
	else
		Process<float>(src, env);

	return src;
}

// Marks filter as multi-threading friendly.
int __stdcall Deblock::SetCacheHints(int cachehints, int frame_range) {
	return cachehints == CachePolicyHint::CACHE_GET_MTMODE ? MT_NICE_FILTER : 0;
}
