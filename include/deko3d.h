// Temporary header file which will be replaced once iface generation works

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>

#if __cplusplus >= 201402L
#define DK_CONSTEXPR constexpr
#else
#define DK_CONSTEXPR static inline
#endif

#define DK_DECL_HANDLE(_typename) \
	typedef struct tag_##_typename *_typename

#ifndef DK_NO_OPAQUE_DUMMY
#define DK_DECL_OPAQUE(_typename, _align, _size) \
	typedef struct _typename \
	{ \
		alignas(_align) uint8_t _storage[_size]; \
	} _typename
#else
#define DK_DECL_OPAQUE(_typename, _align, _size) \
	struct _typename; /* forward declaration */ \
	constexpr unsigned _align_##_typename = _align; \
	constexpr unsigned _size_##_typename = _size
#endif

DK_DECL_HANDLE(DkDevice);
DK_DECL_HANDLE(DkMemBlock);
DK_DECL_OPAQUE(DkFence, 8, 40);
DK_DECL_HANDLE(DkCmdBuf);
DK_DECL_HANDLE(DkQueue);
DK_DECL_OPAQUE(DkShader, 8, 96);
DK_DECL_OPAQUE(DkImageLayout, 8, 128); // size todo
DK_DECL_OPAQUE(DkImage, 8, 128); // size todo
DK_DECL_OPAQUE(DkImageDescriptor, 4, 32);
DK_DECL_OPAQUE(DkSamplerDescriptor, 4, 32);
DK_DECL_HANDLE(DkSwapchain);

typedef enum
{
	DkResult_Success,
	DkResult_Fail,
	DkResult_Timeout,
	DkResult_OutOfMemory,
	DkResult_NotImplemented,
	DkResult_MisalignedSize,
	DkResult_MisalignedData,
	DkResult_BadInput,
	DkResult_BadMemFlags,
	DkResult_BadState,
} DkResult;

#define DK_GPU_ADDR_INVALID (~0ULL)

typedef uint64_t DkGpuAddr;
typedef uintptr_t DkCmdList;
typedef uint32_t DkResHandle;
typedef void (*DkErrorFunc)(void* userData, const char* context, DkResult result);
typedef DkResult (*DkAllocFunc)(void* userData, size_t alignment, size_t size, void** out);
typedef void (*DkFreeFunc)(void* userData, void* mem);
typedef void (*DkCmdBufAddMemFunc)(void* userData, DkCmdBuf cmdbuf, size_t minReqSize);

enum
{
	DkDeviceFlags_DepthZeroToOne     = 0U << 8,
	DkDeviceFlags_DepthMinusOneToOne = 1U << 8,
	DkDeviceFlags_OriginUpperLeft    = 0U << 9,
	DkDeviceFlags_OriginLowerLeft    = 1U << 9,
};

typedef struct DkDeviceMaker
{
	void* userData;
	DkErrorFunc cbError;
	DkAllocFunc cbAlloc;
	DkFreeFunc cbFree;
	uint32_t flags;
} DkDeviceMaker;

DK_CONSTEXPR void dkDeviceMakerDefaults(DkDeviceMaker* maker)
{
	maker->userData = NULL;
	maker->cbError = NULL;
	maker->cbAlloc = NULL;
	maker->cbFree = NULL;
	maker->flags = DkDeviceFlags_DepthZeroToOne | DkDeviceFlags_OriginUpperLeft;
}

#define DK_MEMBLOCK_ALIGNMENT 0x1000
#define DK_CMDMEM_ALIGNMENT 4
#define DK_QUEUE_MIN_CMDMEM_SIZE 0x10000
#define DK_PER_WARP_SCRATCH_MEM_ALIGNMENT 0x200
#define DK_NUM_UNIFORM_BUFS 16
#define DK_NUM_STORAGE_BUFS 16
#define DK_NUM_TEXTURE_BINDINGS 32
#define DK_NUM_IMAGE_BINDINGS 8
#define DK_UNIFORM_BUF_ALIGNMENT 0x100
#define DK_UNIFORM_BUF_MAX_SIZE 0x10000
#define DK_DEFAULT_MAX_COMPUTE_CONCURRENT_JOBS 128
#define DK_SHADER_CODE_ALIGNMENT 0x100
#define DK_IMAGE_DESCRIPTOR_ALIGNMENT 0x20
#define DK_SAMPLER_DESCRIPTOR_ALIGNMENT 0x20
#define DK_MAX_RENDER_TARGETS 8
#define DK_NUM_VIEWPORTS 16
#define DK_NUM_SCISSORS 16
#define DK_MAX_VERTEX_ATTRIBS 32
#define DK_MAX_VERTEX_BUFFERS 16
#define DK_IMAGE_LINEAR_STRIDE_ALIGNMENT 32

enum
{
	DkMemAccess_None = 0U,
	DkMemAccess_Uncached = 1U,
	DkMemAccess_Cached = 2U,
	DkMemAccess_Mask = 3U,
};

enum
{
	DkMemBlockFlags_CpuAccessShift = 0U,
	DkMemBlockFlags_GpuAccessShift = 2U,
	DkMemBlockFlags_CpuUncached    = DkMemAccess_Uncached << DkMemBlockFlags_CpuAccessShift,
	DkMemBlockFlags_CpuCached      = DkMemAccess_Cached   << DkMemBlockFlags_CpuAccessShift,
	DkMemBlockFlags_CpuAccessMask  = DkMemAccess_Mask     << DkMemBlockFlags_CpuAccessShift,
	DkMemBlockFlags_GpuUncached    = DkMemAccess_Uncached << DkMemBlockFlags_GpuAccessShift,
	DkMemBlockFlags_GpuCached      = DkMemAccess_Cached   << DkMemBlockFlags_GpuAccessShift,
	DkMemBlockFlags_GpuAccessMask  = DkMemAccess_Mask     << DkMemBlockFlags_GpuAccessShift,
	DkMemBlockFlags_Code           = 1U << 4,
	DkMemBlockFlags_Image          = 1U << 5,
	DkMemBlockFlags_ZeroFillInit   = 1U << 8,
};

