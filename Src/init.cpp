#define WIN32_LEAN_AND_MEAN
#include "avisynth.h"
#include "deblock.h"

AVSValue __cdecl Create_Deblock(AVSValue args, void*, IScriptEnvironment* env) {
	enum { CLIP, QUANT, AOFFSET, BOFFSET, _PLANES };
	PClip input = args[CLIP].AsClip();
	VideoInfo vi = input->GetVideoInfo();

	const int padWidth = (vi.width & 7) ? 8 - vi.width % 8 : 0;
	const int padHeight = (vi.height & 7) ? 8 - vi.height % 8 : 0;

	if (padWidth || padHeight) {
		AVSValue sargs[5] = { input, vi.width + padWidth, vi.height + padHeight, vi.width + padWidth, vi.height + padHeight };
		const char *nargs[5] = { 0, 0, 0, "src_width", "src_height" };
		input = env->Invoke("PointResize", AVSValue(sargs, 5), nargs).AsClip();
	}

	return new Deblock(input, args[QUANT].AsInt(25), args[AOFFSET].AsInt(0), args[BOFFSET].AsBool(0), args[_PLANES].AsString(), env);
}

const AVS_Linkage *AVS_linkage = nullptr;

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors) {
	AVS_linkage = vectors;
	env->AddFunction("Deblock", "c[quant]i[aOffset]i[bOffset]i[planes]s", Create_Deblock, 0);
	return "Blocks are kawaii uguu~!";
}
