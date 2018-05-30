## Deblock 

Simple deblocking filter by Manao and Fizick. It does a deblocking of the picture, using the deblocking filter of h264.

This version has been back-ported from VapourSynth with high-bit-depth support. All credits go to those who wrote the code.

### Usage
```
Deblock (clip, quant=25, aOffset=0, bOffset=0, planes="yuv")
```
* *quant* - the higher the quant, the stronger the deblocking. It can range from 0 to 60.
* *aOffset* - quant modifier to the blocking detector threshold. Setting it higher means than more edges will deblocked.
* *bOffset* - another quant modifier, for block detecting and for deblocking's strength. There again, the higher, the stronger.
* *planes* - specifies which planes to process between y, u and v.