typedef struct DkMemBlockMaker
{
	DkDevice device;
	uint32_t size;
	uint32_t flags;
	void* storage;
} DkMemBlockMaker;

DK_CONSTEXPR void dkMemBlockMakerDefaults(DkMemBlockMaker* maker, DkDevice device, uint32_t size)
{
	maker->device = device;
	maker->size = size;
	maker->flags = DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached;
	maker->storage = NULL;
}

typedef struct DkCmdBufMaker
{
	DkDevice device;
	void* userData;
	DkCmdBufAddMemFunc cbAddMem;
} DkCmdBufMaker;

DK_CONSTEXPR void dkCmdBufMakerDefaults(DkCmdBufMaker* maker, DkDevice device)
{
	maker->device = device;
	maker->userData = NULL;
	maker->cbAddMem = NULL;
}

enum
{
	DkQueueFlags_Graphics     = 1U << 0,
	DkQueueFlags_Compute      = 1U << 1,
	DkQueueFlags_Transfer     = 1U << 2,
	DkQueueFlags_EnableZcull  = 0U << 4,
	DkQueueFlags_DisableZcull = 1U << 4,
};

typedef struct DkQueueMaker
{
	DkDevice device;
	uint32_t flags;
	uint32_t commandMemorySize;
	uint32_t flushThreshold;
	uint32_t perWarpScratchMemorySize;
	uint32_t maxConcurrentComputeJobs;
} DkQueueMaker;

DK_CONSTEXPR void dkQueueMakerDefaults(DkQueueMaker* maker, DkDevice device)
{
	maker->device = device;
	maker->flags =
		DkQueueFlags_Graphics | DkQueueFlags_Compute | DkQueueFlags_Transfer |
		DkQueueFlags_EnableZcull;
	maker->commandMemorySize = DK_QUEUE_MIN_CMDMEM_SIZE;
	maker->flushThreshold = DK_QUEUE_MIN_CMDMEM_SIZE/8;
	maker->perWarpScratchMemorySize = 4*DK_PER_WARP_SCRATCH_MEM_ALIGNMENT;
	maker->maxConcurrentComputeJobs = DK_DEFAULT_MAX_COMPUTE_CONCURRENT_JOBS;
}

typedef struct DkShaderMaker
{
	DkMemBlock codeMem;
	const void* control;
	uint32_t codeOffset;
	uint32_t programId;
} DkShaderMaker;

DK_CONSTEXPR void dkShaderMakerDefaults(DkShaderMaker* maker, DkMemBlock codeMem, uint32_t codeOffset)
{
	maker->codeMem = codeMem;
	maker->control = NULL;
	maker->codeOffset = codeOffset;
	maker->programId = 0;
}

typedef enum DkStage
{
	DkStage_Vertex   = 0,
	DkStage_TessCtrl = 1,
	DkStage_TessEval = 2,
	DkStage_Geometry = 3,
	DkStage_Fragment = 4,
	DkStage_Compute  = 5,

	DkStage_MaxGraphics = 5,
} DkStage;

enum
{
	DkStageFlag_Vertex   = 1U << DkStage_Vertex,
	DkStageFlag_TessCtrl = 1U << DkStage_TessCtrl,
	DkStageFlag_TessEval = 1U << DkStage_TessEval,
	DkStageFlag_Geometry = 1U << DkStage_Geometry,
	DkStageFlag_Fragment = 1U << DkStage_Fragment,
	DkStageFlag_Compute  = 1U << DkStage_Compute,

	DkStageFlag_GraphicsMask = (1U << DkStage_MaxGraphics) - 1,
};

typedef enum DkBarrier
{
	DkBarrier_None       = 0, // No ordering is performed
	DkBarrier_Tiles      = 1, // Orders the processing of tiles (similar to Vulkan subpasses)
	DkBarrier_Fragments  = 2, // Orders the processing of fragments (similar to Vulkan renderpasses)
	DkBarrier_Primitives = 3, // Completes the processing of all previous primitives and compute jobs
	DkBarrier_Full       = 4, // Completes the processing of all previous commands while disabling command list prefetch
} DkBarrier;

enum
{
	DkInvalidateFlags_Image   = 1U << 0, // Invalidates the image (texture) cache
	DkInvalidateFlags_Code    = 1U << 1, // Invalidates the shader code/data/uniform cache
	DkInvalidateFlags_Pool    = 1U << 2, // Invalidates the image/sampler pool (descriptor) cache
	DkInvalidateFlags_Zcull   = 1U << 3, // Invalidates Zcull state
	DkInvalidateFlags_L2Cache = 1U << 4, // Invalidates the L2 cache
};

typedef enum DkImageType
{
	DkImageType_None         = 0,
	DkImageType_1D           = 1,
	DkImageType_2D           = 2,
	DkImageType_3D           = 3,
	DkImageType_1DArray      = 4,
	DkImageType_2DArray      = 5,
	DkImageType_2DMS         = 6,
	DkImageType_2DMSArray    = 7,
	DkImageType_Rectangle    = 8,
	DkImageType_Cubemap      = 9,
	DkImageType_CubemapArray = 10,
	DkImageType_Buffer       = 11,
} DkImageType;

enum
{
	DkImageFlags_BlockLinear    = 0U << 0, // Image is stored in Nvidia block linear format (default).
	DkImageFlags_PitchLinear    = 1U << 0, // Image is stored in standard pitch linear format.
	DkImageFlags_CustomTileSize = 1U << 1, // Use a custom tile size for block linear images.
	DkImageFlags_HwCompression  = 1U << 2, // Specifies that hardware compression is allowed to be enabled.
	DkImageFlags_D16EnableZbc   = 1U << 3, // For DkImageFormat_Z16 images, specifies that zero-bandwidth clear is preferred as the hardware compression format.

