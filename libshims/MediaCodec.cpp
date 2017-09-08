#include <media/MediaCodecBuffer.h>
#include <media/stagefright/foundation/AHandler.h>
#include <utils/Vector.h>

namespace android {

extern "C" void _ZN7android10MediaCodec12CreateByTypeERKNS_2spINS_7ALooperEEERKNS_7AStringEbPiij(
	const sp<ALooper> &looper, const AString &mime, bool encoder, status_t *err, pid_t pid, uid_t uid);

extern "C" void _ZNK7android10MediaCodec15getInputBuffersEPNS_6VectorINS_2spINS_16MediaCodecBufferEEEEE(Vector<sp<ABuffer> > *buffers);
extern "C" void _ZNK7android10MediaCodec16getOutputBuffersEPNS_6VectorINS_2spINS_16MediaCodecBufferEEEEE(Vector<sp<ABuffer> > *buffers);

extern "C" void _ZN7android10MediaCodec12CreateByTypeERKNS_2spINS_7ALooperEEERKNS_7AStringEbPii(
	const sp<ALooper> &looper, const AString &mime, bool encoder, status_t *err, pid_t pid) {
	_ZN7android10MediaCodec12CreateByTypeERKNS_2spINS_7ALooperEEERKNS_7AStringEbPiij(
		looper, mime, encoder, err, pid, 0);
}

extern "C" void _ZNK7android10MediaCodec15getInputBuffersEPNS_6VectorINS_2spINS_7ABufferEEEEE(Vector<sp<ABuffer> > *buffers) {
	_ZNK7android10MediaCodec15getInputBuffersEPNS_6VectorINS_2spINS_16MediaCodecBufferEEEEE(buffers);
}

extern "C" void _ZNK7android10MediaCodec16getOutputBuffersEPNS_6VectorINS_2spINS_7ABufferEEEEE(Vector<sp<ABuffer> > *buffers) {
	_ZNK7android10MediaCodec16getOutputBuffersEPNS_6VectorINS_2spINS_16MediaCodecBufferEEEEE(buffers);
}
}
