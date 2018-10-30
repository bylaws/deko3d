#include "dk_device.h"

namespace
{
	void defaultErrorFunc(void* userdata, const char* context, DkResult result)
	{
		fatalSimple(MAKERESULT(359, result));
	}

	DkResult defaultAllocFunc(void* userdata, size_t alignment, size_t size, void** out)
	{
		size = (size + alignment - 1) &~ (alignment - 1);
		*out = aligned_alloc(alignment, size);
		return *out ? DkResult_Success : DkResult_OutOfMemory;
	}

	void defaultFreeFunc(void* userdata, void* mem)
	{
		free(mem);
	}
}

DkDevice dkDeviceCreate(DkDeviceMaker const* maker)
{
	DkDeviceMaker m = *maker;
	if (!m.cbError) m.cbError = defaultErrorFunc;
	if (!m.cbAlloc) m.cbAlloc = defaultAllocFunc;
	if (!m.cbFree)  m.cbFree  = defaultFreeFunc;

	return new(m) tag_DkDevice(m);
}

void dkDeviceDestroy(DkDevice obj)
{
	delete obj;
}