	DkImageFlags_UsageRender    = 1U << 8,  // Specifies that the image will be used as a render target.
	DkImageFlags_UsageLoadStore = 1U << 9,  // Specifies that the image will be used with shader image load/store commands.
	DkImageFlags_UsagePresent   = 1U << 10, // Specifies that the image will be presented to a DkWindow.
	DkImageFlags_Usage2DEngine  = 1U << 11, // Specifies that the image will be used with the 2D Engine (e.g. for transfers between images)
	DkImageFlags_UsageVideo     = 1U << 12, // Specifies that the image will be used with hardware video encoding/decoding engines
};

typedef enum DkImageFormat
{
	DkImageFormat_None,
	DkImageFormat_R8_Unorm,
	DkImageFormat_R8_Snorm,
	DkImageFormat_R8_Uint,
	DkImageFormat_R8_Sint,
	DkImageFormat_R16_Float,
	DkImageFormat_R16_Unorm,
	DkImageFormat_R16_Snorm,
	DkImageFormat_R16_Uint,
	DkImageFormat_R16_Sint,
	DkImageFormat_R32_Float,
	DkImageFormat_R32_Uint,
	DkImageFormat_R32_Sint,
	DkImageFormat_RG8_Unorm,
	DkImageFormat_RG8_Snorm,
	DkImageFormat_RG8_Uint,
	DkImageFormat_RG8_Sint,
	DkImageFormat_RG16_Float,
	DkImageFormat_RG16_Unorm,
	DkImageFormat_RG16_Snorm,
	DkImageFormat_RG16_Uint,
	DkImageFormat_RG16_Sint,
	DkImageFormat_RG32_Float,
	DkImageFormat_RG32_Uint,
	DkImageFormat_RG32_Sint,
	DkImageFormat_RGB32_Float,
	DkImageFormat_RGB32_Uint,
	DkImageFormat_RGB32_Sint,
	DkImageFormat_RGBA8_Unorm,
	DkImageFormat_RGBA8_Snorm,
	DkImageFormat_RGBA8_Uint,
	DkImageFormat_RGBA8_Sint,
	DkImageFormat_RGBA16_Float,
	DkImageFormat_RGBA16_Unorm,
	DkImageFormat_RGBA16_Snorm,
	DkImageFormat_RGBA16_Uint,
	DkImageFormat_RGBA16_Sint,
	DkImageFormat_RGBA32_Float,
	DkImageFormat_RGBA32_Uint,
	DkImageFormat_RGBA32_Sint,
	DkImageFormat_S8,
	DkImageFormat_Z16,
	DkImageFormat_Z24X8,
	DkImageFormat_ZF32,
	DkImageFormat_Z24S8,
	DkImageFormat_ZF32_X24S8,
	DkImageFormat_RGBX8_Unorm_sRGB,
	DkImageFormat_RGBA8_Unorm_sRGB,
	DkImageFormat_RGBA4_Unorm,
	DkImageFormat_RGB5_Unorm,
	DkImageFormat_RGB5A1_Unorm,
	DkImageFormat_RGB565_Unorm,
	DkImageFormat_RGB10A2_Unorm,
	DkImageFormat_RGB10A2_Uint,
	DkImageFormat_RG11B10_Float,
	DkImageFormat_E5BGR9_Float,
	DkImageFormat_RGB_DXT1,
	DkImageFormat_RGBA_DXT1,
	DkImageFormat_RGBA_DXT23,
	DkImageFormat_RGBA_DXT45,
	DkImageFormat_RGB_DXT1_sRGB,
	DkImageFormat_RGBA_DXT1_sRGB,
	DkImageFormat_RGBA_DXT23_sRGB,
	DkImageFormat_RGBA_DXT45_sRGB,
	DkImageFormat_R_DXN1_Unorm,
	DkImageFormat_R_DXN1_Snorm,
	DkImageFormat_RG_DXN2_Unorm,
	DkImageFormat_RG_DXN2_Snorm,
	DkImageFormat_RGBA_BC7U_Unorm,
	DkImageFormat_RGBA_BC7U_Unorm_sRGB,
	DkImageFormat_RGBA_BC6H_SF16_Float,
	DkImageFormat_RGBA_BC6H_UF16_Float,
	DkImageFormat_RGBX8_Unorm,
	DkImageFormat_RGBX8_Snorm,
	DkImageFormat_RGBX8_Uint,
	DkImageFormat_RGBX8_Sint,
	DkImageFormat_RGBX16_Float,
	DkImageFormat_RGBX16_Unorm,
	DkImageFormat_RGBX16_Snorm,
	DkImageFormat_RGBX16_Uint,
	DkImageFormat_RGBX16_Sint,
	DkImageFormat_RGBX32_Float,
	DkImageFormat_RGBX32_Uint,
	DkImageFormat_RGBX32_Sint,
	DkImageFormat_RGBA_ASTC_4x4,
	DkImageFormat_RGBA_ASTC_5x4,
	DkImageFormat_RGBA_ASTC_5x5,
	DkImageFormat_RGBA_ASTC_6x5,
	DkImageFormat_RGBA_ASTC_6x6,
	DkImageFormat_RGBA_ASTC_8x5,
	DkImageFormat_RGBA_ASTC_8x6,
	DkImageFormat_RGBA_ASTC_8x8,
	DkImageFormat_RGBA_ASTC_10x5,
	DkImageFormat_RGBA_ASTC_10x6,
	DkImageFormat_RGBA_ASTC_10x8,
	DkImageFormat_RGBA_ASTC_10x10,
	DkImageFormat_RGBA_ASTC_12x10,
	DkImageFormat_RGBA_ASTC_12x12,
	DkImageFormat_RGBA_ASTC_4x4_sRGB,
	DkImageFormat_RGBA_ASTC_5x4_sRGB,
	DkImageFormat_RGBA_ASTC_5x5_sRGB,
	DkImageFormat_RGBA_ASTC_6x5_sRGB,
	DkImageFormat_RGBA_ASTC_6x6_sRGB,
	DkImageFormat_RGBA_ASTC_8x5_sRGB,
	DkImageFormat_RGBA_ASTC_8x6_sRGB,
	DkImageFormat_RGBA_ASTC_8x8_sRGB,
	DkImageFormat_RGBA_ASTC_10x5_sRGB,
	DkImageFormat_RGBA_ASTC_10x6_sRGB,
	DkImageFormat_RGBA_ASTC_10x8_sRGB,
	DkImageFormat_RGBA_ASTC_10x10_sRGB,
	DkImageFormat_RGBA_ASTC_12x10_sRGB,
	DkImageFormat_RGBA_ASTC_12x12_sRGB,
	DkImageFormat_BGR565_Unorm,
	DkImageFormat_BGR5_Unorm,
	DkImageFormat_BGR5A1_Unorm,
	DkImageFormat_A5BGR5_Unorm,
	DkImageFormat_BGRX8_Unorm,
	DkImageFormat_BGRA8_Unorm,
	DkImageFormat_BGRX8_Unorm_sRGB,
	DkImageFormat_BGRA8_Unorm_sRGB,

	DkImageFormat_Count,
} DkImageFormat;

typedef enum DkSwizzle
{
	DkSwizzle_Zero  = 0,
	DkSwizzle_One   = 1,
	DkSwizzle_Red   = 2,
	DkSwizzle_Green = 3,
	DkSwizzle_Blue  = 4,
	DkSwizzle_Alpha = 5,
} DkSwizzle;

typedef enum DkMsMode
{
	DkMsMode_1x = 0,
	DkMsMode_2x = 1,
	DkMsMode_4x = 2,
	DkMsMode_8x = 3,
} DkMsMode;

typedef enum DkDsSource
{
	DkDsSource_Depth   = 0,
	DkDsSource_Stencil = 1,
} DkDsSource;

typedef enum DkTileSize
{
	DkTileSize_OneGob        = 0,
	DkTileSize_TwoGobs       = 1,
	DkTileSize_FourGobs      = 2,
	DkTileSize_EightGobs     = 3,
	DkTileSize_SixteenGobs   = 4,
	DkTileSize_ThirtyTwoGobs = 5,
} DkTileSize;

typedef struct DkImageLayoutMaker
{
	DkDevice device;
	DkImageType type;
	uint32_t flags;
	DkImageFormat format;
	DkMsMode msMode;
	uint32_t dimensions[3];
	uint32_t mipLevels;
	union
	{
		uint32_t pitchStride;
		DkTileSize tileSize;
	};
} DkImageLayoutMaker;

DK_CONSTEXPR void dkImageLayoutMakerDefaults(DkImageLayoutMaker* maker, DkDevice device)
{
	maker->device = device;
	maker->type = DkImageType_2D;
	maker->flags = 0;
	maker->format = DkImageFormat_None;
	maker->msMode = DkMsMode_1x;
	maker->dimensions[0] = 0;
	maker->dimensions[1] = 0;
	maker->dimensions[2] = 0;
	maker->mipLevels = 1;
	maker->pitchStride = 0;
}

typedef struct DkImageView
{
	DkImage const* pImage;
	DkImageType type;
	DkImageFormat format;
	DkSwizzle swizzle[4];
	DkDsSource dsSource;
	uint16_t layerOffset;
	uint16_t layerCount;
	uint8_t mipLevelOffset;
	uint8_t mipLevelCount;
} DkImageView;

DK_CONSTEXPR void dkImageViewDefaults(DkImageView* obj, DkImage const* pImage)
{
	obj->pImage = pImage;
	obj->type = DkImageType_None; // no override
	obj->format = DkImageFormat_None; // no override
	obj->swizzle[0] = DkSwizzle_Red;
	obj->swizzle[1] = DkSwizzle_Green;
	obj->swizzle[2] = DkSwizzle_Blue;
	obj->swizzle[3] = DkSwizzle_Alpha;
	obj->dsSource = DkDsSource_Depth;
	obj->layerOffset = 0;
	obj->layerCount = 0; // no override
	obj->mipLevelOffset = 0;
	obj->mipLevelCount = 0; // no override
}

typedef enum DkFilter
{
	DkFilter_Nearest = 1,
	DkFilter_Linear  = 2,
} DkFilter;

typedef enum DkMipFilter
{
	DkMipFilter_None    = 1,
	DkMipFilter_Nearest = 2,
	DkMipFilter_Linear  = 3,
} DkMipFilter;

typedef enum DkWrapMode
{
	DkWrapMode_Repeat              = 0,
	DkWrapMode_MirroredRepeat      = 1,
	DkWrapMode_ClampToEdge         = 2,
	DkWrapMode_ClampToBorder       = 3,
	DkWrapMode_Clamp               = 4,
	DkWrapMode_MirrorClampToEdge   = 5,
	DkWrapMode_MirrorClampToBorder = 6,
	DkWrapMode_MirrorClamp         = 7,
} DkWrapMode;

typedef enum DkCompareOp
{
	DkCompareOp_Never    = 1,
	DkCompareOp_Less     = 2,
	DkCompareOp_Equal    = 3,
	DkCompareOp_Lequal   = 4,
	DkCompareOp_Greater  = 5,
	DkCompareOp_NotEqual = 6,
	DkCompareOp_Gequal   = 7,
	DkCompareOp_Always   = 8,
} DkCompareOp;

typedef enum DkSamplerReduction
{
	DkSamplerReduction_WeightedAverage = 0,
	DkSamplerReduction_Min = 1,
	DkSamplerReduction_Max = 2,
} DkSamplerReduction;

typedef struct DkSampler
{
	DkFilter minFilter;
	DkFilter magFilter;
	DkMipFilter mipFilter;
	DkWrapMode wrapMode[3];
	float lodClampMin;
	float lodClampMax;
	float lodBias;
	float lodSnap;
	bool compareEnable;
	DkCompareOp compareOp;
	union
	{
		float value_f;
		uint32_t value_ui;
		int32_t value_i;
	} borderColor[4];
	float maxAnisotropy;
	DkSamplerReduction reductionMode;
} DkSampler;

DK_CONSTEXPR void dkSamplerDefaults(DkSampler* obj)
{
	obj->minFilter = DkFilter_Nearest;
	obj->magFilter = DkFilter_Nearest;
	obj->mipFilter = DkMipFilter_None;
	obj->wrapMode[0] = DkWrapMode_Repeat;
	obj->wrapMode[1] = DkWrapMode_Repeat;
	obj->wrapMode[2] = DkWrapMode_Repeat;
	obj->lodClampMin = 0.0f;
	obj->lodClampMax = 1000.0f;
	obj->lodBias = 0.0f;
	obj->lodSnap = 0.0f;
	obj->compareEnable = false;
	obj->compareOp = DkCompareOp_Less;
	obj->borderColor[0].value_ui = 0;
	obj->borderColor[1].value_ui = 0;
	obj->borderColor[2].value_ui = 0;
	obj->borderColor[3].value_ui = 0;
	obj->maxAnisotropy = 1.0f;
	obj->reductionMode = DkSamplerReduction_WeightedAverage;
}

typedef struct DkBufExtents
{
	DkGpuAddr addr;
	uint32_t size;
} DkBufExtents;

DK_CONSTEXPR DkResHandle dkMakeImageHandle(uint32_t id)
{
	return id & ((1U << 20) - 1);
}

DK_CONSTEXPR DkResHandle dkMakeSamplerHandle(uint32_t id)
{
	return id << 20;
}

DK_CONSTEXPR DkResHandle dkMakeTextureHandle(uint32_t imageId, uint32_t samplerId)
{
	return dkMakeImageHandle(imageId) | dkMakeSamplerHandle(samplerId);
}

typedef struct DkViewport
{
	float x;
	float y;
	float width;
	float height;
	float near;
	float far;
} DkViewport;

typedef struct DkScissor
{
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
} DkScissor;

typedef enum DkPolygonMode
{
	DkPolygonMode_Point = 0,
	DkPolygonMode_Line  = 1,
	DkPolygonMode_Fill  = 2,
} DkPolygonMode;

typedef enum DkFace
{
	DkFace_None         = 0,
	DkFace_Front        = 1,
	DkFace_Back         = 2,
	DkFace_FrontAndBack = 3,
} DkFace;

typedef enum DkFrontFace
{
	DkFrontFace_CW  = 0,
	DkFrontFace_CCW = 1,
} DkFrontFace;

typedef struct DkRasterizerState
{
	uint32_t depthClampEnable : 1;
	uint32_t rasterizerDiscardEnable : 1;
	DkPolygonMode polygonMode : 2;
	DkFace cullMode : 2;
	DkFrontFace frontFace : 1;
	uint32_t depthBiasEnable : 1;
	float depthBiasConstantFactor;
	float depthBiasClamp;
	float depthBiasSlopeFactor;
	float lineWidth;
} DkRasterizerState;

DK_CONSTEXPR void dkRasterizerStateDefaults(DkRasterizerState* state)
{
	state->depthClampEnable = 0;
	state->rasterizerDiscardEnable = 0;
	state->polygonMode = DkPolygonMode_Fill;
	state->cullMode = DkFace_Back;
	state->frontFace = DkFrontFace_CCW;
	state->depthBiasEnable = 0;
	state->depthBiasConstantFactor = 0.0f;
	state->depthBiasClamp = 0.0f;
	state->depthBiasSlopeFactor = 0.0f;
	state->lineWidth = 1.0f;
}

typedef enum DkStencilOp
{
	DkStencilOp_Keep     = 1,
	DkStencilOp_Zero     = 2,
	DkStencilOp_Replace  = 3,
	DkStencilOp_Incr     = 4,
	DkStencilOp_Decr     = 5,
	DkStencilOp_Invert   = 6,
	DkStencilOp_IncrWrap = 7,
	DkStencilOp_DecrWrap = 8,
} DkStencilOp;

typedef struct DkDepthStencilState
{
	uint32_t depthTestEnable : 1;
	uint32_t depthWriteEnable : 1;
	uint32_t stencilTestEnable : 1;
	uint32_t : 1;
	DkCompareOp depthCompareOp : 4;
	uint32_t : 24;

	DkStencilOp stencilFrontFailOp : 4;
	DkStencilOp stencilFrontPassOp : 4;
	DkStencilOp stencilFrontDepthFailOp : 4;
	DkCompareOp stencilFrontCompareOp : 4;
	DkStencilOp stencilBackFailOp : 4;
	DkStencilOp stencilBackPassOp : 4;
	DkStencilOp stencilBackDepthFailOp : 4;
	DkCompareOp stencilBackCompareOp : 4;
} DkDepthStencilState;

DK_CONSTEXPR void dkDepthStencilStateDefaults(DkDepthStencilState* state)
{
	state->depthTestEnable = true;
	state->depthWriteEnable = true;
	state->stencilTestEnable = false;
	state->depthCompareOp = DkCompareOp_Less;

	state->stencilFrontFailOp = DkStencilOp_Keep;
	state->stencilFrontPassOp = DkStencilOp_Replace;
	state->stencilFrontDepthFailOp = DkStencilOp_Keep;
	state->stencilFrontCompareOp = DkCompareOp_Always;

	state->stencilBackFailOp = DkStencilOp_Keep;
	state->stencilBackPassOp = DkStencilOp_Replace;
	state->stencilBackDepthFailOp = DkStencilOp_Keep;
	state->stencilBackCompareOp = DkCompareOp_Always;
}

typedef enum DkTiledCacheOp
{
	DkTiledCacheOp_Disable    = 0,
	DkTiledCacheOp_Enable     = 1,
	DkTiledCacheOp_Flush      = 2,
	DkTiledCacheOp_FlushAlt   = 3,
	DkTiledCacheOp_UnkDisable = 4,
	DkTiledCacheOp_UnkEnable  = 5,
} DkTiledCacheOp;

typedef enum DkVtxAttribSize
{
	// One to four 32-bit components
	DkVtxAttribSize_1x32 = 0x12,
	DkVtxAttribSize_2x32 = 0x04,
	DkVtxAttribSize_3x32 = 0x02,
	DkVtxAttribSize_4x32 = 0x01,

	// One to four 16-bit components
	DkVtxAttribSize_1x16 = 0x1b,
	DkVtxAttribSize_2x16 = 0x0f,
	DkVtxAttribSize_3x16 = 0x05,
	DkVtxAttribSize_4x16 = 0x03,

	// One to four 8-bit components
	DkVtxAttribSize_1x8  = 0x1d,
	DkVtxAttribSize_2x8  = 0x18,
	DkVtxAttribSize_3x8  = 0x13,
	DkVtxAttribSize_4x8  = 0x0a,

	// Misc arrangements
	DkVtxAttribSize_10_10_10_2 = 0x30,
	DkVtxAttribSize_11_11_10   = 0x31,
} DkVtxAttribSize;

typedef enum DkVtxAttribType
{
	DkVtxAttribType_None    = 0,
	DkVtxAttribType_Snorm   = 1,
	DkVtxAttribType_Unorm   = 2,
	DkVtxAttribType_Sint    = 3,
	DkVtxAttribType_Uint    = 4,
	DkVtxAttribType_Sscaled = 5,
	DkVtxAttribType_Uscaled = 6,
	DkVtxAttribType_Float   = 7,
} DkVtxAttribType;

typedef struct DkVtxAttribState
{
	uint32_t bufferId : 5;
	uint32_t : 1;
	uint32_t isFixed : 1;
	uint32_t offset : 14;
	DkVtxAttribSize size : 6;
	DkVtxAttribType type : 3;
	uint32_t : 1;
	uint32_t isBgra : 1;
} DkVtxAttribState;

typedef struct DkVtxBufferState
{
	uint32_t stride;
	uint32_t divisor;
} DkVtxBufferState;

typedef enum DkPrimitive
{
	DkPrimitive_Points                 = 0,
	DkPrimitive_Lines                  = 1,
	DkPrimitive_LineLoop               = 2,
	DkPrimitive_LineStrip              = 3,
	DkPrimitive_Triangles              = 4,
	DkPrimitive_TriangleStrip          = 5,
	DkPrimitive_TriangleFan            = 6,
	DkPrimitive_Quads                  = 7,
	DkPrimitive_QuadStrip              = 8,
	DkPrimitive_Polygon                = 9,
	DkPrimitive_LinesAdjacency         = 10,
	DkPrimitive_LineStripAdjacency     = 11,
	DkPrimitive_TrianglesAdjacency     = 12,
	DkPrimitive_TriangleStripAdjacency = 13,
	DkPrimitive_Patches                = 14,
} DkPrimitive;

typedef enum DkIdxFormat
{
	DkIdxFormat_Uint8  = 0,
	DkIdxFormat_Uint16 = 1,
	DkIdxFormat_Uint32 = 2,
} DkIdxFormat;

typedef struct DkDrawIndirectData
{
	uint32_t vertexCount;
	uint32_t instanceCount;
	uint32_t firstVertex;
	uint32_t firstInstance;
} DkDrawIndirectData;

typedef struct DkDrawIndexedIndirectData
{
	uint32_t indexCount;
	uint32_t instanceCount;
	uint32_t firstIndex;
	int32_t  vertexOffset;
	uint32_t firstInstance;
} DkDrawIndexedIndirectData;

typedef struct DkDispatchIndirectData
{
	uint32_t numGroupsX;
	uint32_t numGroupsY;
	uint32_t numGroupsZ;
} DkDispatchIndirectData;

enum
{
	// Flip bits (for all)
	DkBlitFlag_Flip_Mask = 7U << 0,
	DkBlitFlag_FlipX     = 1U << 0, // only for BlitImage and CopyDataToImage
	DkBlitFlag_FlipY     = 1U << 1,
	DkBlitFlag_FlipZ     = 1U << 2,

	// Filter mode (for BlitImage)
	DkBlitFlag_Filter_Mask   = 1U << 4,
	DkBlitFlag_FilterNearest = 0U << 4,
	DkBlitFlag_FilterLinear  = 1U << 4,

	// Blit mode (for BlitImage)
	DkBlitFlag_Mode_Mask        = 7U << 5,
	DkBlitFlag_ModeBlit         = 0U << 5,
	DkBlitFlag_ModeAlphaMask    = 1U << 5,
	DkBlitFlag_ModeAlphaBlend   = 2U << 5,
	DkBlitFlag_ModePremultBlit  = 3U << 5,
	DkBlitFlag_ModePremultBlend = 4U << 5,
};

typedef struct DkBlitRect
{
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
} DkBlitRect;

typedef struct DkCopyBuf
{
	DkGpuAddr addr;
	uint32_t rowLength;
	uint32_t imageHeight;
} DkCopyBuf;

typedef struct DkSwapchainMaker
{
	DkDevice device;
	void* nativeWindow;
	DkImage const* const* pImages;
	uint32_t numImages;
} DkSwapchainMaker;

DK_CONSTEXPR void dkSwapchainMakerDefaults(DkSwapchainMaker* maker, DkDevice device, void* nativeWindow, DkImage const* const pImages[], uint32_t numImages)
{
	maker->device = device;
	maker->nativeWindow = nativeWindow;
	maker->pImages = pImages;
	maker->numImages = numImages;
}

#ifdef __cplusplus
extern "C" {
#endif

DkDevice dkDeviceCreate(DkDeviceMaker const* maker);
void dkDeviceDestroy(DkDevice obj);

DkMemBlock dkMemBlockCreate(DkMemBlockMaker const* maker);
void dkMemBlockDestroy(DkMemBlock obj);
void* dkMemBlockGetCpuAddr(DkMemBlock obj);
DkGpuAddr dkMemBlockGetGpuAddr(DkMemBlock obj);
uint32_t dkMemBlockGetSize(DkMemBlock obj);
DkResult dkMemBlockFlushCpuCache(DkMemBlock obj, uint32_t offset, uint32_t size);
DkResult dkMemBlockInvalidateCpuCache(DkMemBlock obj, uint32_t offset, uint32_t size);

DkResult dkFenceWait(DkFence* obj, int64_t timeout_ns);

DkCmdBuf dkCmdBufCreate(DkCmdBufMaker const* maker);
void dkCmdBufDestroy(DkCmdBuf obj);
void dkCmdBufAddMemory(DkCmdBuf obj, DkMemBlock mem, uint32_t offset, uint32_t size);
DkCmdList dkCmdBufFinishList(DkCmdBuf obj);
void dkCmdBufClear(DkCmdBuf obj);
void dkCmdBufWaitFence(DkCmdBuf obj, DkFence* fence);
void dkCmdBufSignalFence(DkCmdBuf obj, DkFence* fence, bool flush);
void dkCmdBufBarrier(DkCmdBuf obj, DkBarrier mode, uint32_t invalidateFlags);
void dkCmdBufBindShaders(DkCmdBuf obj, uint32_t stageMask, DkShader const* const shaders[], uint32_t numShaders);
void dkCmdBufBindUniformBuffers(DkCmdBuf obj, DkStage stage, uint32_t firstId, DkBufExtents const buffers[], uint32_t numBuffers);
void dkCmdBufBindStorageBuffers(DkCmdBuf obj, DkStage stage, uint32_t firstId, DkBufExtents const buffers[], uint32_t numBuffers);
void dkCmdBufBindTextures(DkCmdBuf obj, DkStage stage, uint32_t firstId, DkResHandle const handles[], uint32_t numHandles);
void dkCmdBufBindImages(DkCmdBuf obj, DkStage stage, uint32_t firstId, DkResHandle const handles[], uint32_t numHandles);
void dkCmdBufBindImageDescriptorSet(DkCmdBuf obj, DkGpuAddr setAddr, uint32_t numDescriptors);
void dkCmdBufBindSamplerDescriptorSet(DkCmdBuf obj, DkGpuAddr setAddr, uint32_t numDescriptors);
void dkCmdBufBindRenderTargets(DkCmdBuf obj, DkImageView const* const colorTargets[], uint32_t numColorTargets, DkImageView const* depthTarget);
void dkCmdBufBindRasterizerState(DkCmdBuf obj, DkRasterizerState const* state);
void dkCmdBufBindDepthStencilState(DkCmdBuf obj, DkDepthStencilState const* state);
void dkCmdBufBindVtxAttribState(DkCmdBuf obj, DkVtxAttribState const attribs[], uint32_t numAttribs);
void dkCmdBufBindVtxBufferState(DkCmdBuf obj, DkVtxBufferState const buffers[], uint32_t numBuffers);
void dkCmdBufBindVtxBuffers(DkCmdBuf obj, uint32_t firstId, DkBufExtents const buffers[], uint32_t numBuffers);
void dkCmdBufBindIdxBuffer(DkCmdBuf obj, DkIdxFormat format, DkGpuAddr address);
void dkCmdBufSetViewports(DkCmdBuf obj, uint32_t firstId, DkViewport const viewports[], uint32_t numViewports);
void dkCmdBufSetScissors(DkCmdBuf obj, uint32_t firstId, DkScissor const scissors[], uint32_t numScissors);
void dkCmdBufSetDepthBounds(DkCmdBuf obj, bool enable, float near, float far);
void dkCmdBufSetStencil(DkCmdBuf obj, DkFace face, uint8_t mask, uint8_t funcRef, uint8_t funcMask);
void dkCmdBufSetPrimitiveRestart(DkCmdBuf obj, bool enable, uint32_t index);
void dkCmdBufSetTileSize(DkCmdBuf obj, uint32_t width, uint32_t height);
void dkCmdBufTiledCacheOp(DkCmdBuf obj, DkTiledCacheOp op);
void dkCmdBufClearColor(DkCmdBuf obj, uint32_t targetId, uint32_t clearMask, const void* clearData);
void dkCmdBufClearDepthStencil(DkCmdBuf obj, bool clearDepth, float depthValue, uint8_t stencilMask, uint8_t stencilValue);
void dkCmdBufDiscardColor(DkCmdBuf obj, uint32_t targetId);
void dkCmdBufDiscardDepthStencil(DkCmdBuf obj);
void dkCmdBufDraw(DkCmdBuf obj, DkPrimitive prim, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
void dkCmdBufDrawIndirect(DkCmdBuf obj, DkPrimitive prim, DkGpuAddr indirect);
void dkCmdBufDrawIndexed(DkCmdBuf obj, DkPrimitive prim, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
void dkCmdBufDrawIndexedIndirect(DkCmdBuf obj, DkPrimitive prim, DkGpuAddr indirect);
void dkCmdBufDispatchCompute(DkCmdBuf obj, uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ);
void dkCmdBufDispatchComputeIndirect(DkCmdBuf obj, DkGpuAddr indirect);
void dkCmdBufPushConstants(DkCmdBuf obj, DkGpuAddr uboAddr, uint32_t uboSize, uint32_t offset, uint32_t size, const void* data);
void dkCmdBufPushData(DkCmdBuf obj, DkGpuAddr addr, const void* data, uint32_t size);
void dkCmdBufCopyBuffer(DkCmdBuf obj, DkGpuAddr srcAddr, DkGpuAddr dstAddr, uint32_t size);
void dkCmdBufCopyImage(DkCmdBuf obj, DkImageView const* srcView, DkBlitRect const* srcRect, DkImageView const* dstView, DkBlitRect const* dstRect, uint32_t flags);
void dkCmdBufBlitImage(DkCmdBuf obj, DkImageView const* srcView, DkBlitRect const* srcRect, DkImageView const* dstView, DkBlitRect const* dstRect, uint32_t flags, uint32_t factor);
void dkCmdBufResolveImage(DkCmdBuf obj, DkImageView const* srcView, DkImageView const* dstView);
void dkCmdBufCopyBufferToImage(DkCmdBuf obj, DkCopyBuf const* src, DkImageView const* dstView, DkBlitRect const* dstRect, uint32_t flags);
void dkCmdBufCopyImageToBuffer(DkCmdBuf obj, DkImageView const* srcView, DkBlitRect const* srcRect, DkCopyBuf const* dst, uint32_t flags);

DkQueue dkQueueCreate(DkQueueMaker const* maker);
void dkQueueDestroy(DkQueue obj);
bool dkQueueIsInErrorState(DkQueue obj);
void dkQueueWaitFence(DkQueue obj, DkFence* fence);
void dkQueueSignalFence(DkQueue obj, DkFence* fence, bool flush);
void dkQueueSubmitCommands(DkQueue obj, DkCmdList cmds);
void dkQueueFlush(DkQueue obj);
void dkQueueWaitIdle(DkQueue obj);
int dkQueueAcquireImage(DkQueue obj, DkSwapchain swapchain);
void dkQueuePresentImage(DkQueue obj, DkSwapchain swapchain, int imageSlot);

void dkShaderInitialize(DkShader* obj, DkShaderMaker const* maker);
bool dkShaderIsValid(DkShader const* obj);
DkStage dkShaderGetStage(DkShader const* obj);

void dkImageLayoutInitialize(DkImageLayout* obj, DkImageLayoutMaker const* maker);
uint64_t dkImageLayoutGetSize(DkImageLayout const* obj);
uint32_t dkImageLayoutGetAlignment(DkImageLayout const* obj);

void dkImageInitialize(DkImage* obj, DkImageLayout const* layout, DkMemBlock memBlock, uint32_t offset);
DkGpuAddr dkImageGetGpuAddr(DkImage const* obj);

void dkImageDescriptorInitialize(DkImageDescriptor* obj, DkImageView const* view, bool usesLoadOrStore, bool decayMS);

void dkSamplerDescriptorInitialize(DkSamplerDescriptor* obj, DkSampler const* sampler);

DkSwapchain dkSwapchainCreate(DkSwapchainMaker const* maker);
void dkSwapchainDestroy(DkSwapchain obj);
void dkSwapchainAcquireImage(DkSwapchain obj, int* imageSlot, DkFence* fence);
void dkSwapchainSetCrop(DkSwapchain obj, int32_t left, int32_t top, int32_t right, int32_t bottom);
void dkSwapchainSetSwapInterval(DkSwapchain obj, uint32_t interval);

static inline void dkCmdBufBindShader(DkCmdBuf obj, DkShader const* shader)
{
	DkShader const* table[] = { shader };
	dkCmdBufBindShaders(obj, 1U << dkShaderGetStage(shader), table, 1);
}

static inline void dkCmdBufBindUniformBuffer(DkCmdBuf obj, DkStage stage, uint32_t id, DkGpuAddr bufAddr, uint32_t bufSize)
{
	DkBufExtents ext = { bufAddr, bufSize };
	dkCmdBufBindUniformBuffers(obj, stage, id, &ext, 1);
}

static inline void dkCmdBufBindStorageBuffer(DkCmdBuf obj, DkStage stage, uint32_t id, DkGpuAddr bufAddr, uint32_t bufSize)
{
	DkBufExtents ext = { bufAddr, bufSize };
	dkCmdBufBindStorageBuffers(obj, stage, id, &ext, 1);
}

static inline void dkCmdBufBindTexture(DkCmdBuf obj, DkStage stage, uint32_t id, DkResHandle handle)
{
	dkCmdBufBindTextures(obj, stage, id, &handle, 1);
}

static inline void dkCmdBufBindImage(DkCmdBuf obj, DkStage stage, uint32_t id, DkResHandle handle)
{
	dkCmdBufBindImages(obj, stage, id, &handle, 1);
}

static inline void dkCmdBufBindVtxBuffer(DkCmdBuf obj, uint32_t id, DkGpuAddr bufAddr, uint32_t bufSize)
{
	DkBufExtents ext = { bufAddr, bufSize };
	dkCmdBufBindVtxBuffers(obj, id, &ext, 1);
}

static inline void dkCmdBufClearColorFloat(DkCmdBuf obj, uint32_t targetId, uint32_t clearMask, float red, float green, float blue, float alpha)
{
	float data[] = { red, green, blue, alpha };
	dkCmdBufClearColor(obj, targetId, clearMask, data);
}

static inline void dkCmdBufClearColorSint(DkCmdBuf obj, uint32_t targetId, uint32_t clearMask, int32_t red, int32_t green, int32_t blue, int32_t alpha)
{
	int32_t data[] = { red, green, blue, alpha };
	dkCmdBufClearColor(obj, targetId, clearMask, data);
}

static inline void dkCmdBufClearColorUint(DkCmdBuf obj, uint32_t targetId, uint32_t clearMask, uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha)
{
	uint32_t data[] = { red, green, blue, alpha };
	dkCmdBufClearColor(obj, targetId, clearMask, data);
}

static inline DkImageLayout const* dkImageGetLayout(DkImage const* obj)
{
	return (DkImageLayout const*)obj;
}

#ifdef __cplusplus
}
#endif
